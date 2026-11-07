/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#include "gucefCOM_CImapResponse.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
TrimSpaces( const CORE::CString& text )
{
    return text.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

bool
IsStatusWord( const CORE::CString& word )
{
    return word.Equals( "OK", false ) || word.Equals( "NO", false ) || word.Equals( "BAD", false ) ||
           word.Equals( "PREAUTH", false ) || word.Equals( "BYE", false );
}

/*-------------------------------------------------------------------------*/

/** After the status word: [CODE arguments] text */
void
ParseStatusRest( CImapValueParser& parser, CImapResponse& response )
{
    parser.SkipSpaces();
    if ( '[' == parser.Peek() )
    {
        parser.Advance();
        CORE::CString inside;
        GUCEF::vector< char > chars;
        UInt32 depth = 0;
        while ( !parser.AtEnd() )
        {
            const char c = parser.Peek();
            if ( ']' == c && 0 == depth )
                break;
            if ( '(' == c ) ++depth;
            if ( ')' == c && depth > 0 ) --depth;
            chars.push_back( c );
            parser.Advance();
        }
        parser.Advance();       // the closing bracket
        if ( !chars.empty() )
            inside = CMimeCodec::SanitizeUtf8( &chars[ 0 ], (UInt32) chars.size() );
        Int32 space = inside.HasChar( ' ' );
        if ( space > 0 )
        {
            response.code = inside.SubstrToIndex( (UInt32) space, true ).Uppercase();
            response.codeArguments = TrimSpaces( inside.SubstrFromRange( (UInt32) space + 1, inside.Length() ) );
        }
        else
            response.code = inside.Uppercase();
    }
    response.text = TrimSpaces( parser.ReadRest() );
}

/*-------------------------------------------------------------------------*/

void
ReadAddressList( const CImapValue& list, CMailAddress::TMailAddressVector& addresses )
{
    if ( !list.IsList() )
        return;
    for ( UInt32 i=0; i<list.GetListSize() && addresses.size() < 500; ++i )
    {
        const CImapValue& entry = list.GetElement( i );
        if ( !entry.IsList() || entry.GetListSize() < 4 )
            continue;
        // personal name, route, mailbox, host: a NIL host is the start or the end of a group
        if ( entry.GetElement( 3 ).IsNil() || entry.GetElement( 2 ).IsNil() )
            continue;
        CMailAddress address;
        address.displayName = CMimeCodec::DecodeHeaderText( entry.GetElement( 0 ).GetString() );
        address.localPart = entry.GetElement( 2 ).GetString();
        address.domain = entry.GetElement( 3 ).GetString();
        if ( address.IsValid( true ) )
            addresses.push_back( address );
    }
}

/*-------------------------------------------------------------------------*/

void
ReadParams( const CImapValue& list, CMimeCodec::TParamVector& params )
{
    params.clear();
    if ( !list.IsList() )
        return;
    for ( UInt32 i=0; i+1<list.GetListSize() && params.size() < 100; i+=2 )
    {
        CMimeCodec::TParam param;
        param.name = list.GetElement( i ).GetString().Lowercase();
        param.value = list.GetElement( i + 1 ).GetString();
        params.push_back( param );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
LowerText( const CImapValue& value )
{
    return value.GetString().Lowercase();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapResponse::CImapResponse( void )
    : kind( KIND_NONE )
    , tag()
    , isStatus( false )
    , status()
    , code()
    , codeArguments()
    , text()
    , hasNumber( false )
    , number( 0 )
    , keyword()
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImapResponse::EndsWithLiteral( const CORE::CString& line, UInt32& literalSize, bool& nonSynchronizing )
{GUCEF_TRACE;

    const char* p = line.C_String();
    const UInt32 size = ByteLength( line );
    if ( size < 3 || '}' != p[ size - 1 ] )
        return false;
    UInt32 i = size - 2;
    nonSynchronizing = false;
    if ( '+' == p[ i ] )
    {
        nonSynchronizing = true;
        if ( 0 == i )
            return false;
        --i;
    }
    UInt32 digits = 0;
    UInt64 count = 0;
    UInt32 multiplier = 1;
    while ( p[ i ] >= '0' && p[ i ] <= '9' && digits < 11 )
    {
        count += (UInt64)( p[ i ] - '0' ) * multiplier;
        multiplier *= 10;
        ++digits;
        if ( 0 == i )
            return false;
        --i;
    }
    if ( 0 == digits || '{' != p[ i ] )
        return false;
    // a size above 32 bits is refused by the caller, which checks it against its limit
    literalSize = count > 0xFFFFFFFFull ? 0xFFFFFFFFu : (UInt32) count;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImapResponse::Parse( const void* bytes, UInt32 size, UInt32 maxValues, CImapResponse& response, CORE::CString& error )
{GUCEF_TRACE;

    response = CImapResponse();
    error.Clear();
    CImapValueParser parser( bytes, size, CImapValueParser::DefaultMaxDepth, maxValues );

    if ( 0 == size )
    {
        error = "An empty response";
        return false;
    }
    const char* data = static_cast< const char* >( bytes );

    if ( '+' == data[ 0 ] )
    {
        response.kind = KIND_CONTINUATION;
        parser.Advance();
        response.text = TrimSpaces( parser.ReadRest() );
        return true;
    }

    if ( '*' == data[ 0 ] )
    {
        response.kind = KIND_UNTAGGED;
        parser.Advance();
        parser.SkipSpaces();

        CORE::CString word = parser.ReadWord();
        if ( word.IsNULLOrEmpty() )
        {
            error = "An untagged response without content";
            return false;
        }

        // "* 3 EXISTS"
        bool allDigits = true;
        for ( UInt32 i=0; i<ByteLength( word ); ++i )
            if ( word.C_String()[ i ] < '0' || word.C_String()[ i ] > '9' )
                allDigits = false;
        if ( allDigits )
        {
            if ( ByteLength( word ) > 10 )
            {
                error = "A number in a response is too large";
                return false;
            }
            UInt64 value = 0;
            for ( UInt32 i=0; i<ByteLength( word ); ++i )
                value = value * 10 + (UInt64)( word.C_String()[ i ] - '0' );
            if ( value > 0xFFFFFFFFull )
            {
                error = "A number in a response is too large";
                return false;
            }
            response.hasNumber = true;
            response.number = (UInt32) value;
            word = parser.ReadWord();
            if ( word.IsNULLOrEmpty() )
            {
                error = "An untagged response has no keyword";
                return false;
            }
        }

        response.keyword = word.Uppercase();
        if ( !response.hasNumber && IsStatusWord( response.keyword ) )
        {
            response.isStatus = true;
            response.status = response.keyword;
            ParseStatusRest( parser, response );
            return true;
        }
        if ( !parser.ParseValuesUntilEnd( response.data ) )
        {
            error = parser.GetError();
            return false;
        }
        return true;
    }

    // tagged: the tag, the status, the rest
    response.kind = KIND_TAGGED;
    response.tag = parser.ReadWord();
    CORE::CString status = parser.ReadWord();
    if ( response.tag.IsNULLOrEmpty() || !( status.Equals( "OK", false ) || status.Equals( "NO", false ) || status.Equals( "BAD", false ) ) )
    {
        error = "A response that is not tagged, untagged or a continuation";
        return false;
    }
    response.isStatus = true;
    response.status = status.Uppercase();
    ParseStatusRest( parser, response );
    return true;
}

/*-------------------------------------------------------------------------*/

CImapEnvelope::CImapEnvelope( void )
    : date()
    , subject()
    , from()
    , sender()
    , replyTo()
    , to()
    , cc()
    , bcc()
    , inReplyTo()
    , messageId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImapEnvelope::FromValue( const CImapValue& value, CImapEnvelope& envelope )
{GUCEF_TRACE;

    envelope = CImapEnvelope();
    if ( !value.IsList() || value.GetListSize() < 10 )
        return false;
    envelope.date = value.GetElement( 0 ).GetString();
    envelope.subject = CMimeCodec::DecodeHeaderText( value.GetElement( 1 ).GetString() );
    ReadAddressList( value.GetElement( 2 ), envelope.from );
    ReadAddressList( value.GetElement( 3 ), envelope.sender );
    ReadAddressList( value.GetElement( 4 ), envelope.replyTo );
    ReadAddressList( value.GetElement( 5 ), envelope.to );
    ReadAddressList( value.GetElement( 6 ), envelope.cc );
    ReadAddressList( value.GetElement( 7 ), envelope.bcc );
    envelope.inReplyTo = value.GetElement( 8 ).GetString();
    envelope.messageId = value.GetElement( 9 ).GetString();
    return true;
}

/*-------------------------------------------------------------------------*/

CImapBodyStructure::CImapBodyStructure( void )
    : isMultipart( false )
    , type()
    , subtype()
    , params()
    , contentId()
    , description()
    , encoding()
    , size( 0 )
    , lines( 0 )
    , disposition()
    , dispositionParams()
    , children()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapBodyStructure::CImapBodyStructure( const CImapBodyStructure& src )
    : isMultipart( false )
    , type()
    , subtype()
    , params()
    , contentId()
    , description()
    , encoding()
    , size( 0 )
    , lines( 0 )
    , disposition()
    , dispositionParams()
    , children()
{GUCEF_TRACE;

    CopyFrom( src );
}

/*-------------------------------------------------------------------------*/

CImapBodyStructure::~CImapBodyStructure()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CImapBodyStructure&
CImapBodyStructure::operator=( const CImapBodyStructure& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Clear();
        CopyFrom( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CImapBodyStructure::CopyFrom( const CImapBodyStructure& src )
{GUCEF_TRACE;

    isMultipart = src.isMultipart;
    type = src.type;
    subtype = src.subtype;
    params = src.params;
    contentId = src.contentId;
    description = src.description;
    encoding = src.encoding;
    size = src.size;
    lines = src.lines;
    disposition = src.disposition;
    dispositionParams = src.dispositionParams;
    for ( size_t i=0; i<src.children.size(); ++i )
        children.push_back( GUCEF_NEW CImapBodyStructure( *src.children[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
CImapBodyStructure::Clear( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<children.size(); ++i )
        GUCEF_DELETE children[ i ];
    children.clear();
    isMultipart = false;
    type.Clear();
    subtype.Clear();
    params.clear();
    contentId.Clear();
    description.Clear();
    encoding.Clear();
    size = 0;
    lines = 0;
    disposition.Clear();
    dispositionParams.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapBodyStructure::GetContentType( void ) const
{GUCEF_TRACE;

    return type + "/" + subtype;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapBodyStructure::GetFileName( void ) const
{GUCEF_TRACE;

    const CORE::CString& name = CMimeCodec::FindParameter( dispositionParams, "filename" );
    if ( !name.IsNULLOrEmpty() )
        return CMimeCodec::DecodeHeaderText( name );
    return CMimeCodec::DecodeHeaderText( CMimeCodec::FindParameter( params, "name" ) );
}

/*-------------------------------------------------------------------------*/

bool
CImapBodyStructure::FromValue( const CImapValue& value, CImapBodyStructure& structure )
{GUCEF_TRACE;

    structure.Clear();
    if ( !value.IsList() || 0 == value.GetListSize() )
        return false;

    // a multipart starts with the lists of its parts, then the subtype and the extension data
    if ( value.GetElement( 0 ).IsList() )
    {
        structure.isMultipart = true;
        structure.type = "multipart";
        UInt32 i = 0;
        while ( i < value.GetListSize() && value.GetElement( i ).IsList() )
        {
            CImapBodyStructure* child = GUCEF_NEW CImapBodyStructure();
            structure.children.push_back( child );
            if ( !FromValue( value.GetElement( i ), *child ) )
            {
                structure.Clear();
                return false;
            }
            ++i;
        }
        structure.subtype = LowerText( value.GetElement( i ) );
        ReadParams( value.GetElement( i + 1 ), structure.params );
        const CImapValue& disposition = value.GetElement( i + 2 );
        if ( disposition.IsList() )
        {
            structure.disposition = LowerText( disposition.GetElement( 0 ) );
            ReadParams( disposition.GetElement( 1 ), structure.dispositionParams );
        }
        return true;
    }

    if ( value.GetListSize() < 7 )
        return false;
    structure.type = LowerText( value.GetElement( 0 ) );
    structure.subtype = LowerText( value.GetElement( 1 ) );
    ReadParams( value.GetElement( 2 ), structure.params );
    structure.contentId = value.GetElement( 3 ).GetString();
    structure.description = CMimeCodec::DecodeHeaderText( value.GetElement( 4 ).GetString() );
    structure.encoding = LowerText( value.GetElement( 5 ) );
    structure.size = (UInt32)( value.GetElement( 6 ).GetNumber() > 0xFFFFFFFFull ? 0xFFFFFFFFull : value.GetElement( 6 ).GetNumber() );

    UInt32 next = 7;
    if ( structure.type == "text" )
    {
        structure.lines = (UInt32) value.GetElement( 7 ).GetNumber();
        next = 8;
    }
    else
    if ( structure.type == "message" && structure.subtype == "rfc822" )
    {
        structure.lines = (UInt32) value.GetElement( 9 ).GetNumber();
        next = 10;
    }
    // md5, then the disposition
    const CImapValue& disposition = value.GetElement( next + 1 );
    if ( disposition.IsList() )
    {
        structure.disposition = LowerText( disposition.GetElement( 0 ) );
        ReadParams( disposition.GetElement( 1 ), structure.dispositionParams );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CImapFetchSection::CImapFetchSection( void )
    : name()
    , hasOrigin( false )
    , origin( 0 )
    , isNil( false )
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImapFetchedItem::CImapFetchedItem( void )
    : sequence( 0 )
    , hasUid( false )
    , uid( 0 )
    , hasFlags( false )
    , flags()
    , hasInternalDate( false )
    , internalDate()
    , hasSize( false )
    , size( 0 )
    , hasEnvelope( false )
    , envelope()
    , hasBodyStructure( false )
    , bodyStructure()
    , sections()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CImapFetchSection*
CImapFetchedItem::FindSection( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<sections.size(); ++i )
        if ( sections[ i ].name.Equals( name, false ) )
            return &sections[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CImapFetchedItem::HasFlag( const CORE::CString& flag ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<flags.size(); ++i )
        if ( flags[ i ].Equals( flag, false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImapFetchedItem::FromResponse( const CImapResponse& response, CImapFetchedItem& item )
{GUCEF_TRACE;

    item = CImapFetchedItem();
    if ( CImapResponse::KIND_UNTAGGED != response.kind || !response.hasNumber || response.keyword != "FETCH" )
        return false;
    item.sequence = response.number;

    const CImapValue& items = response.data.GetElement( 0 );
    if ( !items.IsList() )
        return false;

    for ( UInt32 i=0; i+1<items.GetListSize(); i+=2 )
    {
        const CImapValue& nameValue = items.GetElement( i );
        const CImapValue& value = items.GetElement( i + 1 );
        if ( !nameValue.IsAtom() )
            return false;
        const CORE::CString name = nameValue.GetString().Uppercase();

        if ( name == "UID" )
        {
            item.hasUid = value.IsNumber() && value.GetNumber() <= 0xFFFFFFFFull;
            item.uid = (UInt32) value.GetNumber();
        }
        else
        if ( name == "FLAGS" )
        {
            item.hasFlags = true;
            for ( UInt32 f=0; f<value.GetListSize() && f<200; ++f )
                item.flags.push_back( value.GetElement( f ).GetString() );
        }
        else
        if ( name == "INTERNALDATE" )
        {
            item.hasInternalDate = true;
            item.internalDate = value.GetString();
        }
        else
        if ( name == "RFC822.SIZE" )
        {
            item.hasSize = true;
            item.size = (UInt32)( value.GetNumber() > 0xFFFFFFFFull ? 0xFFFFFFFFull : value.GetNumber() );
        }
        else
        if ( name == "ENVELOPE" )
            item.hasEnvelope = CImapEnvelope::FromValue( value, item.envelope );
        else
        if ( name == "BODYSTRUCTURE" || ( name == "BODY" && value.IsList() ) )
            item.hasBodyStructure = CImapBodyStructure::FromValue( value, item.bodyStructure );
        else
        if ( name == "RFC822" || name == "RFC822.HEADER" || name == "RFC822.TEXT" || name.StartsWith( "BODY[" ) || name.StartsWith( "BODY.PEEK[" ) )
        {
            CImapFetchSection section;
            if ( name == "RFC822" ) section.name = "";
            else if ( name == "RFC822.HEADER" ) section.name = "HEADER";
            else if ( name == "RFC822.TEXT" ) section.name = "TEXT";
            else
            {
                // BODY[section]<origin>
                const Int32 open = nameValue.GetString().HasChar( '[' );
                const Int32 close = nameValue.GetString().HasChar( ']', false );
                if ( open < 0 || close < open )
                    return false;
                section.name = nameValue.GetString().SubstrFromRange( (UInt32) open + 1, (UInt32) close ).Uppercase();
                const CORE::CString after = nameValue.GetString().SubstrFromRange( (UInt32) close + 1, nameValue.GetString().Length() );
                if ( after.StartsWith( "<" ) )
                {
                    section.hasOrigin = true;
                    section.origin = (UInt32) CORE::StringToUInt32( after.SubstrFromRange( 1, after.Length() ).SubstrToChar( '>', true ) );
                }
            }
            section.isNil = value.IsNil();
            if ( value.IsString() )
                section.data = value.GetBytes();
            else
            if ( value.IsAtom() )
                section.data.CopyFrom( ByteLength( value.GetString() ), value.GetString().C_String() );
            item.sections.push_back( section );
        }
        // BINARY, X-GM-MSGID and other things that were not asked for are skipped
    }
    return true;
}

/*-------------------------------------------------------------------------*/

namespace {

const char Base64Modified[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

int
ModifiedBase64Value( char c )
{
    if ( c >= 'A' && c <= 'Z' ) return c - 'A';
    if ( c >= 'a' && c <= 'z' ) return c - 'a' + 26;
    if ( c >= '0' && c <= '9' ) return c - '0' + 52;
    if ( '+' == c ) return 62;
    if ( ',' == c ) return 63;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
FlushUtf16( GUCEF::vector< char >& out, GUCEF::vector< UInt16 >& units )
{
    if ( units.empty() )
        return;
    GUCEF::vector< UInt8 > bytes;
    for ( size_t i=0; i<units.size(); ++i )
    {
        bytes.push_back( (UInt8)( units[ i ] >> 8 ) );
        bytes.push_back( (UInt8)( units[ i ] & 0xFF ) );
    }
    out.push_back( '&' );
    UInt32 accumulator = 0;
    UInt32 bits = 0;
    for ( size_t i=0; i<bytes.size(); ++i )
    {
        accumulator = ( accumulator << 8 ) | bytes[ i ];
        bits += 8;
        while ( bits >= 6 )
        {
            bits -= 6;
            out.push_back( Base64Modified[ ( accumulator >> bits ) & 0x3F ] );
        }
    }
    if ( bits > 0 )
        out.push_back( Base64Modified[ ( accumulator << ( 6 - bits ) ) & 0x3F ] );
    out.push_back( '-' );
    units.clear();
}

}

/*-------------------------------------------------------------------------*/

bool
CImapMailboxName::Encode( const CORE::CString& utf8Name, CORE::CString& wireName )
{GUCEF_TRACE;

    wireName.Clear();
    const CORE::CString clean = CMimeCodec::SanitizeUtf8( utf8Name.C_String(), ByteLength( utf8Name ) );
    const UInt8* p = reinterpret_cast< const UInt8* >( clean.C_String() );
    const UInt32 size = ByteLength( clean );

    GUCEF::vector< char > out;
    GUCEF::vector< UInt16 > units;
    UInt32 i = 0;
    while ( i < size )
    {
        UInt32 cp = p[ i ];
        UInt32 length = 1;
        if ( cp >= 0xF0 ) { cp &= 0x07; length = 4; }
        else if ( cp >= 0xE0 ) { cp &= 0x0F; length = 3; }
        else if ( cp >= 0xC0 ) { cp &= 0x1F; length = 2; }
        for ( UInt32 k=1; k<length && i + k < size; ++k )
            cp = ( cp << 6 ) | ( p[ i + k ] & 0x3F );
        i += length;

        if ( cp < 0x20 || 0x7F == cp )
            return false;
        if ( cp >= 0x20 && cp <= 0x7E )
        {
            FlushUtf16( out, units );
            out.push_back( (char) cp );
            if ( '&' == cp )
                out.push_back( '-' );
        }
        else
        if ( cp < 0x10000 )
            units.push_back( (UInt16) cp );
        else
        {
            cp -= 0x10000;
            units.push_back( (UInt16)( 0xD800 + ( cp >> 10 ) ) );
            units.push_back( (UInt16)( 0xDC00 + ( cp & 0x3FF ) ) );
        }
    }
    FlushUtf16( out, units );
    if ( !out.empty() )
        wireName = CORE::CString( &out[ 0 ], (UInt32) out.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImapMailboxName::Decode( const CORE::CString& wireName, CORE::CString& utf8Name )
{GUCEF_TRACE;

    utf8Name.Clear();
    const char* p = wireName.C_String();
    const UInt32 size = ByteLength( wireName );
    GUCEF::vector< char > out;

    UInt32 i = 0;
    while ( i < size )
    {
        if ( '&' != p[ i ] )
        {
            if ( ( (UInt8) p[ i ] ) >= 0x80 )
            {
                // a server that sends UTF-8 (UTF8=ACCEPT) is read as it is
                out.push_back( p[ i++ ] );
                continue;
            }
            out.push_back( p[ i++ ] );
            continue;
        }
        ++i;
        if ( i < size && '-' == p[ i ] )
        {
            out.push_back( '&' );
            ++i;
            continue;
        }

        GUCEF::vector< UInt8 > bytes;
        UInt32 accumulator = 0;
        UInt32 bits = 0;
        while ( i < size && '-' != p[ i ] )
        {
            const int v = ModifiedBase64Value( p[ i ] );
            if ( v < 0 )
                return false;
            accumulator = ( accumulator << 6 ) | (UInt32) v;
            bits += 6;
            if ( bits >= 8 )
            {
                bits -= 8;
                bytes.push_back( (UInt8)( ( accumulator >> bits ) & 0xFF ) );
            }
            ++i;
        }
        if ( i >= size || bytes.size() % 2 != 0 || bytes.empty() )
            return false;
        ++i;

        for ( size_t u=0; u+1<bytes.size(); u+=2 )
        {
            UInt32 unit = ( (UInt32) bytes[ u ] << 8 ) | bytes[ u + 1 ];
            if ( unit >= 0xD800 && unit <= 0xDBFF )
            {
                if ( u + 3 >= bytes.size() )
                    return false;
                UInt32 low = ( (UInt32) bytes[ u + 2 ] << 8 ) | bytes[ u + 3 ];
                if ( low < 0xDC00 || low > 0xDFFF )
                    return false;
                unit = 0x10000 + ( ( unit - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                u += 2;
            }
            else
            if ( unit >= 0xDC00 && unit <= 0xDFFF )
                return false;

            if ( unit < 0x80 ) out.push_back( (char) unit );
            else if ( unit < 0x800 ) { out.push_back( (char)( 0xC0 | ( unit >> 6 ) ) ); out.push_back( (char)( 0x80 | ( unit & 0x3F ) ) ); }
            else if ( unit < 0x10000 ) { out.push_back( (char)( 0xE0 | ( unit >> 12 ) ) ); out.push_back( (char)( 0x80 | ( ( unit >> 6 ) & 0x3F ) ) ); out.push_back( (char)( 0x80 | ( unit & 0x3F ) ) ); }
            else { out.push_back( (char)( 0xF0 | ( unit >> 18 ) ) ); out.push_back( (char)( 0x80 | ( ( unit >> 12 ) & 0x3F ) ) ); out.push_back( (char)( 0x80 | ( ( unit >> 6 ) & 0x3F ) ) ); out.push_back( (char)( 0x80 | ( unit & 0x3F ) ) ); }
        }
    }
    if ( !out.empty() )
        utf8Name = CMimeCodec::SanitizeUtf8( &out[ 0 ], (UInt32) out.size() );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
