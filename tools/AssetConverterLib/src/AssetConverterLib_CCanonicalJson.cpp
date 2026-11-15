/*
 *  AssetConverterLib: deterministic JSON text of data trees
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#include "AssetConverterLib_CCanonicalJson.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A member of an object: an attribute or a child, by name */
struct SMember
{
    CORE::CString name;
    const CORE::CVariant* attribute;
    const CORE::CDataNode* child;
};

struct SMemberOrder
{
    bool operator()( const SMember& a, const SMember& b ) const
    {
        return a.name < b.name;
    }
};

void
AppendText( CORE::CDynamicBuffer& buffer, const CORE::CString& text )
{
    if ( text.ByteSize() > 1 )
        buffer.Append( text.C_String(), text.ByteSize()-1, true );
}

void
AppendText( CORE::CDynamicBuffer& buffer, const char* text )
{
    buffer.Append( text, (UInt32) strlen( text ), true );
}

void
AppendIndent( CORE::CDynamicBuffer& buffer, UInt32 depth )
{
    for ( UInt32 i=0; i<depth; ++i )
        buffer.Append( "    ", 4, true );
}

bool
IsContainer( const CORE::CDataNode& node )
{
    return GUCEF_DATATYPE_OBJECT == node.GetNodeType() || GUCEF_DATATYPE_ARRAY == node.GetNodeType();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CCanonicalJson::FormatFloat32( Float32 value )
{GUCEF_TRACE;

    if ( value != value || value - value != 0.0f )
        return "null";
    if ( 0.0f == value )
        return "0";
    char text[ 64 ];
    for ( int precision=1; precision<=9; ++precision )
    {
        snprintf( text, sizeof( text ), "%.*g", precision, (double) value );
        if ( (Float32) strtod( text, GUCEF_NULL ) == value )
            break;
    }
    // the decimal separator of a locale is not that of JSON
    for ( char* c=text; '\0' != *c; ++c )
        if ( ',' == *c )
            *c = '.';
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCanonicalJson::FormatFloat64( Float64 value )
{GUCEF_TRACE;

    if ( value != value || value - value != 0.0 )
        return "null";
    if ( 0.0 == value )
        return "0";
    char text[ 64 ];
    for ( int precision=1; precision<=17; ++precision )
    {
        snprintf( text, sizeof( text ), "%.*g", precision, value );
        if ( strtod( text, GUCEF_NULL ) == value )
            break;
    }
    for ( char* c=text; '\0' != *c; ++c )
        if ( ',' == *c )
            *c = '.';
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCanonicalJson::QuoteString( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString result = "\"";
    const char* c = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize()-1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const unsigned char ch = (unsigned char) c[ i ];
        switch ( ch )
        {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            default:
            {
                if ( ch < 0x20 )
                {
                    char escaped[ 8 ];
                    snprintf( escaped, sizeof( escaped ), "\\u%04x", (unsigned int) ch );
                    result += escaped;
                }
                else
                {
                    result += (char) ch;
                }
                break;
            }
        }
    }
    result += "\"";
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCanonicalJson::FormatValue( const CORE::CVariant& value )
{GUCEF_TRACE;

    if ( !value.IsInitialized() || value.IsNIL() || value.IsNULL() )
        return "null";
    if ( value.IsBoolean() )
        return value.AsBool() ? "true" : "false";
    if ( value.IsInteger() )
    {
        if ( value.IsSignedInteger() )
            return CORE::Int64ToString( value.AsInt64() );
        return CORE::UInt64ToString( value.AsUInt64() );
    }
    if ( value.IsFloat() )
    {
        if ( GUCEF_DATATYPE_FLOAT32 == value.GetTypeId() )
            return FormatFloat32( value.AsFloat32() );
        return FormatFloat64( value.AsFloat64() );
    }
    return QuoteString( value.AsString() );
}

/*-------------------------------------------------------------------------*/

void
CCanonicalJson::Append( const CORE::CDataNode& node, UInt32 depth, CORE::CDynamicBuffer& buffer )
{GUCEF_TRACE;

    if ( GUCEF_DATATYPE_ARRAY == node.GetNodeType() )
    {
        if ( !node.HasChildren() )
        {
            AppendText( buffer, "[]" );
            return;
        }
        AppendText( buffer, "[\n" );
        bool first = true;
        CORE::CDataNode::const_iterator i = node.ConstBegin();
        while ( i != node.ConstEnd() )
        {
            if ( !first )
                AppendText( buffer, ",\n" );
            first = false;
            AppendIndent( buffer, depth+1 );
            Append( *(*i), depth+1, buffer );
            ++i;
        }
        AppendText( buffer, "\n" );
        AppendIndent( buffer, depth );
        AppendText( buffer, "]" );
        return;
    }
    if ( GUCEF_DATATYPE_OBJECT != node.GetNodeType() )
    {
        AppendText( buffer, FormatValue( node.GetValue() ) );
        return;
    }

    // an object: the attributes and the children as members, by name
    GUCEF::vector< SMember > members;
    members.reserve( node.GetAttCount() + node.GetNrOfDirectChildNodes() );
    CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin();
    while ( a != node.AttributeEnd() )
    {
        SMember member;
        member.name = (*a).first;
        member.attribute = &(*a).second;
        member.child = GUCEF_NULL;
        members.push_back( member );
        ++a;
    }
    CORE::CDataNode::const_iterator c = node.ConstBegin();
    while ( c != node.ConstEnd() )
    {
        SMember member;
        member.name = (*c)->GetName();
        member.attribute = GUCEF_NULL;
        member.child = (*c);
        members.push_back( member );
        ++c;
    }
    if ( members.empty() )
    {
        AppendText( buffer, "{}" );
        return;
    }
    std::stable_sort( members.begin(), members.end(), SMemberOrder() );

    AppendText( buffer, "{\n" );
    for ( size_t m=0; m<members.size(); ++m )
    {
        if ( m > 0 )
            AppendText( buffer, ",\n" );
        AppendIndent( buffer, depth+1 );
        AppendText( buffer, QuoteString( members[ m ].name ) );
        AppendText( buffer, ": " );
        if ( GUCEF_NULL != members[ m ].attribute )
            AppendText( buffer, FormatValue( *members[ m ].attribute ) );
        else
            Append( *members[ m ].child, depth+1, buffer );
    }
    AppendText( buffer, "\n" );
    AppendIndent( buffer, depth );
    AppendText( buffer, "}" );
}

/*-------------------------------------------------------------------------*/

void
CCanonicalJson::ToJson( const CORE::CDataNode& node, CORE::CDynamicBuffer& buffer )
{GUCEF_TRACE;

    Append( node, 0, buffer );
    AppendText( buffer, "\n" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCanonicalJson::ToJson( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer( 4096, true );
    ToJson( node, buffer );
    const char terminator = '\0';
    buffer.Append( &terminator, 1, true );
    return CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCanonicalJson::Hash( const CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer( 4096, true );
    ToJson( node, buffer );
    return CContentHash::HashBuffer( buffer.GetConstBufferPtr(), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

bool
CCanonicalJson::Store( const CORE::CString& vfsPath, const CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDynamicBuffer buffer( 4096, true );
    ToJson( node, buffer );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = CORE::StripFilename( vfsPath );
    if ( !folder.IsNULLOrEmpty() )
        vfs.CreateDir( folder );
    if ( !vfs.StoreAsFile( vfsPath, buffer, 0, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CanonicalJson: Cannot write " << vfsPath );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

namespace {

/** The codecs give the top object as the node itself or as its only child: the members end up in 'node' either way */
void
HoistTopObject( CORE::CDataNode& loaded, CORE::CDataNode& node )
{
    if ( 0 == loaded.GetAttCount() && 1 == loaded.GetNrOfDirectChildNodes() )
    {
        const CORE::CDataNode* top = loaded.GetFirstChild();
        if ( GUCEF_NULL != top && top->GetName().IsNULLOrEmpty() && IsContainer( *top ) )
        {
            node = *top;
            return;
        }
    }
    node = loaded;
}

}

/*-------------------------------------------------------------------------*/

bool
CCanonicalJson::Load( const CORE::CString& vfsPath, CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDataNode loaded;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( loaded, vfsPath, "json" ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CanonicalJson: Cannot read " << vfsPath << " as JSON (is the JSON codec plugin loaded?)" );
        return false;
    }
    HoistTopObject( loaded, node );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCanonicalJson::Parse( const CORE::CString& text, CORE::CDataNode& node )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CanonicalJson: There is no JSON codec (is the JSON codec plugin loaded?)" );
        return false;
    }
    CORE::CDynamicBuffer buffer( text.C_String(), text.ByteSize() > 0 ? text.ByteSize()-1 : 0, true );
    CORE::CDynamicBufferAccess access( buffer );
    CORE::CDataNode loaded;
    if ( !codec->BuildDataTree( &loaded, &access ) )
        return false;
    HoistTopObject( loaded, node );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
