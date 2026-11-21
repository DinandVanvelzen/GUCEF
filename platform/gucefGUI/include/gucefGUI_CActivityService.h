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

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#define GUCEF_GUI_CACTIVITYSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CACTIVITYINFO_H
#include "gucefGUI_CActivityInfo.h"
#define GUCEF_GUI_CACTIVITYINFO_H
#endif /* GUCEF_GUI_CACTIVITYINFO_H ? */

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

class CGUIContext;

/**
 *  The one place that decides when the user is told that something takes time (the rule: no operation that takes longer than
 *  300 ms is without feedback). An operation announces itself with Begin(): a label, an optional expected duration,
 *  whether it reports progress as a fraction, and whether the user may cancel it. It ends with End(). The service shows the
 *  operation to the user when it has run for the threshold (300 ms by default), or at once (the next Update()) when the
 *  expected duration it announced is longer than the threshold. An operation that ends before that is never shown, so
 *  fast operations cause no flicker.
 *
 *  Feedback comes in layers, all driven from here:
 *    - the busy cursor: while an operation is shown the system cursor of the attached context is CURSORSHAPE_PROGRESS, as an
 *      override of the shape of the context (CGUIContext::SetSystemCursorShapeOverride()), which a driver sets itself every frame;
 *    - the indicator: CActivityIndicatorForm shows the newest operation that is shown, with its progress, the count of the
 *      others and a cancel button; an application puts it in its status area;
 *    - the jobs list: OperationShownEvent, OperationChangedEvent and OperationEndedEvent, and GetShownOperations(), let an
 *      application list the operations that are shown (CActivityInfo), as the jobs overview of a service does.
 *
 *  Threads: Begin(), SetProgress(), SetLabel(), End(), RequestCancel(), IsCancelRequested() and the getters are safe from
 *  any thread: they work on a table behind a lock, on the calling thread, and notifications wait in a queue. Update() runs
 *  on the thread of the interface (an attached context calls it at the start of every frame): it moves operations over the
 *  threshold to shown, and sends the queued notifications, so all the events come from that thread. Nothing here waits for
 *  or pumps another thread. A worker polls IsCancelRequested(), or the owner of the operation subscribes to
 *  CancelRequestedEvent (TCloneableUInt32: the id of the operation) and stops it.
 *
 *  Time: the clock is the precision timer in milliseconds; a test replaces it with SetClock() to run on a clock it moves.
 *
 *  Watchdog: in a debug build, a frame of the attached context that took longer than the stall threshold (100 ms by
 *  default) is reported with the labels of the operations that were running, as a warning in the log, and counted
 *  (GetStallCount()): the interface thread was blocked by whatever the operation was doing. A release build does none of this.
 */
class GUCEF_GUI_PUBLIC_CPP CActivityService : public CORE::CObservingNotifier
{
    public:

    /** Notified when an operation is shown to the user; TCloneableUInt32: the id of the operation */
    static const CORE::CEvent OperationShownEvent;

    /** The label, progress or cancel state of a shown operation changed; TCloneableUInt32: the id */
    static const CORE::CEvent OperationChangedEvent;

    /** A shown operation ended; TCloneableUInt32: the id. Not notified for an operation that was never shown */
    static const CORE::CEvent OperationEndedEvent;

    /** The user asked to cancel an operation (RequestCancel()); TCloneableUInt32: the id. The operation stops and ends */
    static const CORE::CEvent CancelRequestedEvent;

    /** The number of operations that are shown changed, or one of them changed: what an indicator redraws on */
    static const CORE::CEvent ActivityChangedEvent;

    static void RegisterEvents( void );

    enum EDefaults
    {
        DefaultThresholdInMs = 300      ,   /**< the longest an operation runs without the user seeing feedback */
        DefaultStallThresholdInMs = 100     /**< a frame longer than this is a stall of the interface thread */
    };

    /** A clock in milliseconds; the default is the precision timer */
    typedef UInt64 ( *TClockFunction )( void* userData );

    typedef GUCEF::vector< CActivityInfo > TActivityInfoVector;

    CActivityService( void );

    virtual ~CActivityService();

    /*
     *  Settings
     */

    /** How long an operation runs before it is shown, in milliseconds. 0 shows every operation at the next Update() */
    void SetThresholdInMs( UInt32 thresholdInMs );

