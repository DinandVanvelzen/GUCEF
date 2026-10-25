/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CSERVERLIST_H
#include "gucefMULTIPLAY_CServerList.h"
#define GUCEF_MULTIPLAY_CSERVERLIST_H
#endif /* GUCEF_MULTIPLAY_CSERVERLIST_H ? */

#ifndef GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLanListingProvider.h"
#define GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CHttpListListingProvider.h"
#define GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLobbyServiceListingProvider.h"
#define GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CIRCCHATSERVICE_H
#include "gucefMULTIPLAY_CIrcChatService.h"
#define GUCEF_MULTIPLAY_CIRCCHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CIRCCHATSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CLocalIdentityProvider.h"
#define GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOCALIDENTITYPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CAuthIdentityProvider.h"
#define GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CAUTHIDENTITYPROVIDER_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CPINGSERVICE_H
#include "gucefMULTIPLAY_CPingService.h"
#define GUCEF_MULTIPLAY_CPINGSERVICE_H
#endif /* GUCEF_MULTIPLAY_CPINGSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEBROWSER_H
#include "gucefMULTIPLAY_CGameBrowser.h"
#define GUCEF_MULTIPLAY_CGAMEBROWSER_H
#endif /* GUCEF_MULTIPLAY_CGAMEBROWSER_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H
#include "gucefMULTIPLAY_CGucefLobbyRelayAuthenticator.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H
#include "gucefMULTIPLAY_CGucefLobbyService.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#include "gucefMULTIPLAY_CGucefLobby.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBY_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H
#include "gucefMULTIPLAY_CGucefLobbyServer.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H
#include "gucefMULTIPLAY_CLobbyChatService.h"
#define GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYCHATSERVICE_H ? */

#include "gucefMULTIPLAY_CMultiplayGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CMultiplayGlobal::g_dataLock;
CMultiplayGlobal* CMultiplayGlobal::g_instance = GUCEF_NULL;

static TLanListingProviderFactory g_lanListingProviderFactory;
static THttpListListingProviderFactory g_httpListListingProviderFactory;
static TLobbyServiceListingProviderFactory g_lobbyServiceListingProviderFactory;
static TIrcChatServiceFactory g_ircChatServiceFactory;
static TLocalIdentityProviderFactory g_localIdentityProviderFactory;
static TAuthIdentityProviderFactory g_authIdentityProviderFactory;
static TGucefLobbyServiceFactory g_gucefLobbyServiceFactory;
static TLobbyChatServiceFactory g_lobbyChatServiceFactory;
static TGucefLobbyAuthenticatorFactory g_gucefLobbyAuthenticatorFactory;
static TGucefLobbyRelayAuthenticatorFactory g_gucefLobbyRelayAuthenticatorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMultiplayGlobal::CMultiplayGlobal( void )
    : m_listingProviderFactory( GUCEF_NULL )
    , m_lobbyServiceFactory( GUCEF_NULL )
    , m_chatServiceFactory( GUCEF_NULL )
    , m_identityProviderFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMultiplayGlobal::Initialize( void )
{GUCEF_TRACE;

    // The modules we depend on
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    WEB::CWebGlobal::Instance();
    DRN::CDRNGlobal::Instance();
    AUTH::CAuthGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMULTIPLAY Global systems initializing" );

    CServerListProvider::RegisterEvents();
    CServerList::RegisterEvents();
    CIGameListingProvider::RegisterEvents();
    CIIdentityProvider::RegisterEvents();
    CAuthIdentityProvider::RegisterEvents();
    CILobby::RegisterEvents();
    CILobbyService::RegisterEvents();
    CIChatService::RegisterEvents();
    CPingService::RegisterEvents();
    CGameBrowser::RegisterEvents();
    CGucefLobby::RegisterEvents();
    CGucefLobbyService::RegisterEvents();
    CGucefLobbyServer::RegisterEvents();

    m_listingProviderFactory = GUCEF_NEW CGameListingProviderFactory();
    m_lobbyServiceFactory = GUCEF_NEW CLobbyServiceFactory();
    m_chatServiceFactory = GUCEF_NEW CChatServiceFactory();
    m_identityProviderFactory = GUCEF_NEW CIdentityProviderFactory();

    m_listingProviderFactory->RegisterConcreteFactory( CLanListingProvider::ProviderName, &g_lanListingProviderFactory );
    m_listingProviderFactory->RegisterConcreteFactory( CHttpListListingProvider::ProviderName, &g_httpListListingProviderFactory );
    m_listingProviderFactory->RegisterConcreteFactory( CLobbyServiceListingProvider::ProviderName, &g_lobbyServiceListingProviderFactory );
    m_chatServiceFactory->RegisterConcreteFactory( CIrcChatService::ServiceName, &g_ircChatServiceFactory );
    m_chatServiceFactory->RegisterConcreteFactory( CLobbyChatService::ServiceName, &g_lobbyChatServiceFactory );
    m_lobbyServiceFactory->RegisterConcreteFactory( CGucefLobbyService::ServiceName, &g_gucefLobbyServiceFactory );
    DRN::CDRNGlobal::Instance()->GetAuthenticatorFactory().RegisterConcreteFactory( CGucefLobbyProtocol::AuthenticatorType, &g_gucefLobbyAuthenticatorFactory );
    DRN::CDRNGlobal::Instance()->GetAuthenticatorFactory().RegisterConcreteFactory( CGucefLobbyProtocol::RelayAuthenticatorType, &g_gucefLobbyRelayAuthenticatorFactory );
    m_identityProviderFactory->RegisterConcreteFactory( CLocalIdentityProvider::ProviderName, &g_localIdentityProviderFactory );
    m_identityProviderFactory->RegisterConcreteFactory( CAuthIdentityProvider::ProviderName, &g_authIdentityProviderFactory );

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefMULTIPLAY" );
}

