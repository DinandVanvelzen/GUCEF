/*
 *  assetpluginGLTF: the deterministic JSON writer of the glTF exporter
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "assetpluginGLTF_CGltfJsonWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGltfJsonWriter::CGltfJsonWriter( void )
    : m_text( 4096, true )
    , m_levels()
    , m_afterKey( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGltfJsonWriter::~CGltfJsonWriter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::Append( const char* text )
{GUCEF_TRACE;

    m_text.Append( text, (UInt32) strlen( text ) );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::Append( const CORE::CString& text )
{GUCEF_TRACE;

    if ( text.ByteSize() > 1 )
        m_text.Append( text.C_String(), text.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::BeginItem( void )
{GUCEF_TRACE;

    if ( m_afterKey )
    {
        m_afterKey = false;
        return;
    }
    if ( m_levels.empty() )
        return;
    SLevel& level = m_levels.back();
    if ( level.count > 0 )
        Append( "," );
    Append( "\n" );
    for ( size_t i=0; i<m_levels.size(); ++i )
        Append( "  " );
    ++level.count;
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::BeginObject( void )
{GUCEF_TRACE;

    BeginItem();
    Append( "{" );
    SLevel level;
    level.isObject = true;
    level.count = 0;
    m_levels.push_back( level );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::EndObject( void )
{GUCEF_TRACE;

    const bool hadMembers = !m_levels.empty() && m_levels.back().count > 0;
    if ( !m_levels.empty() )
        m_levels.pop_back();
    if ( hadMembers )
    {
        Append( "\n" );
        for ( size_t i=0; i<m_levels.size(); ++i )
            Append( "  " );
    }
    Append( "}" );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::BeginArray( void )
{GUCEF_TRACE;

    BeginItem();
    Append( "[" );
    SLevel level;
    level.isObject = false;
    level.count = 0;
    m_levels.push_back( level );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::EndArray( void )
{GUCEF_TRACE;

    const bool hadItems = !m_levels.empty() && m_levels.back().count > 0;
    if ( !m_levels.empty() )
        m_levels.pop_back();
    if ( hadItems )
    {
        Append( "\n" );
        for ( size_t i=0; i<m_levels.size(); ++i )
            Append( "  " );
    }
    Append( "]" );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::Key( const char* name )
{GUCEF_TRACE;

    BeginItem();
    Append( ASSETCONVERTER::CCanonicalJson::QuoteString( name ) );
    Append( ": " );
    m_afterKey = true;
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::String( const CORE::CString& value )
{GUCEF_TRACE;

    BeginItem();
    Append( ASSETCONVERTER::CCanonicalJson::QuoteString( value ) );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::Float( Float32 value )
{GUCEF_TRACE;

    BeginItem();
    Append( ASSETCONVERTER::CCanonicalJson::FormatFloat32( value ) );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::Int( Int64 value )
{GUCEF_TRACE;

    BeginItem();
    Append( CORE::Int64ToString( value ) );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::Bool( bool value )
{GUCEF_TRACE;

    BeginItem();
    Append( value ? "true" : "false" );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::Node( const CORE::CDataNode& tree )
{GUCEF_TRACE;

    BeginItem();
    CORE::CDynamicBuffer text( 256, true );
    ASSETCONVERTER::CCanonicalJson::ToJson( tree, text );
    // the tree text ends with a line break that belongs to a file, not to a value
    UInt32 size = text.GetDataSize();
    while ( size > 0 && ( '\n' == text.AsConstType< char >( size-1 ) || ' ' == text.AsConstType< char >( size-1 ) ) )
        --size;
    m_text.Append( text.GetConstBufferPtr(), size );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::FloatArray( const Float32* values, UInt32 count )
{GUCEF_TRACE;

    BeginItem();
    Append( "[" );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( i > 0 )
            Append( ", " );
        Append( ASSETCONVERTER::CCanonicalJson::FormatFloat32( values[ i ] ) );
    }
    Append( "]" );
}

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::IntArray( const UInt32* values, UInt32 count )
{GUCEF_TRACE;

    BeginItem();
    Append( "[" );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( i > 0 )
            Append( ", " );
        Append( CORE::UInt32ToString( values[ i ] ) );
    }
    Append( "]" );
}

/*-------------------------------------------------------------------------*/

void CGltfJsonWriter::Member( const char* name, const CORE::CString& value ) { Key( name ); String( value ); }
void CGltfJsonWriter::Member( const char* name, const char* value ) { Key( name ); String( value ); }
void CGltfJsonWriter::Member( const char* name, Float32 value ) { Key( name ); Float( value ); }
void CGltfJsonWriter::Member( const char* name, Int64 value ) { Key( name ); Int( value ); }
void CGltfJsonWriter::Member( const char* name, Int32 value ) { Key( name ); Int( value ); }
void CGltfJsonWriter::Member( const char* name, UInt32 value ) { Key( name ); Int( value ); }
void CGltfJsonWriter::Member( const char* name, bool value ) { Key( name ); Bool( value ); }
void CGltfJsonWriter::MemberFloatArray( const char* name, const Float32* values, UInt32 count ) { Key( name ); FloatArray( values, count ); }
void CGltfJsonWriter::MemberIntArray( const char* name, const UInt32* values, UInt32 count ) { Key( name ); IntArray( values, count ); }
void CGltfJsonWriter::MemberNode( const char* name, const CORE::CDataNode& tree ) { Key( name ); Node( tree ); }

/*-------------------------------------------------------------------------*/

void
CGltfJsonWriter::EndDocument( void )
{GUCEF_TRACE;

    Append( "\n" );
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CGltfJsonWriter::GetText( void ) const
{GUCEF_TRACE;

    return m_text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
