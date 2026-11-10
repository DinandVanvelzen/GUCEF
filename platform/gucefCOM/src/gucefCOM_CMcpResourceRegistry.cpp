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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include <string.h>

#include "gucefCOM_CMcpResourceRegistry.h"

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

const CORE::CEvent CMcpResourceRegistry::ResourceRegisteredEvent = "GUCEF::COM::CMcpResourceRegistry::ResourceRegisteredEvent";
const CORE::CEvent CMcpResourceRegistry::ResourceUnregisteredEvent = "GUCEF::COM::CMcpResourceRegistry::ResourceUnregisteredEvent";
const CORE::CEvent CMcpResourceRegistry::TemplateRegisteredEvent = "GUCEF::COM::CMcpResourceRegistry::TemplateRegisteredEvent";
const CORE::CEvent CMcpResourceRegistry::TemplateUnregisteredEvent = "GUCEF::COM::CMcpResourceRegistry::TemplateUnregisteredEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsUnreserved( unsigned char c )
{GUCEF_TRACE;

    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '.' == c || '_' == c || '~' == c;
}

bool
IsHexDigit( unsigned char c )
{GUCEF_TRACE;

    return ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' );
}

UInt8
HexValue( unsigned char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return (UInt8)( c - '0' );
    if ( c >= 'a' && c <= 'f' )
        return (UInt8)( c - 'a' + 10 );
    return (UInt8)( c - 'A' + 10 );
}

/** varchar of RFC 6570: ALPHA / DIGIT / "_" / pct-encoded, with "." between them */
bool
IsValidVariableName( const CORE::CString& name )
{GUCEF_TRACE;

    const char* data = name.C_String();
    UInt32 size = name.ByteSize() > 0 ? name.ByteSize() - 1 : 0;
    if ( 0 == size || '.' == data[ 0 ] || '.' == data[ size-1 ] )
        return false;
    for ( UInt32 i=0; i<size; ++i )
    {
        unsigned char c = (unsigned char) data[ i ];
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '_' == c || '.' == c )
            continue;
        if ( '%' == c && i+2 < size && IsHexDigit( (unsigned char) data[ i+1 ] ) && IsHexDigit( (unsigned char) data[ i+2 ] ) )
        {
            i += 2;
            continue;
        }
        return false;
    }
    return true;
}

