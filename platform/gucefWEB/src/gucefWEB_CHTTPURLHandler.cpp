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

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CMFILEACCESS_H
#include "CMFileAccess.h"
#define GUCEF_CORE_CMFILEACCESS_H
#endif /* GUCEF_CORE_CMFILEACCESS_H ? */

#ifndef GUCEF_CORE_CURLHANDLERREGISTRY_H
#include "CURLHandlerRegistry.h"
#define GUCEF_CORE_CURLHANDLERREGISTRY_H
#endif /* GUCEF_CORE_CURLHANDLERREGISTRY_H ? */

#include "gucefWEB_CHTTPURLHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CHTTPURLHandler::CHTTPURLHandler( void )
    : CURLHandler()               ,
      m_httpClient( NULL )        ,
      m_transferFinished( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHTTPURLHandler::CHTTPURLHandler( const CHTTPURLHandler& src )
    : CURLHandler( src )          ,
      m_httpClient( NULL )        ,
      m_transferFinished( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHTTPURLHandler::~CHTTPURLHandler()
{GUCEF_TRACE;

    if ( NULL != m_httpClient )
    {
        UnsubscribeFrom( m_httpClient );
        GUCEF_DELETE m_httpClient;
        m_httpClient = NULL;
    }
}

/*-------------------------------------------------------------------------*/

CHTTPURLHandler&
CHTTPURLHandler::operator=( const CHTTPURLHandler& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CHTTPURLHandler::Activate( CORE::CURL& url )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): Activating using URL: " + url.GetURL() );

    m_transferFinished = false;

    GUCEF_DELETE m_httpClient;
    CORE::PulseGeneratorPtr pulseGenerator( url.GetPulseGenerator() );
    m_httpClient = GUCEF_NEW CHTTPClient( pulseGenerator );
    SubscribeTo( m_httpClient );

    return m_httpClient->Get( url.GetURL() );
}

/*-------------------------------------------------------------------------*/

void
CHTTPURLHandler::Deactivate( CORE::CURL& url )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): Deactivating" );

    if ( NULL != m_httpClient )
    {
        m_httpClient->Close();
    }
}

/*-------------------------------------------------------------------------*/

bool
CHTTPURLHandler::IsActive( const CORE::CURL& url ) const
{GUCEF_TRACE;

    if ( NULL != m_httpClient )
    {
        return m_httpClient->IsConnected();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CHTTPURLHandler::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CHTTPURLHandler( *this );
}

/*-------------------------------------------------------------------------*/

void
CHTTPURLHandler::OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    // We only accept events from our own HTTP client
    if ( notifier == m_httpClient )
    {
        if ( ( eventid == CHTTPClient::HTTPErrorEvent )       ||
             ( eventid == CHTTPClient::ConnectionErrorEvent )  )
        {

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): Data retrieval error" );
            NotifyObservers( CIURLEvents::URLDataRetrievalErrorEvent );
            return;
        }
        if ( eventid == CHTTPClient::HTTPDataRecievedEvent )
        {
            // pass on the eventdata since it's a cloneable buffer it  will sort itself out
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): Data received" );
            NotifyObservers( CIURLEvents::URLDataRecievedEvent, eventdata );
            return;
        }
        if ( eventid == CHTTPClient::HTTPTransferFinishedEvent )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): Transfer finished" );
            m_transferFinished = true;
            NotifyObservers( CIURLEvents::URLAllDataRecievedEvent );
            return;
        }
        if ( eventid == CHTTPClient::ConnectingEvent )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): Connecting" );
            NotifyObservers( CIURLEvents::URLActivateEvent );
            return;
        }
        if ( eventid == CHTTPClient::DisconnectedEvent )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): Disconnected" );

            // Check if the transfer has already been completed. If so then the URLAllDataRecievedEvent event was the
            // final event and we don't have to send anything. If we where still expecting data then we will send the
            // URLDeactivateEvent event to notify that we deactivated prematurely.
            if ( !m_transferFinished )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CHTTPURLHandler(" + CORE::PointerToString( this ) + "): The transfer was not finished when we where disconnected!" );
                NotifyObservers( CIURLEvents::URLDeactivateEvent );
            }
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CHTTPURLHandler::Register( void )
{GUCEF_TRACE;

    CORE::CURLHandlerRegistry* registry = &CORE::CCoreGlobal::Instance()->GetUrlHandlerRegistry();
    if ( !registry->IsRegistered( "http" ) )
    {
        registry->Register( "http", CORE::CURLHandlerRegistry::TRegisteredObjTypedPtr( GUCEF_NEW CHTTPURLHandler() ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CHTTPURLHandler::Unregister( void )
{GUCEF_TRACE;

    CORE::CURLHandlerRegistry* registry = &CORE::CCoreGlobal::Instance()->GetUrlHandlerRegistry();
    if ( registry->IsRegistered( "http" ) )
    {
        registry->Unregister( "http" );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CHTTPURLHandler::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::COM::CHTTPURLHandler";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
