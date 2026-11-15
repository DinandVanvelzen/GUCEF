/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#include "gucefASSETSTORE_CAssetStorageLocation.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStorageLocation::CAssetStorageLocation( void )
    : CORE::CIDataNodeSerializable()
    , locationId()
    , name()
    , vfsRoot()
    , fileSystemPath()
    , writeable( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStorageLocation::~CAssetStorageLocation()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStorageLocation::Clear( void )
{GUCEF_TRACE;

    *this = CAssetStorageLocation();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStorageLocation::Serialize( CORE::CDataNode& domRootNode                        ,
                                 const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "vfsRoot", vfsRoot ) && success;
    success = domRootNode.SetAttribute( "fileSystemPath", fileSystemPath ) && success;
    success = domRootNode.SetAttribute( "writeable", writeable ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStorageLocation::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetStorageLocation defaults;
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString( defaults.locationId );
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( defaults.name );
    vfsRoot = domRootNode.GetAttributeValueOrChildValueByName( "vfsRoot" ).AsString( defaults.vfsRoot );
    fileSystemPath = domRootNode.GetAttributeValueOrChildValueByName( "fileSystemPath" ).AsString( defaults.fileSystemPath );
    writeable = domRootNode.GetAttributeValueOrChildValueByName( "writeable" ).AsBool( defaults.writeable, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStorageLocation::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStorageLocation( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStorageLocation::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStorageLocation::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStorageLocation";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStorageLocation::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStorageLocation";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
