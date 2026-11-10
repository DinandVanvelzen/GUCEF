/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#define GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H
#include "gucefWEB_CHttpClientBackendSelection.h"
#define GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H
#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

class CHttpClientRequestRunner;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Notifier through which requests report their progress to the CHTTPClient that started them. It is shared
 *  between the client and its requests so a request that outlives its client can still report safely.
 *  Thread-safe: requests report from the threads they run on, the client observes via its thread-safe observer.
 */
class GUCEF_HIDDEN CHttpClientProgressNotifier : public CORE::CTSGNotifier
{
    public:

    CHttpClientProgressNotifier( void );
    virtual ~CHttpClientProgressNotifier() GUCEF_VIRTUAL_OVERRIDE;

    bool Report( const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef CORE::CTSharedPtr< CHttpClientProgressNotifier, MT::CMutex > CHttpClientProgressNotifierPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Everything about an asynchronous HTTP request started with CHTTPClient::ExecuteAsync(): the request as given
 *  and, once the future is ready, the response and the transfer info. Obtained via
 *  future.GetResult()->GetTaskData(), see CHTTPClient::GetRequestTaskData().
 */
class GUCEF_WEB_PUBLIC_CPP CHttpClientRequestTaskData : public CORE::CICloneable
{
    public:

    CHttpRequestData request;
    CHttpClientRequestOptions options;
    CHttpResponseData response;
    CHttpClientTransferInfo transferInfo;

    /** the backends to use, shared with the client that started the request */
    CHttpClientBackendSelectionPtr backends;

    /** whether the task notifies its progress via events, used by CHTTPClient for its classic event based API */
    bool reportProgress;

    /** identifies the request in its progress events */
    UInt64 progressRequestId;

    /** where the progress events are reported */
    CHttpClientProgressNotifierPtr progressNotifier;

    CHttpClientRequestTaskData( void );
    CHttpClientRequestTaskData( const CHttpClientRequestTaskData& src );
    virtual ~CHttpClientRequestTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Data of the progress events of a request, which fields are filled depends on the event
 */
class GUCEF_HIDDEN CHttpClientRequestProgressData : public CORE::CICloneable
{
    public:

    UInt64 requestId;
    Int32 statusCode;               /**< ResponseHeadersEvent */
    UInt64 contentLength;           /**< ResponseHeadersEvent, 0 if unknown */
    bool resumeSupported;           /**< ResponseHeadersEvent */
    CORE::CDynamicBuffer data;      /**< BodyDataEvent */
    UInt32 byteCount;               /**< DataSentEvent */
    CString url;                    /**< RedirectEvent */
    CHttpClientTransferInfo transferInfo; /**< RequestFinishedEvent */

    CHttpClientRequestProgressData( UInt64 id );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Task consumer which carries out a HTTP request on the thread of the task, on the pulse generator of the
 *  task delegator. Used by CHTTPClient::ExecuteAsync()
 */
class GUCEF_HIDDEN CHttpClientRequestTaskConsumer : public CORE::CTaskConsumer      ,
                                                    public CIHttpClientBackendObserver
{
    public:

    static const CORE::CString TaskType;

    /** progress events, only sent when the task data asks for them. Event data: CHttpClientRequestProgressData */
    static const CORE::CEvent ConnectingEvent;
    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent ResponseHeadersEvent;
    static const CORE::CEvent BodyDataEvent;
    static const CORE::CEvent DataSentEvent;
    static const CORE::CEvent RedirectEvent;
    static const CORE::CEvent RequestFinishedEvent;

    static void RegisterEvents( void );

    CHttpClientRequestTaskConsumer( void );
    virtual ~CHttpClientRequestTaskConsumer() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnding( CORE::CTaskPtr task, bool willBeForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHttpClientConnected( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientResponseHeadersComplete( const CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientBodyData( const void* data, UInt32 dataSize ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientDataSent( UInt32 bytesSent ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnHttpClientRedirect( const CString& newUrl ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CHttpClientRequestTaskConsumer( const CHttpClientRequestTaskConsumer& src );            /**< not supported */
    CHttpClientRequestTaskConsumer& operator=( const CHttpClientRequestTaskConsumer& src ); /**< not supported */

    bool Report( const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CHttpClientRequestRunner* m_runner;
    bool m_reportProgress;
    UInt64 m_progressRequestId;
    CHttpClientProgressNotifierPtr m_progressNotifier;
};

typedef CORE::CTFactory< CORE::CTaskConsumer, CHttpClientRequestTaskConsumer, MT::CMutex > THttpClientRequestTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H ? */
