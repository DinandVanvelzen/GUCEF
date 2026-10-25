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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefWEB_CWinHttpClientBackend.h"

#ifdef GUCEF_MSWIN_BUILD

#include <string.h>
#include <deque>
#include <vector>
#include <string>

#include <windows.h>
#include <winhttp.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H
#include "gucefWEB_CHttpClientBackendRequestBase.h"
#define GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H
#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H ? */

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

const CString CWinHttpClientBackend::BackendName = "winhttp";

static const UInt32 PulseIntervalInMs = 10;
static const UInt32 ReadBlockSize = 64 * 1024;
static const UInt32 WriteBlockSize = 64 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The WinHTTP session, shared by all requests of a backend. It outlives the backend for as long as
 *  requests need it.
 */
class CWinHttpSession
{
    public:

    CWinHttpSession( void )
        : m_session( NULL )
    {GUCEF_TRACE;

        // Asynchronous, which means every operation reports its completion via a callback instead of blocking.
        // Automatic proxy detection needs Windows 8.1, older versions use the proxy settings of the system
        m_session = WinHttpOpen( L"GUCEF", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC );
        if ( NULL == m_session )
            m_session = WinHttpOpen( L"GUCEF", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC );
        if ( NULL == m_session )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WinHttpClientBackend: WinHttpOpen() failed with error " + CORE::UInt32ToString( (UInt32) GetLastError() ) );
            return;
        }

        // The same floor as everywhere else: TLS 1.2 and newer. Without this the protocols the system has enabled for WinHTTP are used, which on
        // systems that still have TLS 1.0 and 1.1 switched on for WinHTTP includes those. Not verified on Windows: where the system does not know
        // the flags (Windows 7 without the update for TLS 1.2) the option is refused and the defaults of the system stay
        DWORD secureProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        #ifdef WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3
        secureProtocols |= WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
        #endif
        if ( !WinHttpSetOption( m_session, WINHTTP_OPTION_SECURE_PROTOCOLS, &secureProtocols, sizeof( secureProtocols ) ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WinHttpClientBackend: The minimum protocol of TLS 1.2 could not be set, error " << (UInt32) GetLastError() << ": the defaults of the system apply" );
    }

    ~CWinHttpSession()
    {GUCEF_TRACE;

        if ( NULL != m_session )
            WinHttpCloseHandle( m_session );
        m_session = NULL;
    }

    HINTERNET GetHandle( void ) const { return m_session; }

    private:

    CWinHttpSession( const CWinHttpSession& src );
    CWinHttpSession& operator=( const CWinHttpSession& src );

    HINTERNET m_session;
};

typedef CORE::CTSharedPtr< CWinHttpSession, MT::CMutex > TWinHttpSessionPtr;

/*-------------------------------------------------------------------------*/

enum EWinHttpEventType
{
    WINHTTPEVENT_CONNECTED = 0      ,
    WINHTTPEVENT_SENDREQUEST_DONE   ,
    WINHTTPEVENT_HEADERS_AVAILABLE  ,
    WINHTTPEVENT_DATA_AVAILABLE     ,   /**< value: number of bytes available to read */
    WINHTTPEVENT_READ_DONE          ,   /**< value: number of bytes read into the read buffer */
    WINHTTPEVENT_WRITE_DONE         ,   /**< value: number of bytes written */
    WINHTTPEVENT_SECURE_FAILURE     ,   /**< value: the WINHTTP_CALLBACK_STATUS_FLAG_ flags */
    WINHTTPEVENT_ERROR                  /**< value: the operation that failed, error: the error code */
};

struct SWinHttpEvent
{
    EWinHttpEventType type;
    DWORD value;
    DWORD error;
};

/**
 *  What WinHTTP threads and the thread of the request share. WinHTTP threads note down events, the request
 *  handles them on the pulses of its pulse generator. It can live longer than the request, since WinHTTP can still
 *  report something while a handle is being closed.
 */
class CWinHttpRequestState
{
    public:

    CWinHttpRequestState( CORE::PulseGeneratorPtr pulseGenerator )
        : m_lock()
        , m_events()
        , m_pulseGenerator( pulseGenerator )
        , m_readBuffer( ReadBlockSize )
    {GUCEF_TRACE;

    }

