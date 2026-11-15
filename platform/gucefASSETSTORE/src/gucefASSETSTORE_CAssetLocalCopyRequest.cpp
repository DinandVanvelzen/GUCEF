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
#include "gucefASSETSTORE_CAssetLocalCopyRequest.h"

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

CAssetLocalCopyRequest::CAssetLocalCopyRequest( void )
    : CORE::CIDataNodeSerializable()
    , packId()
    , assetIds()
    , exportJobId()
    , destination()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLocalCopyRequest::~CAssetLocalCopyRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetLocalCopyRequest::Clear( void )
{GUCEF_TRACE;

    *this = CAssetLocalCopyRequest();
}

/*-------------------------------------------------------------------------*/

bool
CAssetLocalCopyRequest::Serialize( CORE::CDataNode& domRootNode                        ,
                                  const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetValuesOfChildByName( "assetIds", assetIds ) && success;
    success = domRootNode.SetAttribute( "exportJobId", exportJobId ) && success;
    success = domRootNode.SetAttribute( "destination", destination ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLocalCopyRequest::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( packId );
    domRootNode.GetValuesOfChildByName( "assetIds", assetIds );
    exportJobId = domRootNode.GetAttributeValueOrChildValueByName( "exportJobId" ).AsString( exportJobId );
    destination = domRootNode.GetAttributeValueOrChildValueByName( "destination" ).AsString( destination );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLocalCopyRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLocalCopyRequest( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLocalCopyRequest::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLocalCopyRequest::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetLocalCopyRequest";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLocalCopyRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLocalCopyRequest";
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
