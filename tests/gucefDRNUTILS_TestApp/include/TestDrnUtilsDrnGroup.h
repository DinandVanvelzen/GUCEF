/*
 *  gucefDRNUTILS_TestApp: GUCEF test application for gucefDRNUTILS
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

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#include "gucefDRN_CDRNEndpointList.h"
#define GUCEF_DRN_CDRNENDPOINTLIST_H
#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#include "TestDrnUtilsRpcWorld.h"
#define GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H
#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSRPCWORLD_H ? */

namespace DRNUTILSTEST {

using namespace GUCEF;

/** Key agreement, signatures and the AEAD ciphers of codecspluginMONOCYPHER: the default security policy of a node requires encryption */
inline bool
EnsureCryptoCodecPluginLoaded( void )
{
    if ( DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsSignatureAvailable() )
        return true;
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginMONOCYPHER_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return DRN::CDRNCrypto::IsKeyAgreementAvailable() && DRN::CDRNCrypto::IsSignatureAvailable();
}

/**
 *  A DRN session with members on nodes of their own, all on loopback tcp (run the test application in a network
 *  namespace with loopback up: unshare -rn). The nodes pulse on the global pulse generator (the test thread) unless
 *  ownThread is set. The first member hosts; the session of member 0 is registered in the session registry.
 */
class CDrnUtilsGroup
{
    public:

    GUCEF::vector< DRN::CDRNNode* > nodes;
    GUCEF::vector< DRN::CDRNSession* > sessions;
    GUCEF::vector< DRN::CDRNEndpoint > endpoints;
    DRN::CDRNNodeSettings nodeSettings;
    CORE::CString name;
    DRN::CDRNSession* registered;

    explicit CDrnUtilsGroup( const CORE::CString& sessionName )
        : nodes(), sessions(), endpoints(), nodeSettings(), name( sessionName ), registered( GUCEF_NULL )
    {
        nodeSettings.echoIntervalMs = 50;
        nodeSettings.pulseIntervalMs = 1;
    }

    ~CDrnUtilsGroup()
    {
        Stop();
    }

    DRN::CDRNSession& operator[]( UInt32 index ) { return *sessions[ index ]; }

    bool AddNode( void )
    {
        EnsureCryptoCodecPluginLoaded();
        DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
        node->SetSettings( nodeSettings );
        nodes.push_back( node );
        UInt16 port = TESTFW::FindFreeTcpAndUdpPort( 0 );
        endpoints.push_back( DRN::CDRNEndpoint( DRN::CDRNEndpoint::SchemeForChannelType( "tcp" ) + "://127.0.0.1:" + CORE::UInt32ToString( port ) ) );
        sessions.push_back( node->CreateSession() );
        return node->Listen( endpoints.back() );
    }

    bool Start( UInt32 members )
    {
        if ( !AddNode() )
            return false;
        DRN::CDRNSessionSettings settings;
        settings.sessionName = name;
        if ( !sessions[ 0 ]->Host( settings ) )
            return false;
        for ( UInt32 i=1; i<members; ++i )
        {
            if ( !AddNode() )
                return false;
            DRN::CDRNSessionSettings joinSettings;
            joinSettings.sessionName = name;
            if ( !sessions.back()->Join( DRN::CDRNEndpointList( endpoints[ 0 ] ), joinSettings ) )
                return false;
        }
        class CMembership : public CCondition
        {
            public:
            CDrnUtilsGroup& g;
            explicit CMembership( CDrnUtilsGroup& group ) : g( group ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
            {
                for ( UInt32 i=0; i<g.sessions.size(); ++i )
                {
                    MT::CScopeMutex lock( g.sessions[ i ]->GetNode().GetLock() );
                    if ( !g.sessions[ i ]->IsActive() || g.sessions[ i ]->GetMemberCount() != g.sessions.size() )
                        return false;
                }
                return true;
            }
        };
        CMembership membership( *this );
        if ( !PumpUntil( membership, 15000 ) )
            return false;
        return Register( 0 );
    }

    /** The session of the member becomes the one the registry finds by the session name */
    bool Register( UInt32 member )
    {
        DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
        if ( GUCEF_NULL != registered )
            registry.Unregister( *registered );
        registered = sessions[ member ];
        return registry.Register( *registered );
    }

    void Stop( void )
    {
        if ( GUCEF_NULL != registered )
            DRN::CDRNGlobal::Instance()->GetSessionRegistry().Unregister( *registered );
        registered = GUCEF_NULL;
        for ( UInt32 i=0; i<nodes.size(); ++i )
        {
            nodes[ i ]->Stop();
            GUCEF_DELETE nodes[ i ];
        }
        nodes.clear();
        sessions.clear();
        endpoints.clear();
    }
};

}; /* namespace DRNUTILSTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_TESTAPP_TESTDRNUTILSDRNGROUP_H ? */
