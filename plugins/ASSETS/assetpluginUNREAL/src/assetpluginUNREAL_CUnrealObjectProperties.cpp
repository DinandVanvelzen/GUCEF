/*
 *  assetpluginUNREAL: Typed access to a property tree by name path
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealObjectProperties::CUnrealObjectProperties( const CORE::CDataNode& root )
    : m_root( &root )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode*
CUnrealObjectProperties::Find( const CORE::CString& path ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = m_root;
    if ( path.IsNULLOrEmpty() )
        return node;
    const CORE::CString::StringVector parts = path.ParseElements( '/', false );
    for ( size_t i=0; i<parts.size() && GUCEF_NULL != node; ++i )
        node = node->FindChild( parts[ i ] );
    return node;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealObjectProperties::Has( const CORE::CString& path ) const
{GUCEF_TRACE;

    return GUCEF_NULL != Find( path );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealObjectProperties::GetBool( const CORE::CString& path, bool defaultValue ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    return GUCEF_NULL == node ? defaultValue : node->GetValue().AsBool( defaultValue );
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealObjectProperties::GetInt32( const CORE::CString& path, Int32 defaultValue ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    return GUCEF_NULL == node ? defaultValue : node->GetValue().AsInt32( defaultValue );
}

/*-------------------------------------------------------------------------*/

Int64
CUnrealObjectProperties::GetInt64( const CORE::CString& path, Int64 defaultValue ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    return GUCEF_NULL == node ? defaultValue : node->GetValue().AsInt64( defaultValue );
}

/*-------------------------------------------------------------------------*/

Float64
CUnrealObjectProperties::GetDouble( const CORE::CString& path, Float64 defaultValue ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    return GUCEF_NULL == node ? defaultValue : node->GetValue().AsFloat64( defaultValue );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealObjectProperties::GetString( const CORE::CString& path, const CORE::CString& defaultValue ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    if ( GUCEF_NULL == node )
        return defaultValue;
    const CORE::CDataNode::TKeyValuePair* resolved = node->GetAttribute( "path" );
    if ( GUCEF_NULL != resolved )
        return resolved->second.AsString( defaultValue );
    return node->GetValue().AsString( defaultValue );
}

/*-------------------------------------------------------------------------*/

Float64
CUnrealObjectProperties::GetComponent( const CORE::CString& path, const char* component, Float64 defaultValue ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    if ( GUCEF_NULL == node )
        return defaultValue;
    const CORE::CDataNode::TKeyValuePair* attribute = node->GetAttribute( CORE::CString( component ) );
    return GUCEF_NULL == attribute ? defaultValue : attribute->second.AsFloat64( defaultValue );
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealObjectProperties::GetCount( const CORE::CString& path ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    if ( GUCEF_NULL == node )
        return 0;
    const CORE::CDataNode::TKeyValuePair* packed = node->GetAttribute( "count" );
    if ( GUCEF_NULL != packed && node->GetAttributeValue( "packed" ).AsBool() )
        return packed->second.AsUInt32();
    return node->GetNrOfDirectChildNodes();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealObjectProperties::GetType( const CORE::CString& path ) const
{GUCEF_TRACE;

    const CORE::CDataNode* node = Find( path );
    if ( GUCEF_NULL == node )
        return CORE::CString();
    return node->GetAttributeValue( "type" ).AsString();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
