/*
 *  webviewpluginNULL: gucefWEB plugin with the web view engine "Null"
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#include "gucefWEB_CWebViewGlobal.h"
#define GUCEF_WEB_CWEBVIEWGLOBAL_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H
#include "gucefWEB_CWebViewEngineRegistry.h"
#define GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEREGISTRY_H ? */

#include "webviewpluginNULL_CNullWebViewEngine.h"
#include "webviewpluginNULL.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBVIEWPLUGINNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The engine of this plugin while it is registered; made and released by Load and Unload, never a static object */
static WEB::TWebViewEnginePtr* g_engine = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

}; /* namespace WEBVIEWPLUGINNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( GUCEF::CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    using namespace GUCEF;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Load called on web view plugin NULL" );
    if ( GUCEF_NULL != WEBVIEWPLUGINNULL::g_engine )
        return 1;
    WEB::TWebViewEnginePtr engine( GUCEF_NEW WEBVIEWPLUGINNULL::CNullWebViewEngine() );
    if ( !WEB::CWebViewGlobal::Instance()->GetEngineRegistry().Register( engine ) )
        return 0;
    WEBVIEWPLUGINNULL::g_engine = GUCEF_NEW WEB::TWebViewEnginePtr( engine );
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    using namespace GUCEF;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Unload called on web view plugin NULL" );
    if ( GUCEF_NULL == WEBVIEWPLUGINNULL::g_engine )
        return;
    // the engine is code of this plugin: it goes before the plugin does
    WEB::CWebViewGlobal::Instance()->GetEngineRegistry().Unregister( CORE::CString( "Null" ) );
    ( *WEBVIEWPLUGINNULL::g_engine )->Shutdown( WEB::CWebViewGlobal::Instance()->GetSettings().shutdownTimeoutMs );
    GUCEF_DELETE WEBVIEWPLUGINNULL::g_engine;
    WEBVIEWPLUGINNULL::g_engine = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( GUCEF::CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != versionInfo )
    {
        versionInfo->major = 0;
        versionInfo->minor = 1;
        versionInfo->patch = 0;
        versionInfo->release = 0;
    }
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefWEB plugin with the web view engine \"Null\": blank pages, input taken, no network";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefMT,gucefCORE,gucefIMAGE,gucefWEB";
}

/*-------------------------------------------------------------------------*/
