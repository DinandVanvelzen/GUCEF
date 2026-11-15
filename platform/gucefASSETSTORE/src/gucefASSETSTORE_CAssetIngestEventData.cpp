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
#include "gucefASSETSTORE_CAssetIngestEventData.h"

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

CAssetIngestEventData::CAssetIngestEventData( void )
    : CORE::CIDataNodeSerializable()
    , ingestId()
    , packId()
    , step()
    , status()
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetIngestEventData::~CAssetIngestEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetIngestEventData::Clear( void )
{GUCEF_TRACE;

    *this = CAssetIngestEventData();
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestEventData::Serialize( CORE::CDataNode& domRootNode                        ,
                                 const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "ingestId", ingestId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "step", step ) && success;
    success = domRootNode.SetAttribute( "status", status ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestEventData::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetIngestEventData defaults;
    ingestId = domRootNode.GetAttributeValueOrChildValueByName( "ingestId" ).AsString( defaults.ingestId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( defaults.packId );
    step = domRootNode.GetAttributeValueOrChildValueByName( "step" ).AsString( defaults.step );
    status = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( defaults.status );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( defaults.message );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetIngestEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetIngestEventData( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetIngestEventData::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetIngestEventData";
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
