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

#ifndef GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#include "gucefASSETSTORE_CAssetStoreServiceConfig.h"
#define GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESERVICECONFIG_H ? */

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

const CORE::CString CAssetStoreServiceConfig::LocalImplementationId = "local";
const CORE::CString CAssetStoreServiceConfig::RemoteImplementationId = "remote";
const CORE::CString CAssetStoreServiceConfig::StateVfsRoot = "assetstore-state";
const CORE::CString CAssetStoreServiceConfig::ThumbnailsVfsRoot = "assetstore-thumbnails/thumbnails";
const CORE::CString CAssetStoreServiceConfig::QueueFileName = "downloads.json";
const CORE::CString CAssetStoreServiceConfig::ServiceStateFileName = "service-state.json";

/*-------------------------------------------------------------------------*/

CAssetStoreServiceConfig::CAssetStoreServiceConfig( void )
    : CORE::CIDataNodeSerializable()
    , implementationId( LocalImplementationId )
    , primaryStore()
    , downloads()
    , reprocess()
    , imageCacheFolder()
    , threadPoolName( "AssetStoreService" )
    , loadDownloadQueue( true )
    , startDownloads( true )
    , stateDirectory()
    , thumbnailsDirectory()
    , resumeIngestsOnStart( true )
    , ingestResumeAttempts( 3 )
    , downloadStagingKeepHours( 48 )
    , implementationSettings( "implementationSettings", GUCEF_DATATYPE_OBJECT )
    , pulseGenerator()
    , credentialStore( GUCEF_NULL )
    , converterHook( GUCEF_NULL )
    , renderHook( GUCEF_NULL )
    , signIn()
    , signInPromptSeconds( 600 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreServiceConfig::~CAssetStoreServiceConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceConfig::Serialize( CORE::CDataNode& domRootNode                        ,
                                     const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "implementationId", implementationId );
    success = domRootNode.SetAttribute( "imageCacheFolder", imageCacheFolder ) && success;
    success = domRootNode.SetAttribute( "threadPoolName", threadPoolName ) && success;
    success = domRootNode.SetAttribute( "loadDownloadQueue", loadDownloadQueue ) && success;
    success = domRootNode.SetAttribute( "startDownloads", startDownloads ) && success;
    success = domRootNode.SetAttribute( "stateDirectory", stateDirectory ) && success;
    success = domRootNode.SetAttribute( "thumbnailsDirectory", thumbnailsDirectory ) && success;
    success = domRootNode.SetAttribute( "resumeIngestsOnStart", resumeIngestsOnStart ) && success;
    success = domRootNode.SetAttribute( "ingestResumeAttempts", ingestResumeAttempts ) && success;
    success = domRootNode.SetAttribute( "downloadStagingKeepHours", downloadStagingKeepHours ) && success;
    success = domRootNode.SetAttribute( "signInPromptSeconds", signInPromptSeconds ) && success;
    CORE::CDataNode* store = domRootNode.AddChild( "primaryStore", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* downloadNode = domRootNode.AddChild( "downloads", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == store || GUCEF_NULL == downloadNode )
        return false;
    success = primaryStore.Serialize( *store, settings ) && success;
    success = downloads.Serialize( *downloadNode, settings ) && success;
    CORE::CDataNode* reprocessNode = domRootNode.AddChild( "reprocess", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != reprocessNode )
        success = reprocess.Serialize( *reprocessNode, settings ) && success;
    CORE::CDataNode* implementation = domRootNode.AddChild( implementationSettings );
    if ( GUCEF_NULL != implementation )
        implementation->SetName( "implementationSettings" );
    return success && GUCEF_NULL != implementation;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreServiceConfig::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    implementationId = domRootNode.GetAttributeValueOrChildValueByName( "implementationId" ).AsString( implementationId, true );
    imageCacheFolder = domRootNode.GetAttributeValueOrChildValueByName( "imageCacheFolder" ).AsString( imageCacheFolder, true );
    threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( threadPoolName, true );
    loadDownloadQueue = domRootNode.GetAttributeValueOrChildValueByName( "loadDownloadQueue" ).AsBool( loadDownloadQueue, true );
    startDownloads = domRootNode.GetAttributeValueOrChildValueByName( "startDownloads" ).AsBool( startDownloads, true );
    stateDirectory = domRootNode.GetAttributeValueOrChildValueByName( "stateDirectory" ).AsString( stateDirectory, true );
    thumbnailsDirectory = domRootNode.GetAttributeValueOrChildValueByName( "thumbnailsDirectory" ).AsString( thumbnailsDirectory, true );
    resumeIngestsOnStart = domRootNode.GetAttributeValueOrChildValueByName( "resumeIngestsOnStart" ).AsBool( resumeIngestsOnStart, true );
    ingestResumeAttempts = domRootNode.GetAttributeValueOrChildValueByName( "ingestResumeAttempts" ).AsUInt32( ingestResumeAttempts, true );
    downloadStagingKeepHours = domRootNode.GetAttributeValueOrChildValueByName( "downloadStagingKeepHours" ).AsUInt32( downloadStagingKeepHours, true );
    signInPromptSeconds = domRootNode.GetAttributeValueOrChildValueByName( "signInPromptSeconds" ).AsUInt32( signInPromptSeconds, true );
    bool success = true;
    const CORE::CDataNode* store = domRootNode.FindChild( "primaryStore" );
    if ( GUCEF_NULL != store )
        success = primaryStore.Deserialize( *store, settings ) && success;
    const CORE::CDataNode* downloadNode = domRootNode.FindChild( "downloads" );
    if ( GUCEF_NULL != downloadNode )
        success = downloads.Deserialize( *downloadNode, settings ) && success;
    const CORE::CDataNode* reprocessNode = domRootNode.FindChild( "reprocess" );
    if ( GUCEF_NULL != reprocessNode )
        success = reprocess.Deserialize( *reprocessNode, settings ) && success;
    const CORE::CDataNode* implementation = domRootNode.FindChild( "implementationSettings" );
    if ( GUCEF_NULL != implementation )
    {
        implementationSettings.Copy( *implementation );
        implementationSettings.SetName( "implementationSettings" );
    }
    return success;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreServiceConfig::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreServiceConfig( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreServiceConfig::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreServiceConfig::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreServiceConfig";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreServiceConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreServiceConfig";
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