/*-------------------------------------------------------------------------*/

CMultiplayGlobal::~CMultiplayGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefMULTIPLAY Global systems shutting down" );

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefMULTIPLAY" );

    DRN::CDRNGlobal::Instance()->GetAuthenticatorFactory().UnregisterConcreteFactory( CGucefLobbyProtocol::AuthenticatorType );
    DRN::CDRNGlobal::Instance()->GetAuthenticatorFactory().UnregisterConcreteFactory( CGucefLobbyProtocol::RelayAuthenticatorType );
    m_listingProviderFactory->UnregisterAllConcreteFactories();
    m_lobbyServiceFactory->UnregisterAllConcreteFactories();
    m_chatServiceFactory->UnregisterAllConcreteFactories();
    m_identityProviderFactory->UnregisterAllConcreteFactories();

    GUCEF_DELETE m_listingProviderFactory;
    m_listingProviderFactory = GUCEF_NULL;
    GUCEF_DELETE m_lobbyServiceFactory;
    m_lobbyServiceFactory = GUCEF_NULL;
    GUCEF_DELETE m_chatServiceFactory;
    m_chatServiceFactory = GUCEF_NULL;
    GUCEF_DELETE m_identityProviderFactory;
    m_identityProviderFactory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CMultiplayGlobal*
CMultiplayGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CMultiplayGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CMultiplayGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CMultiplayGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CGameListingProviderFactory&
CMultiplayGlobal::GetListingProviderFactory( void )
{GUCEF_TRACE;

    return *m_listingProviderFactory;
}

/*-------------------------------------------------------------------------*/

CLobbyServiceFactory&
CMultiplayGlobal::GetLobbyServiceFactory( void )
{GUCEF_TRACE;

    return *m_lobbyServiceFactory;
}

/*-------------------------------------------------------------------------*/

CChatServiceFactory&
CMultiplayGlobal::GetChatServiceFactory( void )
{GUCEF_TRACE;

    return *m_chatServiceFactory;
}

/*-------------------------------------------------------------------------*/

CIdentityProviderFactory&
CMultiplayGlobal::GetIdentityProviderFactory( void )
{GUCEF_TRACE;

    return *m_identityProviderFactory;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
