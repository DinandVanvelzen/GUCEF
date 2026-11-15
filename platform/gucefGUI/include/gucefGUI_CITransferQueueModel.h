/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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
#ifndef GUCEF_GUI_CITRANSFERQUEUEMODEL_H
#define GUCEF_GUI_CITRANSFERQUEUEMODEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The queue of transfers that a CTransferQueueForm shows: downloads, uploads, copies, background jobs of any kind that
 *  have a name, a size or a progress and a state. The application implements it on its own queue (a download manager, an
 *  ingest pipeline, a render farm); the form knows nothing of what a transfer is.
 *
 *  The model gives its entries with GetEntries() and tells that something changed with EntriesChangedEvent, which it raises on
 *  the thread the GUI runs on (a model whose queue lives on other threads hands the changes over and raises the event from a
 *  pulse of the GUI thread). The form reads the entries again at most once per refresh interval however often the event comes,
 *  so a model may raise it for every progress report.
 *
 *  What the user asks for with the buttons of an entry reaches the model through PerformAction(): the model carries the
 *  request out on its queue and answers whether it did. Which actions an entry offers follows from its state (IsActionAvailable()
 *  has the rules, a model overrides it for its own: a job that cannot be paused).
 */
class GUCEF_GUI_PUBLIC_CPP CITransferQueueModel : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent EntriesChangedEvent;      /**< no data: entries were added, removed or changed */

    static void RegisterEvents( void );

    /** Where a transfer is in its life */
    enum ETransferState
    {
        TRANSFERSTATE_QUEUED = 0    ,   /**< waiting for its turn */
        TRANSFERSTATE_RUNNING       ,
        TRANSFERSTATE_PAUSED        ,
        TRANSFERSTATE_FINISHING     ,   /**< the data is there, the transfer is checked or processed (verifying, indexing) */
        TRANSFERSTATE_DONE          ,
        TRANSFERSTATE_FAILED        ,
        TRANSFERSTATE_CANCELLED     ,

        TRANSFERSTATE_COUNT
    };

    /** What the user can ask of a transfer */
    enum ETransferAction
    {
        TRANSFERACTION_PAUSE = 0    ,
        TRANSFERACTION_RESUME       ,
        TRANSFERACTION_CANCEL       ,
        TRANSFERACTION_RETRY        ,
        TRANSFERACTION_REMOVE       ,   /**< takes a transfer that has ended out of the list */
        TRANSFERACTION_OPEN_FOLDER  ,

        TRANSFERACTION_COUNT
    };

    /** One transfer */
    struct GUCEF_GUI_PUBLIC_CPP SEntry
    {
        CString id;                     /**< stays with the transfer for as long as it is in the queue */
        CString name;                   /**< what the person knows it by */
        CString detail;                 /**< a second line of text: where it comes from, what it is for; may be empty */
        CString kind;                   /**< "Download", "Ingest", ...: shown with the detail; may be empty */
        UInt64 totalBytes;              /**< 0 when not known */
        UInt64 doneBytes;
        Float32 fraction;               /**< the progress from 0 to 1 for a transfer that is not counted in bytes; -1 to take it from the bytes */
        UInt64 bytesPerSecond;          /**< 0 when not known */
        UInt64 secondsLeft;             /**< 0 when not known */
        ETransferState state;
        CString stateText;              /**< the state in words of the model ("verifying", "indexing"); empty for the standard word of the state */
        CString message;                /**< why it failed, or a note; may be empty */
        CString folder;                 /**< where the result is; empty when there is no folder to open */

        SEntry( void );

        /** The progress from 0 to 1: the fraction when given, else the bytes done of the total (0 without a total, 1 for a transfer that is done) */
        Float32 GetProgress( void ) const;

        /** Whether the transfer is over: done, failed or cancelled */
        bool IsEnded( void ) const;
    };
    typedef struct SEntry TEntry;
    typedef GUCEF::vector< TEntry > TEntryVector;

    CITransferQueueModel( void );

    virtual ~CITransferQueueModel() GUCEF_VIRTUAL_OVERRIDE;

    /** Every transfer of the queue in the order it is shown in */
    virtual void GetEntries( TEntryVector& entries ) const = 0;

    /** Carries out what the user asked for; false when the model did not (the transfer is gone, the action does not apply). The default does nothing and says false */
    virtual bool PerformAction( const CString& entryId, ETransferAction action );

    /**
     *  Whether the entry offers the action: pause for a queued or running transfer, resume for a paused one, cancel for one that
     *  has not ended, retry for a failed or cancelled one, remove for one that has ended and open folder for one with a folder
     */
    virtual bool IsActionAvailable( const TEntry& entry, ETransferAction action ) const;

    /** The standard word of a state: "Queued", "Running", "Paused", "Finishing", "Done", "Failed", "Cancelled" */
    static CString StateToString( ETransferState state );

    /** The word of an action: "Pause", "Resume", "Cancel", "Retry", "Remove", "Open folder" */
    static CString ActionToString( ETransferAction action );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CITransferQueueModel( const CITransferQueueModel& src );
    CITransferQueueModel& operator=( const CITransferQueueModel& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CITRANSFERQUEUEMODEL_H ? */