CORE::CString
PercentDecode( const char* data, UInt32 size )
{GUCEF_TRACE;

    // the matched text holds whole percent-encoded triplets only
    GUCEF::vector< char > decoded;
    decoded.reserve( size );
    for ( UInt32 i=0; i<size; ++i )
    {
        unsigned char c = (unsigned char) data[ i ];
        if ( '%' == c && i+2 < size && IsHexDigit( (unsigned char) data[ i+1 ] ) && IsHexDigit( (unsigned char) data[ i+2 ] ) )
        {
            c = (unsigned char)( ( HexValue( (unsigned char) data[ i+1 ] ) << 4 ) | HexValue( (unsigned char) data[ i+2 ] ) );
            i += 2;
        }
        decoded.push_back( (char) c );
    }
    return decoded.empty() ? CORE::CString() : CORE::CString( &decoded[ 0 ], (UInt32) decoded.size() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

struct CMcpUriTemplate::SMatchState
{
    const char* uri;
    UInt32 size;
    UInt32 steps;
    GUCEF::vector< UInt32 > starts;
    GUCEF::vector< UInt32 > ends;
};

/*-------------------------------------------------------------------------*/

CMcpUriTemplate::CMcpUriTemplate( void )
    : m_segments()
    , m_variableCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpUriTemplate::Parse( const CORE::CString& uriTemplate, CORE::CString& error )
{GUCEF_TRACE;

    m_segments.clear();
    m_variableCount = 0;

    const char* data = uriTemplate.C_String();
    UInt32 size = uriTemplate.ByteSize() > 0 ? uriTemplate.ByteSize() - 1 : 0;
    if ( 0 == size )
    {
        error = "the URI template is empty";
        return false;
    }

    UInt32 literalStart = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '}' == data[ i ] )
        {
            error = "the URI template has a '}' without '{'";
            m_segments.clear();
            return false;
        }
        if ( '{' != data[ i ] )
            continue;

        if ( i > literalStart )
        {
            SSegment literal;
            literal.kind = SEGMENT_LITERAL;
            literal.text = CORE::CString( data + literalStart, i - literalStart );
            m_segments.push_back( literal );
        }

        UInt32 close = i+1;
        while ( close < size && '}' != data[ close ] && '{' != data[ close ] )
            ++close;
        if ( close >= size || '}' != data[ close ] )
        {
            error = "the URI template has an expression without '}'";
            m_segments.clear();
            return false;
        }

        UInt32 nameStart = i+1;
        SSegment variable;
        variable.kind = SEGMENT_SIMPLE;
        if ( nameStart < close && '+' == data[ nameStart ] )
        {
            variable.kind = SEGMENT_RESERVED;
            ++nameStart;
        }
        variable.text = CORE::CString( data + nameStart, close - nameStart );
        if ( !IsValidVariableName( variable.text ) )
        {
            error = "the URI template expression \"" + CORE::CString( data + i, close - i + 1 ) +
                    "\" is not supported: only {name} and {+name} with one variable and no modifier";
            m_segments.clear();
            return false;
        }
        if ( !m_segments.empty() && SEGMENT_LITERAL != m_segments.back().kind )
        {
            error = "the URI template has two expressions without text between them, which cannot be told apart";
            m_segments.clear();
            return false;
        }
        m_segments.push_back( variable );
        ++m_variableCount;
        i = close;
        literalStart = close+1;
    }
    if ( literalStart < size )
    {
        SSegment literal;
        literal.kind = SEGMENT_LITERAL;
        literal.text = CORE::CString( data + literalStart, size - literalStart );
        m_segments.push_back( literal );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpUriTemplate::GetVariableCount( void ) const
{GUCEF_TRACE;

    return m_variableCount;
}

/*-------------------------------------------------------------------------*/

bool
CMcpUriTemplate::MatchFrom( SMatchState& state, UInt32 segmentIndex, UInt32 position ) const
{GUCEF_TRACE;

    if ( ++state.steps > MaxMatchSteps )
        return false;
    if ( segmentIndex >= m_segments.size() )
        return position == state.size;

    const SSegment& segment = m_segments[ segmentIndex ];
    if ( SEGMENT_LITERAL == segment.kind )
    {
        UInt32 length = segment.text.ByteSize() > 0 ? segment.text.ByteSize() - 1 : 0;
        if ( position + length > state.size || 0 != ::memcmp( state.uri + position, segment.text.C_String(), length ) )
            return false;
        return MatchFrom( state, segmentIndex+1, position + length );
    }

    // the longest run this variable can take, then shorter ones
    UInt32 end = position;
    if ( SEGMENT_RESERVED == segment.kind )
    {
        end = state.size;
    }
    else
    {
        while ( end < state.size )
        {
            unsigned char c = (unsigned char) state.uri[ end ];
            if ( IsUnreserved( c ) )
                ++end;
            else
            if ( '%' == c && end+2 < state.size && IsHexDigit( (unsigned char) state.uri[ end+1 ] ) && IsHexDigit( (unsigned char) state.uri[ end+2 ] ) )
                end += 3;
            else
                break;
        }
    }

    UInt32 variableIndex = 0;
    for ( UInt32 n=0; n<segmentIndex; ++n )
        if ( SEGMENT_LITERAL != m_segments[ n ].kind )
            ++variableIndex;

    for ( UInt32 candidate = end; candidate > position; --candidate )
    {
        if ( SEGMENT_SIMPLE == segment.kind )
        {
            // never end inside a percent-encoded triplet
            if ( candidate >= position+1 && '%' == state.uri[ candidate-1 ] )
                continue;
            if ( candidate >= position+2 && '%' == state.uri[ candidate-2 ] )
                continue;
        }
        state.starts[ variableIndex ] = position;
        state.ends[ variableIndex ] = candidate;
        if ( MatchFrom( state, segmentIndex+1, candidate ) )
            return true;
        if ( state.steps > MaxMatchSteps )
            return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMcpUriTemplate::Match( const CORE::CString& uri, CORE::CValueList& variables ) const
{GUCEF_TRACE;

    variables.Clear();
    if ( m_segments.empty() )
        return false;

    SMatchState state;
    state.uri = uri.C_String();
    state.size = uri.ByteSize() > 0 ? uri.ByteSize() - 1 : 0;
    state.steps = 0;
    state.starts.resize( m_variableCount, 0 );
    state.ends.resize( m_variableCount, 0 );
    if ( !MatchFrom( state, 0, 0 ) )
        return false;

    UInt32 variableIndex = 0;
    for ( size_t n=0; n<m_segments.size(); ++n )
    {
        const SSegment& segment = m_segments[ n ];
        if ( SEGMENT_LITERAL == segment.kind )
            continue;
        const char* start = state.uri + state.starts[ variableIndex ];
        UInt32 length = state.ends[ variableIndex ] - state.starts[ variableIndex ];
        if ( SEGMENT_SIMPLE == segment.kind )
            variables.Set( segment.text, CORE::CVariant( PercentDecode( start, length ) ) );
        else
            variables.Set( segment.text, CORE::CVariant( CORE::CString( start, length ) ) );
        ++variableIndex;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CMcpResourceEntry::CMcpResourceEntry( void )
    : resource()
    , descriptor()
    , descriptorNode()
    , generation( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpResourceTemplateEntry::CMcpResourceTemplateEntry( void )
    : resourceTemplate()
    , descriptor()
    , matcher()
    , descriptorNode()
    , generation( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMcpResourceRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    CMcpRegistryBase::RegisterEvents();
    ResourceRegisteredEvent.Initialize();
    ResourceUnregisteredEvent.Initialize();
    TemplateRegisteredEvent.Initialize();
    TemplateUnregisteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMcpResourceRegistry::CMcpResourceRegistry( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CMcpRegistryBase( pulseGenerator )
    , m_resources()
    , m_templates()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMcpResourceRegistry::~CMcpResourceRegistry()
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_rwLock );
    m_resources.clear();
    m_templates.clear();
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceRegistry::RegisterResource( const CIMcpResourcePtr& resource, CORE::CString& error )
{GUCEF_TRACE;

    error.Clear();
    if ( resource.IsNULL() )
    {
        error = "no resource";
        return false;
    }
    CMcpResourceEntryPtr entry( GUCEF_NEW CMcpResourceEntry() );
    entry->resource = resource;
    if ( !resource->GetDescriptor( entry->descriptor ) )
        error = "the resource gave no descriptor";
    else
    if ( entry->descriptor.uri.IsNULLOrEmpty() )
        error = "a resource needs a URI";
    else
    if ( entry->descriptor.name.IsNULLOrEmpty() )
        error = "the resource \"" + entry->descriptor.uri + "\" needs a name";
    else
    {
        // requests are resolved by the normal form of their URI, so a resource is registered by that form only
        CORE::CString normalForm;
        CORE::CString problem;
        if ( !NormalizeUri( entry->descriptor.uri, normalForm, problem ) )
            error = "the URI \"" + entry->descriptor.uri + "\" is refused: " + problem;
        else
        if ( normalForm != entry->descriptor.uri )
            error = "the URI \"" + entry->descriptor.uri + "\" is not in its normal form \"" + normalForm + "\"";
    }
    if ( !error.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpResourceRegistry: A resource was refused: " << error );
        return false;
    }
    entry->descriptor.ToDataNode( entry->descriptorNode );

    const CORE::CString uri = entry->descriptor.uri;
    {
        MT::CScopeWriterLock lock( m_rwLock );
        TResourceEntryMap::iterator existing = m_resources.find( uri );
        if ( existing != m_resources.end() && (*existing).second->resource != resource )
        {
            error = "another resource is registered with the URI \"" + uri + "\"";
            lock.EarlyUnlock();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpResourceRegistry: A resource was refused: " << error );
            return false;
        }
        entry->generation = OnChangedWhileLocked();
        m_resources[ uri ] = entry;
    }

    RaiseNameEvent( ResourceRegisteredEvent, uri );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceRegistry::UnregisterResource( const CORE::CString& uri )
{GUCEF_TRACE;

    {
        MT::CScopeWriterLock lock( m_rwLock );
        TResourceEntryMap::iterator i = m_resources.find( uri );
        if ( i == m_resources.end() )
            return false;
        m_resources.erase( i );
        OnChangedWhileLocked();
    }

    RaiseNameEvent( ResourceUnregisteredEvent, uri );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceRegistry::RegisterTemplate( const CIMcpResourceTemplatePtr& resourceTemplate, CORE::CString& error )
{GUCEF_TRACE;

    error.Clear();
    if ( resourceTemplate.IsNULL() )
    {
        error = "no resource template";
        return false;
    }
    CMcpResourceTemplateEntryPtr entry( GUCEF_NEW CMcpResourceTemplateEntry() );
    entry->resourceTemplate = resourceTemplate;
    if ( !resourceTemplate->GetDescriptor( entry->descriptor ) )
        error = "the resource template gave no descriptor";
    else
    if ( entry->descriptor.name.IsNULLOrEmpty() )
        error = "a resource template needs a name";
    else
    {
        CORE::CString templateError;
        if ( !entry->matcher.Parse( entry->descriptor.uriTemplate, templateError ) )
            error = "the resource template \"" + entry->descriptor.name + "\" is refused: " + templateError;
    }
    if ( !error.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpResourceRegistry: A resource template was refused: " << error );
        return false;
    }
    entry->descriptor.ToDataNode( entry->descriptorNode );

    const CORE::CString name = entry->descriptor.name;
    {
        MT::CScopeWriterLock lock( m_rwLock );
        TTemplateEntryMap::iterator existing = m_templates.find( name );
        if ( existing != m_templates.end() && (*existing).second->resourceTemplate != resourceTemplate )
        {
            error = "another resource template is registered with the name \"" + name + "\"";
            lock.EarlyUnlock();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpResourceRegistry: A resource template was refused: " << error );
            return false;
        }
        entry->generation = OnChangedWhileLocked();
        m_templates[ name ] = entry;
    }

    RaiseNameEvent( TemplateRegisteredEvent, name );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceRegistry::UnregisterTemplate( const CORE::CString& name )
{GUCEF_TRACE;

    {
        MT::CScopeWriterLock lock( m_rwLock );
        TTemplateEntryMap::iterator i = m_templates.find( name );
        if ( i == m_templates.end() )
            return false;
        m_templates.erase( i );
        OnChangedWhileLocked();
    }

    RaiseNameEvent( TemplateUnregisteredEvent, name );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpResourceRegistry::UnregisterAll( void )
{GUCEF_TRACE;

    TResourceEntryMap resources;
    TTemplateEntryMap templates;
    {
        MT::CScopeWriterLock lock( m_rwLock );
        if ( m_resources.empty() && m_templates.empty() )
            return;
        resources.swap( m_resources );
        templates.swap( m_templates );
        OnChangedWhileLocked();
    }

    for ( TResourceEntryMap::iterator i = resources.begin(); i != resources.end(); ++i )
        RaiseNameEvent( ResourceUnregisteredEvent, (*i).first );
    for ( TTemplateEntryMap::iterator i = templates.begin(); i != templates.end(); ++i )
        RaiseNameEvent( TemplateUnregisteredEvent, (*i).first );
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceRegistry::TryGetResource( const CORE::CString& uri, CMcpResourceEntryPtr& entry ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    TResourceEntryMap::const_iterator i = m_resources.find( uri );
    if ( i == m_resources.end() )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceRegistry::TryGetTemplate( const CORE::CString& name, CMcpResourceTemplateEntryPtr& entry ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    TTemplateEntryMap::const_iterator i = m_templates.find( name );
    if ( i == m_templates.end() )
        return false;
    entry = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes are UTF-8 (RFC 3629): no overlong forms, no surrogates, nothing above U+10FFFF */
static bool
IsUtf8Text( const unsigned char* bytes, UInt32 size )
{GUCEF_TRACE;

    UInt32 i = 0;
    while ( i < size )
    {
        const unsigned char c = bytes[ i ];
        if ( c < 0x80 )
        {
            ++i;
            continue;
        }
        UInt32 length = 0;
        UInt32 codePoint = 0;
        if ( c >= 0xC2 && c <= 0xDF ) { length = 2; codePoint = c & 0x1F; }
        else
        if ( c >= 0xE0 && c <= 0xEF ) { length = 3; codePoint = c & 0x0F; }
        else
        if ( c >= 0xF0 && c <= 0xF4 ) { length = 4; codePoint = c & 0x07; }
        else
            return false;
        if ( i + length > size )
            return false;
        for ( UInt32 n=1; n<length; ++n )
        {
            if ( 0x80 != ( bytes[ i+n ] & 0xC0 ) )
                return false;
            codePoint = ( codePoint << 6 ) | ( bytes[ i+n ] & 0x3F );
        }
        if ( ( 3 == length && codePoint < 0x800 ) || ( 4 == length && ( codePoint < 0x10000 || codePoint > 0x10FFFF ) ) ||
             ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
            return false;
        i += length;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static int
HexValue( char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

/**
 *  One component of a URI in its normal form: percent-encoded unreserved characters decoded, the hexadecimal digits of
 *  the other percent-encodings in upper case, the host part in lower case. What it decodes to is checked as UTF-8.
 *  @param inPath       the authority or the path: there an encoded '/' or '\' is refused, it would hide a segment
 *  @param lowerCase    for the authority: the characters that are not percent-encoded are written in lower case
 */
static bool
NormalizeUriComponent( const char* text, UInt32 size, bool inPath, bool lowerCase, CORE::CString& out, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CString decoded;
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = text[ i ];
        if ( '%' != c )
        {
            if ( lowerCase && c >= 'A' && c <= 'Z' )
                c = (char) ( c - 'A' + 'a' );
            if ( inPath && '\\' == c )
            {
                problem = "a backslash in the path";
                return false;
            }
            out += c;
            decoded += c;
            continue;
        }
        const int high = i+2 < size ? HexValue( text[ i+1 ] ) : -1;
        const int low = i+2 < size ? HexValue( text[ i+2 ] ) : -1;
        if ( high < 0 || low < 0 )
        {
            problem = "a malformed percent-encoding";
            return false;
        }
        const unsigned char byte = (unsigned char) ( high * 16 + low );
        i += 2;
        if ( byte < 0x20 || 0x7F == byte )
        {
            problem = "a percent-encoded control character";
            return false;
        }
        if ( inPath && ( '/' == byte || '\\' == byte ) )
        {
            problem = "a percent-encoded slash";
            return false;
        }
        decoded += (char) byte;
        if ( ( byte >= 'a' && byte <= 'z' ) || ( byte >= 'A' && byte <= 'Z' ) || ( byte >= '0' && byte <= '9' ) ||
             '-' == byte || '.' == byte || '_' == byte || '~' == byte )
        {
            // an unreserved character is the same character when it is encoded (RFC 3986 2.3)
            char plain = (char) byte;
            if ( lowerCase && plain >= 'A' && plain <= 'Z' )
                plain = (char) ( plain - 'A' + 'a' );
            out += plain;
        }
        else
        {
            static const char* hexDigits = "0123456789ABCDEF";
            out += '%';
            out += hexDigits[ byte >> 4 ];
            out += hexDigits[ byte & 0x0F ];
        }
    }
    if ( !IsUtf8Text( reinterpret_cast< const unsigned char* >( decoded.C_String() ), decoded.ByteSize() > 0 ? decoded.ByteSize() - 1 : 0 ) )
    {
        problem = "text that is no UTF-8";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The dot segments of an absolute path removed, as RFC 3986 5.2.4 does it */
static CORE::CString
RemoveDotSegments( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString::StringVector output;
    const char* text = path.C_String();
    const UInt32 size = path.ByteSize() > 0 ? path.ByteSize() - 1 : 0;
    UInt32 start = 1;      // the path starts with '/'
    bool endsWithSlash = false;
    while ( start <= size )
    {
        UInt32 end = start;
        while ( end < size && '/' != text[ end ] )
            ++end;
        const CORE::CString segment( text + start, end - start );
        const bool last = end >= size;
        if ( "." == segment )
        {
            endsWithSlash = last;
        }
        else
        if ( ".." == segment )
        {
            if ( !output.empty() )
                output.pop_back();
            endsWithSlash = last;
        }
        else
        {
            output.push_back( segment );
            endsWithSlash = false;
        }
        start = end + 1;
    }
    CORE::CString result;
    for ( size_t i=0; i<output.size(); ++i )
        result += "/" + output[ i ];
    if ( endsWithSlash || result.IsNULLOrEmpty() )
        result += "/";
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CMcpResourceRegistry::NormalizeUri( const CORE::CString& uri, CORE::CString& normalized, CORE::CString& problem )
{GUCEF_TRACE;

    normalized.Clear();
    problem.Clear();
    const char* text = uri.C_String();
    const UInt32 size = uri.ByteSize() > 0 ? uri.ByteSize() - 1 : 0;
    if ( 0 == size )
    {
        problem = "the URI is empty";
        return false;
    }

    // raw characters a URI cannot hold, and raw bytes that are no UTF-8 (an IRI may hold UTF-8 as it is)
    for ( UInt32 i=0; i<size; ++i )
    {
        const unsigned char c = (unsigned char) text[ i ];
        if ( c <= 0x20 || 0x7F == c || GUCEF_NULL != memchr( "\"<>\\^`{|}", (int) c, 9 ) )
        {
            problem = "the URI holds a character a URI cannot have";
            return false;
        }
    }
    if ( !IsUtf8Text( reinterpret_cast< const unsigned char* >( text ), size ) )
    {
        problem = "the URI is no UTF-8";
        return false;
    }

    // scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    UInt32 colon = 0;
    while ( colon < size && ':' != text[ colon ] )
        ++colon;
    bool schemeValid = colon > 0 && colon < size && ( ( text[ 0 ] >= 'a' && text[ 0 ] <= 'z' ) || ( text[ 0 ] >= 'A' && text[ 0 ] <= 'Z' ) );
    for ( UInt32 i=1; i<colon && schemeValid; ++i )
    {
        const char c = text[ i ];
        schemeValid = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '+' == c || '-' == c || '.' == c;
    }
    if ( !schemeValid )
    {
        problem = "the URI has no scheme";
        return false;
    }
    CORE::CString result = CORE::CString( text, colon ).Lowercase() + ":";

    UInt32 position = colon + 1;
    if ( position + 1 < size && '/' == text[ position ] && '/' == text[ position+1 ] )
    {
        position += 2;
        UInt32 end = position;
        while ( end < size && '/' != text[ end ] && '?' != text[ end ] && '#' != text[ end ] )
            ++end;
        CORE::CString authority;
        if ( !NormalizeUriComponent( text + position, end - position, true, true, authority, problem ) )
            return false;
        result += "//" + authority;
        position = end;
    }

    UInt32 pathEnd = position;
    while ( pathEnd < size && '?' != text[ pathEnd ] && '#' != text[ pathEnd ] )
        ++pathEnd;
    CORE::CString path;
    if ( !NormalizeUriComponent( text + position, pathEnd - position, true, false, path, problem ) )
        return false;
    if ( !path.IsNULLOrEmpty() && '/' == path[ 0 ] )
        path = RemoveDotSegments( path );
    result += path;

    if ( pathEnd < size )
    {
        // the query and the fragment keep their structure; an encoded '/' is data there
        CORE::CString rest;
        if ( !NormalizeUriComponent( text + pathEnd, size - pathEnd, false, false, rest, problem ) )
            return false;
        result += rest;
    }
    normalized = result;
    return true;
}

/*-------------------------------------------------------------------------*/

CMcpResourceRegistry::EResolveResult
CMcpResourceRegistry::Resolve( const CORE::CString& uri                     ,
                               CMcpResourceEntryPtr& resourceEntry          ,
                               CMcpResourceTemplateEntryPtr& templateEntry  ,
                               CORE::CValueList& variables                  ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock lock( m_rwLock );
    TResourceEntryMap::const_iterator i = m_resources.find( uri );
    if ( i != m_resources.end() )
    {
        resourceEntry = (*i).second;
        return RESOLVE_RESOURCE;
    }
    for ( TTemplateEntryMap::const_iterator n = m_templates.begin(); n != m_templates.end(); ++n )
    {
        if ( (*n).second->matcher.Match( uri, variables ) )
        {
            templateEntry = (*n).second;
            return RESOLVE_TEMPLATE;
        }
    }
    variables.Clear();
    return RESOLVE_NONE;
}

/*-------------------------------------------------------------------------*/

void
CMcpResourceRegistry::GetResources( TResourceEntryVector& entries ) const
{GUCEF_TRACE;

    entries.clear();
    MT::CScopeReaderLock lock( m_rwLock );
    entries.reserve( m_resources.size() );
    for ( TResourceEntryMap::const_iterator i = m_resources.begin(); i != m_resources.end(); ++i )
        entries.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

void
CMcpResourceRegistry::GetTemplates( TTemplateEntryVector& entries ) const
{GUCEF_TRACE;

    entries.clear();
    MT::CScopeReaderLock lock( m_rwLock );
    entries.reserve( m_templates.size() );
    for ( TTemplateEntryMap::const_iterator i = m_templates.begin(); i != m_templates.end(); ++i )
        entries.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpResourceRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpResourceRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
