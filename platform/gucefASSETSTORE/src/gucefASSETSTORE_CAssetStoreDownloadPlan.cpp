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
#include "gucefASSETSTORE_CAssetStoreDownloadPlan.h"

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

CAssetStoreDownloadPlan::CAssetStoreDownloadPlan( void )
    : CORE::CIDataNodeSerializable()
    , providerId()
    , itemId()
    , formatId()
    , fileIds()
    , transfers()
    , createdAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadPlan::~CAssetStoreDownloadPlan()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAssetStoreTransfer*
CAssetStoreDownloadPlan::FindTransfer( const CORE::CString& fileId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<transfers.size(); ++i )
        if ( transfers[ i ].fileId == fileId )
            return &transfers[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreDownloadPlan::GetTotalSize( void ) const
{GUCEF_TRACE;

    UInt64 total = 0;
    for ( size_t i=0; i<transfers.size(); ++i )
        total += transfers[ i ].size;
    return total;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadPlan::Serialize( CORE::CDataNode& domRootNode                        ,
                                    const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "providerId", providerId );
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = domRootNode.SetAttribute( "formatId", formatId ) && success;
    success = domRootNode.SetValuesOfChildByName( "fileIds", fileIds ) && success;
    success = domRootNode.SetAttribute( "createdAtMs", createdAtMs ) && success;
    CORE::CDataNode* transfersNode = domRootNode.AddChild( "transfers", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == transfersNode )
        return false;
    for ( size_t i=0; i<transfers.size(); ++i )
    {
        CORE::CDataNode* transferNode = transfersNode->AddChild( "transfer", GUCEF_DATATYPE_OBJECT );
        success = GUCEF_NULL != transferNode && transfers[ i ].Serialize( *transferNode, settings ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadPlan::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                      const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    providerId = domRootNode.GetAttributeValueOrChildValueByName( "providerId" ).AsString();
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString();
    formatId = domRootNode.GetAttributeValueOrChildValueByName( "formatId" ).AsString();
    domRootNode.GetValuesOfChildByName( "fileIds", fileIds );
    createdAtMs = domRootNode.GetAttributeValueOrChildValueByName( "createdAtMs" ).AsInt64( 0, true );
    transfers.clear();
    const CORE::CDataNode* transfersNode = domRootNode.FindChild( "transfers" );
    if ( GUCEF_NULL != transfersNode )
    {
        CORE::CDataNode::const_iterator i = transfersNode->ConstBegin();
        while ( i != transfersNode->ConstEnd() )
        {
            CAssetStoreTransfer transfer;
            if ( transfer.Deserialize( *(*i), settings ) )
                transfers.push_back( transfer );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreDownloadPlan::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreDownloadPlan( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreDownloadPlan::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreDownloadPlan";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreDownloadPlan::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreDownloadPlan::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreDownloadPlan";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