    /** called from any WinHTTP thread */
    void Push( EWinHttpEventType type, DWORD value, DWORD error )
    {GUCEF_TRACE;

        SWinHttpEvent event;
        event.type = type;
        event.value = value;
        event.error = error;
        {
            MT::CScopeMutex lock( m_lock );
            m_events.push_back( event );
        }
        // wakes up the pulse generator of the request
        m_pulseGenerator->RequestImmediatePulse();
    }

    void Take( std::deque< SWinHttpEvent >& events )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( m_lock );
        events.swap( m_events );
    }

    char* GetReadBuffer( void ) { return &m_readBuffer[ 0 ]; }

    private:

    CWinHttpRequestState( const CWinHttpRequestState& src );
    CWinHttpRequestState& operator=( const CWinHttpRequestState& src );

    MT::CMutex m_lock;
    std::deque< SWinHttpEvent > m_events;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    std::vector< char > m_readBuffer;
};

typedef CORE::CTSharedPtr< CWinHttpRequestState, MT::CMutex > TWinHttpRequestStatePtr;

/*-------------------------------------------------------------------------*/

/**
 *  A single HTTP exchange carried out by WinHTTP. The WinHTTP calls are made from the thread of the pulse
 *  generator and complete asynchronously, which is reported as events that are handled in Update().
 */
