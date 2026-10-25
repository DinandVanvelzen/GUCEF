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

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#define GUCEF_WEB_CHTTPCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"	     /* TCP Client socket */
#define CTCPCLIENTSOCKET_H
#endif /* CTCPCLIENTSOCKET_H ? */

#ifndef CDVSTRING_H
#include "gucefCORE_CString.h"
#define CDVSTRING_H
#endif /* CDVSTRING_H ? */

#ifndef CVALUELIST_H
#include "CValueList.h"
#define CVALUELIST_H
#endif /* CVALUELIST_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_WEB_HTTPCODE_H
#include "gucefWEB_HTTPCode.h"
#define GUCEF_WEB_HTTPCODE_H
#endif /* GUCEF_WEB_HTTPCODE_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#include "gucefWEB_CHttpClientRequestTask.h"
#define GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H
#endif /* GUCEF_WEB_CHTTPCLIENTREQUESTTASK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  HTTP client. The requests are carried out by a backend: one provided by a plugin, for example using curl
 *  which also supports https, or the native backend of the platform for plain http. See SetPreferredBackends().
 *
 *  Requests are described by a CHttpRequestData plus CHttpClientRequestOptions and can be carried out:
 *  - with Execute() which blocks the calling thread until done and carries out all work on that thread
 *  - with ExecuteAsync() which carries out the request as a task on a thread pool and returns a future
 *  Both do the same work, they only differ in which thread does it. Any number of requests can be in progress.
 *
 *  The classic API (Get, Post and the events) is still available, it carries out one request at a time as a
 *  task and raises its events on the thread of the pulse generator of the client.
 *
 *  The client applies redirects, cookies (see SetCookieJar()), byte ranges and timeouts alike for all backends.
 */
class GUCEF_WEB_PUBLIC_CPP CHTTPClient : public CORE::CTSGNotifier
{
	public:

    static const CORE::CEvent ConnectingEvent;
    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent DisconnectedEvent;
    static const CORE::CEvent ConnectionErrorEvent;
    static const CORE::CEvent HTTPErrorEvent;
    static const CORE::CEvent HTTPRedirectEvent;
    static const CORE::CEvent HTTPContentEvent;
    static const CORE::CEvent HTTPDataRecievedEvent;
    static const CORE::CEvent HTTPDataSentEvent;
    static const CORE::CEvent HTTPTransferFinishedEvent;

	static void RegisterEvents( void );

	public:

    struct SHTTPContentEventData   /**< stores data about the HTTP transmission payload */
    {
        UInt32 contentSize;        /**< the size of the content in the HTTP payload in bytes */
        bool resumeSupported;      /**< does the remote host support resume ? */
        THTTPCODE HTTPcode;        /**< HTTP code for the given content */
    };

    typedef CORE::CTCloneableObj< struct SHTTPContentEventData >  THTTPContentEventData;      /**< see struct SHTTPContentEventData for details */
    typedef CORE::TLinkedCloneableBuffer                          THTTPDataRecievedEventData; /**< contains buffer with the recieved data */
    typedef CORE::TLinkedCloneableBuffer                          THTTPDataSendEventData;     /**< contains buffer with the dispatched data */
    typedef CORE::TCloneableString                                THTTPRedirectEventData;     /**< contains the URL to which the client is beeing redirected */
    typedef CORE::CTCloneableObj< THTTPCODE >                     THTTPErrorEventData;        /**< contains the relevant HTTP error code */
    typedef COMCORE::CTCPClientSocket::TSocketErrorEventData      TConnectionErrorEventData;  /**< error code */

	public:

    /** events of the classic API are raised on the thread of the global pulse generator */
    CHTTPClient( void );

