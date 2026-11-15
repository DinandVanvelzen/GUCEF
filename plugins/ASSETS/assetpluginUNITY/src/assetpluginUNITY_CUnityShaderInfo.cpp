/*
 *  assetpluginUNITY: the properties a shader of a package declares
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

#include "assetpluginUNITY_CUnityShaderInfo.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline bool IsIdentifierChar( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '_' == c;
}

/** A cursor over ShaderLab text that skips white space and comments */
class CShaderLabCursor
{
    public:

    CShaderLabCursor( const char* text, const char* end ) : p( text ), m_end( end ) {}

    void SkipSpace( void )
    {
        while ( p < m_end )
        {
            if ( ' ' == *p || '\t' == *p || '\r' == *p || '\n' == *p )
            {
                ++p;
            }
            else
            if ( '/' == *p && p+1 < m_end && '/' == p[ 1 ] )
            {
                while ( p < m_end && '\n' != *p )
                    ++p;
            }
            else
            if ( '/' == *p && p+1 < m_end && '*' == p[ 1 ] )
            {
                p += 2;
                while ( p+1 < m_end && !( '*' == *p && '/' == p[ 1 ] ) )
                    ++p;
                p = p+2 <= m_end ? p+2 : m_end;
            }
            else
            {
                break;
            }
        }
    }

    bool AtEnd( void ) { SkipSpace(); return p >= m_end; }

    bool Accept( char c )
    {
        SkipSpace();
        if ( p < m_end && *p == c )
        {
            ++p;
            return true;
        }
        return false;
    }

    CORE::CString ReadIdentifier( void )
    {
        SkipSpace();
        const char* start = p;
        while ( p < m_end && IsIdentifierChar( *p ) )
            ++p;
        return start == p ? CORE::CString::Empty : CORE::CString( start, (UInt32) ( p - start ) );
    }

    CORE::CString ReadString( void )
    {
        SkipSpace();
        if ( p >= m_end || '"' != *p )
            return CORE::CString::Empty;
        const char* start = ++p;
        while ( p < m_end && '"' != *p )
            ++p;
        const CORE::CString value = p > start ? CORE::CString( start, (UInt32) ( p - start ) ) : CORE::CString::Empty;
        if ( p < m_end )
            ++p;
        return value;
    }

    /** The text up to the bracket that closes one already opened ('(' or '[' or '{'), without it */
    CORE::CString ReadUntilClose( char open, char close )
    {
        const char* start = p;
        Int32 depth = 1;
        while ( p < m_end )
        {
            if ( *p == open )
                ++depth;
            else
            if ( *p == close && 0 == --depth )
                break;
            ++p;
        }
        const CORE::CString value = p > start ? CORE::CString( start, (UInt32) ( p - start ) ) : CORE::CString::Empty;
        if ( p < m_end )
            ++p;
        return value.Trim( true ).Trim( false );
    }

    /** Moves to the first occurrence of the word outside comments; false when there is none */
    bool FindWord( const char* word )
    {
        const size_t length = strlen( word );
        while ( true )
        {
            SkipSpace();
            if ( p >= m_end )
                return false;
            if ( '"' == *p )
            {
                ReadString();
                continue;
            }
            if ( (size_t) ( m_end - p ) >= length && 0 == strncmp( p, word, length ) && ( p + length == m_end || !IsIdentifierChar( p[ length ] ) ) )
            {
                p += length;
                return true;
            }
            if ( IsIdentifierChar( *p ) )
            {
                while ( p < m_end && IsIdentifierChar( *p ) )
                    ++p;
            }
            else
            {
                ++p;
            }
        }
    }

    const char* p;

    private:

    const char* m_end;
};

/** The text of a JSON string member after 'position' (the key name), also when it is escaped inside a string */
CORE::CString
ReadJsonValueAfter( const char* position, const char* end )
{
    const char* p = position;
    while ( p < end && ':' != *p )
        ++p;
    if ( p >= end )
        return CORE::CString::Empty;
    ++p;
    while ( p < end && ( ' ' == *p || '\t' == *p || '\\' == *p || '\r' == *p || '\n' == *p ) )
        ++p;
    if ( p >= end || '"' != *p )
        return CORE::CString::Empty;
    const char* start = ++p;
    while ( p < end && !( '"' == *p ) )
        ++p;
    const char* stop = p;
    if ( stop > start && '\\' == stop[ -1 ] )
        --stop;     // an escaped quote ends an escaped string
    return stop > start ? CORE::CString( start, (UInt32) ( stop - start ) ) : CORE::CString::Empty;
}

