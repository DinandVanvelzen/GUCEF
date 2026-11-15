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
#include "gucefASSETSTORE_CAssetStoreDownloadSettings.h"

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

const CORE::CString CAssetStoreDownloadSettings::DefaultThreadPoolName = "AssetStoreTransfers";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreDownloadSettings::CAssetStoreDownloadSettings( void )
    : CORE::CIDataNodeSerializable()
    , maxConcurrentJobs( 4 )
    , maxConcurrentJobsPerProvider( 2 )
    , maxAttempts( 5 )
    , retryBaseDelayInMs( 1000 )
    , retryMaxDelayInMs( 60000 )
    , progressIntervalInMs( 250 )
    , connectTimeoutInMs( 30000 )
    , maxBytesPerSecond( 0 )
    , queueFilePath()
    , threadPoolName( DefaultThreadPoolName )
    , autoSaveQueue( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadSettings::~CAssetStoreDownloadSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "maxConcurrentJobs", maxConcurrentJobs );
    success = domRootNode.SetAttribute( "maxConcurrentJobsPerProvider", maxConcurrentJobsPerProvider ) && success;
    success = domRootNode.SetAttribute( "maxAttempts", maxAttempts ) && success;
    success = domRootNode.SetAttribute( "retryBaseDelayInMs", retryBaseDelayInMs ) && success;
    success = domRootNode.SetAttribute( "retryMaxDelayInMs", retryMaxDelayInMs ) && success;
    success = domRootNode.SetAttribute( "progressIntervalInMs", progressIntervalInMs ) && success;
    success = domRootNode.SetAttribute( "connectTimeoutInMs", connectTimeoutInMs ) && success;
    success = domRootNode.SetAttribute( "maxBytesPerSecond", maxBytesPerSecond ) && success;
    success = domRootNode.SetAttribute( "queueFilePath", queueFilePath ) && success;
    success = domRootNode.SetAttribute( "threadPoolName", threadPoolName ) && success;
    success = domRootNode.SetAttribute( "autoSaveQueue", autoSaveQueue ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAssetStoreDownloadSettings defaults;
    maxConcurrentJobs = domRootNode.GetAttributeValueOrChildValueByName( "maxConcurrentJobs" ).AsUInt32( defaults.maxConcurrentJobs, true );
    maxConcurrentJobsPerProvider = domRootNode.GetAttributeValueOrChildValueByName( "maxConcurrentJobsPerProvider" ).AsUInt32( defaults.maxConcurrentJobsPerProvider, true );
    maxAttempts = domRootNode.GetAttributeValueOrChildValueByName( "maxAttempts" ).AsUInt32( defaults.maxAttempts, true );
    retryBaseDelayInMs = domRootNode.GetAttributeValueOrChildValueByName( "retryBaseDelayInMs" ).AsUInt32( defaults.retryBaseDelayInMs, true );
    retryMaxDelayInMs = domRootNode.GetAttributeValueOrChildValueByName( "retryMaxDelayInMs" ).AsUInt32( defaults.retryMaxDelayInMs, true );
    progressIntervalInMs = domRootNode.GetAttributeValueOrChildValueByName( "progressIntervalInMs" ).AsUInt32( defaults.progressIntervalInMs, true );
    connectTimeoutInMs = domRootNode.GetAttributeValueOrChildValueByName( "connectTimeoutInMs" ).AsUInt32( defaults.connectTimeoutInMs, true );
    maxBytesPerSecond = domRootNode.GetAttributeValueOrChildValueByName( "maxBytesPerSecond" ).AsUInt64( defaults.maxBytesPerSecond, true );
    queueFilePath = domRootNode.GetAttributeValueOrChildValueByName( "queueFilePath" ).AsString( defaults.queueFilePath, true );
    threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( defaults.threadPoolName, true );
    if ( threadPoolName.IsNULLOrEmpty() )
        threadPoolName = DefaultThreadPoolName;
    autoSaveQueue = domRootNode.GetAttributeValueOrChildValueByName( "autoSaveQueue" ).AsBool( defaults.autoSaveQueue, true );
    if ( 0 == maxAttempts )
        maxAttempts = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreDownloadSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreDownloadSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreDownloadSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreDownloadSettings";
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
