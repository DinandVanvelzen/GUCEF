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

#ifndef TESTDOWNLOADEVENTRECORDER_H
#define TESTDOWNLOADEVENTRECORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts the events of a download manager, one handler per event */
class CDownloadEventRecorder : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::UInt32 added;
    GUCEF::UInt32 stateChanges;
    GUCEF::UInt32 progress;
    GUCEF::UInt32 finished;
    GUCEF::UInt32 failed;
    GUCEF::UInt64 lastProgressBytes;
    GUCEF::UInt64 lastRate;
    GUCEF::ASSETSTORE::CAssetStoreDownloadJob lastFailed;
    GUCEF::CORE::CString::StringVector states;

    CDownloadEventRecorder( void );
    virtual ~CDownloadEventRecorder();

    void Observe( GUCEF::ASSETSTORE::CAssetStoreDownloadManager& manager );

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::CORE::CTEventHandlerFunctor< CDownloadEventRecorder > TEventCallback;

    void OnJobAdded( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void OnJobStateChanged( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void OnJobProgress( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void OnJobFinished( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
    void OnJobFailed( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData );
};

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTDOWNLOADEVENTRECORDER_H ? */
