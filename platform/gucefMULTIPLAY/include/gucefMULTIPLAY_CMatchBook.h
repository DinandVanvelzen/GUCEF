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

#ifndef GUCEF_MULTIPLAY_CMATCHBOOK_H
#define GUCEF_MULTIPLAY_CMATCHBOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_MULTIPLAY_CIMATCHPROVIDER_H
#include "gucefMULTIPLAY_CIMatchProvider.h"
#define GUCEF_MULTIPLAY_CIMATCHPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIMATCHPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHENTRY_H
#include "gucefMULTIPLAY_CMatchEntry.h"
#define GUCEF_MULTIPLAY_CMATCHENTRY_H
#endif /* GUCEF_MULTIPLAY_CMATCHENTRY_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHSTORE_H
#include "gucefMULTIPLAY_CMatchStore.h"
#define GUCEF_MULTIPLAY_CMATCHSTORE_H
#endif /* GUCEF_MULTIPLAY_CMATCHSTORE_H ? */

#ifndef GUCEF_MULTIPLAY_CIMATCHCLOCK_H
#include "gucefMULTIPLAY_CIMatchClock.h"
#define GUCEF_MULTIPLAY_CIMATCHCLOCK_H
#endif /* GUCEF_MULTIPLAY_CIMATCHCLOCK_H ? */

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