    UInt32 GetThresholdInMs( void ) const;

    /** Replaces the clock, for tests; NULL goes back to the precision timer */
    void SetClock( TClockFunction clock, void* userData );

    /** The busy cursor over the attached context, on by default */
    void SetBusyCursorEnabled( bool enabled );

    bool IsBusyCursorEnabled( void ) const;

    /** Whether the busy cursor is set now */
    bool IsBusyCursorShown( void ) const;

    /** The frame time (milliseconds) above which a frame is reported as a stall, debug builds only */
    void SetStallThresholdInMs( UInt32 thresholdInMs );

    UInt32 GetStallThresholdInMs( void ) const;

    /** The number of stalls seen since the start; always 0 in a release build */
    UInt32 GetStallCount( void ) const;

    /*
     *  The interface thread
     */

    /**
     *  Takes the context under watch: its frames drive Update() (FrameStartedEvent), its system cursor is the busy cursor,
     *  and the watchdog measures its frames. One context at a time; the context must outlive the attachment or be destroyed
     *  while attached (the service notices)
     */
    bool Attach( CGUIContext* context );

    void Detach( void );

    CGUIContext* GetContext( void ) const;

    /** Moves operations over the threshold to shown, sends the queued notifications and sets the cursor; the thread of the interface only */
    void Update( void );

    /*
     *  Operations, any thread
     */

    /**
     *  An operation starts. Answers its id (never 0) for the other calls. A determinate operation says its progress with
     *  SetProgress(); an indeterminate one only that it is busy. The expected duration is in milliseconds, 0 when unknown
     */
    UInt32 Begin( const CORE::CString& label, UInt32 expectedDurationInMs = 0, bool determinate = false, bool cancellable = false );

    /** The fraction done, between 0 and 1; ignored for an operation that is not determinate */
    void SetProgress( UInt32 id, Float32 fraction );

    void SetLabel( UInt32 id, const CORE::CString& label );

    /** The operation is over, completed, failed or cancelled */
    void End( UInt32 id );

    /** Asks the operation to stop: CancelRequestedEvent follows from Update(); false when it is not there or cannot be cancelled */
    bool RequestCancel( UInt32 id );

    bool IsCancelRequested( UInt32 id ) const;

    /** The operation is there (begun and not ended) */
    bool IsActive( UInt32 id ) const;

    /*
     *  What is going on
     */

    /** The number of operations that are shown now */
    UInt32 GetShownCount( void ) const;

    /** The number of operations that are going on, shown or not */
    UInt32 GetActiveCount( void ) const;

    /** The shown operations, the one that was shown first at the start */
    void GetShownOperations( TActivityInfoVector& operations ) const;

    /** One operation; false when it is not there */
    bool GetOperation( UInt32 id, CActivityInfo& info ) const;

    protected:

    /** The destruction of the attached context: nothing is put back on it (the standard events reach only this) */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CActivityService > TEventCallback;

    enum ENoticeType
    {
        NOTICE_SHOWN = 0    ,
        NOTICE_CHANGED      ,
        NOTICE_ENDED        ,
        NOTICE_CANCEL
    };

    struct SNotice
    {
        ENoticeType type;
        UInt32 id;
    };
    typedef GUCEF::vector< SNotice > TNoticeVector;

    struct SRecord
    {
        CActivityInfo info;
        bool ended;
        bool changedQueued;
        SRecord( void );
    };
    typedef GUCEF::vector< SRecord > TRecordVector;

    UInt64 Now( void ) const;

    SRecord* Find( UInt32 id );

    const SRecord* Find( UInt32 id ) const;

    void QueueNotice( ENoticeType type, UInt32 id );

    void QueueChanged( SRecord& record );

    void UpdateCursor( void );

    void OnFrameStarted( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    CActivityService( const CActivityService& src );
    CActivityService& operator=( const CActivityService& src );

    private:

    mutable MT::CMutex m_lock;
    TRecordVector m_records;
    TNoticeVector m_notices;
    UInt32 m_nextId;
    UInt32 m_thresholdInMs;
    TClockFunction m_clock;
    void* m_clockData;
    CGUIContext* m_context;
    bool m_busyCursorEnabled;
    bool m_cursorSet;
    UInt32 m_lastShownCount;
    UInt32 m_stallThresholdInMs;
    UInt32 m_stallCount;
    bool m_inUpdate;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */
