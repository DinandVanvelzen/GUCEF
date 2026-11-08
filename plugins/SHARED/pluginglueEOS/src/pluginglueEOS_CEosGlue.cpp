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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

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

    // The lock of the glue is a leaf: platforms are made, shut down and deleted without it, since a platform subscribes
    // its timer to the pulse generator of the application and lets go of it again, while the channels of a DRN node
    // ask for their platform under the node lock, which a pulse of that pulse generator takes (gucefDRN design 9.1).
    // One thread at a time makes or lets go of the platform of a table: it stays registered while that happens, marked,
    // and whoever acquires it meanwhile waits outside the lock until it is ready or gone. Two creates of the SDK
    // platform never overlap, and nobody is handed a platform that the SDK is still making or already releasing.
    MT::SThreadIds thisThreadIds;
    MT::GetCurrentThreadIds( &thisThreadIds );
    const TThreadId thisThread = thisThreadIds.threadId;
    CEosPlatform* candidate = GUCEF_NULL;
    for ( ;; )
    {
        GUCEF::vector< CEosPlatform* > retired;
        CEosPlatform* existing = GUCEF_NULL;
        bool registered = false;
        bool busy = false;
        bool busyOnThisThread = false;
        bool refused = false;
        CEosPlatformSettings useSettings;
        {
            MT::CScopeMutex lock( m_lock );

            // The memory of platforms that were let go goes below
            retired.swap( g_retiredPlatforms );

            useSettings = GUCEF_NULL != settings ? *settings : m_settings;
            existing = FindPlatform( use );
            if ( GUCEF_NULL != existing )
            {
                if ( CEosPlatform::GLUESTATE_READY != existing->m_glueState )
                {
                    busy = true;
                    busyOnThisThread = existing->m_glueBusyThreadId == thisThread;
                }
                else
                if ( useSettings.IsConfigured() && existing->GetSettings().IsConfigured() && !existing->GetSettings().IsSameProduct( useSettings ) )
                {
                    reason = "The Epic Online Services platform is made for another product, client or sandbox already";
                    existing = GUCEF_NULL;
                    refused = true;
                }
                else
                {
                    ++existing->m_refCount;
                }
            }
            else
            if ( GUCEF_NULL != candidate )
            {
                // This thread makes it: the platform is registered as being made, who comes after waits for it
                candidate->m_glueState = CEosPlatform::GLUESTATE_CREATING;
                candidate->m_glueBusyThreadId = thisThread;
                GUCEF_SYSTEM_THREAD_ID_CODE( candidate->m_glueBusySystemThreadId = thisThreadIds.systemThreadId; )
                candidate->m_refCount = 1;
                m_platforms.push_back( candidate );
                registered = true;
            }
        }
        DeletePlatforms( retired );

        if ( registered )
            break;
        if ( busy )
        {
            if ( busyOnThisThread )
            {
                // Asked for from within its own create or release: waiting would never end
                reason = "The Epic Online Services platform is being made or released by this thread";
                GUCEF_DELETE candidate;
                return GUCEF_NULL;
            }
            MT::ThreadDelay( 1 );
            continue;
        }
        if ( GUCEF_NULL != existing || refused )
        {
            // A platform that is there is shared, or refused for another product; a platform made in vain had nothing
            // of the SDK yet
            GUCEF_DELETE candidate;
            return existing;
        }

        // None yet: one is made outside the lock, then registered if still nobody else did
        candidate = GUCEF_NEW CEosPlatform( use, useSettings );
    }

    // Only this thread makes the platform of this table now
    bool created = candidate->Create( reason );
    if ( created && isDefault )
    {
        SEosP2PShim p2p;
        if ( GetDefaultP2PShim( p2p ) )
            candidate->AttachP2P( p2p );
    }
    {
        MT::CScopeMutex lock( m_lock );
        candidate->m_glueBusyThreadId = 0;
        GUCEF_SYSTEM_THREAD_ID_CODE( candidate->m_glueBusySystemThreadId = 0; )
        if ( created )
        {
            candidate->m_glueState = CEosPlatform::GLUESTATE_READY;
        }
        else
        {
            candidate->m_refCount = 0;
            ErasePlatform( candidate );
        }
    }
    if ( !created )
    {
        // Whoever waited for it tries itself
        GUCEF_DELETE candidate;
        return GUCEF_NULL;
    }
    return candidate;
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::ErasePlatform( CEosPlatform* platform )
{GUCEF_TRACE;

    // under the lock of the glue
    for ( size_t i=0; i<m_platforms.size(); ++i )
    {
        if ( m_platforms[ i ] == platform )
        {
            m_platforms.erase( m_platforms.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::DeletePlatforms( GUCEF::vector< CEosPlatform* >& platforms )
{GUCEF_TRACE;

    // without the lock of the glue, see AcquirePlatform()
    for ( size_t i=0; i<platforms.size(); ++i )
        GUCEF_DELETE platforms[ i ];
    platforms.clear();
}

/*-------------------------------------------------------------------------*/

CEosPlatform*
CEosGlue::AcquirePlatformOfUser( const CORE::CString& productUserId, CORE::CString& reason )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    CEosPlatform* found = GUCEF_NULL;
    for ( size_t i=0; i<m_platforms.size(); ++i )
    {
        // A platform being made or let go is not handed out: this is asked for under the node lock, so it never waits
        if ( CEosPlatform::GLUESTATE_READY != m_platforms[ i ]->m_glueState )
            continue;
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
        if ( m_platforms[ i ] == platform && CEosPlatform::GLUESTATE_READY == platform->m_glueState )
            ++platform->m_refCount;
}

/*-------------------------------------------------------------------------*/

void
CEosGlue::ReleasePlatform( CEosPlatform* platform )
{GUCEF_TRACE;

    if ( GUCEF_NULL == platform )
        return;

    {
        MT::CScopeMutex lock( m_lock );
        bool held = false;
        for ( size_t i=0; i<m_platforms.size(); ++i )
        {
            if ( m_platforms[ i ] == platform )
            {
                held = CEosPlatform::GLUESTATE_READY == platform->m_glueState && platform->m_refCount > 0;
                break;
            }
        }
        if ( !held )
            return;
        if ( platform->m_refCount > 1 )
        {
            --platform->m_refCount;
            return;
        }

        // The last holder: the platform stays registered while it goes, so that an acquire meanwhile waits for it
        // instead of finding the SDK platform that is being released
        platform->m_refCount = 0;
        platform->m_glueState = CEosPlatform::GLUESTATE_RELEASING;
        MT::SThreadIds busyIds;
        MT::GetCurrentThreadIds( &busyIds );
        platform->m_glueBusyThreadId = busyIds.threadId;
        GUCEF_SYSTEM_THREAD_ID_CODE( platform->m_glueBusySystemThreadId = busyIds.systemThreadId; )
    }

    // The SDK platform goes now, without the lock of the glue (see AcquirePlatform()), the object when nothing of it is
    // running any more
    platform->Shutdown();
    MT::CScopeMutex lock( m_lock );
    ErasePlatform( platform );
    g_retiredPlatforms.push_back( platform );
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