enum EMatchBookChangeKind
{
    MATCHBOOKCHANGE_ADDED   = 0 ,
    MATCHBOOKCHANGE_UPDATED = 1 ,   /**< a newer state arrived */
    MATCHBOOKCHANGE_REMOVED = 2 ,
    MATCHBOOKCHANGE_OPENED  = 3 ,   /**< the player opened the match: match alerts for it are cleared */
    MATCHBOOKCHANGE_FLAGS   = 4     /**< seen, muted, pinned or archived changed */
};
typedef enum EMatchBookChangeKind TMatchBookChangeKind;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of MatchBookChangedEvent */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchBookChange : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    TMatchBookChangeKind kind;

    CMatchBookChange( void );

    CMatchBookChange( const CORE::CString& id, TMatchBookChangeKind changeKind );

    CMatchBookChange( const CMatchBookChange& src );

    virtual ~CMatchBookChange();

    CMatchBookChange& operator=( const CMatchBookChange& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** The data of YourTurnEvent: the match and the revision that made it the player's turn */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchTurnEventData : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    UInt32 revision;

    CMatchTurnEventData( void );

    CMatchTurnEventData( const CORE::CString& id, UInt32 turnRevision );

    CMatchTurnEventData( const CMatchTurnEventData& src );

    virtual ~CMatchTurnEventData();

    CMatchTurnEventData& operator=( const CMatchTurnEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The local library of matches of a player: owns the providers, merges what they tell (the previews) into one entry
 *  per match, groups the entries for a user interface, keeps them on disk and tells when something changed.
 *
 *  Merging: a preview replaces the one of an entry only when it has a higher revision (a lower one is old, an equal
 *  one with another package loses against the first that was accepted). The providers are responsible for
 *  checking what they hand over (signatures, the chain); the book trusts a provider for the matches it carries.
 *
 *  Your turn: the player's turn is when the match is ACTIVE, the preview names the local user id, and the local
 *  participant (when the info is known) takes part. yourTurnSince is the time the turn arrived: the issuer's time
 *  (preview lastActivity) when the provider has a trusted clock (MATCHCAPABILITY_SERVER_CLOCK), otherwise the time
 *  this machine learned of it. YourTurnEvent is raised once per match and revision: the revision is kept in the
 *  entry (notifiedRevision), so restarting does not raise it again.
 *
 *  Reconciling: the previews of all providers are asked for at Start(), on Resume(), on RefreshAll() (also for a drop
 *  of a push channel) and on a slow interval; the match that is open is refreshed on a fast one. In the background
 *  (SetBackground()) the timers do nothing. Update() is what the timer does, it can be called with a clock of the
 *  caller's choosing (SetClock()).
 *
 *  Config: storeDirectory, storeCodec (default "json"), fastIntervalMs (default 15000), slowIntervalMs (default 300000),
 *  tickMs (default 1000), and any number of "Provider" children: attribute type (the name in the provider factory) and a
 *  "Config" child for the provider. The book is used from the thread that pulses its notifiers.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchBook : public CORE::CObservingNotifier ,
                                              public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent MatchBookChangedEvent;    /**< TMatchBookChangedEventData */
    static const CORE::CEvent YourTurnEvent;            /**< TYourTurnEventData: once per match and revision */
    static const CORE::CEvent MatchFinishedEvent;       /**< TMatchIdEventData: the match id of a match that ended */
    static const CORE::CEvent InviteReceivedEvent;      /**< TMatchIdEventData: the match id of a match the player is invited to */
    static const CORE::CEvent ProviderProblemEvent;     /**< TProviderProblemEventData: a provider is offline or reports an error */

    typedef CMatchBookChange                TMatchBookChangedEventData;
    typedef CMatchTurnEventData             TYourTurnEventData;
    typedef CORE::TCloneableString          TMatchIdEventData;
    typedef CMatchProviderState             TProviderProblemEventData;
    typedef GUCEF::vector< CIMatchProviderPtr > TProviderVector;

    static const UInt32 DefaultFastIntervalMs = 15000;
    static const UInt32 DefaultSlowIntervalMs = 300000;
    static const UInt32 MaxLocalTitleBytes = 64;

    static void RegisterEvents( void );

    CMatchBook( void );

    virtual ~CMatchBook();

    /** The user id of the player: whose turn is the player's turn */
    void SetLocalUserId( const CORE::CString& userId );

    const CORE::CString& GetLocalUserId( void ) const;

    /** The clock the book uses, which has to outlive the book; NULL for the clock of the system */
    void SetClock( const CIMatchClock* clock );

    /** Where the entries are kept; an empty directory keeps nothing on disk */
    void SetStoreDirectory( const CORE::CString& directory );

    CMatchStore& GetStore( void );

    /** Reads the entries from the store, MatchBookChangedEvent (added) is raised for each; false when there is no store */
    bool LoadFromStore( void );

    bool AddProvider( CIMatchProviderPtr provider );

    /** Makes a provider with the provider factory of CMultiplayGlobal and the config, then adds it */
    bool AddProvider( const CORE::CString& providerType, const CORE::CDataNode* config );

    const TProviderVector& GetProviders( void ) const;

    CIMatchProviderPtr FindProvider( const CORE::CString& providerName ) const;

    /** The provider that carries the match, a NULL pointer when the match is not known */
    CIMatchProviderPtr GetProviderOfMatch( const CORE::CString& matchId ) const;

    /** Starts the providers and the timer and asks for every preview */
    bool Start( void );

    void Stop( void );

    bool IsStarted( void ) const;

    /** Asks every provider for the previews of all matches */
    bool RefreshAll( void );

    bool Refresh( const CORE::CString& matchId );

    /** In the background no reconcile timer runs */
    void SetBackground( bool background );

    bool IsBackground( void ) const;

    /** Leaves the background and reconciles at once */
    void Resume( void );

    /** What the timer does: reconciles when an interval has passed (fast for the open match, slow for the rest) */
    void Update( void );

    void SetIntervals( UInt32 fastIntervalMs, UInt32 slowIntervalMs );

    /** The player opens a match: it is the open match, it is seen, and match alerts for it can be cleared */
    bool OpenMatch( const CORE::CString& matchId );

    void CloseMatch( void );

    const CORE::CString& GetOpenMatchId( void ) const;

    bool MarkSeen( const CORE::CString& matchId );

    bool SetMuted( const CORE::CString& matchId, bool muted );

    bool SetPinned( const CORE::CString& matchId, bool pinned );

    bool SetArchived( const CORE::CString& matchId, bool archived );

    /**
     *  Gives the match the player's own name, shown in the lists instead of the shared title and never shared. Control
     *  characters and the blanks around the name are dropped and the name is cut to MaxLocalTitleBytes (whole characters); an
     *  empty name takes the player's name away.
     *
     *  @return false when the match is not known
     */
    bool SetLocalTitle( const CORE::CString& matchId, const CORE::CString& title );

    /** Forgets a match here: its entry and its record on disk */
    bool RemoveMatch( const CORE::CString& matchId );

    bool GetEntry( const CORE::CString& matchId, CMatchEntry& entry ) const;

    UInt32 GetEntryCount( void ) const;

    void GetEntries( TMatchEntryVector& entries ) const;

    /** The group the entry is in for the local player */
    TMatchGroup GetGroupOf( const CMatchEntry& entry ) const;

    /**
     *  The state of the entry for the local player in one word, for the label of a row: forming (the match has not begun), invited, cancelled,
     *  incompatible (the player watches because the game cannot be played here), spectating, your turn, their turn, finished, abandoned.
     *  A cancelled match is in the finished group, a spectator's match in their turn
     */
    TMatchEntryState GetStateOf( const CMatchEntry& entry ) const;

    /** The entries of a group, pinned first, then by deadline (the nearest first, none last), then the latest activity first */
    void GetGroup( TMatchGroup group, TMatchEntryVector& entries, bool includeArchived = false ) const;

    UInt32 GetYourTurnCount( void ) const;

    /**
     *  The match auto advance goes to: the one in YOUR_TURN that has been ready the longest (the oldest yourTurnSince),
     *  not counting exceptMatchId (the match the player just finished)
     *
     *  @return false when no other match waits for the player
     */
    bool GetNextYourTurn( const CORE::CString& exceptMatchId, CORE::CString& matchId ) const;

    /**
     *  Hands over a turn through the provider of the match, expecting the match to be at the revision the book knows. The
     *  answer is merged into the entry (the preview that was handed over when the answer is OK, the current preview
     *  when it is STALE)
     */
    bool Submit( const CORE::CString& matchId, const CMatchPreview& preview, const CMatchPackage& package, CORE::CFutureResult& future );

    bool Fetch( const CORE::CString& matchId, UInt32 revision, CORE::CFutureResult& future );

    /** Starts a match with the provider of the given name */
    bool CreateMatch( const CORE::CString& providerName, const CMatchRecord& record, const CMatchPackage& firstPackage, const CMatchPreview& firstPreview, CORE::CFutureResult& future );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMatchBook > TEventCallback;
    typedef GUCEF::map< CORE::CString, CMatchEntry > TEntryMap;
    typedef GUCEF::map< CORE::CString, CMatchPreview > TPreviewMap;

    /** What happened to an entry, raised once the lock is released */
    struct SPendingEvent
    {
        TMatchBookChangeKind changeKind;
        bool hasChange;
        bool yourTurn;
        bool finished;
        bool invite;
        CORE::CString matchId;
        UInt32 revision;
    };
    typedef struct SPendingEvent TPendingEvent;
    typedef GUCEF::vector< TPendingEvent > TPendingEventVector;

    CMatchBook( const CMatchBook& src );
    CMatchBook& operator=( const CMatchBook& src );

    Int64 GetNowMs( void ) const;
    bool IsYourTurn( const CMatchEntry& entry ) const;
    bool IsInvited( const CMatchEntry& entry ) const;

    /** Whether the match has not started: it is forming (invitations are out) or its creator is still initializing it */
    bool IsForming( const CMatchEntry& entry ) const;
    bool MergeUpdate( const CMatchUpdate& update, TPendingEventVector& events );
    void Raise( const TPendingEventVector& events );
    bool ChangeFlag( const CORE::CString& matchId, int flag, bool value );
    bool Persist( const CMatchEntry& entry );
    void SubscribeToProvider( CIMatchProvider* provider );
    CIMatchProvider* FindProviderByNotifier( CORE::CNotifier* notifier ) const;

    void OnMatchAdded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMatchUpdated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMatchRemoved( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSubmitResult( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnProviderState( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnProviderError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTimerUpdate( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    TProviderVector m_providers;
    TEntryMap m_entries;
    TPreviewMap m_pendingSubmits;
    CORE::CString::StringSet m_offlineProviders;
    CMatchStore m_store;
    CORE::CString m_localUserId;
    CORE::CString m_openMatchId;
    const CIMatchClock* m_clock;
    CORE::CTimer m_timer;
    UInt32 m_fastIntervalMs;
    UInt32 m_slowIntervalMs;
    Int64 m_lastFastRefresh;
    Int64 m_lastSlowRefresh;
    bool m_started;
    bool m_background;
    mutable MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHBOOK_H ? */
