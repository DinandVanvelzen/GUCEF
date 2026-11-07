/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDER_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COM_CIMAILCREDENTIALPROVIDER_H
#include "gucefCOM_CIMailCredentialProvider.h"
#define GUCEF_COM_CIMAILCREDENTIALPROVIDER_H
#endif /* GUCEF_COM_CIMAILCREDENTIALPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H
#include "matchprovidermail_CMailMatchProviderSettings.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDERSETTINGS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H
#include "matchprovidermail_CMailMatchCodec.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHCODEC_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHSTATE_H
#include "matchprovidermail_CMailMatchState.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHSTATE_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHSTATE_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILOUTBOX_H
#include "matchprovidermail_CMailOutbox.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILOUTBOX_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILOUTBOX_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINBOX_H
#include "matchprovidermail_CMailInbox.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINBOX_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINBOX_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTIONS_H
#include "matchprovidermail_CMailIntroductions.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTIONS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILINTRODUCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the provider tells besides the events of CIMatchProvider: a turn that was refused, a participant who signed two
 *  successors of a revision, a reminder, a proposal or a vote that arrived. No addresses and no contents of packages.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailMatchNotice : public CORE::CICloneable
{
    public:

    CORE::CString matchId;
    UInt32 revision;
    CORE::CString code;                 /**< see CMailRefusal::code, or PROPOSAL, VOTE, NUDGE */
    CORE::CString description;
    CORE::CString userId;               /**< who it is about, when that is known */
    CORE::CString proposalId;           /**< the proposal a PROPOSAL or VOTE notice is about */

    CMailMatchNotice( void );
    CMailMatchNotice( const CMailMatchNotice& src );
    virtual ~CMailMatchNotice() GUCEF_VIRTUAL_OVERRIDE;
    CMailMatchNotice& operator=( const CMailMatchNotice& src );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The match provider "mail": matches between players who each have an ordinary mailbox; no server of ours is involved.
 *  Capabilities: NUDGE, RESIGN and REMATCH. Not: LIVE, PUSH, SERVER_CLOCK (without a trusted clock there are no automatic
 *  timeouts: a vote or the boss decides about an absent player).
 *
 *  Trust. Nobody hosts a match: every participant signs what they send. A turn that arrives is applied only when, in this order,
 *  (1) it is signed by the pinned key of the participant whose turn it was and follows the newest trusted revision and package
 *  (CMatchChainVerifier), (2) the game's own check agrees (CIMatchPackageVerifier, which replays the commands),
 *  (3) the evidence of the decisions it applies is good (CMatchDecisionVerifier). Only then the match is updated and
 *  MatchUpdatedEvent is raised. What fails is kept on disk with the reason and announced with PackageRefusedEvent, never applied.
 *  Two different signed successors of a revision by one participant are announced with EquivocationDetectedEvent; the first
 *  valid one stays. Without a package verifier a turn is not applied, unless SetAllowUnverifiedPackages() says so.
 *
 *  Match life. CreateMatch() mails the signed record to every invitee (INVITE, with the first turn); Respond() mails the
 *  signed acceptance (ACCEPT or DECLINE) to the creator and the other participants. The match is ACTIVE when every
 *  participant accepted, or when the first turn after revision 1 was played, which is the signed start without the
 *  participants who did not accept: the creator may play it once everybody has answered. Submit() applies a turn locally first
 *  and mails it (the future ends OK when the package is signed and in the outbox); how many get the mail follows from the match
 *  settings mailDistribution (NEXT_ONLY: the next holder; ALL: the next holder in To, everybody else in Cc) and how it travels
 *  from mailTransfer (ATTACHMENT or BODY_TEXT); both forms are accepted in every match.
 *
 *  Time comes from the clock of SetClock() (the system clock by default), the work from the timer of the provider on the
 *  global pulse generator; the provider is used from the thread that pulses it.
 *
 *  Config: see CMailMatchProviderSettings; the config of the provider is its settings.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailMatchProvider : public MULTIPLAY::CIMatchProvider
{
    public:

    static const CORE::CString ProviderName;                    /**< "mail" */

    static const CORE::CEvent PackageRefusedEvent;              /**< TNoticeEventData: a turn was received and not applied; code and description say why */
    static const CORE::CEvent EquivocationDetectedEvent;        /**< TNoticeEventData */
    static const CORE::CEvent DeliveryStateChangedEvent;        /**< TDeliveryEventData: a mail of the player is queued, sent, delivered or failed */
    static const CORE::CEvent NudgeReceivedEvent;               /**< TNoticeEventData: a participant reminds the player */
    static const CORE::CEvent DecisionMessageEvent;             /**< TNoticeEventData: a proposal (code PROPOSAL) or a vote (code VOTE) arrived */
    static const CORE::CEvent MailProblemEvent;                 /**< TNoticeEventData: a mail could not be read or sent; no matchId when it is not about a match */
    static const CORE::CEvent LobbyChangedEvent;                /**< TNoticeEventData: the table of a match that has not started changed. code: CANCELLED (the creator or a boss closed it; userId is who), PLAYER_SPECTATING (userId watches by choice), PLAYER_INCOMPATIBLE (userId cannot play the game here; description is the reason), PLAYERS_REPLACED (a force start: the computer replaces the participants who did not answer), PLAYER_KICKED (userId is gone) */
    static const CORE::CEvent IntroductionEvent;                /**< TNoticeEventData: something about an introduction (design 18). code: INTRO_REQUEST (a request arrived), INTRO_INTRODUCED, INTRO_DECLINED, INTRO_KEY_CHANGED, INTRO_EXPIRED (a player of a draft), INTRO_JOINED (the player joined a game they were introduced to), INTRO_REFUSED (a mail that did not pass; the description is the result code) */

    static const CORE::CEvent RosterChangedEvent;               /**< TNoticeEventData: the table of a game that has no record yet changed (design 22). code: ROSTER (a roster of the host was taken; description is its revision), ROSTER_EQUIVOCATION (the host signed two rosters for one revision), ROSTER_KEY_CHANGED (a roster gives somebody another key: it waits for ConfirmDraftKeyChange()), ROSTER_REFUSED (a roster that did not pass; the description is the result code), RECORD_DEVIATES (the record of the game is not the table of the last roster) */
    static const CORE::CEvent VoteReceivedEvent;                /**< TNoticeEventData: a proposal (DRAFT_PROPOSAL) or a vote (DRAFT_VOTE) of the draft stage was taken, the player's own included; userId is who made it, proposalId what it is about */
    static const CORE::CEvent DraftDecisionAppliedEvent;        /**< TNoticeEventData: the host carried out a decision of the draft stage. code: KICK, FORCE_START or MOVE_TO_SPECTATOR; userId the participant it is about, proposalId the decision */
    typedef CMailMatchNotice        TNoticeEventData;
    typedef CMailDeliveryEvent      TDeliveryEventData;

    static void RegisterEvents( void );

    CMailMatchProvider( void );

    virtual ~CMailMatchProvider() GUCEF_VIRTUAL_OVERRIDE;

    /* ---- what the provider needs from the application ---- */

    /** The identity of the player: signs the acceptances, leaves, reminders and answers that the provider makes itself. A copy is kept */
    void SetIdentity( const MULTIPLAY::CPlayerIdentity& identity );

    /** The game's check of a turn (replay). Not owned. Without one no package is applied, see SetAllowUnverifiedPackages() */
    void SetPackageVerifier( MULTIPLAY::CIMatchPackageVerifier* verifier );

    /** For games that have no replay: packages are applied after the signatures and the chain agree, without the game's check. Off by default */
    void SetAllowUnverifiedPackages( bool allow );

    /** Where the mail credentials come from instead of the user name and password of the settings. Not owned */
    void SetCredentialProvider( COM::CIMailCredentialProvider* provider );

    /** The clock for the provider, which has to outlive it; NULL for the clock of the system */
    void SetClock( const MULTIPLAY::CIMatchClock* clock );

    /** The links in the readable text of the mails */
    void SetMatchLink( const MULTIPLAY::CMatchLink& link );

    /** Replaces the settings; not while started */
    bool SetSettings( const CMailMatchProviderSettings& settings );

    const CMailMatchProviderSettings& GetSettings( void ) const;

    /* ---- CIMatchProvider ---- */

    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsStarted( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Checks the mailbox and tells again about every match */
    virtual bool RefreshAll( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Refresh( const CORE::CString& matchId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateMatch( const MULTIPLAY::CMatchRecord& record, const MULTIPLAY::CMatchPackage& firstPackage, const MULTIPLAY::CMatchPreview& firstPreview, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** Answers a signed acceptance or decline of the local player to the record: mailed to the creator and the other participants */
    virtual bool Respond( const CORE::CString& matchId, const MULTIPLAY::CMatchAcceptance& acceptance, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** Answers from the turns kept here (NOT_FOUND for a revision that is not held) */
    virtual bool Fetch( const CORE::CString& matchId, UInt32 revision, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Submit( const CORE::CString& matchId, UInt32 expectedRevision, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** Signed leave of the local player, mailed to every participant */
    virtual bool Resign( const CORE::CString& matchId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** A signed reminder to a participant, at most one per participant per nudge interval */
    virtual bool Nudge( const CORE::CString& matchId, const CORE::CString& userId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Closes a match that has not started, the way a lobby is closed: under the BOSS policy the creator (or a boss) closes a match with a record alone; the
     *  signed decision (CANCEL) is mailed to every participant that was invited, plays or watches, the match is cancelled here and the mails that wait for
     *  confirmation are not sent again. Under VOTE nobody does, the creator included: this is refused and a cancel is a proposal (Propose) that the players who
     *  accepted vote on, carried out by every machine that counts the passing votes. For a game that waits for introductions (a draft, no record, nobody but the
     *  creator holds keys) the creator closes it under both policies: the signed decision goes to every contact that was asked and the draft is gone. Every
     *  receiver checks it against the key it holds for the creator
     */
    bool Cancel( const CORE::CString& matchId, CORE::CFutureResult& future );

    /**
     *  The local player watches instead of playing, before the match starts: a signed statement (MOVE_TO_SPECTATOR, reason VOLUNTARY) is mailed to everybody
     *  and applied by all. The creator does not watch. The statement a client makes by itself when it refuses the first turn is the same with a reason code
     */
    bool ChooseSpectator( const CORE::CString& matchId, CORE::CFutureResult& future );

    /** Forgets the match here: its files and its mails in the outbox. Mail that arrives for it later is ignored */
    virtual bool Remove( const CORE::CString& matchId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    /** A new match with the same participants and settings and a new id: the signed record is mailed to the participants. The result has the new match id; the first turn is submitted with Submit() (expected revision 0) */
    virtual bool Rematch( const CORE::CString& matchId, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Propose( const CORE::CString& matchId, const MULTIPLAY::CMatchProposal& proposal, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Vote( const CORE::CString& matchId, const MULTIPLAY::CMatchVote& vote, CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /* ---- more than the interface ---- */

    /** Sends the mails of a revision again at once, to the participants that did not confirm them */
    bool Resend( const CORE::CString& matchId, UInt32 revision );

    /** Asks the table for the turn of a revision (RESEND mail); wantText asks for the answer as text in the body, for a carrier that removes attachments */
    bool RequestResend( const CORE::CString& matchId, UInt32 revision, bool wantText );

    TMailDeliveryState GetDeliveryState( const CORE::CString& matchId, UInt32 revision, UInt32* attempts = GUCEF_NULL ) const;

    /** The match as known here, with the statuses that followed from acceptances, leaves and decisions */
    bool GetMatchInfo( const CORE::CString& matchId, MULTIPLAY::CMatchInfo& info ) const;

    /** The newest trusted revision, 0 when the match has no turn yet or is unknown */
    UInt32 GetTrustedRevision( const CORE::CString& matchId ) const;

    /** The turn of a revision that is trusted here */
    bool GetStoredTurn( const CORE::CString& matchId, UInt32 revision, MULTIPLAY::CMatchPreview& preview, MULTIPLAY::CMatchPackage& package ) const;

    UInt32 GetRefusals( const CORE::CString& matchId, TMailRefusalVector& refusals ) const;

    UInt32 GetEquivocations( const CORE::CString& matchId, TMailEquivocationVector& equivocations ) const;

    /** The proposals of the match that are known here, the ones of the other participants included; their votes are in GetDecisionEvidence() */
    bool GetProposals( const CORE::CString& matchId, GUCEF::vector< MULTIPLAY::CMatchProposal >& proposals ) const;

    /** The signed proposal and the signed votes that were cast on it, for the package that applies the decision */
    bool GetDecisionEvidence( const CORE::CString& matchId, const CORE::CString& proposalId, MULTIPLAY::CMatchDecisionEvidence& evidence ) const;

    /**
     *  The decisions for the time before the start (force start, kick, a participant who watches) that passed and still apply to the table as it is known here:
     *  what the creator puts in the first turn of the game. False when the match is not known or has started
     */
    bool GetPreStartEvidence( const CORE::CString& matchId, GUCEF::vector< MULTIPLAY::CMatchDecisionEvidence >& evidence ) const;

    /** How the proposal stands: the tally with the rules of a lobby */
    bool EvaluateDecision( const CORE::CString& matchId, const CORE::CString& proposalId, MULTIPLAY::CMatchDecisionResult& result ) const;

    /** The mail provider's view of the mailbox, for a display or a test */
    const CMailInbox& GetInbox( void ) const;
    const CMailOutbox& GetOutbox( void ) const;

    /* ---- introduction by address (design 18) ---- */

    /**
     *  Starts a game without the keys of the invitees: a draft with the id of the match to come, shown to the match book as a forming match
     *  with the creator only. The contacts that have a player code or a key in the address book are introduced at once; the others are
     *  sent an INTRO_REQUEST. 'settings' is kept with the draft for whoever builds the game. False with a reason when the input is not valid
     *  or the provider is not started
     */
    bool BeginIntroductions( const CORE::CString& matchId, const CORE::CString& title, const GUCEF::map< CORE::CString, CORE::CString >& settings, const GUCEF::vector< CMailIntroContactInput >& contacts, CORE::CString& reason );

    /** The contacts of a draft with their states, in the order of the table */
    bool GetIntroductions( const CORE::CString& matchId, GUCEF::vector< CMailIntroContact >& contacts ) const;

    bool GetIntroductionDraft( const CORE::CString& matchId, CMailIntroDraft& draft ) const;

    bool GetIntroductionSummary( const CORE::CString& matchId, CMailIntroSummary& summary ) const;

    /** Sends the request again (a new nonce when the old one is used up or expired); at most once per introResendIntervalMs. 'askAgain' also asks a contact that is introduced already (from a code or the address book), who then has to answer */
    TMailIntroResult ResendIntroduction( const CORE::CString& matchId, const CORE::CString& address, bool askAgain = true );

    /** The creator decides about a key that differs from the address book */
    TMailIntroResult ConfirmIntroductionKey( const CORE::CString& matchId, const CORE::CString& address, bool accept );

    /** The requests of other players that wait for an answer of the player, and the ones that were answered and wait for the invitation */
    void GetPendingIntroductions( GUCEF::vector< CMailInboundIntro >& pending ) const;

    /** Agrees to an introduction: a signed reply goes to the creator. 'confirmKeyChange' is the player's yes to a creator whose key is not the one of the address book */
    TMailIntroResult AcceptIntroduction( const CORE::CString& matchId, bool confirmKeyChange );

    /** Declines: the unsigned decline goes to the creator once */
    TMailIntroResult DeclineIntroduction( const CORE::CString& matchId );

    /**
     *  The creator removes a player from a game that waits for introductions: the contact is out of the draft and is told, signed, that the invitation
     *  is withdrawn (the player sees it as cancelled by the creator). Under the BOSS policy the creator does it directly; under VOTE it is refused (with the
     *  reason) because the players decide by vote (ProposeInDraft) and the creator carries out what passed; only the entry of a player who declined may be
     *  tidied away. False with a reason when the draft or the contact is not there
     */
    bool RemoveIntroducedPlayer( const CORE::CString& matchId, const CORE::CString& address, CORE::CString& reason );

    /**
     *  Adds a player to a game that waits for introductions (the creator's side): the player is checked like the first ones, asked by mail with the table as
     *  it is now and announced in the draft. The settings of the game that depend on the number of players are the application's (SetIntroductionSetting())
     */
    bool AddIntroducedPlayer( const CORE::CString& matchId, const CMailIntroContactInput& player, CORE::CString& reason );

    /** Changes a setting kept with a draft (the number of seats, when a player was removed) */
    bool SetIntroductionSetting( const CORE::CString& matchId, const CORE::CString& key, const CORE::CString& value );

    /** The player is done with an introduction that was cancelled (or answered): it is gone from the list */
    bool DismissIntroduction( const CORE::CString& matchId );

    CMailAddressBook& GetAddressBook( void );
    const CMailAddressBook& GetAddressBook( void ) const;
    /* ---- the table and the decisions of a game that has no record yet (design 22) ---- */
    /** The board of the draft stage of a game: the rosters of the host, the proposals and the votes. A copy. False when the game has none here */
    bool GetDraftBoard( const CORE::CString& matchId, MULTIPLAY::CMatchDraftBoard& board ) const;
    /** The roster of the host that is in force here (a player), or the one the host published last */
    bool GetDraftRoster( const CORE::CString& matchId, MULTIPLAY::CMatchRoster& roster ) const;
    /** How the proposals of the draft stage stand: the same figures on every machine that holds the same mails */
    UInt32 GetDraftTallies( const CORE::CString& matchId, GUCEF::vector< MULTIPLAY::CMatchDraftTally >& tallies ) const;
    /**
     *  The player proposes a decision of the draft stage: FORCE_START (the participants who have not answered are replaced by the computer), KICK of 'targetUserId'
     *  or MOVE_TO_SPECTATOR (the player's own). Signed against the roster that is in force, mailed to the host and to every other player.
     *  False with a reason when it cannot be made
     */
    bool ProposeInDraft( const CORE::CString& matchId, MULTIPLAY::TMatchDecisionKind kind, const CORE::CString& targetUserId, CORE::CString& proposalId, CORE::CString& reason );
    /** The player votes on a proposal of the draft stage; the vote is mailed to the host and to every other player. A vote cast already is mailed again */
    bool VoteInDraft( const CORE::CString& matchId, const CORE::CString& proposalId, bool yes, CORE::CString& reason );
    /** A roster that gives somebody another key than an earlier roster did waits for the player: taken or refused */
    bool ConfirmDraftKeyChange( const CORE::CString& matchId, bool accept );
    /** The host carries out the decisions of the draft stage that passed and still apply (done by itself when applyDraftDecisions is on); returns how many */
    UInt32 ApplyDraftDecisions( const CORE::CString& matchId );

    /* ---- the manual carrier ---- */

    /** The turn as a file (the bytes of the container; the name is up to the caller, see CMailMatchCodec::MakeAttachmentName()); written through a temporary file and a rename */
    static bool ExportToFile( const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, const CORE::CString& path, CORE::CString& reason );

    /** The turn as armored text that can be pasted into any message */
    static bool ExportToText( const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, CORE::CString& text, CORE::CString& reason );

    /** Takes the turn in a file as if it had arrived by mail: the same checks, the same outcome */
    bool ImportFile( const CORE::CString& path, CORE::CString& reason );

    /** Takes the turn in pasted text as if it had arrived by mail */
    bool ImportText( const CORE::CString& text, CORE::CString& reason );

    private:

    typedef CORE::CTEventHandlerFunctor< CMailMatchProvider > TEventCallback;

    struct SMatch
    {
        CMailMatchState state;
        MULTIPLAY::CMatchPreview trustedPreview;
        MULTIPLAY::CMatchPackage trustedPackage;
        bool hasTrusted;
        bool announced;
        UInt32 refusedCounter;
        CORE::CString directory;
        SMatch( void ) : state(), trustedPreview(), trustedPackage(), hasTrusted( false ), announced( false ), refusedCounter( 0 ), directory() {}
    };
    typedef GUCEF::map< CORE::CString, SMatch > TMatchMap;

    CMailMatchProvider( const CMailMatchProvider& src );
    CMailMatchProvider& operator=( const CMailMatchProvider& src );

    // time, files, state
    Int64 GetNowMs( void ) const;
    CORE::CString GetMatchesDirectory( void ) const;
    CORE::CString GetTurnPath( const SMatch& match, UInt32 revision ) const;
    bool SaveMatch( SMatch& match );
    bool SaveTurn( const SMatch& match, UInt32 revision, const void* container, UInt32 size );
    bool LoadTurn( const SMatch& match, UInt32 revision, MULTIPLAY::CMatchContainer& container ) const;
    bool LoadMatches( void );
    SMatch* FindMatch( const CORE::CString& matchId );
    const SMatch* FindMatch( const CORE::CString& matchId ) const;
    SMatch* AddMatch( const MULTIPLAY::CMatchRecord& record );
    bool IsRemoved( const CORE::CString& matchId ) const;
    bool SaveRemoved( void );
    void UpdateStatus( SMatch& match );

    // what is told to the match book and the application
    void AnnounceMatch( SMatch& match );
    void MakeUpdate( const SMatch& match, MULTIPLAY::CMatchUpdate& update ) const;
    void ReportNotice( const CORE::CEvent& eventId, const CMailMatchNotice& notice );
    void Notice( const CORE::CEvent& eventId, const CORE::CString& matchId, UInt32 revision, const char* code, const CORE::CString& description, const CORE::CString& userId, const CORE::CString& proposalId = CORE::CString::Empty );
    void MailProblem( const CORE::CString& matchId, UInt32 revision, const char* code, const CORE::CString& description );
    void Succeed( CORE::CFutureResult& future, const CORE::CString& matchId, const MULTIPLAY::CMatchPreview* preview );
    void Reject( CORE::CFutureResult& future, const CORE::CString& matchId, MULTIPLAY::TMatchOperationStatus status, const char* reason );
    void CompleteFuture( CORE::CFutureResult& future, const MULTIPLAY::CMatchOperationResult& result );

    // sending
    bool GetContact( const SMatch& match, const CORE::CString& userId, COM::CMailAddress& address ) const;
    CORE::CString GetNameOf( const SMatch& match, const CORE::CString& userId ) const;
    bool FindParticipantByAddress( const SMatch& match, const CORE::CString& address, CORE::CString& userId ) const;
    CORE::CString BuildLinkText( const CORE::CString& matchId ) const;
    void GetAudience( const SMatch& match, bool includeInvited, GUCEF::vector< CORE::CString >& userIds, bool includeSpectators = false ) const;
    void FillOutgoingBase( const SMatch& match, TMailMatchKind kind, CMailOutgoing& outgoing ) const;
    void NoteThread( SMatch& match, const CORE::CString& messageId );
    bool QueueMail( SMatch& match, CMailOutgoing& outgoing, bool needsConfirmation );
    bool MailTurn( SMatch& match, const MULTIPLAY::CMatchPreview& preview, const CORE::CDynamicBuffer& container, bool useTransfer, TMailTransfer transfer, const CORE::CString& onlyToAddress );
    bool MailControl( SMatch& match, TMailMatchKind kind, const CORE::CIDataNodeSerializable& data, UInt32 revision, const CORE::CString& dataPart, const CORE::CString& toUserId, const CORE::CString& firstToUserId, bool includeInvited, bool needsConfirmation, bool includeSpectators = false );
    void SendAck( SMatch& match, const MULTIPLAY::CMatchPreview& preview );
    void RequestMissing( SMatch& match, UInt32 fromRevision, UInt32 toRevision, bool wantText );
    bool StoreProposal( SMatch& match, const MULTIPLAY::CMatchProposal& proposal );

    // the decisions before the start
    bool HasStarted( const SMatch& match ) const;
    bool DeclareSpectator( SMatch& match, const CORE::CString& code, const CORE::CString& reason, CORE::CString& why );
    bool MailDraftCancel( const CMailIntroDraft& draft, const CMailIntroContact& contact, const MULTIPLAY::CMatchProposal& cancel );
    bool CancelDraft( const CORE::CString& matchId, CORE::CFutureResult& future );
    void ReportLobbyDecision( const SMatch& match, const MULTIPLAY::CMatchProposal& proposal );
    void HandleCancel( const CORE::CString& matchId, const void* data, UInt32 size );
    void HandleIncompatible( const CORE::CString& matchId, const void* data, UInt32 size );
    bool ApplyEvidenceOutsideTurn( SMatch& match, const MULTIPLAY::CMatchProposal& proposal, CORE::CString& reason );
    bool ApplyEvidenceOutsideTurn( SMatch& match, const MULTIPLAY::CMatchDecisionEvidence& evidence, CORE::CString& reason );
    void ApplyPassedCancels( SMatch& match );
    MULTIPLAY::TMatchDecisionPolicy GetDraftPolicy( const CMailIntroDraft& draft ) const;
    bool IsForceStartCovering( const SMatch& match, const MULTIPLAY::CMatchPackage& package ) const;

    // introductions
    bool MailIntroRequest( const CMailIntroDraft& draft, const CMailIntroRequest& request );
    bool MailIntroReply( const CMailInboundIntro& intro, const CMailIntroReply& reply );
    void HandleIntroRequest( const CMailInboxMessage& mail );
    void HandleIntroReply( const CMailInboxMessage& mail );
    void IntroNotice( const char* code, const CORE::CString& matchId, const CORE::CString& description );
    void AnnounceDraft( const CMailIntroDraft& draft, bool added );
    void TryJoinIntroduced( SMatch& match, const CMailInboxMessage& mail );
    CORE::CString GetIntroLinkText( const CORE::CString& matchId ) const;
    void ConfigureIntroductions( void );
    void CheckIntroductionTimes( Int64 now );
    void MakeDraftInfo( const CMailIntroDraft& draft, MULTIPLAY::CMatchInfo& info ) const;
    CORE::CString GetOwnName( void ) const;
    // the draft stage: the roster and the decisions
    typedef GUCEF::map< CORE::CString, MULTIPLAY::CMatchDraftBoard > TBoardMap;
    MULTIPLAY::CMatchDraftBoard* FindBoard( const CORE::CString& matchId );
    const MULTIPLAY::CMatchDraftBoard* FindBoard( const CORE::CString& matchId ) const;
    MULTIPLAY::CMatchDraftBoard* EnsureHostBoard( const CORE::CString& matchId );
    MULTIPLAY::CMatchDraftBoard* EnsureParticipantBoard( const CORE::CString& matchId );
    CORE::CString GetBoardsDirectory( void ) const;
    void SaveBoard( const CORE::CString& matchId );
    void LoadBoards( void );
    void ForgetBoard( const CORE::CString& matchId );
    CORE::CString GetDraftTitle( const CORE::CString& matchId ) const;
    void PublishDraftRoster( const CMailIntroDraft& draft );
    bool MailDraftData( TMailMatchKind kind, const CORE::CIDataNodeSerializable& data, const CORE::CString& matchId, UInt32 revision, const CORE::CString& dataPart, const GUCEF::vector< CORE::CString >& addresses, const CORE::CString& title, const CORE::CString& actorName, const CORE::CString& note, TMailTransfer transfer );
    bool MailToDraftAudience( TMailMatchKind kind, const CORE::CIDataNodeSerializable& data, const CORE::CString& matchId, const char* dataPart );
    void HandleRoster( const CORE::CString& matchId, const void* data, UInt32 size, const CORE::CString& fromAddress );
    void HandleDraftProposal( const CORE::CString& matchId, const void* data, UInt32 size, const CORE::CString& fromAddress );
    void HandleDraftVote( const CORE::CString& matchId, const void* data, UInt32 size, const CORE::CString& fromAddress );
    void AfterDraftMessage( const CORE::CString& matchId );
    bool RemoveContactInternal( const CORE::CString& matchId, const CORE::CString& address, bool lowerSeats, CORE::CString& reason );
    void ImportDraftStatements( SMatch& match );
    void CheckRecordAgainstRoster( SMatch& match );
    bool StoreVote( SMatch& match, const MULTIPLAY::CMatchVote& vote );

    // receiving
    void OnTimerTick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnInboxMessage( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnInboxRefused( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnInboxState( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnInboxNotice( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOutboxDelivery( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool ReadData( const CMailInboxMessage& mail, const CORE::CString& label, UInt32 maxBytes, CORE::CDynamicBuffer& data, TMailTransfer& used );
    bool ReadFirstTurn( const CMailInboxMessage& mail, CORE::CDynamicBuffer& container );
    void HandleMail( const CMailInboxMessage& mail );
    void HandleInvite( const CMailInboxMessage& mail );
    void HandleTurnMail( const CMailInboxMessage& mail );
    void HandleAck( const CMailInboxMessage& mail );
    void HandleResendRequest( const CMailInboxMessage& mail );
    void HandleBlob( TMailMatchKind kind, const CORE::CString& matchId, const void* data, UInt32 size );
    bool ProcessContainer( const void* data, UInt32 size, const CORE::CString& expectedMatchId, const CORE::CString& fromAddress, CORE::CString& reason );
    bool ApplyIncoming( SMatch& match, const MULTIPLAY::CMatchContainer& container, const CORE::CDynamicBuffer& raw, const CORE::CString& fromAddress, bool fromPendingCheck, CORE::CString& reason );
    bool IsTurnOfAbsentHolders( const SMatch& match, const MULTIPLAY::CMatchPreview& trusted, const MULTIPLAY::CMatchPreview& next, const MULTIPLAY::CMatchPackage& package ) const;
    bool ApplyTurn( SMatch& match, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, bool isLocal, const CORE::CDynamicBuffer* knownContainer, const CORE::CString& fromAddress, CORE::CString& reason );
    void Refuse( SMatch& match, const MULTIPLAY::CMatchPreview& preview, const MULTIPLAY::CMatchPackage& package, const void* container, UInt32 containerSize, const char* code, const CORE::CString& reason );
    void NoteSignOfLife( SMatch& match, const CORE::CString& userId, UInt32 revision );
    void KeepPending( SMatch& match, UInt32 revision, const CORE::CString& sha256, const CORE::CDynamicBuffer& container );
    void TryApplyPending( SMatch& match );
    void KeepOrphan( const CORE::CString& matchId, UInt8 kind, const void* data, UInt32 size );
    void ProcessOrphans( const CORE::CString& matchId );

    private:

    CMailMatchProviderSettings m_settings;
    MULTIPLAY::CPlayerIdentity m_identity;
    MULTIPLAY::CIMatchPackageVerifier* m_verifier;
    bool m_allowUnverified;
    COM::CIMailCredentialProvider* m_credentials;
    const MULTIPLAY::CIMatchClock* m_clock;
    MULTIPLAY::CMatchLink m_link;
    CORE::CTimer m_timer;
    CMailOutbox m_outbox;
    CMailInbox m_inbox;
    CMailIntroductions m_intros;
    Int64 m_lastIntroCheck;
    GUCEF::map< CORE::CString, bool > m_announcedDrafts;
    TBoardMap m_boards;
    GUCEF::map< CORE::CString, Int64 > m_rosterResent;          /**< the host: when the roster was last sent again to an address that referred to an older one */
    TMatchMap m_matches;
    CORE::CString::StringSet m_removed;
    bool m_started;
};

typedef CORE::CTFactory< MULTIPLAY::CIMatchProvider, CMailMatchProvider, MT::CMutex > TMailMatchProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILMATCHPROVIDER_H ? */
