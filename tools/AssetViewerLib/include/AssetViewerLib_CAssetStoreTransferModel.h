/*
 *  AssetViewerLib: the asset sources of the viewer: online stores, the primary store and the transfers
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

#ifndef ASSETVIEWERLIB_CASSETSTORETRANSFERMODEL_H
#define ASSETVIEWERLIB_CASSETSTORETRANSFERMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CITRANSFERQUEUEMODEL_H
#include "gucefGUI_CITransferQueueModel.h"
#define GUCEF_GUI_CITRANSFERQUEUEMODEL_H
#endif /* GUCEF_GUI_CITRANSFERQUEUEMODEL_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The transfers of the viewer for a CTransferQueueForm: the jobs of the asset store service (ASSETSTORE::CIAssetStoreService),
 *  in the order of the service: the downloads in the order of the queue, then the ingests, the conversions and the jobs of the
 *  converter pipeline in the order they started. The actions of the form are asked of the service (ControlJob()): pause,
 *  resume, cancel, retry and remove; the service says per job which of them apply now. The events of the service
 *  (JobChangedEvent, JobRemovedEvent) raise EntriesChangedEvent. The service is not owned and has to outlive the model.
 *
 *  With a service in this process the entries are read from it whenever the form asks, as the parts of the store answer at
 *  once. With a service that is not in process the model mirrors the list: it is read in the background at the start and
 *  after each event, the form gets the mirror, and an action is sent and answered by the events that follow (section 5.6 of
 *  the service design).
 *
 *  StartConversion() asks the service to convert a pack as a job, StartPendingConversion() to convert every pack that is
 *  "conversion pending"; their progress (the assets done of the total) is that of their job.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CAssetStoreTransferModel : public GUI::CITransferQueueModel
{
    public:

    typedef CORE::CTEventHandlerFunctor< CAssetStoreTransferModel > TEventCallback;

    static const CORE::CString IngestIdPrefix;      /**< "ingest:": the ids of the ingest jobs of the service start with it */
    static const CORE::CString ConversionIdPrefix;  /**< "convert:": the ids of the conversion jobs of the service start with it */

    explicit CAssetStoreTransferModel( ASSETSTORE::CIAssetStoreService& service );

    virtual ~CAssetStoreTransferModel() GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetEntries( TEntryVector& entries ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool PerformAction( const CORE::CString& entryId, ETransferAction action ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActionAvailable( const TEntry& entry, ETransferAction action ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Takes the answers of the background reads of a service that is not in process; call it between two frames */
    void Update( void );

    /** The number of ingests the model knows of, ended or not */
    UInt32 GetIngestCount( void ) const;

    /** Converts a pack as a job of the service and puts it in the list; false when the service refuses */
    bool StartConversion( const CORE::CString& packId );

    /** Converts the packs that are "conversion pending", one after the other, as one job of the service */
    bool StartPendingConversion( void );

    /** The number of conversions the model knows of, ended or not */
    UInt32 GetConversionCount( void ) const;

    /** The entry of a job of the service as the form shows it */
    static void ToEntry( const ASSETSTORE::CAssetJobEntry& job, TEntry& entry );

    /** The state of a job as the form shows it */
    static ETransferState ToTransferState( ASSETSTORE::TAssetJobState state );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void OnJobChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnJobRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ReadJobs( void ) const;
    void RequestJobs( void );
    UInt32 CountJobs( const CORE::CString& kind ) const;

    CAssetStoreTransferModel( const CAssetStoreTransferModel& src );
    CAssetStoreTransferModel& operator=( const CAssetStoreTransferModel& src );

    private:

    ASSETSTORE::CIAssetStoreService& m_service;
    mutable ASSETSTORE::TAssetJobEntryVector m_jobs;
    CORE::CFutureResult m_jobsFuture;
    bool m_readAgain;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CASSETSTORETRANSFERMODEL_H ? */
