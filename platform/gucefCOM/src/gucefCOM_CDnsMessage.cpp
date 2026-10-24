/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#include "gucefCOM_NetworkByteOrder.h"
#define GUCEF_COM_NETWORKBYTEORDER_H
#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */

#include "gucefCOM_CDnsMessage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt16 CDnsMessage::ClassIn = 1;
const UInt16 CDnsMessage::FlagResponse = 0x8000;
const UInt16 CDnsMessage::FlagAuthoritative = 0x0400;
const UInt16 CDnsMessage::FlagTruncated = 0x0200;

static const UInt16 ClassTopBit = 0x8000;
static const UInt32 MaxLabelsInName = 128;
static const UInt32 MaxCompressionJumps = 64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::map< CORE::CString, UInt16 > TCompressionMap;

/*-------------------------------------------------------------------------*/

/** Splits an escaped dotted name in its raw labels */
void
SplitLabels( const CORE::CString& name, CORE::CString::StringVector& labels )
{GUCEF_TRACE;

    labels.clear();
    const char* str = name.C_String();
    UInt32 size = CNetworkByteWriter::GetStringByteLength( name );
    CORE::CString::StringVector::size_type expected = 1;
    for ( UInt32 i=0; i<size; ++i )
        if ( '.' == str[ i ] )
            ++expected;
    labels.reserve( expected );

    CORE::CString label;
    char buffer[ 256 ];
    UInt32 labelSize = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = str[ i ];
        if ( '\\' == c && i+1 < size )
        {
            c = str[ ++i ];
        }
        else
        if ( '.' == c )
        {
            label.Set( buffer, labelSize );
            labels.push_back( label );
            labelSize = 0;
            continue;
        }
        if ( labelSize < sizeof( buffer ) )
            buffer[ labelSize++ ] = c;
    }
    if ( labelSize > 0 )
    {
        label.Set( buffer, labelSize );
        labels.push_back( label );
    }
}

/*-------------------------------------------------------------------------*/

/** Appends a raw label to a dotted name, escaping what has to be escaped */
void
AppendLabel( CORE::CString& name, const char* label, UInt32 labelSize )
{GUCEF_TRACE;

    if ( !name.IsNULLOrEmpty() )
        name += '.';

    char buffer[ 512 ];
    UInt32 n = 0;
    for ( UInt32 i=0; i<labelSize; ++i )
    {
        char c = label[ i ];
        if ( '.' == c || '\\' == c )
            buffer[ n++ ] = '\\';
        buffer[ n++ ] = c;
    }
    name.Append( buffer, n );
}

/*-------------------------------------------------------------------------*/

void
WriteName( CNetworkByteWriter& writer, const CORE::CString& name, TCompressionMap& compression )
{GUCEF_TRACE;

    CORE::CString::StringVector labels;
    SplitLabels( name, labels );

    for ( size_t i=0; i<labels.size(); ++i )
    {
        // The rest of the name from this label on, the way it is looked up for compression
        CORE::CString suffix;
        for ( size_t n=i; n<labels.size(); ++n )
            AppendLabel( suffix, labels[ n ].C_String(), CNetworkByteWriter::GetStringByteLength( labels[ n ] ) );
        suffix = suffix.Lowercase();

        TCompressionMap::iterator known = compression.find( suffix );
        if ( known != compression.end() )
        {
            writer.WriteUInt16( (UInt16)( 0xC000 | (*known).second ) );
            return;
        }
        if ( writer.GetSize() < 0x3FFF )
            compression[ suffix ] = (UInt16) writer.GetSize();

        UInt32 labelSize = CNetworkByteWriter::GetStringByteLength( labels[ i ] );
        if ( labelSize > 63 )
            labelSize = 63;
        writer.WriteUInt8( (UInt8) labelSize );
        writer.WriteBytes( labels[ i ].C_String(), labelSize );
    }
    writer.WriteUInt8( 0 );
}

/*-------------------------------------------------------------------------*/

