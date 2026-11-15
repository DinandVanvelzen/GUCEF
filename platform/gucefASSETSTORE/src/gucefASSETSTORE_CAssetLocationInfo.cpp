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
#include "gucefASSETSTORE_CAssetLocationInfo.h"

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

CAssetLocationInfo::CAssetLocationInfo( void )
    : CORE::CIDataNodeSerializable()
    , location()
    , online( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLocationInfo::~CAssetLocationInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetLocationInfo::Clear( void )
{GUCEF_TRACE;

    *this = CAssetLocationInfo();
}

/*-------------------------------------------------------------------------*/

bool
CAssetLocationInfo::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = location.Serialize( *domRootNode.AddChild( "location", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "online", online ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLocationInfo::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    const CORE::CDataNode* locationNode = domRootNode.FindChild( "location" );
    if ( GUCEF_NULL != locationNode )
        location.Deserialize( *locationNode, settings );
    online = domRootNode.GetAttributeValueOrChildValueByName( "online" ).AsBool( online, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLocationInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLocationInfo( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLocationInfo::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLocationInfo::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetLocationInfo";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLocationInfo::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLocationInfo";
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
