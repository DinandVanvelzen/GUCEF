/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHALERTROUTER_H
#define GUCEF_MULTIPLAY_CMATCHALERTROUTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHBOOK_H
#include "gucefMULTIPLAY_CMatchBook.h"
#define GUCEF_MULTIPLAY_CMATCHBOOK_H
#endif /* GUCEF_MULTIPLAY_CMATCHBOOK_H ? */

#ifndef GUCEF_MULTIPLAY_CIMATCHALERTSINK_H
#include "gucefMULTIPLAY_CIMatchAlertSink.h"
#define GUCEF_MULTIPLAY_CIMATCHALERTSINK_H
#endif /* GUCEF_MULTIPLAY_CIMATCHALERTSINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How often the player wants to be interrupted */
enum EMatchAlertMode
{
    MATCHALERTMODE_EVERY_TURN        = 0 ,    /**< for every turn that arrives */
    MATCHALERTMODE_ONCE_UNTIL_I_PLAY = 1 ,    /**< once per match, until the player has played in it */
    MATCHALERTMODE_DAILY_DIGEST      = 2      /**< one summary a day at the digest time */
};
typedef enum EMatchAlertMode TMatchAlertMode;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decides what reaches the match alert sinks. It listens to a match book (YourTurnEvent, InviteReceivedEvent,
 *  MatchFinishedEvent and the changes of the book) and to its own clock (Update(), driven by a timer or by the caller).
 *
 *  - dedup: a match and revision is notified once.
 *  - mute: nothing for a muted match; with onlyPinned nothing for a match that is not pinned.
 *  - sinks that do not interrupt (the toast and counter in the application) get every match alert that passes the
 *    two rules above at once; sinks that interrupt get what follows.
 *  - mode: every turn, once per match until the player plays in it, or a daily digest at the digest time.
 *  - quiet hours: nothing interrupts between the start and the end minute of the day (the window may cross midnight);
 *    what waited is delivered when it ends.
 *  - coalescing: match alerts for several matches that wait at the same time are delivered as one COALESCED
 *    match alert with the count ("3 games are waiting for you"); the first one waits coalesceWindowMs for company.
 *  - minimum interval: at least minIntervalMs between two deliveries to the interrupting sinks.
 *  - clearing: opening a match, playing it anywhere or removing it takes its match alerts back from all sinks.
 *
 *  Config: mode (EVERY_TURN, ONCE_UNTIL_I_PLAY, DAILY_DIGEST), quietHoursEnabled, quietStartMinute, quietEndMinute,
 *  minIntervalMs, coalesceWindowMs, digestMinute, onlyPinned. The sinks and the book are not owned.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchAlertRouter : public CORE::CObservingNotifier ,
                                                       public CORE::CIConfigurable
{
    public:

    CMatchAlertRouter( void );

    virtual ~CMatchAlertRouter();

    /** The book to listen to, NULL to stop listening */
    void SetBook( CMatchBook* book );

    void AddSink( CIMatchAlertSink* sink );

    void RemoveSink( CIMatchAlertSink* sink );

    /** The clock the router uses, which has to outlive it; NULL for the clock of the system */
    void SetClock( const CIMatchClock* clock );

    void SetMode( TMatchAlertMode mode );

    TMatchAlertMode GetMode( void ) const;

    /** Quiet hours from start (inclusive) to end (exclusive), as minutes of the day 0 to 1439; start equal to end is no quiet time */
    void SetQuietHours( bool enabled, Int32 startMinute, Int32 endMinute );

    void SetMinimumIntervalMs( UInt32 intervalMs );

    void SetCoalesceWindowMs( UInt32 windowMs );

    /** The minute of the day the daily digest goes out */
    void SetDigestMinute( Int32 minuteOfDay );

    void SetOnlyPinned( bool onlyPinned );

    /** What the timer does: delivers what was held back when the time has come */
    void Update( void );

    /** Takes the match alerts of the match back from all sinks and out of the queues */
    void ClearForMatch( const CORE::CString& matchId );

    /** The number of match alerts that wait for quiet hours, the minimum interval or the coalesce window */
    UInt32 GetHeldCount( void ) const;

    /** Deliveries made to the sinks that interrupt */
    UInt32 GetInterruptingDeliveryCount( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Only the destruction of the book is handled here, every other event has its own handler */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMatchAlertRouter > TEventCallback;
    typedef GUCEF::vector< CIMatchAlertSink* > TSinkVector;
    typedef GUCEF::map< CORE::CString, UInt32 > TRevisionMap;

    CMatchAlertRouter( const CMatchAlertRouter& src );
    CMatchAlertRouter& operator=( const CMatchAlertRouter& src );

    Int64 GetNowMs( void ) const;
    Int32 GetMinuteOfDay( void ) const;
    bool IsQuietNow( void ) const;
    bool MakeMatchAlert( const CORE::CString& matchId, TMatchAlertKind kind, UInt32 revision, CMatchAlert& alert, bool& muted, bool& pinned );
    void DeliverToPassiveSinks( const CMatchAlert& alert );
    void DeliverToInterruptingSinks( const CMatchAlert& alert );
    void Hold( const CMatchAlert& alert );
    void Flush( void );
    void SendDigest( void );
    void ForgetMatch( const CORE::CString& matchId, bool alsoWaiting );

    void OnYourTurn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnInvite( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFinished( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnBookChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimerUpdate( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CMatchBook* m_book;
    TSinkVector m_sinks;
    const CIMatchClock* m_clock;
    CORE::CTimer m_timer;
    TMatchAlertMode m_mode;
    bool m_quietEnabled;
    Int32 m_quietStart;
    Int32 m_quietEnd;
    UInt32 m_minIntervalMs;
    UInt32 m_coalesceWindowMs;
    Int32 m_digestMinute;
    bool m_onlyPinned;
    TRevisionMap m_notifiedRevisions;           /**< the newest revision per match that was handled: dedup */
    CORE::CString::StringSet m_waiting;         /**< matches the player was told about and has not played since (ONCE_UNTIL_I_PLAY) */
    TMatchAlertVector m_held;                 /**< for the interrupting sinks, one per match */
    Int64 m_firstHeldAt;
    Int64 m_lastDeliveryAt;
    bool m_hasDelivered;
    bool m_digestArmed;
    bool m_digestArmKnown;
    UInt32 m_interruptingDeliveries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHALERTROUTER_H ? */
