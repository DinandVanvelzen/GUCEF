/*
 *  httpclientpluginCURL: GUCEF plugin providing a HTTP client backend using libcurl
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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H
#include "httpclientpluginCURL_CCurlHttpClientRequest.h"
#define HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H
#endif /* HTTPCLIENTPLUGIN_CURL_CCURLHTTPCLIENTREQUEST_H ? */

#include "httpclientpluginCURL_CCurlMultiDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEBPLUGIN {
namespace HTTPCLIENTCURL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** same pulse interval the TCP client socket uses while it has work to do */
static const UInt32 PulseIntervalInMs = 10;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCurlMultiDriver::CCurlMultiDriver( CORE::PulseGeneratorPtr pulseGenerator )
    : CORE::CObserver()
    , m_pulseGenerator( pulseGenerator )
    , m_multi( curl_multi_init() )
    , m_activeRequests( 0 )
    , m_inPulse( false )
    , m_pulsesRequested( false )
{GUCEF_TRACE;

    CORE::CTEventHandlerFunctor< CCurlMultiDriver > callback( this, &CCurlMultiDriver::OnPulse );
    SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent   ,
                 callback                            );
}

/*-------------------------------------------------------------------------*/

CCurlMultiDriver::~CCurlMultiDriver()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( m_pulsesRequested )
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    if ( GUCEF_NULL != m_multi )
        curl_multi_cleanup( m_multi );
    m_multi = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCurlMultiDriver::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEBPLUGIN::HTTPCLIENTCURL::CCurlMultiDriver";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CCurlMultiDriver::AddRequest( CCurlHttpClientRequest* request )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_multi || GUCEF_NULL == request || GUCEF_NULL == request->GetEasyHandle() )
        return false;

    if ( CURLM_OK != curl_multi_add_handle( m_multi, request->GetEasyHandle() ) )
        return false;

    request->SetIsInMulti( true );
    ++m_activeRequests;

    if ( !m_pulsesRequested )
    {
        m_pulsesRequested = true;
        m_pulseGenerator->RequestPeriodicPulses( this, PulseIntervalInMs );
    }
    m_pulseGenerator->RequestImmediatePulse();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCurlMultiDriver::RemoveRequest( CCurlHttpClientRequest* request )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_multi || GUCEF_NULL == request || GUCEF_NULL == request->GetEasyHandle() )
        return;

    curl_multi_remove_handle( m_multi, request->GetEasyHandle() );
    request->SetIsInMulti( false );
    if ( m_activeRequests > 0 )
        --m_activeRequests;

    if ( 0 == m_activeRequests && m_pulsesRequested )
    {
        m_pulsesRequested = false;
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CCurlMultiDriver::GetActiveRequestCount( void ) const
{GUCEF_TRACE;

    return m_activeRequests;
}

/*-------------------------------------------------------------------------*/

bool
CCurlMultiDriver::IsInPulse( void ) const
{GUCEF_TRACE;

    return m_inPulse;
}

/*-------------------------------------------------------------------------*/

CORE::CPulseGenerator*
CCurlMultiDriver::GetPulseGeneratorPointer( void ) const
{GUCEF_TRACE;

    // the pointer is copied since its const form only gives access to a const object
    CORE::PulseGeneratorPtr pulseGenerator = m_pulseGenerator;
    return pulseGenerator.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

void
CCurlMultiDriver::OnNotify( CORE::CNotifier* notifier   ,
                            const CORE::CEvent& eventId ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CCurlMultiDriver::OnPulse( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( 0 == m_activeRequests || GUCEF_NULL == m_multi || m_inPulse )
        return;

    m_inPulse = true;

    int stillRunning = 0;
    curl_multi_perform( m_multi, &stillRunning );

    // Completed transfers. Requests finish themselves which removes their handle from the multi handle,
    // which curl allows while reading messages
    int messagesLeft = 0;
    CURLMsg* message = GUCEF_NULL;
    while ( GUCEF_NULL != ( message = curl_multi_info_read( m_multi, &messagesLeft ) ) )
    {
        if ( CURLMSG_DONE != message->msg )
            continue;

        CCurlHttpClientRequest* request = GUCEF_NULL;
        curl_easy_getinfo( message->easy_handle, CURLINFO_PRIVATE, &request );
        if ( GUCEF_NULL != request )
            request->OnCurlDone( message->data.result );
        else
            curl_multi_remove_handle( m_multi, message->easy_handle );
    }

    // curl can have more to do right away, for example when a connect completed, do not wait for the next period
    if ( m_activeRequests > 0 )
    {
        long timeoutInMs = -1;
        if ( CURLM_OK == curl_multi_timeout( m_multi, &timeoutInMs ) && 0 == timeoutInMs )
            m_pulseGenerator->RequestImmediatePulse();
    }

    m_inPulse = false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HTTPCLIENTCURL */
}; /* namespace WEBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
