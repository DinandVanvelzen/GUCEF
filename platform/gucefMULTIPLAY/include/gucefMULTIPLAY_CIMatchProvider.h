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

#ifndef GUCEF_MULTIPLAY_CIMATCHPROVIDER_H
#define GUCEF_MULTIPLAY_CIMATCHPROVIDER_H

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

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHRECORD_H
#include "gucefMULTIPLAY_CMatchRecord.h"
#define GUCEF_MULTIPLAY_CMATCHRECORD_H
#endif /* GUCEF_MULTIPLAY_CMATCHRECORD_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#include "gucefMULTIPLAY_CMatchPreview.h"
#define GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#endif /* GUCEF_MULTIPLAY_CMATCHPREVIEW_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#include "gucefMULTIPLAY_CMatchPackage.h"
#define GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#endif /* GUCEF_MULTIPLAY_CMATCHPACKAGE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#include "gucefMULTIPLAY_CMatchDecisionEvidence.h"
#define GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H
#endif /* GUCEF_MULTIPLAY_CMATCHDECISIONEVIDENCE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHOPERATIONRESULT_H
#include "gucefMULTIPLAY_CMatchOperationResult.h"
#define GUCEF_MULTIPLAY_CMATCHOPERATIONRESULT_H
#endif /* GUCEF_MULTIPLAY_CMATCHOPERATIONRESULT_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHUPDATE_H
#include "gucefMULTIPLAY_CMatchUpdate.h"
#define GUCEF_MULTIPLAY_CMATCHUPDATE_H
#endif /* GUCEF_MULTIPLAY_CMATCHUPDATE_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHPROVIDERSTATE_H
#include "gucefMULTIPLAY_CMatchProviderState.h"
#define GUCEF_MULTIPLAY_CMATCHPROVIDERSTATE_H
#endif /* GUCEF_MULTIPLAY_CMATCHPROVIDERSTATE_H ? */

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

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchOperationStatusToString( TMatchOperationStatus status );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A way matches are carried, one per backend (a live session, a shared folder, mail, a relay service). The match
 *  book owns the providers and merges what they tell into its entries; game code never sees which one carries a match.
 *
 *  The calls that take a future return whether the request could be made; the future completes with a
 *  CMatchOperationResult as data (see CMatchOperationResult::TryGetFromFuture()), or fails when the provider could
 *  not do it (no connection). Providers that are event driven by nature (IMAP IDLE, sockets) report changes
 *  with the events at any time; nothing here pumps a pulse generator that the provider does not own.
 *  The operations a provider does not support return false and have a capability flag (EMatchCapabilities) the
 *  user interface looks at to hide them.
 *
 *  Submit() is conditional and idempotent: it names the revision the caller expects the match to be at. A
 *  provider that has a newer revision answers STALE with the current preview. A repeated submission of the
 *  package the provider holds for that revision answers OK again (resending after a lost answer is safe).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CIMatchProvider : public CORE::CObservingNotifier ,
                                                   public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent MatchAddedEvent;          /**< TMatchUpdateEventData: a match the book may not know */
    static const CORE::CEvent MatchUpdatedEvent;        /**< TMatchUpdateEventData: a newer state of a match */
    static const CORE::CEvent MatchRemovedEvent;        /**< TMatchRemovedEventData: the match id of a match that is gone */
    static const CORE::CEvent SubmitResultEvent;        /**< TSubmitResultEventData: the answer to a Submit(), also when the future was dropped */
    static const CORE::CEvent ProviderStateEvent;       /**< TProviderStateEventData: online or offline */
    static const CORE::CEvent ProviderErrorEvent;       /**< TProviderErrorEventData: a description of the problem */

    typedef CMatchUpdate                    TMatchUpdateEventData;
    typedef CORE::TCloneableString          TMatchRemovedEventData;
    typedef CMatchOperationResult           TSubmitResultEventData;
    typedef CMatchProviderState             TProviderStateEventData;
    typedef CORE::TCloneableString          TProviderErrorEventData;

    static void RegisterEvents( void );

    CIMatchProvider( void );

    virtual ~CIMatchProvider();

    virtual bool Start( void ) = 0;

    virtual void Stop( void ) = 0;

    virtual bool IsStarted( void ) const = 0;

    /** EMatchCapabilities as bit flags */
    virtual UInt32 GetCapabilities( void ) const = 0;

    /** The name the provider is registered with in the factory, for example "mail" */
    virtual const CORE::CString& GetProviderName( void ) const = 0;

    /** The previews of every match of the player, one call. The result comes as events */
    virtual bool RefreshAll( void ) = 0;

    /** The preview of one match. The result comes as events */
    virtual bool Refresh( const CORE::CString& matchId ) = 0;

    /** Starts a match: the signed record, the first package and the preview signed for it */
    virtual bool CreateMatch( const CMatchRecord& record, const CMatchPackage& firstPackage, const CMatchPreview& firstPreview, CORE::CFutureResult& future ) = 0;

    virtual bool Invite( const CORE::CString& matchId, const CMatchParticipant& participant, CORE::CFutureResult& future );

    virtual bool Respond( const CORE::CString& matchId, const CMatchAcceptance& acceptance, CORE::CFutureResult& future );

    /** The package of a revision; the future completes with a CMatchOperationResult that has the package */
    virtual bool Fetch( const CORE::CString& matchId, UInt32 revision, CORE::CFutureResult& future ) = 0;

    /** Hands over a turn: see the class description. The preview is the one the player signed for the package */
    virtual bool Submit( const CORE::CString& matchId, UInt32 expectedRevision, const CMatchPreview& preview, const CMatchPackage& package, CORE::CFutureResult& future ) = 0;

    virtual bool Resign( const CORE::CString& matchId, CORE::CFutureResult& future );

    virtual bool Nudge( const CORE::CString& matchId, const CORE::CString& userId, CORE::CFutureResult& future );

    virtual bool Remove( const CORE::CString& matchId, CORE::CFutureResult& future );

    virtual bool Rematch( const CORE::CString& matchId, CORE::CFutureResult& future );

    /** Hands a proposal of a decision to the participants (decisions are decided by the evidence, see CMatchDecisionVerifier) */
    virtual bool Propose( const CORE::CString& matchId, const CMatchProposal& proposal, CORE::CFutureResult& future );

    virtual bool Vote( const CORE::CString& matchId, const CMatchVote& vote, CORE::CFutureResult& future );

    bool HasCapability( TMatchCapabilities capability ) const;

    protected:

    /** @return false if this object was destroyed by an observer of the event */
    bool ReportMatchAdded( const CMatchUpdate& update );
    bool ReportMatchUpdated( const CMatchUpdate& update );
    bool ReportMatchRemoved( const CORE::CString& matchId );
    bool ReportSubmitResult( const CMatchOperationResult& result );
    bool ReportProviderState( bool online, const CORE::CString& description );
    bool ReportProviderError( const CORE::CString& description );

    private:

    CIMatchProvider( const CIMatchProvider& src );
    CIMatchProvider& operator=( const CIMatchProvider& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIMatchProvider, MT::CMutex >   CMatchProviderFactory;
typedef CMatchProviderFactory::TProductPtr                                      CIMatchProviderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CIMATCHPROVIDER_H ? */
