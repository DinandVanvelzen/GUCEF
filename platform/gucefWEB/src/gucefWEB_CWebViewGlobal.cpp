/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_IWEBVIEW_H
#include "gucefWEB_IWebView.h"
#define GUCEF_WEB_IWEBVIEW_H
#endif /* GUCEF_WEB_IWEBVIEW_H ? */

#ifndef GUCEF_WEB_IWEBVIEWENGINE_H
#include "gucefWEB_IWebViewEngine.h"
#define GUCEF_WEB_IWEBVIEWENGINE_H
#endif /* GUCEF_WEB_IWEBVIEWENGINE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H
#include "gucefWEB_CWebViewEngineRegistry.h"
#define GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H ? */

#ifndef GUCEF_WEB_CIWEBVIEWPRESENTER_H
#include "gucefWEB_CIWebViewPresenter.h"
#define GUCEF_WEB_CIWEBVIEWPRESENTER_H
#endif /* GUCEF_WEB_CIWEBVIEWPRESENTER_H ? */

#include "gucefWEB_CWebViewGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewGlobal*
CWebViewGlobal::Instance( void )
{GUCEF_TRACE;

    // owned by the global of the module, which makes it when the module is first used
    return CWebGlobal::Instance()->GetWebViewGlobal();
}

/*-------------------------------------------------------------------------*/

CWebViewGlobal::CWebViewGlobal( void )
    : m_lock()
    , m_registry( GUCEF_NULL )
    , m_settings()
    , m_applicationName()
    , m_mustNotWait()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewGlobal::~CWebViewGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefWEB: shutting down the web view systems" );
    Shutdown();
    GUCEF_DELETE m_registry;
    m_registry = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWebViewGlobal::Initialize( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefWEB: initializing the web view systems" );
    IWebView::RegisterEvents();
    IWebViewEngine::RegisterEvents();
    CWebViewEngineRegistry::RegisterEvents();
    CIWebViewPresenter::RegisterEvents();
    m_registry = GUCEF_NEW CWebViewEngineRegistry();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefWEB: the web view systems are initialized" );
}

/*-------------------------------------------------------------------------*/

CWebViewEngineRegistry&
CWebViewGlobal::GetEngineRegistry( void )
{GUCEF_TRACE;

    return *m_registry;
}

/*-------------------------------------------------------------------------*/

CWebViewGlobalSettings
CWebViewGlobal::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CWebViewGlobal::SetSettings( const CWebViewGlobalSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

void
CWebViewGlobal::SetApplicationName( const CString& applicationName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_applicationName = applicationName;
}

/*-------------------------------------------------------------------------*/

CString
CWebViewGlobal::GetApplicationName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_applicationName;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewGlobal::IsLastResortEngine( const CString& engineName )
{GUCEF_TRACE;

    return engineName.Equals( "ChromeDevTools", false );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewGlobal::FindEngine( UInt32 requiredCapabilities, const CString::StringVector& preferredOrder, CWebViewEngineSelection& selection )
{GUCEF_TRACE;

    selection = CWebViewEngineSelection();
    const CWebViewGlobalSettings settings = GetSettings();
    const CString::StringVector& order = preferredOrder.empty() ? settings.engineOrder : preferredOrder;

    // engines asked for that no plugin registered are reported as not loaded
    for ( CString::StringVector::const_iterator n=order.begin(); n!=order.end(); ++n )
    {
        if ( !m_registry->IsRegistered( (*n) ) )
        {
            CWebViewEngineSelection::TCandidate candidate;
            candidate.engineName = (*n);
            candidate.outcome = CWebViewEngineSelection::OUTCOME_NOT_LOADED;
            candidate.reason = "no plugin registered the engine";
            selection.candidates.push_back( candidate );
        }
    }

    CWebViewEngineRegistry::TEngineVector engines = m_registry->GetEnginesInOrder( order );
    for ( CWebViewEngineRegistry::TEngineVector::iterator i=engines.begin(); i!=engines.end(); ++i )
    {
        TWebViewEnginePtr engine = (*i);
        CWebViewEngineSelection::TCandidate candidate;
        candidate.engineName = engine->GetEngineName();
        if ( selection.HasEngine() )
        {
            candidate.outcome = CWebViewEngineSelection::OUTCOME_NOT_TRIED;
        }
        else
        if ( IsLastResortEngine( candidate.engineName ) && !settings.allowLastResortEngines )
        {
            candidate.outcome = CWebViewEngineSelection::OUTCOME_NOT_AVAILABLE;
            candidate.reason = "a last resort engine, not allowed by the settings";
        }
        else
        if ( !engine->IsAvailable() )
        {
            candidate.outcome = CWebViewEngineSelection::OUTCOME_NOT_AVAILABLE;
            candidate.reason = engine->GetAvailabilityDescription();
        }
        else
        if ( engine->IsStalled() )
        {
            candidate.outcome = CWebViewEngineSelection::OUTCOME_STALLED;
            candidate.reason = "the engine thread stopped answering";
        }
        else
        if ( !engine->GetCapabilities().Has( requiredCapabilities ) )
        {
            candidate.outcome = CWebViewEngineSelection::OUTCOME_CAPABILITY_MISSING;
            candidate.missingCapabilities = engine->GetCapabilities().GetMissing( requiredCapabilities );
            candidate.reason = "it lacks " + CWebViewCapabilities::FlagsToString( candidate.missingCapabilities );
        }
        else
        {
            candidate.outcome = CWebViewEngineSelection::OUTCOME_CHOSEN;
            candidate.reason = engine->GetEngineVersion();
            selection.engine = engine;
        }
        selection.candidates.push_back( candidate );
    }
    return selection.HasEngine();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewGlobal::CheckEngineWatchdogs( void )
{GUCEF_TRACE;

    const UInt32 watchdogMs = GetSettings().engineWatchdogMs;
    UInt32 stalled = 0;
    CWebViewEngineRegistry::TEngineVector engines = m_registry->GetEngines();
    for ( CWebViewEngineRegistry::TEngineVector::iterator i=engines.begin(); i!=engines.end(); ++i )
        if ( (*i)->CheckWatchdog( watchdogMs ) )
            ++stalled;
    return stalled;
}

/*-------------------------------------------------------------------------*/

void
CWebViewGlobal::Shutdown( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_registry )
        return;
    const UInt32 timeoutMs = GetSettings().shutdownTimeoutMs;
    CWebViewEngineRegistry::TEngineVector engines = m_registry->GetEngines();
    for ( CWebViewEngineRegistry::TEngineVector::iterator i=engines.begin(); i!=engines.end(); ++i )
        (*i)->Shutdown( timeoutMs );
}

/*-------------------------------------------------------------------------*/

void
CWebViewGlobal::SetThreadMustNotWait( TThreadId threadId, bool mustNotWait )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( mustNotWait )
        m_mustNotWait.insert( threadId );
    else
        m_mustNotWait.erase( threadId );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewGlobal::MayWaitOnCurrentThread( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_mustNotWait.find( MT::GetCurrentThreadID() ) == m_mustNotWait.end();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
