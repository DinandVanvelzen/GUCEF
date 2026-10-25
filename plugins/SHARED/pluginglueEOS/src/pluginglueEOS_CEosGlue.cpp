/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H
#include "pluginglueEOS_CEosPlatformSdk.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H ? */

#include "pluginglueEOS_CEosGlue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CEosGlue::g_dataLock;
CEosGlue* CEosGlue::g_instance = GUCEF_NULL;

/** Platforms that were let go of: their memory goes at the next acquire, never from within one of their own events */
static GUCEF::vector< CEosPlatform* > g_retiredPlatforms;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosGlue::CEosGlue( void )
    : m_lock()
    , m_settings()
    , m_platforms()
    , m_haveOverride( false )
    , m_overridePlatform()
    , m_overrideP2P()
{GUCEF_TRACE;

    CEosPlatform::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CEosGlue::~CEosGlue()
{GUCEF_TRACE;

    if ( !m_platforms.empty() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosGlue: " << m_platforms.size() << " platform(s) are still held at shutdown, they are released now" );
    for ( size_t i=0; i<m_platforms.size(); ++i )
        GUCEF_DELETE m_platforms[ i ];
    m_platforms.clear();
    for ( size_t i=0; i<g_retiredPlatforms.size(); ++i )
        GUCEF_DELETE g_retiredPlatforms[ i ];
    g_retiredPlatforms.clear();
}

/*-------------------------------------------------------------------------*/

CEosGlue*
CEosGlue::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex scopeLock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
            g_instance = GUCEF_NEW CEosGlue();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CEosGlue::IsSdkCompiledIn( void )
{GUCEF_TRACE;

    return CEosPlatformSdk::IsCompiledIn();
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::SetSettings( const CEosPlatformSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

CEosPlatformSettings
CEosGlue::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CEosGlue::LoadAuthSection( const CORE::CDataNode& authSection )
{GUCEF_TRACE;

    CEosPlatformSettings loaded( GetSettings() );
    if ( !loaded.LoadAuthSection( authSection ) )
        return false;
    SetSettings( loaded );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::SetDefaultShims( const SEosPlatformShim& platform, const SEosP2PShim& p2p )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_haveOverride = true;
    m_overridePlatform = platform;
    m_overrideP2P = p2p;
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::ClearDefaultShims( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_haveOverride = false;
    m_overridePlatform = SEosPlatformShim();
    m_overrideP2P = SEosP2PShim();
}

/*-------------------------------------------------------------------------*/

bool
CEosGlue::GetDefaultPlatformShim( SEosPlatformShim& shim ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_haveOverride )
        {
            shim = m_overridePlatform;
            return true;
        }
    }
    return CEosPlatformSdk::FillPlatformShim( shim );
}

/*-------------------------------------------------------------------------*/

bool
CEosGlue::GetDefaultP2PShim( SEosP2PShim& shim ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_haveOverride )
        {
            shim = m_overrideP2P;
            return shim.IsUsable();
        }
    }
    return CEosPlatformSdk::FillP2PShim( shim );
}

/*-------------------------------------------------------------------------*/

CEosPlatform*
CEosGlue::FindPlatform( const SEosPlatformShim& shim ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_platforms.size(); ++i )
        if ( m_platforms[ i ]->GetPlatformShim().IsSamePlatform( shim ) )
            return m_platforms[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CEosPlatform*
CEosGlue::AcquirePlatform( const SEosPlatformShim* shim, const CEosPlatformSettings* settings, CORE::CString& reason )
{GUCEF_TRACE;

    SEosPlatformShim use;
    bool isDefault = GUCEF_NULL == shim || ( !shim->CanCreatePlatform() && GUCEF_NULL == shim->isPlatformReady );
    if ( isDefault )
    {
        if ( !GetDefaultPlatformShim( use ) )
        {
            reason = "The Epic Online Services SDK is not part of this build";
            return GUCEF_NULL;
        }
    }
    else
    {
        use = *shim;
    }

    MT::CScopeMutex lock( m_lock );

    // The memory of platforms that were let go
    for ( size_t i=0; i<g_retiredPlatforms.size(); ++i )
        GUCEF_DELETE g_retiredPlatforms[ i ];
    g_retiredPlatforms.clear();

    CEosPlatformSettings useSettings = GUCEF_NULL != settings ? *settings : m_settings;
    CEosPlatform* existing = FindPlatform( use );
    if ( GUCEF_NULL != existing )
    {
        if ( useSettings.IsConfigured() && existing->GetSettings().IsConfigured() && !existing->GetSettings().IsSameProduct( useSettings ) )
        {
            reason = "The Epic Online Services platform is made for another product, client or sandbox already";
            return GUCEF_NULL;
        }
        ++existing->m_refCount;
        return existing;
    }

    CEosPlatform* platform = GUCEF_NEW CEosPlatform( use, useSettings );
    if ( !platform->Create( reason ) )
    {
        GUCEF_DELETE platform;
        return GUCEF_NULL;
    }
    platform->m_refCount = 1;
    m_platforms.push_back( platform );

    if ( isDefault )
    {
        SEosP2PShim p2p;
        if ( GetDefaultP2PShim( p2p ) )
            platform->AttachP2P( p2p );
    }
    return platform;
}

/*-------------------------------------------------------------------------*/

CEosPlatform*
CEosGlue::AcquirePlatformOfUser( const CORE::CString& productUserId, CORE::CString& reason )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    CEosPlatform* found = GUCEF_NULL;
    for ( size_t i=0; i<m_platforms.size(); ++i )
    {
        CEosLoginState state = m_platforms[ i ]->GetLoginState();
        if ( !state.IsLoggedIn() )
            continue;
        if ( !productUserId.IsNULLOrEmpty() && state.productUserId != productUserId )
            continue;
        if ( GUCEF_NULL != found )
        {
            reason = "More than one Epic Online Services user is logged in, name the product user";
            return GUCEF_NULL;
        }
        found = m_platforms[ i ];
    }
    if ( GUCEF_NULL == found )
    {
        reason = productUserId.IsNULLOrEmpty() ? CORE::CString( "Nobody is logged in at Epic Online Services" ) : CORE::CString( "That product user is not logged in at Epic Online Services" );
        return GUCEF_NULL;
    }
    ++found->m_refCount;
    return found;
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::AddRef( CEosPlatform* platform )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_platforms.size(); ++i )
        if ( m_platforms[ i ] == platform )
            ++platform->m_refCount;
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::ReleasePlatform( CEosPlatform* platform )
{GUCEF_TRACE;

    if ( GUCEF_NULL == platform )
        return;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_platforms.size(); ++i )
    {
        if ( m_platforms[ i ] != platform )
            continue;
        if ( platform->m_refCount > 1 )
        {
            --platform->m_refCount;
            return;
        }
        platform->m_refCount = 0;
        m_platforms.erase( m_platforms.begin() + i );

        // The SDK platform goes now, the object when nothing of it is running any more
        platform->Shutdown();
        g_retiredPlatforms.push_back( platform );
        return;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CEosGlue::GetPlatformCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_platforms.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