bool
ReadName( CNetworkByteReader& reader, CORE::CString& name )
{GUCEF_TRACE;

    name.Clear();
    const UInt8* data = reader.GetData();
    UInt32 size = reader.GetSize();
    UInt32 pos = reader.GetOffset();
    UInt32 endOfName = 0;
    bool jumped = false;
    UInt32 jumps = 0;
    UInt32 labelCount = 0;

    while ( true )
    {
        if ( pos >= size )
            return false;

        UInt8 length = data[ pos ];
        if ( 0 == length )
        {
            if ( !jumped )
                endOfName = pos + 1;
            break;
        }
        if ( 0xC0 == ( length & 0xC0 ) )
        {
            if ( pos + 1 >= size )
                return false;
            UInt32 pointer = ( (UInt32)( length & 0x3F ) << 8 ) | data[ pos+1 ];
            if ( !jumped )
                endOfName = pos + 2;
            jumped = true;
            if ( ++jumps > MaxCompressionJumps || pointer >= size )
                return false;
            pos = pointer;
            continue;
        }
        if ( 0 != ( length & 0xC0 ) )
            return false;
        if ( pos + 1 + length > size || ++labelCount > MaxLabelsInName )
            return false;

        AppendLabel( name, reinterpret_cast< const char* >( data + pos + 1 ), length );
        pos += 1 + length;
    }
    return reader.Seek( endOfName );
}

/*-------------------------------------------------------------------------*/

void
WriteRecord( CNetworkByteWriter& writer, CORE::CDynamicBuffer& buffer, const CDnsMessage::CResourceRecord& record, TCompressionMap& compression )
{GUCEF_TRACE;

    WriteName( writer, record.name, compression );
    writer.WriteUInt16( record.type );
    writer.WriteUInt16( (UInt16)( record.recordClass | ( record.cacheFlush ? ClassTopBit : 0 ) ) );
    writer.WriteUInt32( record.ttlInSecs );

    UInt32 lengthOffset = writer.GetSize();
    writer.WriteUInt16( 0 );
    UInt32 dataStart = writer.GetSize();

    switch ( record.type )
    {
        case CDnsMessage::RECORDTYPE_A:
        {
            // Already in network byte order: written as the bytes are in memory
            writer.WriteBytes( &record.ipv4AddressInNetworkByteOrder, 4 );
            break;
        }
        case CDnsMessage::RECORDTYPE_PTR:
        {
            WriteName( writer, record.target, compression );
            break;
        }
        case CDnsMessage::RECORDTYPE_SRV:
        {
            writer.WriteUInt16( record.priority );
            writer.WriteUInt16( record.weight );
            writer.WriteUInt16( record.port );
            WriteName( writer, record.target, compression );
            break;
        }
        case CDnsMessage::RECORDTYPE_TXT:
        {
            // A TXT record always has at least one string, an empty one when there is nothing to say
            if ( record.texts.empty() )
            {
                writer.WriteUInt8( 0 );
            }
            for ( size_t i=0; i<record.texts.size(); ++i )
            {
                UInt32 textSize = CNetworkByteWriter::GetStringByteLength( record.texts[ i ] );
                if ( textSize > 255 )
                    textSize = 255;
                writer.WriteUInt8( (UInt8) textSize );
                writer.WriteBytes( record.texts[ i ].C_String(), textSize );
            }
            break;
        }
        default:
        {
            writer.WriteBytes( record.rawData.GetConstBufferPtr(), record.rawData.GetDataSize() );
            break;
        }
    }

    writer.PatchUInt16( lengthOffset, (UInt16)( writer.GetSize() - dataStart ) );
}

/*-------------------------------------------------------------------------*/

