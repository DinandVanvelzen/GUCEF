/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_MACHINE_H
#define GUCEF_LABLINK_MACHINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#include "gucefLabLink_Settings.h"
#include "gucefLabLink_Store.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A cluster as its discovery entry describes it */
struct SClusterSighting
{
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString address;
    GUCEF::CORE::CString key;          /**< the public key of its lab endpoint */
    GUCEF::UInt16 labPort;
    GUCEF::UInt16 accessPort;

    SClusterSighting( void ) : name(), address(), key(), labPort( 0 ), accessPort( 0 ) {}
};

/** What a machine got from the cluster in one visit */
struct SClusterAnswers
{
    GUCEF::CORE::CString cluster;
    GUCEF::CORE::CString k3sVersion;
    GUCEF::CORE::CString caCertificate;
    GUCEF::CORE::CString mcpToken;
    GUCEF::CORE::CString::StringVector nodes;   /**< "name url" */
    bool joinGiven;
    GUCEF::CORE::CString joinServer;
    GUCEF::CORE::CString joinVersion;
    GUCEF::CORE::CString joinToken;
    bool joinEphemeral;

    SClusterAnswers( void ) : cluster(), k3sVersion(), caCertificate(), mcpToken(), nodes(), joinGiven( false ), joinServer(), joinVersion(),
                              joinToken(), joinEphemeral( false ) {}
};

/**
 *  The machine side: finds the cluster through discovery, links to it with a token of the lab secret, of its approval,
 *  or asks for access; checks the cluster, joins it as a node when allowed and sets up Claude Code. Visits the cluster
 *  again every refresh interval, and at once when the cluster is seen anew. Runs on the pulse generator of the
 *  application; a visit pumps it while it waits.
 */
class CMachineRole : public GUCEF::CORE::CObserver
{
    public:

    CMachineRole( const SSettings& settings, CStore& store );

    virtual ~CMachineRole() GUCEF_VIRTUAL_OVERRIDE;

    bool Start( GUCEF::CORE::CString& problem );

    void Stop( void );

    /** From the main loop: visits the cluster when it is time to */
    void Update( void );

    /** One visit of the cluster, without discovery: for --once and tests. @return whether it linked */
    bool Visit( const SClusterSighting& cluster );

    /** What the last visit learned */
    const SClusterAnswers& GetLastAnswers( void ) const;

    /** What the last visit did or why it failed, for the log and tests */
    const GUCEF::CORE::CString& GetLastOutcome( void ) const;

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CMachineRole( const CMachineRole& src );
    CMachineRole& operator=( const CMachineRole& src );

    /** The token for the lab endpoint; asks for access when there is none. @return false when there is none (yet) */
    bool GetToken( const SClusterSighting& cluster, GUCEF::CORE::CString& token );

    bool AskForAccess( const SClusterSighting& cluster, GUCEF::CORE::CString& token );

    bool TalkToLab( const SClusterSighting& cluster, const GUCEF::CORE::CString& token, SClusterAnswers& answers );

    void RunJoin( const SClusterAnswers& answers );

    void ConfigureClaude( const SClusterAnswers& answers );

    bool RunClaude( const GUCEF::CORE::CProcess::TStringVector& args, std::string& output );

    SSettings m_settings;
    CStore& m_store;
    SIdentity m_identity;
    GUCEF::CORE::CString m_secret;
    SClusterSighting m_sighting;
    bool m_haveSighting;
    GUCEF::UInt64 m_nextVisitInMs;
    SClusterAnswers m_lastAnswers;
    GUCEF::CORE::CString m_lastOutcome;
    GUCEF::CORE::CString::StringVector m_discoveryClients;
    bool m_claudeMissingReported;               /**< that the user has no Claude Code is said once */
};

/** Milliseconds of a monotonic clock */
GUCEF::UInt64 NowInMs( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_MACHINE_H ? */