    /** events of the classic API are raised on the thread of the given pulse generator */
    CHTTPClient( CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CHTTPClient();

    /**
     *  Carries out the request on the calling thread, blocking it until the request is done
     *
     *  @param transferInfo optional, receives the details of how the request was carried out
     *  @return whether a response was received, also true for responses with an error status code
     */
    bool Execute( const CHttpRequestData& request          ,
                  const CHttpClientRequestOptions& options ,
                  CHttpResponseData& response              ,
                  CHttpClientTransferInfo* transferInfo = GUCEF_NULL );

    /** as above with the default options of the client */
    bool Execute( const CHttpRequestData& request ,
                  CHttpResponseData& response     );

    /**
     *  Carries out the request as a task on the thread pool of the client
     *
     *  @return the future of the request. Once ready the task data holds the response and transfer info,
     *          see GetRequestTaskData(). Cancelling the future cancels the request.
     */
    CORE::CFutureResult ExecuteAsync( const CHttpRequestData& request          ,
                                      const CHttpClientRequestOptions& options );

    /** as above with the default options of the client */
    CORE::CFutureResult ExecuteAsync( const CHttpRequestData& request );

    /**
     *  @return the request, response and transfer info of a request started with ExecuteAsync(), from the task obtained
     *          via the future of the request (future.GetResult()). Remains valid while the task is referenced.
     */
    static CHttpClientRequestTaskData* GetRequestTaskData( CORE::CTaskPtr task );

    /** the thread pool used by ExecuteAsync() and the classic API, by default the default thread pool of the task manager */
    void SetThreadPool( CORE::ThreadPoolPtr threadPool );

    /** use the thread pool with the given name, created if needed */
    void SetThreadPoolName( const CString& threadPoolName );

    CORE::ThreadPoolPtr GetThreadPool( void ) const;

    /** the names of the backends to use in order of preference, the first that supports the scheme of a URL is used */
    void SetPreferredBackends( const CString::StringVector& backendNames );

    CString::StringVector GetPreferredBackends( void ) const;

    /** @return the name of the backend that would carry out a request for the given scheme, empty if none */
    CString GetBackendNameForScheme( const CString& scheme ) const;

    /**
     *  Whether a request for the scheme ("http", "https") can be carried out at this moment by the default backends:
     *  the registered ones, which for https means the native backend has a TLS provider (on Linux the OpenSSL of the system), or a plugin
     *  such as httpclientpluginCURL has been loaded (the WinHTTP backend is part of the build on Windows). Nothing is logged when it cannot.
     *  Asked again later it can give another answer when plugins were loaded or unloaded in between.
     */
    static bool IsSchemeSupported( const CString& scheme );

    /**
     *  Why a request for the scheme cannot be carried out, for a message to the user. For https that is "no HTTPS support:" followed by
     *  what the TLS provider says, for example the library that cannot be found. Meaningful when IsSchemeSupported() is false
     */
    static CString DescribeUnsupportedScheme( const CString& scheme );

    /** The default backends that are registered at this moment, as a line for a log: "native, curl (https)" */
    static CString DescribeAvailableBackends( void );

    /** cookies for requests whose options have no cookie jar of their own, none by default */
    void SetCookieJar( CHttpCookieJarPtr cookieJar );

    CHttpCookieJarPtr GetCookieJar( void ) const;

    /** options used by the overloads without options and by the classic API */
    void SetDefaultOptions( const CHttpClientRequestOptions& options );

    const CHttpClientRequestOptions& GetDefaultOptions( void ) const;

    /**
     *  Classic API: starts a POST request with the value list as content
     */
    bool Post( const CORE::CString& host                               ,
               UInt16 port                                             ,
               const CORE::CString& path                               ,
               const CORE::CValueList* valuelistAsContent = GUCEF_NULL ,
               const CORE::CString& contentType = "text/plain"         );

    bool Post( const CORE::CString& urlstring                          ,
               const CORE::CValueList* valuelistAsContent = GUCEF_NULL ,
               const CORE::CString& contentType = "text/plain"         );

    bool Post( const CORE::CString& urlstring      ,
               const CORE::CString& contentType    ,
               const CORE::CDynamicBuffer& payload );

    bool Post( const CORE::CString& host           ,
               UInt16 port                         ,
               const CORE::CString& path           ,
               const CORE::CString& contentType    ,
               const CORE::CDynamicBuffer& payload );

    /**
     *  Classic API: starts a GET request, from the given byte offset if not 0
     */
    bool Get( const CORE::CString& host                ,
              UInt16 port                              ,
              const CORE::CString& path                ,
              const UInt32 byteoffset = 0              ,
              const CORE::CValueList* valuelist = NULL );

    bool Get( const CORE::CString& urlstring           ,
              const UInt32 byteoffset = 0              ,
              const CORE::CValueList* valuelist = NULL );

    /** Classic API: cancels the current request */
    void Close( void );

    /** Classic API: whether the current request is connected */
    bool IsConnected( void ) const;

    UInt32 GetBytesReceived( bool resetCounter );

    UInt32 GetBytesTransmitted( bool resetCounter );

    bool SetHTTPProxy( const CORE::CString& proxyHost ,
                       const UInt16 port = 80         );

    void GetHTTPProxy( CORE::CString& proxyHost ,
                       UInt16& port             );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPumpedNotify( CORE::CNotifier* notifier                 ,
                                 const CORE::CEvent& eventid               ,
                                 CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CHTTPClient( const CHTTPClient& src );
    CHTTPClient& operator=( const CHTTPClient& src );

    CHttpClientRequestOptions PrepareOptions( const CHttpClientRequestOptions& options ) const;

    CORE::CFutureResult QueueRequest( const CHttpRequestData& request          ,
                                      const CHttpClientRequestOptions& options ,
                                      bool reportProgress                      ,
                                      UInt64 progressRequestId                 );

    bool StartClassicRequest( const CHttpRequestData& request, UInt32 byteOffset );

    static CString BuildUrl( const CORE::CString& host, UInt16 port, const CORE::CString& path, const CORE::CValueList* valuelist );

    CHttpClientBackendSelectionPtr m_backends;
    CHttpClientProgressNotifierPtr m_progressNotifier;
    CORE::ThreadPoolPtr m_threadPool;
    CString m_threadPoolName;
    CHttpCookieJarPtr m_cookieJar;
    CHttpClientRequestOptions m_defaultOptions;
    CORE::CString m_proxyHost;
    UInt16 m_proxyPort;

    CORE::CFutureResult* m_classicRequest;
    UInt64 m_classicRequestId;
    bool m_classicRequestActive;
    bool m_classicConnected;
    bool m_classicErrorReported;
    UInt32 m_bytesReceived;
    UInt32 m_bytesTransmitted;
    mutable MT::CMutex m_settingsLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 03-03-2007 :
        - Dinand: changed the code a bit so it also works for multi-part
          transmissions with the Apache server
- 07-02-2007 :
        - Dinand: Added basic HTTP proxy support
- 08-11-2006 :
        - Dinand: Updated notification system usage to the latest revision
- 07-10-2006 :
        - Dinand: Converted class to a notification based event handling system
          instead of the customized event handler interface classes.
- 18-09-2005 :
        - Dinand: Completed switch to a Post/Get based system
- 09-08-2005 :
        - Dinand: Class renamed to CHTTPClient
        - Dinand: Interface and code modified, only loosely based on the previous
          version called CHTTPDownload.
- 31-05-2005 :
        - Dinand: Commenced integration into new GUCEF codebase
- 19-04-2004 :
        - Dinand: If a file is not found (404) error there was no way to detect it.
          Thus i changed the OnError() event handler to include an error code.
          The error code given is one of the new enum values.
        - Dinand: Got rid of the use of atoi() because it's not always available.
        - Dinand: Added external include guards.
        - Dinand: Added HTTP_Code() member function.
- 19-12-2003 :
        - Dinand: Changed handling of OnDisconnect event for the socket.
          This event generated by the socket will now always cause the
          OnDisconnect to be called of the interface. This may or may not be
          preceded with an OnFinish event.
        - Dinand: Fixed a bug in error code debug output.
        - Dinand: Replaced printf() debugging output with the tsprintf() version.
- 05-11-2003 :
        - Dinand: Created overloaded versions of the Download() member function.
          In order to accomplish this some functions call other overloaded
          versions. Only accepting a complete URL is a bit of a drag if all you
          want to change is the file to download in an entire batch of downloads
        - Dinand: Made several inlined get functions const
        - Dinand: Removed some unneeded return values.
        - Dinand: Removed several data members which I replaced with a local variable
          in the relevant member functions.
- 02-11-2003 :
        - Dinand: Received initial version of this class from Logan Benjamin. Many
          thanks to him for contributing this class to the DVNETCOM codebase.
        - Dinand: Modified the file layout to conform with my latest standard.

---------------------------------------------------------------------------*/