bool
ReadRecord( CNetworkByteReader& reader, CDnsMessage::CResourceRecord& record )
{GUCEF_TRACE;

    UInt16 recordClass = 0;
    UInt16 dataLength = 0;
    if ( !ReadName( reader, record.name )        ||
         !reader.ReadUInt16( record.type )       ||
         !reader.ReadUInt16( recordClass )       ||
         !reader.ReadUInt32( record.ttlInSecs )  ||
         !reader.ReadUInt16( dataLength )         )
        return false;

    record.recordClass = (UInt16)( recordClass & ~ClassTopBit );
    record.cacheFlush = 0 != ( recordClass & ClassTopBit );

    UInt32 dataStart = reader.GetOffset();
    UInt32 dataEnd = dataStart + dataLength;
    if ( dataLength > reader.GetRemaining() )
        return false;

    switch ( record.type )
    {
        case CDnsMessage::RECORDTYPE_A:
        {
            if ( 4 != dataLength || !reader.ReadBytes( &record.ipv4AddressInNetworkByteOrder, 4 ) )
                return false;
            break;
        }
        case CDnsMessage::RECORDTYPE_PTR:
        {
            if ( !ReadName( reader, record.target ) )
                return false;
            break;
        }
        case CDnsMessage::RECORDTYPE_SRV:
        {
            if ( !reader.ReadUInt16( record.priority ) ||
                 !reader.ReadUInt16( record.weight )   ||
                 !reader.ReadUInt16( record.port )     ||
                 !ReadName( reader, record.target )     )
                return false;
            break;
        }
        case CDnsMessage::RECORDTYPE_TXT:
        {
            record.texts.clear();
            CORE::CString text;
            while ( reader.GetOffset() < dataEnd )
            {
                UInt8 textSize = 0;
                if ( !reader.ReadUInt8( textSize ) || reader.GetOffset() + textSize > dataEnd || !reader.ReadString( textSize, text ) )
                    return false;
                if ( textSize > 0 )
                    record.texts.push_back( text );
            }
            break;
        }
        default:
        {
            record.rawData.SetDataSize( 0 );
            if ( dataLength > 0 )
                record.rawData.Append( reader.GetData() + dataStart, dataLength, true );
            break;
        }
    }

    // Whatever the record data held, the next record starts after it
    return reader.Seek( dataEnd );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDnsMessage::CQuestion::CQuestion( void )
    : name()
    , type( RECORDTYPE_ANY )
    , recordClass( ClassIn )
    , unicastResponse( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDnsMessage::CQuestion::CQuestion( const CORE::CString& questionName, UInt16 questionType )
    : name( questionName )
    , type( questionType )
    , recordClass( ClassIn )
    , unicastResponse( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDnsMessage::CResourceRecord::CResourceRecord( void )
    : name()
    , type( 0 )
    , recordClass( ClassIn )
    , cacheFlush( false )
    , ttlInSecs( 0 )
    , target()
    , priority( 0 )
    , weight( 0 )
    , port( 0 )
    , ipv4AddressInNetworkByteOrder( 0 )
    , texts()
    , rawData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDnsMessage::CResourceRecord
CDnsMessage::CResourceRecord::MakePtr( const CORE::CString& name, const CORE::CString& target, UInt32 ttlInSecs )
{GUCEF_TRACE;

    CResourceRecord record;
    record.name = name;
    record.type = RECORDTYPE_PTR;
    record.ttlInSecs = ttlInSecs;
    record.target = target;
    return record;
}

/*-------------------------------------------------------------------------*/

CDnsMessage::CResourceRecord
CDnsMessage::CResourceRecord::MakeSrv( const CORE::CString& name, const CORE::CString& host, UInt16 port, UInt32 ttlInSecs )
{GUCEF_TRACE;

    CResourceRecord record;
    record.name = name;
    record.type = RECORDTYPE_SRV;
    record.cacheFlush = true;
    record.ttlInSecs = ttlInSecs;
    record.target = host;
    record.port = port;
    return record;
}

/*-------------------------------------------------------------------------*/

CDnsMessage::CResourceRecord
CDnsMessage::CResourceRecord::MakeTxt( const CORE::CString& name, const CORE::CString::StringVector& texts, UInt32 ttlInSecs )
{GUCEF_TRACE;

    CResourceRecord record;
    record.name = name;
    record.type = RECORDTYPE_TXT;
    record.cacheFlush = true;
    record.ttlInSecs = ttlInSecs;
    record.texts = texts;
    return record;
}

/*-------------------------------------------------------------------------*/

CDnsMessage::CResourceRecord
CDnsMessage::CResourceRecord::MakeA( const CORE::CString& name, UInt32 ipv4AddressInNetworkByteOrder, UInt32 ttlInSecs )
{GUCEF_TRACE;

    CResourceRecord record;
    record.name = name;
    record.type = RECORDTYPE_A;
    record.cacheFlush = true;
    record.ttlInSecs = ttlInSecs;
    record.ipv4AddressInNetworkByteOrder = ipv4AddressInNetworkByteOrder;
    return record;
}

/*-------------------------------------------------------------------------*/

CDnsMessage::CDnsMessage( void )
    : id( 0 )
    , flags( 0 )
    , questions()
    , answers()
    , authorities()
    , additionals()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDnsMessage::IsResponse( void ) const
{GUCEF_TRACE;

    return 0 != ( flags & FlagResponse );
}

/*-------------------------------------------------------------------------*/

void
CDnsMessage::Clear( void )
{GUCEF_TRACE;

    id = 0;
    flags = 0;
    questions.clear();
    answers.clear();
    authorities.clear();
    additionals.clear();
}

/*-------------------------------------------------------------------------*/

bool
CDnsMessage::Serialize( CORE::CDynamicBuffer& buffer ) const
{GUCEF_TRACE;

    if ( questions.size() > 0xFFFF || answers.size() > 0xFFFF || authorities.size() > 0xFFFF || additionals.size() > 0xFFFF )
        return false;

    buffer.SetDataSize( 0 );
    CNetworkByteWriter writer( buffer );
    TCompressionMap compression;

    writer.WriteUInt16( id );
    writer.WriteUInt16( flags );
    writer.WriteUInt16( (UInt16) questions.size() );
    writer.WriteUInt16( (UInt16) answers.size() );
    writer.WriteUInt16( (UInt16) authorities.size() );
    writer.WriteUInt16( (UInt16) additionals.size() );

    for ( size_t i=0; i<questions.size(); ++i )
    {
        const CQuestion& question = questions[ i ];
        WriteName( writer, question.name, compression );
        writer.WriteUInt16( question.type );
        writer.WriteUInt16( (UInt16)( question.recordClass | ( question.unicastResponse ? ClassTopBit : 0 ) ) );
    }
    for ( size_t i=0; i<answers.size(); ++i )
        WriteRecord( writer, buffer, answers[ i ], compression );
    for ( size_t i=0; i<authorities.size(); ++i )
        WriteRecord( writer, buffer, authorities[ i ], compression );
    for ( size_t i=0; i<additionals.size(); ++i )
        WriteRecord( writer, buffer, additionals[ i ], compression );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDnsMessage::Deserialize( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    Clear();
    CNetworkByteReader reader( data, dataSize );

    UInt16 questionCount = 0;
    UInt16 answerCount = 0;
    UInt16 authorityCount = 0;
    UInt16 additionalCount = 0;
    if ( !reader.ReadUInt16( id )              ||
         !reader.ReadUInt16( flags )           ||
         !reader.ReadUInt16( questionCount )   ||
         !reader.ReadUInt16( answerCount )     ||
         !reader.ReadUInt16( authorityCount )  ||
         !reader.ReadUInt16( additionalCount )  )
        return false;

    // Each question takes at least 5 bytes and each record at least 11: refuse counts the data cannot hold
    UInt32 minimumSize = questionCount * 5 + ( answerCount + authorityCount + additionalCount ) * 11;
    if ( minimumSize > reader.GetRemaining() )
        return false;

    questions.resize( questionCount );
    for ( UInt16 i=0; i<questionCount; ++i )
    {
        CQuestion& question = questions[ i ];
        UInt16 questionClass = 0;
        if ( !ReadName( reader, question.name ) || !reader.ReadUInt16( question.type ) || !reader.ReadUInt16( questionClass ) )
            return false;
        question.recordClass = (UInt16)( questionClass & ~ClassTopBit );
        question.unicastResponse = 0 != ( questionClass & ClassTopBit );
    }

    TResourceRecordVector* sections[ 3 ] = { &answers, &authorities, &additionals };
    UInt16 counts[ 3 ] = { answerCount, authorityCount, additionalCount };
    for ( int s=0; s<3; ++s )
    {
        sections[ s ]->resize( counts[ s ] );
        for ( UInt16 i=0; i<counts[ s ]; ++i )
        {
            if ( !ReadRecord( reader, (*sections[ s ])[ i ] ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDnsMessage::NamesAreEqual( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    UInt32 sizeA = CNetworkByteWriter::GetStringByteLength( a );
    if ( sizeA != CNetworkByteWriter::GetStringByteLength( b ) )
        return false;

    const char* strA = a.C_String();
    const char* strB = b.C_String();
    for ( UInt32 i=0; i<sizeA; ++i )
    {
        char ca = strA[ i ];
        char cb = strB[ i ];
        if ( ca >= 'A' && ca <= 'Z' ) ca = (char)( ca - 'A' + 'a' );
        if ( cb >= 'A' && cb <= 'Z' ) cb = (char)( cb - 'A' + 'a' );
        if ( ca != cb )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDnsMessage::MakeName( const CORE::CString& label, const CORE::CString& domain )
{GUCEF_TRACE;

    CORE::CString name;
    AppendLabel( name, label.C_String(), CNetworkByteWriter::GetStringByteLength( label ) > 63 ? 63 : CNetworkByteWriter::GetStringByteLength( label ) );
    if ( !domain.IsNULLOrEmpty() )
    {
        name += '.';
        name += domain;
    }
    return name;
}

/*-------------------------------------------------------------------------*/

bool
CDnsMessage::SplitFirstLabel( const CORE::CString& name, CORE::CString& label, CORE::CString& rest )
{GUCEF_TRACE;

    const char* str = name.C_String();
    UInt32 size = CNetworkByteWriter::GetStringByteLength( name );
    char buffer[ 256 ];
    UInt32 labelSize = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = str[ i ];
        if ( '\\' == c && i+1 < size )
        {
            c = str[ ++i ];
        }
        else
        if ( '.' == c )
        {
            label.Set( buffer, labelSize );
            rest.Set( str + i + 1, size - i - 1 );
            return true;
        }
        if ( labelSize < sizeof( buffer ) )
            buffer[ labelSize++ ] = c;
    }
    label.Set( buffer, labelSize );
    rest.Clear();
    return labelSize > 0;
}

/*-------------------------------------------------------------------------*/

void
CDnsMessage::AttributesToTexts( const CORE::CValueList& attributes, CORE::CString::StringVector& texts )
{GUCEF_TRACE;

    UInt32 keyCount = attributes.GetKeyCount();
    for ( UInt32 k=0; k<keyCount; ++k )
    {
        CORE::CString key = attributes.GetKey( k ).AsString();
        const CORE::CValueList::TVariantVector& values = attributes.GetValueVector( k );
        for ( size_t v=0; v<values.size(); ++v )
        {
            CORE::CString text = key;
            text += '=';
            text += values[ v ].AsString();
            texts.push_back( text );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDnsMessage::TextsToAttributes( const CORE::CString::StringVector& texts, CORE::CValueList& attributes )
{GUCEF_TRACE;

    attributes.SetAllowMultipleValues( true );
    for ( size_t i=0; i<texts.size(); ++i )
    {
        const CORE::CString& text = texts[ i ];
        Int32 separator = text.HasChar( '=' );
        if ( 0 == separator )
            continue;   // RFC 6763: a string that starts with = has no key and is ignored
        if ( separator < 0 )
            attributes.Set( text, CORE::CString::Empty );
        else
            attributes.Set( text.SubstrToIndex( (UInt32) separator, true ), text.CutChars( (UInt32) separator + 1, true, 0 ) );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
