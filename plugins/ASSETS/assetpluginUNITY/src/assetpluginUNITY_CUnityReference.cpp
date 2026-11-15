/*
 *  assetpluginUNITY: a reference to an object of a Unity file
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#include "assetpluginUNITY_CUnityReference.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CUnityReference::BuiltinDefaultGuid = "0000000000000000e000000000000000";
const CORE::CString CUnityReference::BuiltinExtraGuid = "0000000000000000f000000000000000";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityReference::CUnityReference( void )
    : fileId( 0 )
    , guid()
    , type( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityReference::CUnityReference( Int64 id, const CORE::CString& assetGuid, Int32 referenceType )
    : fileId( id )
    , guid( assetGuid )
    , type( referenceType )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnityReference::IsNull( void ) const
{GUCEF_TRACE;

    return 0 == fileId;
}

/*-------------------------------------------------------------------------*/

bool
CUnityReference::IsLocal( void ) const
{GUCEF_TRACE;

    return 0 != fileId && guid.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CUnityReference::IsBuiltin( void ) const
{GUCEF_TRACE;

    return guid == BuiltinDefaultGuid || guid == BuiltinExtraGuid;
}

/*-------------------------------------------------------------------------*/

bool
CUnityReference::Read( const CORE::CDataNode* node )
{GUCEF_TRACE;

    *this = CUnityReference();
    if ( GUCEF_NULL == node || GUCEF_DATATYPE_OBJECT != node->GetNodeType() || GUCEF_NULL == node->GetAttribute( "fileID" ) )
        return false;
    fileId = CUnityYamlReader::GetInt64( *node, "fileID", 0 );
    guid = CUnityYamlReader::GetString( *node, "guid" ).Lowercase();
    type = CUnityYamlReader::GetInt32( *node, "type", 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

CUnityReference
CUnityReference::Get( const CORE::CDataNode& mapping, const CORE::CString& key )
{GUCEF_TRACE;

    CUnityReference reference;
    reference.Read( mapping.FindChild( key ) );
    return reference;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnityReference::ToString( void ) const
{GUCEF_TRACE;

    if ( guid.IsNULLOrEmpty() )
        return "{fileID: " + CORE::Int64ToString( fileId ) + "}";
    return "{fileID: " + CORE::Int64ToString( fileId ) + ", guid: " + guid + ", type: " + CORE::Int32ToString( type ) + "}";
}

/*-------------------------------------------------------------------------*/

bool
CUnityReference::operator==( const CUnityReference& other ) const
{GUCEF_TRACE;

    return fileId == other.fileId && guid == other.guid && type == other.type;
}

/*-------------------------------------------------------------------------*/

bool
CUnityReference::operator!=( const CUnityReference& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CUnityReference::operator<( const CUnityReference& other ) const
{GUCEF_TRACE;

    if ( guid != other.guid )
        return guid < other.guid;
    if ( fileId != other.fileId )
        return fileId < other.fileId;
    return type < other.type;
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
