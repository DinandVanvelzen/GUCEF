/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
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

#ifndef GUCEF_CORE_CMETRICSCLIENTMANAGER_H
#include "gucefCORE_CMetricsClientManager.h"
#define GUCEF_CORE_CMETRICSCLIENTMANAGER_H
#endif /* GUCEF_CORE_CMETRICSCLIENTMANAGER_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_CHTTPURLHANDLER_H
#include "gucefWEB_CHTTPURLHandler.h"
#define GUCEF_WEB_CHTTPURLHANDLER_H
#endif /* GUCEF_WEB_CHTTPURLHANDLER_H ? */

#ifndef GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CAsyncHttpServerRequestHandler.h"
#define GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H
#include "gucefWEB_CAsyncHttpServerResponseHandler.h"
#define GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H
#endif /* GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H ? */

#ifndef GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#include "gucefWEB_CGlobalHttpCodecLinks.h"
#define GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#endif /* GUCEF_WEB_CGLOBALHTTPCODECLINKS_H ? */

#include "gucefWEB_CWebGlobal.h"  /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CWebGlobal::g_dataLock;
CWebGlobal* CWebGlobal::g_instance = GUCEF_NULL;
TAsyncHttpServerRequestHandlerFactory g_asyncHttpServerRequestHandlerFactory;
TAsyncHttpServerResponseHandlerFactory g_asyncHttpServerResponseHandlerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebGlobal::CWebGlobal( void )
    : m_globalHttpCodecLinks( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebGlobal::Initialize( void )
{GUCEF_TRACE;

    // Initialize modules we depend on
    CORE::CCoreGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefWEB Global systems initializing" );

    CHTTPClient::RegisterEvents();
    CHTTPURLHandler::Register();

    m_globalHttpCodecLinks = GUCEF_NEW CGlobalHttpCodecLinks();

    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAsyncHttpServerRequestHandler::TaskType, &g_asyncHttpServerRequestHandlerFactory );
    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAsyncHttpServerResponseHandler::TaskType, &g_asyncHttpServerResponseHandlerFactory );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefWEB" );
}

/*-------------------------------------------------------------------------*/

CWebGlobal::~CWebGlobal()
{GUCEF_TRACE;
  
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefWEB Global systems shutting down" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefWEB" );

    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAsyncHttpServerRequestHandler::TaskType );
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAsyncHttpServerResponseHandler::TaskType );

    m_globalHttpCodecLinks->RemoveEncodingCodecLinks();
    m_globalHttpCodecLinks->RemoveMimeCodecLinks();
    
    CHTTPURLHandler::Unregister();

    GUCEF_DELETE m_globalHttpCodecLinks;
    m_globalHttpCodecLinks = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWebGlobal*
CWebGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex scopeLock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CWebGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CWebGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CWebGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CGlobalHttpCodecLinks& 
CWebGlobal::GetGlobalHttpCodecLinks( void ) const
{GUCEF_TRACE;

    return *m_globalHttpCodecLinks;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
