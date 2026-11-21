/*
 *  AssetWorkbenchLib: the remote transfers of the workbench with their feedback
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

#ifndef ASSETWORKBENCHLIB_CREMOTEFETCHJOBS_H
#define ASSETWORKBENCHLIB_CREMOTEFETCHJOBS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#include "gucefGUI_CActivityService.h"
#define GUCEF_GUI_CACTIVITYSERVICE_H
#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H
#include "gucefASSETSTORE_CRemoteFileTransfer.h"
#define GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H
#endif /* GUCEF_ASSETSTORE_CREMOTEFILETRANSFER_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#include "AssetWorkbenchLib_CWorkbenchJobRegistry.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBREGISTRY_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The transfers of files of a remote asset store service that the workbench runs (the files of an asset that is opened, a pack that is
 *  downloaded to a folder, a pack that is kept offline), with the feedback the workbench gives for long work: each one runs as a task
 *  of a thread pool (ASSETSTORE::CRemoteAssetFileSystem::StartTransfer()), is an operation of the activity service (the indicator shows
 *  it after 300 ms, with its progress and a cancel button) and a job of the registry of the workbench, so it is in the jobs list and in
 *  the transfer queue with its progress and a cancel action. Update() is called on the thread of the interface; it carries the progress
 *  over, ends the operation and the job when the transfer ended and raises JobEndedEvent.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CRemoteFetchJobs : public CORE::CObservingNotifier
{
    public:

    /** What a job ended with */
    struct ASSETWORKBENCHLIB_PUBLIC_CPP SEnded
    {
        UInt32 id;
        CORE::CString kind;                 /**< "open", "download" or "offline" */
        CORE::CString name;
        CORE::CString openPath;             /**< what was to be opened when it was done, empty for none */
        ASSETSTORE::TAssetStoreStatus status;
        bool cancelled;
        CORE::CString message;              /**< how it ended, in words */
        UInt64 bytes;                       /**< the bytes it moved */
        SEnded( void );
    };
    typedef CORE::CTCloneableObj< SEnded > TEndedEventData;

    static const CORE::CEvent JobEndedEvent;        /**< TEndedEventData */

    static void RegisterEvents( void );

    /** Both may be NULL (no feedback); not owned */
    CRemoteFetchJobs( GUI::CActivityService* activity, CWorkbenchJobRegistry* registry );

    virtual ~CRemoteFetchJobs() GUCEF_VIRTUAL_OVERRIDE;

    /** The file system the transfers run on (not owned); another one cancels what runs */
    void SetFileSystem( ASSETSTORE::CRemoteAssetFileSystem* system );

    /**
     *  Starts a transfer. 'kind' is what the jobs list says ("open", "download", "offline"), 'label' what the indicator shows ("Fetching
     *  Crate.fbx"), 'openPath' the path to open when it is done. The id of the job, 0 when it could not be started
     */
    UInt32 Start( const ASSETSTORE::TRemoteFileTransferPtr& transfer, const CORE::CString& kind, const CORE::CString& label, const CORE::CString& openPath, UInt32 expectedDurationInMs = 0 );

    /** Carries the progress of the transfers over, and ends the ones that ended; call it between two frames */
    void Update( void );

    bool Cancel( UInt32 id );
    void CancelAll( void );

    UInt32 GetActiveCount( void ) const;

    /** The progress of a job in 0 to 1000, -1 for a job that is not there */
    Int32 GetProgress( UInt32 id ) const;

    /** The transfer of a job, a null pointer for a job that is not there */
    ASSETSTORE::TRemoteFileTransferPtr GetTransfer( UInt32 id ) const;

    /** The outcome of the job that ended last, for tests and the console */
    const SEnded& GetLastEnded( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteFetchJobs > TEventCallback;

    struct SJob
    {
        UInt32 id;
        ASSETSTORE::TRemoteFileTransferPtr transfer;
        CORE::CFutureResult future;
        CORE::CString kind;
        CORE::CString name;
        CORE::CString openPath;
        UInt32 activityId;
        UInt32 jobNumber;
        SJob( void );
    };
    typedef GUCEF::vector< SJob > TJobVector;

    CRemoteFetchJobs( const CRemoteFetchJobs& src );
    CRemoteFetchJobs& operator=( const CRemoteFetchJobs& src );

    void OnCancelRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    GUI::CActivityService* m_activity;
    CWorkbenchJobRegistry* m_registry;
    ASSETSTORE::CRemoteAssetFileSystem* m_system;
    TJobVector m_jobs;
    UInt32 m_nextId;
    SEnded m_lastEnded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CREMOTEFETCHJOBS_H ? */