class CWinHttpRequest : public CHttpClientBackendRequestBase ,
                        public CORE::CObserver
{
    public:

    CWinHttpRequest( TWinHttpSessionPtr session                  ,
                     const CHttpRequestData& request             ,
                     const CHttpClientRequestOptions& options    ,
                     CHttpResponseData& response                 ,
                     CHttpClientTransferInfo& transferInfo       ,
                     CIHttpClientBackendObserver& observer       )
        : CHttpClientBackendRequestBase( request, options, response, transferInfo, observer )
        , CORE::CObserver()
        , m_session( session )
        , m_state()
        , m_pulseGenerator()
        , m_winConnection( NULL )
        , m_winRequest( NULL )
        , m_context( NULL )
        , m_pulsesRequested( false )
        , m_startTicks( 0 )
        , m_uploadSize( 0 )
        , m_uploadedBytes( 0 )
        , m_bodyInMemory( false )
        , m_bodyBuffer()
        , m_writeBuffer()
        , m_secureFailureFlags( 0 )
    {GUCEF_TRACE;

    }

    virtual ~CWinHttpRequest() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        StopAndClose();
    }

    /** sets up the WinHTTP request and sends it. On failure the request is finished */
    bool Start( CORE::PulseGeneratorPtr pulseGenerator )
    {GUCEF_TRACE;

        m_pulseGenerator = pulseGenerator;
        m_startTicks = MT::PrecisionTickCount();

        if ( NULL == m_session->GetHandle() )
        {
            Finish( false, "No WinHTTP session available" );
            return false;
        }

        CString hostName = m_request.GetHostName();
        if ( hostName.IsNULLOrEmpty() )
        {
            Finish( false, "The request has no host to connect to" );
            return false;
        }
        bool isSecure = m_request.requestProtocol.Equals( "https", false );

        // what we ask for: the path and query
        CString target = m_request.requestUri;
        if ( target.IsNULLOrEmpty() || '/' != target[ 0 ] )
            target = "/" + target;
        if ( !m_request.requestUriParams.IsNULLOrEmpty() )
            target += "?" + m_request.requestUriParams;

        m_winConnection = WinHttpConnect( m_session->GetHandle(), CORE::ToWString( hostName ).c_str(), (INTERNET_PORT) m_request.GetPort(), 0 );
        if ( NULL == m_winConnection )
        {
            Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
            return false;
        }

        m_winRequest = WinHttpOpenRequest( m_winConnection, CORE::ToWString( GetRequestMethod() ).c_str(), CORE::ToWString( target ).c_str(),
                                        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, isSecure ? WINHTTP_FLAG_SECURE : 0 );
        if ( NULL == m_winRequest )
        {
            Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
            return false;
        }

        // Callbacks arrive on WinHTTP threads and note down what happens in a state that can outlive us.
        // The state is referenced via the context, which is released when WinHTTP is done with the handle
        m_state = TWinHttpRequestStatePtr( GUCEF_NEW CWinHttpRequestState( pulseGenerator ) );
        TWinHttpRequestStatePtr* contextPtr = GUCEF_NEW TWinHttpRequestStatePtr( m_state );
        DWORD_PTR context = reinterpret_cast< DWORD_PTR >( contextPtr );
        if ( !WinHttpSetOption( m_winRequest, WINHTTP_OPTION_CONTEXT_VALUE, &context, sizeof( context ) ) )
        {
            DWORD error = GetLastError();
            GUCEF_DELETE contextPtr;
            Finish( false, DescribeError( error ), (Int32) error );
            return false;
        }
        m_context = contextPtr;

        if ( WINHTTP_INVALID_STATUS_CALLBACK == WinHttpSetStatusCallback( m_winRequest, StatusCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0 ) )
        {
            // without a callback nothing will release the context for us
            DWORD error = GetLastError();
            GUCEF_DELETE m_context;
            m_context = NULL;
            Finish( false, DescribeError( error ), (Int32) error );
            return false;
        }

        if ( !SetupRequest() )
            return false;

        // from here on we run on the pulses
        CORE::CTEventHandlerFunctor< CWinHttpRequest > callback( this, &CWinHttpRequest::OnPulse );
        SubscribeTo( m_pulseGenerator.GetPointerAlways() ,
                     CORE::CPulseGenerator::PulseEvent   ,
                     callback                            );
        m_pulsesRequested = true;
        m_pulseGenerator->RequestPeriodicPulses( this, PulseIntervalInMs );

        return SendRequest();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "GUCEF::WEB::CWinHttpRequest";
        return classTypeName;
    }

    protected:

    virtual void OnFinish( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        StopAndClose();
    }

    virtual void OnNotify( CORE::CNotifier* notifier     ,
                           const CORE::CEvent& eventId   ,
                           CORE::CICloneable* eventData  ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

    }

    private:

    CWinHttpRequest( void );                                    /**< not implemented */
    CWinHttpRequest( const CWinHttpRequest& src );              /**< not implemented */
    CWinHttpRequest& operator=( const CWinHttpRequest& src );   /**< not implemented */

    /** Text for an error code of WinHTTP or Windows */
    /** the kind of failure a WinHTTP error stands for, a failed secure connection is told by the flags of the secure failure notification */
    static THttpTransferErrorClass ClassifyError( DWORD error, DWORD secureFailureFlags )
    {GUCEF_TRACE;

        if ( 0 != secureFailureFlags )
            return HTTPERRORCLASS_TLS;
        switch ( error )
        {
            case ERROR_WINHTTP_NAME_NOT_RESOLVED :          return HTTPERRORCLASS_DNS;
            case ERROR_WINHTTP_CANNOT_CONNECT :
            case ERROR_WINHTTP_CONNECTION_ERROR :           return HTTPERRORCLASS_CONNECT;
            case ERROR_WINHTTP_SECURE_FAILURE :
            case ERROR_WINHTTP_SECURE_CHANNEL_ERROR :
            case ERROR_WINHTTP_SECURE_INVALID_CA :
            case ERROR_WINHTTP_SECURE_INVALID_CERT :
            case ERROR_WINHTTP_SECURE_CERT_REVOKED :
            case ERROR_WINHTTP_SECURE_CERT_WRONG_USAGE :
            case ERROR_WINHTTP_SECURE_CERT_CN_INVALID :
            case ERROR_WINHTTP_SECURE_CERT_DATE_INVALID :   return HTTPERRORCLASS_TLS;
            case ERROR_WINHTTP_TIMEOUT :                    return HTTPERRORCLASS_TIMEOUT;
            default :                                       return HTTPERRORCLASS_OTHER;
        }
    }

    static CString DescribeError( DWORD error )
    {GUCEF_TRACE;

        wchar_t buffer[ 512 ];
        DWORD length = FormatMessageW( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       GetModuleHandleW( L"winhttp.dll" ), error, 0, buffer, sizeof( buffer ) / sizeof( wchar_t ), NULL );
        CString description = "WinHTTP error " + CORE::UInt32ToString( (UInt32) error );
        if ( length > 0 )
        {
            // the system terminates its messages with a line end
            std::wstring text( buffer, length );
            while ( !text.empty() && ( L'\r' == text[ text.size()-1 ] || L'\n' == text[ text.size()-1 ] || L' ' == text[ text.size()-1 ] ) )
                text.erase( text.size()-1 );
            description += ": " + CORE::ToUtf8String( text );
        }
        return description;
    }

    /** Text for the reasons a certificate was not accepted */
    static CString DescribeSecureFailure( DWORD flags )
    {GUCEF_TRACE;

        CString description;
        if ( 0 != ( flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REV_FAILED ) )        description += " certificate revocation check failed;";
        if ( 0 != ( flags & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CERT ) )           description += " certificate is invalid;";
        if ( 0 != ( flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_REVOKED ) )           description += " certificate was revoked;";
        if ( 0 != ( flags & WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA ) )             description += " certificate authority is not trusted;";
        if ( 0 != ( flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_CN_INVALID ) )        description += " certificate is for another host;";
        if ( 0 != ( flags & WINHTTP_CALLBACK_STATUS_FLAG_CERT_DATE_INVALID ) )      description += " certificate expired or is not valid yet;";
        if ( 0 != ( flags & WINHTTP_CALLBACK_STATUS_FLAG_SECURITY_CHANNEL_ERROR ) ) description += " security channel error;";
        return description;
    }

    static void CALLBACK StatusCallback( HINTERNET handle                ,
                                         DWORD_PTR context               ,
                                         DWORD internetStatus            ,
                                         LPVOID statusInformation        ,
                                         DWORD statusInformationLength   )
    {
        // Note that this runs on a thread of WinHTTP and must not touch the request itself, only its state
        if ( 0 == context )
            return;

        TWinHttpRequestStatePtr* statePtr = reinterpret_cast< TWinHttpRequestStatePtr* >( context );
        CWinHttpRequestState* state = statePtr->GetPointerAlways();
        switch ( internetStatus )
        {
            case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER :
            {
                state->Push( WINHTTPEVENT_CONNECTED, 0, 0 );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE :
            {
                state->Push( WINHTTPEVENT_SENDREQUEST_DONE, 0, 0 );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE :
            {
                state->Push( WINHTTPEVENT_HEADERS_AVAILABLE, 0, 0 );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE :
            {
                state->Push( WINHTTPEVENT_DATA_AVAILABLE, NULL != statusInformation ? *static_cast< DWORD* >( statusInformation ) : 0, 0 );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_READ_COMPLETE :
            {
                state->Push( WINHTTPEVENT_READ_DONE, statusInformationLength, 0 );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE :
            {
                state->Push( WINHTTPEVENT_WRITE_DONE, NULL != statusInformation ? *static_cast< DWORD* >( statusInformation ) : 0, 0 );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE :
            {
                state->Push( WINHTTPEVENT_SECURE_FAILURE, NULL != statusInformation ? *static_cast< DWORD* >( statusInformation ) : 0, 0 );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR :
            {
                const WINHTTP_ASYNC_RESULT* result = static_cast< const WINHTTP_ASYNC_RESULT* >( statusInformation );
                if ( NULL != result )
                    state->Push( WINHTTPEVENT_ERROR, (DWORD) result->dwResult, result->dwError );
                else
                    state->Push( WINHTTPEVENT_ERROR, 0, ERROR_WINHTTP_INTERNAL_ERROR );
                break;
            }
            case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING :
            {
                // The last callback of the handle, nothing will use the context after this
                GUCEF_DELETE statePtr;
                break;
            }
            default :
            {
                break;
            }
        }
    }

    /** The options of the WinHTTP request: what we do ourselves is switched off in WinHTTP */
    bool SetupRequest( void )
    {GUCEF_TRACE;

        // exactly one exchange per request, the redirects, cookies and authentication are done for all backends alike
        DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
        WinHttpSetOption( m_winRequest, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof( redirectPolicy ) );
        DWORD disabledFeatures = WINHTTP_DISABLE_COOKIES | WINHTTP_DISABLE_REDIRECTS | WINHTTP_DISABLE_AUTHENTICATION;
        WinHttpSetOption( m_winRequest, WINHTTP_OPTION_DISABLE_FEATURE, &disabledFeatures, sizeof( disabledFeatures ) );

        if ( !m_options.verifyPeer )
        {
            DWORD securityFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                                  SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
            WinHttpSetOption( m_winRequest, WINHTTP_OPTION_SECURITY_FLAGS, &securityFlags, sizeof( securityFlags ) );
        }

        if ( !m_options.proxyHost.IsNULLOrEmpty() )
        {
            std::wstring proxy = CORE::ToWString( m_options.proxyHost + ":" + CORE::UInt32ToString( (UInt32) m_options.proxyPort ) );
            WINHTTP_PROXY_INFO proxyInfo;
            proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            proxyInfo.lpszProxy = const_cast< LPWSTR >( proxy.c_str() );
            proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
            WinHttpSetOption( m_winRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof( proxyInfo ) );
        }

        // A time out of 0 would mean no waiting at all to WinHTTP, whereas for us it is unlimited
        int connectTimeout = 0 == m_options.connectTimeoutInMs ? 0 : (int) m_options.connectTimeoutInMs;
        int receiveTimeout = 0 == m_options.totalTimeoutInMs ? 0 : (int) m_options.totalTimeoutInMs;
        WinHttpSetTimeouts( m_winRequest, 0, connectTimeout, receiveTimeout, receiveTimeout );

        // The header fields as given: WinHTTP derives the Host, and for a body the Content-Length, by itself
        CString::StringVector fields;
        GetRequestHeaderFields( fields );
        std::wstring headers;
        for ( CString::StringVector::iterator i = fields.begin(); i != fields.end(); ++i )
            headers += CORE::ToWString( (*i) ) + L"\r\n";
        if ( !headers.empty() )
        {
            if ( !WinHttpAddRequestHeaders( m_winRequest, headers.c_str(), (DWORD) -1L, WINHTTP_ADDREQ_FLAG_ADD ) )
            {
                Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
                return false;
            }
        }

        return true;
    }

    bool SendRequest( void )
    {GUCEF_TRACE;

        m_uploadSize = 0;
        m_uploadedBytes = 0;
        if ( HasRequestBody() )
        {
            if ( IsRequestBodySizeKnown() )
            {
                UInt64 size = GetRequestBodySize();
                if ( size > 0xFFFFFFFFULL )
                {
                    Finish( false, "A request body of more than 4GB cannot be sent using WinHTTP" );
                    return false;
                }
                m_uploadSize = (DWORD) size;
            }
            else
            {
                // WinHTTP wants to know the size of the body, so a body of unknown size is read in its entirety first
                char block[ 16 * 1024 ];
                UInt32 bytesRead = 0;
                while ( 0 != ( bytesRead = ReadRequestBody( block, sizeof( block ) ) ) )
                    m_bodyBuffer.insert( m_bodyBuffer.end(), block, block + bytesRead );
                if ( m_bodyBuffer.size() > 0xFFFFFFFFULL )
                {
                    Finish( false, "A request body of more than 4GB cannot be sent using WinHTTP" );
                    return false;
                }
                m_bodyInMemory = true;
                m_uploadSize = (DWORD) m_bodyBuffer.size();
            }
        }

        // the body, if any, follows when WinHTTP has sent the request and is ready for it
        if ( !WinHttpSendRequest( m_winRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, m_uploadSize, reinterpret_cast< DWORD_PTR >( m_context ) ) )
        {
            Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
            return false;
        }
        return true;
    }

    /** Writes the next block of the request body, or asks for the response when all of it was sent */
    void ContinueUpload( void )
    {GUCEF_TRACE;

        if ( m_uploadedBytes >= m_uploadSize )
        {
            ReceiveResponse();
            return;
        }

        m_writeBuffer.resize( WriteBlockSize );
        UInt32 bytesToWrite = 0;
        if ( m_bodyInMemory )
        {
            UInt64 left = m_uploadSize - m_uploadedBytes;
            bytesToWrite = left > WriteBlockSize ? WriteBlockSize : (UInt32) left;
            memcpy( &m_writeBuffer[ 0 ], &m_bodyBuffer[ (size_t) m_uploadedBytes ], bytesToWrite );
        }
        else
        {
            bytesToWrite = ReadRequestBody( &m_writeBuffer[ 0 ], WriteBlockSize );
            if ( bytesToWrite > m_uploadSize - m_uploadedBytes )
                bytesToWrite = m_uploadSize - (DWORD) m_uploadedBytes;      // more than announced is not sent
        }

        if ( 0 == bytesToWrite )
        {
            Finish( false, "The request body ended before the announced size was reached" );
            return;
        }
        if ( !WinHttpWriteData( m_winRequest, &m_writeBuffer[ 0 ], bytesToWrite, NULL ) )
            Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
    }

    void ReceiveResponse( void )
    {GUCEF_TRACE;

        if ( !WinHttpReceiveResponse( m_winRequest, NULL ) )
            Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
    }

    /** The status line and header fields are in, hand them over to the base which maps them onto the response */
    void OnHeadersAvailable( void )
    {GUCEF_TRACE;

        DWORD size = 0;
        WinHttpQueryHeaders( m_winRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &size, WINHTTP_NO_HEADER_INDEX );
        if ( ERROR_INSUFFICIENT_BUFFER != GetLastError() || 0 == size )
        {
            Finish( false, "Unable to obtain the response headers: " + DescribeError( GetLastError() ), (Int32) GetLastError() );
            return;
        }

        std::vector< wchar_t > buffer( ( size / sizeof( wchar_t ) ) + 1, L'\0' );
        if ( !WinHttpQueryHeaders( m_winRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, &buffer[ 0 ], &size, WINHTTP_NO_HEADER_INDEX ) )
        {
            Finish( false, "Unable to obtain the response headers: " + DescribeError( GetLastError() ), (Int32) GetLastError() );
            return;
        }

        // the status line followed by the header fields, each ending in a line end
        CString rawHeaders = CORE::ToUtf8String( std::wstring( &buffer[ 0 ] ) );
        CString::StringVector lines = rawHeaders.ParseElements( '\n', false );
        bool isStatusLine = true;
        for ( CString::StringVector::iterator i = lines.begin(); i != lines.end(); ++i )
        {
            const CString& line = (*i);
            if ( line.IsNULLOrEmpty() || ( 1 == line.Length() && '\r' == line[ 0 ] ) )
                continue;
            if ( isStatusLine )
            {
                if ( !OnResponseStatusLine( line.C_String(), line.Length() ) )
                {
                    Finish( false, "The server did not reply with a HTTP response" );
                    return;
                }
                isStatusLine = false;
            }
            else
            {
                OnResponseHeaderLine( line.C_String(), line.Length() );
            }
        }
        OnResponseHeadersComplete();
        if ( IsFinished() )
            return;

        if ( IsHeadRequest() )
            Finish( true, CString::Empty );
        else
            QueryData();
    }

    void QueryData( void )
    {GUCEF_TRACE;

        if ( !WinHttpQueryDataAvailable( m_winRequest, NULL ) )
            Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
    }

    void OnDataAvailable( DWORD bytesAvailable )
    {GUCEF_TRACE;

        if ( 0 == bytesAvailable )
        {
            // all of the body was received
            Finish( true, CString::Empty );
            return;
        }

        DWORD bytesToRead = bytesAvailable > ReadBlockSize ? ReadBlockSize : bytesAvailable;
        if ( !WinHttpReadData( m_winRequest, m_state->GetReadBuffer(), bytesToRead, NULL ) )
            Finish( false, DescribeError( GetLastError() ), (Int32) GetLastError() );
    }

    void OnReadDone( DWORD bytesRead )
    {GUCEF_TRACE;

        if ( 0 == bytesRead )
        {
            Finish( true, CString::Empty );
            return;
        }

        OnDataReceived( bytesRead );
        if ( !OnResponseBody( m_state->GetReadBuffer(), bytesRead ) )
            return;     // the request was finished by the base
        QueryData();
    }

    void OnWriteDone( DWORD bytesWritten )
    {GUCEF_TRACE;

        m_uploadedBytes += bytesWritten;
        OnDataSent( bytesWritten );
        ContinueUpload();
    }

    void HandleEvent( const SWinHttpEvent& event )
    {GUCEF_TRACE;

        switch ( event.type )
        {
            case WINHTTPEVENT_CONNECTED :           { OnConnected(); break; }
            case WINHTTPEVENT_SENDREQUEST_DONE :
            {
                if ( HasRequestBody() && m_uploadSize > 0 )
                    ContinueUpload();
                else
                    ReceiveResponse();
                break;
            }
            case WINHTTPEVENT_HEADERS_AVAILABLE :   { OnHeadersAvailable(); break; }
            case WINHTTPEVENT_DATA_AVAILABLE :      { OnDataAvailable( event.value ); break; }
            case WINHTTPEVENT_READ_DONE :           { OnReadDone( event.value ); break; }
            case WINHTTPEVENT_WRITE_DONE :          { OnWriteDone( event.value ); break; }
            case WINHTTPEVENT_SECURE_FAILURE :      { m_secureFailureFlags |= event.value; break; }
            case WINHTTPEVENT_ERROR :
            {
                CString description = DescribeError( event.error );
                if ( 0 != m_secureFailureFlags )
                    description += ", secure connection failed:" + DescribeSecureFailure( m_secureFailureFlags );
                m_transferInfo.timedOut = ERROR_WINHTTP_TIMEOUT == event.error;
                m_transferInfo.errorClass = ClassifyError( event.error, m_secureFailureFlags );
                Finish( false, description, (Int32) event.error );
                break;
            }
        }
    }

    void OnPulse( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData )
    {GUCEF_TRACE;

        if ( IsFinished() || m_state.IsNULL() )
            return;

        // Everything WinHTTP reported since the last pulse, handling an event can start the next operation
        std::deque< SWinHttpEvent > events;
        m_state->Take( events );
        while ( !events.empty() && !IsFinished() )
        {
            HandleEvent( events.front() );
            events.pop_front();
        }

        // The time out for the whole request is ours, WinHTTP only knows time outs per operation
        if ( !IsFinished() && m_options.totalTimeoutInMs > 0 )
        {
            Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
            if ( ( MT::PrecisionTickCount() - m_startTicks ) / ticksPerMs > m_options.totalTimeoutInMs )
            {
                m_transferInfo.timedOut = true;
                Finish( false, "The request did not complete within " + CORE::UInt32ToString( m_options.totalTimeoutInMs ) + "ms", (Int32) ERROR_WINHTTP_TIMEOUT );
            }
        }
    }

    /** Stops the pulses and lets go of the WinHTTP handles, which cancels what is still going on */
    void StopAndClose( void )
    {GUCEF_TRACE;

        if ( m_pulsesRequested && !m_pulseGenerator.IsNULL() )
        {
            m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
            m_pulsesRequested = false;
        }
        UnsubscribeAllFromObserver();

        // Closing a handle with an operation in progress cancels the operation. WinHTTP reports
        // that and the closing of the handle through the callback, which releases the context.
        // Those only note down events in the state, so it does not matter that we are going away
        if ( NULL != m_winRequest )
        {
            HINTERNET request = m_winRequest;
            m_winRequest = NULL;
            WinHttpCloseHandle( request );
        }
        if ( NULL != m_winConnection )
        {
            HINTERNET connection = m_winConnection;
            m_winConnection = NULL;
            WinHttpCloseHandle( connection );
        }
    }

    private:

    TWinHttpSessionPtr m_session;
    TWinHttpRequestStatePtr m_state;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    HINTERNET m_winConnection;
    HINTERNET m_winRequest;
    TWinHttpRequestStatePtr* m_context;
    bool m_pulsesRequested;
    UInt64 m_startTicks;
    DWORD m_uploadSize;
    UInt64 m_uploadedBytes;
    bool m_bodyInMemory;
    std::vector< char > m_bodyBuffer;
    std::vector< char > m_writeBuffer;
    DWORD m_secureFailureFlags;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWinHttpClientBackend::CWinHttpClientBackend( void )
    : CIHttpClientBackend()
    , m_session( GUCEF_NEW CWinHttpSession() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWinHttpClientBackend::~CWinHttpClientBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CWinHttpClientBackend::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

bool
CWinHttpClientBackend::SupportsScheme( const CString& scheme ) const
{GUCEF_TRACE;

    return scheme.Equals( "http", false ) || scheme.Equals( "https", false );
}

/*-------------------------------------------------------------------------*/

CIHttpClientBackendRequestPtr
CWinHttpClientBackend::StartRequest( const CHttpRequestData& request          ,
                                     const CHttpClientRequestOptions& options ,
                                     CHttpResponseData& response              ,
                                     CHttpClientTransferInfo& transferInfo    ,
                                     CORE::PulseGeneratorPtr pulseGenerator   ,
                                     CIHttpClientBackendObserver& observer    )
{GUCEF_TRACE;

    if ( !SupportsScheme( request.requestProtocol ) )
    {
        transferInfo.success = false;
        transferInfo.errorCode = -1;
        transferInfo.errorDescription = "The WinHTTP HTTP client backend does not support \"" + request.requestProtocol + "\"";
        return CIHttpClientBackendRequestPtr();
    }

    CWinHttpRequest* winHttpRequest = GUCEF_NEW CWinHttpRequest( m_session, request, options, response, transferInfo, observer );
    CIHttpClientBackendRequestPtr requestPtr( winHttpRequest );
    if ( !winHttpRequest->Start( pulseGenerator ) )
        return CIHttpClientBackendRequestPtr();
    return requestPtr;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MSWIN_BUILD ? */