/** The first occurrence of the text in [begin, end), NULL when there is none */
const char*
FindText( const char* begin, const char* end, const char* text )
{
    const size_t length = strlen( text );
    for ( const char* p = begin; p + length <= end; ++p )
        if ( 0 == strncmp( p, text, length ) )
            return p;
    return GUCEF_NULL;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityShaderInfo::CUnityShaderInfo( void )
    : name()
    , kind()
    , properties()
    , transparent( false )
    , alphaTested( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnityShaderInfo::ParseShaderLab( const CORE::CString& text )
{GUCEF_TRACE;

    *this = CUnityShaderInfo();
    kind = "shader";
    const char* begin = text.C_String();
    if ( GUCEF_NULL == begin )
        return false;
    const char* end = begin + strlen( begin );

    CShaderLabCursor cursor( begin, end );
    if ( !cursor.FindWord( "Shader" ) )
        return false;
    name = cursor.ReadString();

    // the Properties block
    CShaderLabCursor block( cursor.p, end );
    if ( block.FindWord( "Properties" ) && block.Accept( '{' ) )
    {
        while ( !block.AtEnd() )
        {
            if ( block.Accept( '}' ) )
                break;
            SProperty property;
            while ( block.Accept( '[' ) )
                property.attributes.push_back( block.ReadUntilClose( '[', ']' ) );
            property.name = block.ReadIdentifier();
            if ( property.name.IsNULLOrEmpty() || !block.Accept( '(' ) )
                break;
            property.displayName = block.ReadString();
            if ( !block.Accept( ',' ) )
                break;
            const CORE::CString type = block.ReadUntilClose( '(', ')' );
            property.type = type.HasChar( '(' ) >= 0 ? type.SubstrToChar( '(', true ).Trim( true ).Trim( false ) : type;
            if ( block.Accept( '=' ) )
            {
                block.SkipSpace();
                if ( block.p < end && '"' == *block.p )
                {
                    property.defaultValue = block.ReadString();
                    if ( block.Accept( '{' ) )
                        block.ReadUntilClose( '{', '}' );
                }
                else
                if ( block.Accept( '(' ) )
                {
                    property.defaultValue = block.ReadUntilClose( '(', ')' );
                }
                else
                {
                    const char* start = block.p;
                    while ( block.p < end && ( IsIdentifierChar( *block.p ) || '.' == *block.p || '-' == *block.p || '+' == *block.p ) )
                        ++block.p;
                    property.defaultValue = block.p > start ? CORE::CString( start, (UInt32) ( block.p - start ) ) : CORE::CString::Empty;
                }
            }
            properties.push_back( property );
        }
    }

    // the blending, from the surface shader options and the tags
    CORE::CString compact = text.Lowercase().RemoveChar( ' ' ).RemoveChar( '\t' );
    transparent = compact.HasSubstr( "alpha:fade" ) >= 0 || compact.HasSubstr( "alpha:blend" ) >= 0 || compact.HasSubstr( "alpha:premul" ) >= 0 ||
                  compact.HasSubstr( "\"queue\"=\"transparent" ) >= 0 || compact.HasSubstr( "blendsrcalphaoneminussrcalpha" ) >= 0;
    alphaTested = compact.HasSubstr( "alphatest:" ) >= 0 || compact.HasSubstr( "\"queue\"=\"alphatest" ) >= 0;
    return !name.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CUnityShaderInfo::ParseShaderGraph( const CORE::CString& text, const CORE::CString& assetName )
{GUCEF_TRACE;

    *this = CUnityShaderInfo();
    kind = "shaderGraph";
    name = "Shader Graphs/" + assetName;
    const char* begin = text.C_String();
    if ( GUCEF_NULL == begin )
        return false;
    const char* end = begin + strlen( begin );

    // every serialised property names its type as "...<Type>ShaderProperty"; its names follow it
    const char* marker = "ShaderProperty";
    const char* occurrence = FindText( begin, end, marker );
    while ( GUCEF_NULL != occurrence )
    {
        const char* next = FindText( occurrence + 1, end, marker );
        const char* segmentEnd = GUCEF_NULL != next ? next : end;
        const char* typeStart = occurrence;
        while ( typeStart > begin && IsIdentifierChar( typeStart[ -1 ] ) )
            --typeStart;
        const CORE::CString typeWord = occurrence > typeStart ? CORE::CString( typeStart, (UInt32) ( occurrence - typeStart ) ) : CORE::CString::Empty;

        SProperty property;
        if ( typeWord.StartsWith( "Texture2DArray" ) ) property.type = "2DArray";
        else if ( typeWord.StartsWith( "Texture2D" ) ) property.type = "2D";
        else if ( typeWord.StartsWith( "Texture3D" ) ) property.type = "3D";
        else if ( typeWord.StartsWith( "Cubemap" ) ) property.type = "Cube";
        else if ( typeWord.StartsWith( "Color" ) ) property.type = "Color";
        else if ( typeWord.StartsWith( "Vector1" ) || typeWord.StartsWith( "Float" ) || typeWord.StartsWith( "Boolean" ) ) property.type = "Float";
        else if ( typeWord.StartsWith( "Vector" ) ) property.type = "Vector";
        else if ( typeWord.StartsWith( "Matrix" ) ) property.type = "Matrix";
        else property.type = typeWord;

        const char* overrideName = FindText( occurrence, segmentEnd, "m_OverrideReferenceName" );
        const char* defaultName = FindText( occurrence, segmentEnd, "m_DefaultReferenceName" );
        const char* displayName = FindText( occurrence, segmentEnd, "m_Name" );
        if ( GUCEF_NULL != overrideName )
            property.name = ReadJsonValueAfter( overrideName + strlen( "m_OverrideReferenceName" ), segmentEnd );
        if ( property.name.IsNULLOrEmpty() && GUCEF_NULL != defaultName )
            property.name = ReadJsonValueAfter( defaultName + strlen( "m_DefaultReferenceName" ), segmentEnd );
        if ( GUCEF_NULL != displayName )
            property.displayName = ReadJsonValueAfter( displayName + strlen( "m_Name" ), segmentEnd );
        if ( !property.name.IsNULLOrEmpty() && GUCEF_NULL == FindProperty( property.name ) && !typeWord.IsNULLOrEmpty() )
            properties.push_back( property );
        occurrence = next;
    }

    // a transparent surface: m_SurfaceType 1
    const char* surface = FindText( begin, end, "m_SurfaceType" );
    if ( GUCEF_NULL != surface )
    {
        const char* p = surface + strlen( "m_SurfaceType" );
        while ( p < end && !( *p >= '0' && *p <= '9' ) && '\n' != *p )
            ++p;
        transparent = p < end && '1' == *p;
    }
    alphaTested = GUCEF_NULL != FindText( begin, end, "\"m_AlphaClip\": true" ) || GUCEF_NULL != FindText( begin, end, "\"m_AlphaClip\":true" );
    return true;
}

/*-------------------------------------------------------------------------*/

const CUnityShaderInfo::SProperty*
CUnityShaderInfo::FindProperty( const CORE::CString& propertyName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<properties.size(); ++i )
        if ( properties[ i ].name == propertyName )
            return &properties[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CUnityShaderInfo::HasAttribute( const SProperty& property, const CORE::CString& attribute ) const
{GUCEF_TRACE;

    const CORE::CString wanted = attribute.Lowercase();
    for ( size_t i=0; i<property.attributes.size(); ++i )
    {
        CORE::CString attributeName = property.attributes[ i ];
        if ( attributeName.HasChar( '(' ) >= 0 )
            attributeName = attributeName.SubstrToChar( '(', true );
        if ( attributeName.Trim( true ).Trim( false ).Lowercase() == wanted )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CUnityShaderInfo::IsTextureType( const CORE::CString& type )
{GUCEF_TRACE;

    const CORE::CString lower = type.Lowercase();
    return "2d" == lower || "3d" == lower || "cube" == lower || "2darray" == lower || "any" == lower;
}

/*-------------------------------------------------------------------------*/

bool
CUnityShaderInfo::IsColourType( const CORE::CString& type )
{GUCEF_TRACE;

    return "color" == type.Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CUnityShaderInfo::IsNumberType( const CORE::CString& type )
{GUCEF_TRACE;

    const CORE::CString lower = type.Lowercase();
    return "float" == lower || "range" == lower || "int" == lower || "integer" == lower;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
