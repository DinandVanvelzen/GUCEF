/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#include "gucefASSETSTORE_CAssetStoreDownloadJobEventData.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADJOBEVENTDATA_H ? */
#include "TestDownloadEventRecorder.h"

using namespace GUCEF;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDownloadEventRecorder::CDownloadEventRecorder( void )
    : CORE::CObserver()
    , added( 0 )
    , stateChanges( 0 )
    , progress( 0 )
    , finished( 0 )
    , failed( 0 )
    , lastProgressBytes( 0 )
    , lastRate( 0 )
    , lastFailed()
    , states()
{
}

/*-------------------------------------------------------------------------*/

CDownloadEventRecorder::~CDownloadEventRecorder()
{
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDownloadEventRecorder::GetClassTypeName( void ) const
{
    static const CORE::CString classTypeName = "ASSETSTORETEST::CDownloadEventRecorder";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDownloadEventRecorder::Observe( ASSETSTORE::CAssetStoreDownloadManager& manager )
{
    TEventCallback onAdded( this, &CDownloadEventRecorder::OnJobAdded );
    TEventCallback onState( this, &CDownloadEventRecorder::OnJobStateChanged );
    TEventCallback onProgress( this, &CDownloadEventRecorder::OnJobProgress );
    TEventCallback onFinished( this, &CDownloadEventRecorder::OnJobFinished );
    TEventCallback onFailed( this, &CDownloadEventRecorder::OnJobFailed );
    SubscribeTo( &manager, ASSETSTORE::CAssetStoreDownloadManager::JobAddedEvent, onAdded );
    SubscribeTo( &manager, ASSETSTORE::CAssetStoreDownloadManager::JobStateChangedEvent, onState );
    SubscribeTo( &manager, ASSETSTORE::CAssetStoreDownloadManager::JobProgressEvent, onProgress );
    SubscribeTo( &manager, ASSETSTORE::CAssetStoreDownloadManager::JobFinishedEvent, onFinished );
    SubscribeTo( &manager, ASSETSTORE::CAssetStoreDownloadManager::JobFailedEvent, onFailed );
}

/*-------------------------------------------------------------------------*/

void
CDownloadEventRecorder::OnJobAdded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    ++added;
}

/*-------------------------------------------------------------------------*/

void
CDownloadEventRecorder::OnJobStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    ++stateChanges;
    const ASSETSTORE::CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL != data )
        states.push_back( ASSETSTORE::AssetDownloadJobStateToString( data->job.state ) );
}

/*-------------------------------------------------------------------------*/

void
CDownloadEventRecorder::OnJobProgress( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    ++progress;
    const ASSETSTORE::CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL != data )
    {
        lastProgressBytes = data->job.GetBytesDone();
        lastRate = data->bytesPerSecond;
    }
}

/*-------------------------------------------------------------------------*/

void
CDownloadEventRecorder::OnJobFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    ++finished;
}

/*-------------------------------------------------------------------------*/

void
CDownloadEventRecorder::OnJobFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{
    ++failed;
    const ASSETSTORE::CAssetStoreDownloadJobEventData* data = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetStoreDownloadJobEventData >( eventData );
    if ( GUCEF_NULL != data )
        lastFailed = data->job;
}

/*-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
