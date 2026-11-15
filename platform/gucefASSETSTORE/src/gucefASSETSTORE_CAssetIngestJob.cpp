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
#include "gucefASSETSTORE_CAssetIngestJob.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetIngestJob::ManifestFileName = "ingest.json";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetIngestJob::CAssetIngestJob( void )
    : CORE::CIDataNodeSerializable()
    , ingestId()
    , request()
    , lastStep( ASSETINGEST_STEP_NONE )
    , failedStep( ASSETINGEST_STEP_NONE )
    , failureStatus( ASSETSTORE_STATUS_OK )
    , failureMessage()
    , locationId()
    , stagingFolder()
    , contentRoot()
    , contentPrefix()
    , inPlace( false )
    , contentInStaging( false )
    , archives()
    , pack()
    , assets()
    , relations()
    , problems()
    , startedAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetIngestJob::~CAssetIngestJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetIngestJob::GetManifestPath( void ) const
{GUCEF_TRACE;

    return stagingFolder + "/" + ManifestFileName;
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestJob::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "ingestId", ingestId );
    success = request.Serialize( *domRootNode.AddChild( "request", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "lastStep", CORE::CString( AssetIngestStepToString( lastStep ) ) ) && success;
    success = domRootNode.SetAttribute( "failedStep", CORE::CString( AssetIngestStepToString( failedStep ) ) ) && success;
    success = domRootNode.SetAttribute( "failureStatus", CORE::CString( AssetStoreStatusToString( failureStatus ) ) ) && success;
    success = domRootNode.SetAttribute( "failureMessage", failureMessage ) && success;
    success = domRootNode.SetAttribute( "locationId", locationId ) && success;
    success = domRootNode.SetAttribute( "stagingFolder", stagingFolder ) && success;
    success = domRootNode.SetAttribute( "contentRoot", contentRoot ) && success;
    success = domRootNode.SetAttribute( "contentPrefix", contentPrefix ) && success;
    success = domRootNode.SetAttribute( "inPlace", inPlace ) && success;
    success = domRootNode.SetAttribute( "contentInStaging", contentInStaging ) && success;
    success = domRootNode.SetValuesOfChildByName( "archives", archives ) && success;
    success = domRootNode.SetValuesOfChildByName( "problems", problems ) && success;
    success = domRootNode.SetAttribute( "startedAtMs", startedAtMs ) && success;
    success = pack.Serialize( *domRootNode.AddChild( "pack", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    CORE::CDataNode* assetsNode = domRootNode.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
    for ( size_t a=0; GUCEF_NULL != assetsNode && a<assets.size(); ++a )
        success = assets[ a ].Serialize( *assetsNode->AddChild( "asset", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    CORE::CDataNode* relationsNode = domRootNode.AddChild( "relations", GUCEF_DATATYPE_ARRAY );
    for ( size_t r=0; GUCEF_NULL != relationsNode && r<relations.size(); ++r )
        success = relations[ r ].Serialize( *relationsNode->AddChild( "relation", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestJob::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetIngestJob();
    ingestId = domRootNode.GetAttributeValueOrChildValueByName( "ingestId" ).AsString();
    const CORE::CDataNode* requestNode = domRootNode.FindChild( "request" );
    if ( GUCEF_NULL != requestNode )
        request.Deserialize( *requestNode, settings );
    lastStep = AssetIngestStepFromString( domRootNode.GetAttributeValueOrChildValueByName( "lastStep" ).AsString() );
    failedStep = AssetIngestStepFromString( domRootNode.GetAttributeValueOrChildValueByName( "failedStep" ).AsString() );
    failureStatus = AssetStoreStatusFromString( domRootNode.GetAttributeValueOrChildValueByName( "failureStatus" ).AsString( "ok" ) );
    failureMessage = domRootNode.GetAttributeValueOrChildValueByName( "failureMessage" ).AsString();
    locationId = domRootNode.GetAttributeValueOrChildValueByName( "locationId" ).AsString();
    stagingFolder = domRootNode.GetAttributeValueOrChildValueByName( "stagingFolder" ).AsString();
    contentRoot = domRootNode.GetAttributeValueOrChildValueByName( "contentRoot" ).AsString();
    contentPrefix = domRootNode.GetAttributeValueOrChildValueByName( "contentPrefix" ).AsString();
    inPlace = domRootNode.GetAttributeValueOrChildValueByName( "inPlace" ).AsBool( false, true );
    contentInStaging = domRootNode.GetAttributeValueOrChildValueByName( "contentInStaging" ).AsBool( false, true );
    domRootNode.GetValuesOfChildByName( "archives", archives );
    domRootNode.GetValuesOfChildByName( "problems", problems );
    startedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "startedAtMs" ).AsInt64( 0, true );
    const CORE::CDataNode* packNode = domRootNode.FindChild( "pack" );
    if ( GUCEF_NULL != packNode )
        pack.Deserialize( *packNode, settings );
    const CORE::CDataNode* assetsNode = domRootNode.FindChild( "assets" );
    if ( GUCEF_NULL != assetsNode )
    {
        CORE::CDataNode::const_iterator i = assetsNode->ConstBegin();
        while ( i != assetsNode->ConstEnd() )
        {
            CAssetRecord asset;
            if ( asset.Deserialize( *(*i), settings ) )
                assets.push_back( asset );
            ++i;
        }
    }
    const CORE::CDataNode* relationsNode = domRootNode.FindChild( "relations" );
    if ( GUCEF_NULL != relationsNode )
    {
        CORE::CDataNode::const_iterator i = relationsNode->ConstBegin();
        while ( i != relationsNode->ConstEnd() )
        {
            CAssetRelationRecord relation;
            if ( relation.Deserialize( *(*i), settings ) )
                relations.push_back( relation );
            ++i;
        }
    }
    return !ingestId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetIngestJob::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetIngestJob( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetIngestJob::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetIngestJob";
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
