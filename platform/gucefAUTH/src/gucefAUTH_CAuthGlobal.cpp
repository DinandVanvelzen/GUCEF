/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGSPROFILE_H
#include "gucefAUTH_CAuthSettingsProfile.h"
#define GUCEF_AUTH_CAUTHSETTINGSPROFILE_H
#endif /* GUCEF_AUTH_CAUTHSETTINGSPROFILE_H ? */

#ifndef GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H
#include "gucefAUTH_CDescriptorAuthProvider.h"
#define GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H
#endif /* GUCEF_AUTH_CDESCRIPTORAUTHPROVIDER_H ? */

#include "gucefAUTH_CAuthGlobal.h"

#ifndef GUCEF_AUTH_COAUTH2ENGINE_H
#include "gucefAUTH_COAuth2Engine.h"
#define GUCEF_AUTH_COAUTH2ENGINE_H
#endif /* GUCEF_AUTH_COAUTH2ENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CAuthGlobal::g_dataLock;
CAuthGlobal* CAuthGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAuthGlobal::CAuthGlobal( void )
    : m_providerFactory( GUCEF_NULL )
    , m_descriptorRegistry( GUCEF_NULL )
    , m_openId2Profiles( GUCEF_NULL )
    , m_manager( GUCEF_NULL )
    , m_flowEngine( GUCEF_NULL )
    , m_oauth2Engine( GUCEF_NULL )
    , m_descriptorFactories()
    , m_descriptorFactoryNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAuthGlobal::Initialize( void )
{GUCEF_TRACE;

    // The modules we depend on
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    WEB::CWebGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefAUTH Global systems initializing" );

    CIAuthProvider::RegisterEvents();
    CAuthManager::RegisterEvents();
    CAuthSettingsProfile::RegisterEvents();

    m_providerFactory = GUCEF_NEW CAuthProviderFactory();
    m_descriptorRegistry = GUCEF_NEW CAuthProviderRegistry();
    m_descriptorRegistry->RegisterBuiltIns();
    m_openId2Profiles = GUCEF_NEW COpenId2ProfileRegistry();

    CORE::CString::StringVector names;
    m_descriptorRegistry->GetNames( names );
    for ( size_t i=0; i<names.size(); ++i )
        RegisterDescriptorProvider( names[ i ] );

    m_manager = GUCEF_NEW CAuthManager();

    // The engine that makes the descriptor providers work
    m_oauth2Engine = GUCEF_NEW COAuth2Engine();
    RegisterBuiltInEngine();

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefAUTH" );
}

/*-------------------------------------------------------------------------*/

CAuthGlobal::~CAuthGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefAUTH Global systems shutting down" );

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefAUTH" );

    // The manager holds providers that are destroyed through their factories
    GUCEF_DELETE m_manager;
    m_manager = GUCEF_NULL;

    // The engine goes after the providers it worked for
    m_flowEngine = GUCEF_NULL;
    GUCEF_DELETE m_oauth2Engine;
    m_oauth2Engine = GUCEF_NULL;

    if ( GUCEF_NULL != m_providerFactory )
    {
        for ( size_t i=0; i<m_descriptorFactoryNames.size(); ++i )
            m_providerFactory->UnregisterConcreteFactory( m_descriptorFactoryNames[ i ] );
        m_providerFactory->UnregisterAllConcreteFactories();
    }
    for ( size_t i=0; i<m_descriptorFactories.size(); ++i )
        GUCEF_DELETE m_descriptorFactories[ i ];
    m_descriptorFactories.clear();
    m_descriptorFactoryNames.clear();

    GUCEF_DELETE m_providerFactory;
    m_providerFactory = GUCEF_NULL;
    GUCEF_DELETE m_openId2Profiles;
    m_openId2Profiles = GUCEF_NULL;
    GUCEF_DELETE m_descriptorRegistry;
    m_descriptorRegistry = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAuthGlobal*
CAuthGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CAuthGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CAuthGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CAuthGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAuthProviderFactory&
CAuthGlobal::GetProviderFactory( void )
{GUCEF_TRACE;

    return *m_providerFactory;
}

/*-------------------------------------------------------------------------*/

CAuthProviderRegistry&
CAuthGlobal::GetDescriptorRegistry( void )
{GUCEF_TRACE;

    return *m_descriptorRegistry;
}

/*-------------------------------------------------------------------------*/

COpenId2ProfileRegistry&
CAuthGlobal::GetOpenId2Profiles( void )
{GUCEF_TRACE;

    return *m_openId2Profiles;
}

/*-------------------------------------------------------------------------*/

CAuthManager&
CAuthGlobal::GetManager( void )
{GUCEF_TRACE;

    return *m_manager;
}

/*-------------------------------------------------------------------------*/

IAuthFlowEngine*
CAuthGlobal::GetFlowEngine( void ) const
{GUCEF_TRACE;

    return m_flowEngine;
}

/*-------------------------------------------------------------------------*/

void
CAuthGlobal::SetFlowEngine( IAuthFlowEngine* engine )
{GUCEF_TRACE;

    m_flowEngine = engine;
}

/*-------------------------------------------------------------------------*/

COAuth2Engine*
CAuthGlobal::GetOAuth2Engine( void ) const
{GUCEF_TRACE;

    return m_oauth2Engine;
}

/*-------------------------------------------------------------------------*/

void
CAuthGlobal::RegisterBuiltInEngine( void )
{GUCEF_TRACE;

    m_flowEngine = m_oauth2Engine;
}

/*-------------------------------------------------------------------------*/

bool
CAuthGlobal::RegisterDescriptorProvider( const CORE::CString& descriptorName )
{GUCEF_TRACE;

    if ( !m_descriptorRegistry->HasDescriptor( descriptorName ) || m_providerFactory->IsConstructible( descriptorName ) )
        return false;

    CDescriptorAuthProviderFactory* factory = GUCEF_NEW CDescriptorAuthProviderFactory( descriptorName );
    m_descriptorFactories.push_back( factory );
    m_descriptorFactoryNames.push_back( descriptorName );
    m_providerFactory->RegisterConcreteFactory( descriptorName, factory );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthGlobal::UnregisterDescriptorProvider( const CORE::CString& descriptorName )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_descriptorFactoryNames.size(); ++i )
    {
        if ( m_descriptorFactoryNames[ i ] == descriptorName )
        {
            m_providerFactory->UnregisterConcreteFactory( descriptorName );
            GUCEF_DELETE m_descriptorFactories[ i ];
            m_descriptorFactories.erase( m_descriptorFactories.begin() + i );
            m_descriptorFactoryNames.erase( m_descriptorFactoryNames.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
