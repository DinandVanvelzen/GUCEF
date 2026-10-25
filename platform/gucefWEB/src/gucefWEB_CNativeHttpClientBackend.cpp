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

#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#include "gucefCOMCORE_CTlsClientConnection.h"
#define GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#endif /* GUCEF_COMCORE_CTLSCLIENTCONNECTION_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CHTTPPROXYENVIRONMENT_H
#include "gucefWEB_CHttpProxyEnvironment.h"
#define GUCEF_WEB_CHTTPPROXYENVIRONMENT_H
#endif /* GUCEF_WEB_CHTTPPROXYENVIRONMENT_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H
#include "gucefWEB_CHttpClientBackendRequestBase.h"
#define GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H
#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDREQUESTBASE_H ? */

#include "gucefWEB_CNativeHttpClientBackend.h"

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

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

const CString CNativeHttpClientBackend::BackendName = "native";

static const UInt32 UploadChunkSize = 64 * 1024;
static const UInt32 MaxHeaderSize = 256 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A single request carried out over a TCP connection driven by the pulses of the pulse generator it is given
 */
class CNativeHttpClientRequest : public CHttpClientBackendRequestBase ,
                                 public CORE::CObserver
{
    public:

    CNativeHttpClientRequest( const CHttpRequestData& request          ,
                              const CHttpClientRequestOptions& options ,
                              CHttpResponseData& response              ,
                              CHttpClientTransferInfo& transferInfo    ,
                              CORE::PulseGeneratorPtr pulseGenerator   ,
                              CIHttpClientBackendObserver& observer    );

    virtual ~CNativeHttpClientRequest() GUCEF_VIRTUAL_OVERRIDE;

    bool Start( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnFinish( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CNativeHttpClientRequest > TEventCallback;

    enum EBodyFraming
    {
        BODY_NONE,
        BODY_CONTENT_LENGTH,
        BODY_CHUNKED,
        BODY_UNTIL_CLOSE
    };

    enum EChunkState
    {
        CHUNK_SIZE_LINE,
        CHUNK_DATA,
        CHUNK_DATA_END,
        CHUNK_TRAILERS
    };

    /** how far the tunnel through a proxy is for a https request: the CONNECT is answered, then TLS starts inside it */
    enum ETunnelState
    {
        TUNNEL_NONE,            /**< no tunnel: direct, or http through a proxy */
        TUNNEL_CONNECTING,      /**< connecting to the proxy */
        TUNNEL_WAIT_RESPONSE,   /**< CONNECT sent, the answer of the proxy is awaited */
        TUNNEL_HANDSHAKE,       /**< TLS with the server inside the tunnel */
        TUNNEL_DONE
    };

    void BuildRequestHeader( std::string& header ) const;
    void BuildTlsSettings( COMCORE::CTlsSettings& settings ) const;
    void SendConnect( void );
    void ProcessProxyResponse( const char* data, UInt32 dataSize );
    void OnTlsError( const COMCORE::CTlsError& error );
    void SendRequest( void );
    void SendMoreRequestBody( void );
    void ProcessReceivedData( const char* data, UInt32 dataSize );
    bool ParseResponseHeader( const std::string& headerBlock );
    void ProcessBodyData( const char* data, UInt32 dataSize );
    void ProcessChunkedData( const char* data, UInt32 dataSize );

    void RegisterSocketEventHandlers( void );
    void OnSocketConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketHandshakeCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketTlsError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketDataReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketDataSent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    COMCORE::CTlsClientConnection m_socket;
    COMCORE::CTlsSettings m_tlsSettings;
    bool m_secure;                  /**< a https request */
    bool m_viaProxy;
    CString m_proxyHost;
    UInt16 m_proxyPort;
    CString m_proxyAuthorization;   /**< value of Proxy-Authorization, from the credentials in the proxy of the environment */
    ETunnelState m_tunnelState;
    CORE::CDynamicBuffer m_proxyResponseBuffer;     /**< raw bytes: what follows the proxy header is TLS */

    bool m_finalHeaderReceived;
    std::string m_headerBuffer;
    EBodyFraming m_framing;
    UInt64 m_bodyBytesLeft;
    EChunkState m_chunkState;
    std::string m_chunkLine;
    bool m_uploadComplete;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The position of the first occurrence of 'needle' in 'data', 'dataSize' when there is none */
static UInt32
FindBytes( const char* data, UInt32 dataSize, const char* needle, UInt32 needleSize )
{
    if ( dataSize < needleSize )
        return dataSize;
    for ( UInt32 i = 0; i + needleSize <= dataSize; ++i )
    {
        if ( 0 == memcmp( data + i, needle, needleSize ) )
            return i;
    }
    return dataSize;
}

/*-------------------------------------------------------------------------*/

static std::string
ToStdString( const CString& str )
{
    return std::string( str.C_String(), str.ByteSize() > 0 ? str.ByteSize()-1 : 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNativeHttpClientRequest::CNativeHttpClientRequest( const CHttpRequestData& request          ,
                                                    const CHttpClientRequestOptions& options ,
                                                    CHttpResponseData& response              ,
                                                    CHttpClientTransferInfo& transferInfo    ,
                                                    CORE::PulseGeneratorPtr pulseGenerator   ,
                                                    CIHttpClientBackendObserver& observer    )
    : CHttpClientBackendRequestBase( request, options, response, transferInfo, observer )
    , CORE::CObserver()
    , m_socket( pulseGenerator, false )
    , m_tlsSettings()
    , m_secure( false )
    , m_viaProxy( false )
    , m_proxyHost()
    , m_proxyPort( 0 )
    , m_proxyAuthorization()
    , m_tunnelState( TUNNEL_NONE )
    , m_proxyResponseBuffer()
    , m_finalHeaderReceived( false )
    , m_headerBuffer()
    , m_framing( BODY_NONE )
    , m_bodyBytesLeft( 0 )
    , m_chunkState( CHUNK_SIZE_LINE )
    , m_chunkLine()
    , m_uploadComplete( true )
{GUCEF_TRACE;

    RegisterSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::RegisterSocketEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback connectedCallback( this, &CNativeHttpClientRequest::OnSocketConnected );
    SubscribeTo( &m_socket                                        ,
                 COMCORE::CTCPClientSocket::ConnectedEvent        ,
                 connectedCallback                                );

    TEventCallback handshakeCallback( this, &CNativeHttpClientRequest::OnSocketHandshakeCompleted );
    SubscribeTo( &m_socket                                        ,
                 COMCORE::CTlsClientConnection::HandshakeCompletedEvent ,
                 handshakeCallback                                );

    TEventCallback tlsErrorCallback( this, &CNativeHttpClientRequest::OnSocketTlsError );
    SubscribeTo( &m_socket                                        ,
                 COMCORE::CTlsClientConnection::TlsErrorEvent     ,
                 tlsErrorCallback                                 );

    TEventCallback dataReceivedCallback( this, &CNativeHttpClientRequest::OnSocketDataReceived );
    SubscribeTo( &m_socket                                        ,
                 COMCORE::CTCPClientSocket::DataRecievedEvent     ,
                 dataReceivedCallback                             );

    TEventCallback dataSentCallback( this, &CNativeHttpClientRequest::OnSocketDataSent );
    SubscribeTo( &m_socket                                        ,
                 COMCORE::CTCPClientSocket::DataSentEvent         ,
                 dataSentCallback                                 );

    TEventCallback disconnectedCallback( this, &CNativeHttpClientRequest::OnSocketDisconnected );
    SubscribeTo( &m_socket                                        ,
                 COMCORE::CTCPClientSocket::DisconnectedEvent     ,
                 disconnectedCallback                             );

    TEventCallback errorCallback( this, &CNativeHttpClientRequest::OnSocketError );
    SubscribeTo( &m_socket                                        ,
                 COMCORE::CTCPClientSocket::SocketErrorEvent      ,
                 errorCallback                                    );
}

/*-------------------------------------------------------------------------*/

CNativeHttpClientRequest::~CNativeHttpClientRequest()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

const CString&
CNativeHttpClientRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CNativeHttpClientRequest";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CNativeHttpClientRequest::Start( void )
{GUCEF_TRACE;

    m_secure = m_request.requestProtocol.Equals( "https", false );
    CString targetHost = m_request.GetHostName();
    UInt16 targetPort = m_request.GetPort();

    // The proxy that is asked for in the options, else the one of the environment
    m_viaProxy = false;
    if ( !m_options.proxyHost.IsNULLOrEmpty() )
    {
        m_viaProxy = true;
        m_proxyHost = m_options.proxyHost;
        m_proxyPort = m_options.proxyPort;
    }
    else
    if ( m_options.useEnvironmentProxy )
    {
        CHttpProxyEnvironment::TProxy proxy;
        if ( CHttpProxyEnvironment::Lookup( m_request.requestProtocol, targetHost, targetPort, proxy ) )
        {
            m_viaProxy = true;
            m_proxyHost = proxy.host;
            m_proxyPort = proxy.port;
            m_proxyAuthorization = CHttpProxyEnvironment::GetBasicAuthorization( proxy );
        }
    }

    if ( m_secure )
    {
        // https needs a TLS provider, which is the one thing the system may not have: everything else works without
        CString whyNot;
        if ( !CNativeHttpClientBackend::IsHttpsAvailable( whyNot ) )
        {
            m_transferInfo.errorClass = HTTPERRORCLASS_NO_BACKEND;
            Finish( false, "no HTTPS support: " + whyNot );
            return false;
        }
        BuildTlsSettings( m_tlsSettings );
    }

    CString host = m_viaProxy ? m_proxyHost : targetHost;
    UInt16 port = m_viaProxy ? m_proxyPort : targetPort;
    m_tunnelState = m_viaProxy && m_secure ? TUNNEL_CONNECTING : TUNNEL_NONE;

    // https straight to the server: TLS from the start. Through a proxy the connection is plain until the proxy opened the tunnel
    bool started = m_secure && !m_viaProxy ? m_socket.ConnectTo( host, port, &m_tlsSettings, false ) : m_socket.ConnectTo( host, port, false );
    if ( !started )
    {
        if ( m_socket.GetLastError().IsError() )
        {
            OnTlsError( m_socket.GetLastError() );
            return false;
        }

        // The authority of a URL can carry credentials as user info, which the description of the failure must not repeat
        CString target = host + ":" + CORE::ToString( port );
        if ( host.HasChar( '@', true ) >= 0 )
            target = CORE::CRedactor::ForUrl().RedactUrl( target );

        // The socket only does not start for a host that has no address: an unknown name
        bool isName = false;
        for ( UInt32 i=0; i<host.Length(); ++i )
        {
            char c = host[ i ];
            if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) )
                isName = true;
        }
        m_transferInfo.errorClass = isName ? HTTPERRORCLASS_DNS : HTTPERRORCLASS_CONNECT;
        Finish( false, "Unable to connect to " + target );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::BuildTlsSettings( COMCORE::CTlsSettings& settings ) const
{GUCEF_TRACE;

    settings.AddAlpnProtocol( "http/1.1" );
    if ( !m_options.verifyPeer )
        settings.DisableVerificationInsecurely();

    // The authorities that were asked for are the ones that count
    if ( !m_options.caBundlePath.IsNULLOrEmpty() )
    {
        settings.SetCaBundlePath( m_options.caBundlePath );
        settings.SetUseSystemTrust( false );
    }
    if ( m_options.connectTimeoutInMs > 0 )
        settings.SetHandshakeTimeoutInMs( m_options.connectTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnTlsError( const COMCORE::CTlsError& error )
{GUCEF_TRACE;

    // A failure of the socket underneath has its own event and description
    if ( COMCORE::TLSERR_CONNECT == error.errorClass )
        return;

    switch ( error.errorClass )
    {
        case COMCORE::TLSERR_NO_PROVIDER:
            m_transferInfo.errorClass = HTTPERRORCLASS_NO_BACKEND;
            break;
        case COMCORE::TLSERR_HANDSHAKE_TIMEOUT:
            m_transferInfo.errorClass = HTTPERRORCLASS_TIMEOUT;
            break;
        case COMCORE::TLSERR_PEER_CLOSED:
        case COMCORE::TLSERR_TRUNCATED:
            m_transferInfo.errorClass = HTTPERRORCLASS_CONNECT;
            break;
        case COMCORE::TLSERR_INTERNAL:
            m_transferInfo.errorClass = HTTPERRORCLASS_OTHER;
            break;
        default:
            m_transferInfo.errorClass = HTTPERRORCLASS_TLS;
            break;
    }

    // The host is named here, not by the error: the user info of a URL never gets in
    CString host = CORE::CRedactor::ForUrl().RedactUrl( m_request.GetHostName() );
    Finish( false, error.GetDescription( host ), GUCEF_COMCORE_TLS_ERRORCODE_BASE + (Int32) error.errorClass );
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::SendConnect( void )
{GUCEF_TRACE;

    CString authority = m_request.GetHostName() + ":" + CORE::ToString( m_request.GetPort() );
    CString connect = "CONNECT " + authority + " HTTP/1.1\r\n";
    connect += "Host: " + authority + "\r\n";
    if ( !m_proxyAuthorization.IsNULLOrEmpty() )
        connect += "Proxy-Authorization: " + m_proxyAuthorization + "\r\n";
    connect += "\r\n";

    m_tunnelState = TUNNEL_WAIT_RESPONSE;
    m_proxyResponseBuffer.Clear();
    m_socket.Send( connect.C_String(), connect.ByteSize() > 0 ? connect.ByteSize()-1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::ProcessProxyResponse( const char* data, UInt32 dataSize )
{GUCEF_TRACE;

    m_proxyResponseBuffer.Append( data, dataSize );
    const char* buffered = static_cast< const char* >( m_proxyResponseBuffer.GetConstBufferPtr() );
    UInt32 bufferedSize = m_proxyResponseBuffer.GetDataSize();
    UInt32 headerEnd = FindBytes( buffered, bufferedSize, "\r\n\r\n", 4 );
    UInt32 terminatorSize = 4;
    if ( bufferedSize == headerEnd )
    {
        headerEnd = FindBytes( buffered, bufferedSize, "\n\n", 2 );
        terminatorSize = 2;
    }
    if ( bufferedSize == headerEnd )
    {
        if ( bufferedSize > MaxHeaderSize )
            Finish( false, "The proxy sent a response header that is too large" );
        return;
    }

    // The status line is in the first bytes: a bounded, zero terminated copy of those is read
    char statusLine[ 64 ];
    UInt32 statusLineSize = headerEnd < sizeof( statusLine ) - 1 ? headerEnd : (UInt32) sizeof( statusLine ) - 1;
    memcpy( statusLine, buffered, statusLineSize );
    statusLine[ statusLineSize ] = '\0';
    int status = 0;
    if ( 1 != sscanf( statusLine, "HTTP/%*d.%*d %d", &status ) )
    {
        m_transferInfo.errorClass = HTTPERRORCLASS_CONNECT;
        Finish( false, "The proxy did not reply with a valid HTTP response to the CONNECT" );
        return;
    }
    if ( status < 200 || status > 299 )
    {
        m_transferInfo.errorClass = HTTPERRORCLASS_CONNECT;
        Finish( false, "The proxy refused to open a tunnel to " + CORE::CRedactor::ForUrl().RedactUrl( m_request.GetHostName() ) + ": HTTP status " + CORE::ToString( status ), status );
        return;
    }

    // The tunnel is open: TLS with the server starts. Whatever came behind the header of the proxy is the start of that
    CORE::CDynamicBuffer leftover;
    leftover.CopyFrom( m_proxyResponseBuffer, 0, bufferedSize - ( headerEnd + terminatorSize ), headerEnd + terminatorSize );
    m_proxyResponseBuffer.Clear();
    m_tunnelState = TUNNEL_HANDSHAKE;
    if ( !m_socket.StartTls( m_tlsSettings, m_request.GetHostName(), leftover.GetConstBufferPtr(), leftover.GetDataSize() ) )
    {
        if ( m_socket.GetLastError().IsError() )
            OnTlsError( m_socket.GetLastError() );
        else
            Finish( false, "TLS could not be started in the tunnel" );
    }
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnFinish( void )
{GUCEF_TRACE;

    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::BuildRequestHeader( std::string& header ) const
{GUCEF_TRACE;

    // A proxy is given the absolute URL, a server the path. A https request has a tunnel through the proxy: the server gets that path
    CString target;
    if ( m_viaProxy && !m_secure )
    {
        target = m_request.GetUrl();
    }
    else
    {
        target = m_request.requestUri.IsNULLOrEmpty() ? CString( "/" ) : m_request.requestUri;
        if ( !m_request.requestUriParams.IsNULLOrEmpty() )
            target += "?" + m_request.requestUriParams;
    }

    header.reserve( 512 );
    header = ToStdString( GetRequestMethod() ) + " " + ToStdString( target ) + " HTTP/1.1\r\n";
    header += "Host: " + ToStdString( m_request.requestHost ) + "\r\n";
    header += "Connection: close\r\n";
    if ( m_viaProxy && !m_secure && !m_proxyAuthorization.IsNULLOrEmpty() )
        header += "Proxy-Authorization: " + ToStdString( m_proxyAuthorization ) + "\r\n";

    CString::StringVector fields;
    GetRequestHeaderFields( fields );
    for ( size_t i=0; i<fields.size(); ++i )
    {
        header.append( fields[ i ].C_String(), fields[ i ].ByteSize() > 0 ? fields[ i ].ByteSize()-1 : 0 );
        header += "\r\n";
    }

    if ( HasRequestBody() )
    {
        if ( IsRequestBodySizeKnown() )
            header += "Content-Length: " + ToStdString( CORE::ToString( GetRequestBodySize() ) ) + "\r\n";
        else
            header += "Transfer-Encoding: chunked\r\n";
    }
    header += "\r\n";
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::SendRequest( void )
{GUCEF_TRACE;

    std::string header;
    BuildRequestHeader( header );

    if ( !IsRequestBodyStreamed() && GetRequestBodySize() > 0 )
    {
        // Small bodies held in memory go out together with the header
        size_t headerSize = header.size();
        header.resize( headerSize + (size_t) GetRequestBodySize() );
        ReadRequestBody( &header[ headerSize ], (UInt32) GetRequestBodySize() );
        m_socket.Send( header.data(), (UInt32) header.size() );
    }
    else
    {
        m_socket.Send( header.data(), (UInt32) header.size() );
        if ( IsRequestBodyStreamed() )
        {
            m_uploadComplete = false;
            SendMoreRequestBody();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::SendMoreRequestBody( void )
{GUCEF_TRACE;

    // Keep a limited amount queued in the socket so that bodies of any size are streamed
    std::vector< char > buffer;
    bool chunked = !IsRequestBodySizeKnown();
    while ( !m_uploadComplete && !IsFinished() && m_socket.GetBufferedDataToSendInBytes() < UploadChunkSize )
    {
        if ( buffer.empty() )
            buffer.resize( UploadChunkSize + 32 );

        // room is left in front for the chunk size line
        UInt32 bytesRead = ReadRequestBody( &buffer[ 16 ], UploadChunkSize );
        if ( chunked )
        {
            // chunk: size in hex, CRLF, data, CRLF. A zero size chunk ends the body
            char sizeLine[ 16 ];
            int sizeLineLength = sprintf( sizeLine, "%X\r\n", (unsigned int) bytesRead );
            char* chunkStart = &buffer[ 16 ] - sizeLineLength;
            memcpy( chunkStart, sizeLine, sizeLineLength );
            buffer[ 16 + bytesRead ] = '\r';
            buffer[ 16 + bytesRead + 1 ] = '\n';
            // a zero size chunk followed by the empty trailer line ("0\r\n\r\n") ends the body
            m_socket.Send( chunkStart, sizeLineLength + bytesRead + 2 );
            if ( 0 == bytesRead )
                m_uploadComplete = true;
        }
        else
        {
            if ( bytesRead > 0 )
                m_socket.Send( &buffer[ 16 ], bytesRead );
            if ( GetRequestBodyBytesRead() >= GetRequestBodySize() )
                m_uploadComplete = true;
            else
            if ( 0 == bytesRead )
            {
                Finish( false, "The request body source ended before its stated size" );
                return;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CNativeHttpClientRequest::ParseResponseHeader( const std::string& headerBlock )
{GUCEF_TRACE;

    // The status line, then the header lines. The base class takes care of the fields and their folding
    size_t lineEnd = headerBlock.find( '\n' );
    if ( !OnResponseStatusLine( headerBlock.data(), (UInt32) ( std::string::npos == lineEnd ? headerBlock.size() : lineEnd ) ) )
        return false;

    size_t pos = std::string::npos == lineEnd ? headerBlock.size() : lineEnd + 1;
    while ( pos < headerBlock.size() )
    {
        size_t end = headerBlock.find( '\n', pos );
        if ( std::string::npos == end )
            end = headerBlock.size();
        OnResponseHeaderLine( headerBlock.data() + pos, (UInt32) ( end - pos ) );
        pos = end + 1;
    }

    // How the body is delimited, RFC 9112 section 6.3
    int status = m_response.statusCode;
    if ( IsHeadRequest() || ( status >= 100 && status < 200 ) || 204 == status || 304 == status )
        m_framing = BODY_NONE;
    else
    if ( IsResponseChunked() )
        m_framing = BODY_CHUNKED;
    else
    if ( HasResponseContentLength() )
    {
        m_framing = GetResponseContentLength() > 0 ? BODY_CONTENT_LENGTH : BODY_NONE;
        m_bodyBytesLeft = GetResponseContentLength();
    }
    else
        m_framing = BODY_UNTIL_CLOSE;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::ProcessChunkedData( const char* data, UInt32 dataSize )
{GUCEF_TRACE;

    UInt32 offset = 0;
    while ( offset < dataSize && !IsFinished() )
    {
        switch ( m_chunkState )
        {
            case CHUNK_SIZE_LINE:
            case CHUNK_DATA_END:
            case CHUNK_TRAILERS:
            {
                // Lines are gathered until complete since they can be split over reads
                const char* lineEnd = (const char*) memchr( data + offset, '\n', dataSize - offset );
                UInt32 take = GUCEF_NULL == lineEnd ? dataSize - offset : (UInt32) ( lineEnd - ( data + offset ) ) + 1;
                m_chunkLine.append( data + offset, take );
                offset += take;
                if ( m_chunkLine.size() > MaxHeaderSize )
                {
                    Finish( false, "Malformed chunked response body" );
                    return;
                }
                if ( GUCEF_NULL == lineEnd )
                    break;

                std::string line;
                line.swap( m_chunkLine );
                while ( !line.empty() && ( '\n' == line[ line.size()-1 ] || '\r' == line[ line.size()-1 ] ) )
                    line.erase( line.size()-1 );

                if ( CHUNK_DATA_END == m_chunkState )
                {
                    // the CRLF after the data of a chunk, anything else means the chunk size was wrong
                    if ( !line.empty() )
                    {
                        Finish( false, "Malformed chunked response body: chunk data longer than its size" );
                        return;
                    }
                    m_chunkState = CHUNK_SIZE_LINE;
                }
                else
                if ( CHUNK_TRAILERS == m_chunkState )
                {
                    // trailer fields are ignored, the empty line ends the body
                    if ( line.empty() )
                    {
                        Finish( true, CString::Empty );
                        return;
                    }
                }
                else
                {
                    // size in hex, optionally followed by chunk extensions
                    unsigned long long chunkSize = 0;
                    if ( 1 != sscanf( line.c_str(), "%llx", &chunkSize ) )
                    {
                        Finish( false, "Malformed chunked response body" );
                        return;
                    }
                    if ( 0 == chunkSize )
                        m_chunkState = CHUNK_TRAILERS;
                    else
                    {
                        m_bodyBytesLeft = (UInt64) chunkSize;
                        m_chunkState = CHUNK_DATA;
                    }
                }
                break;
            }
            case CHUNK_DATA:
            {
                UInt32 available = dataSize - offset;
                UInt32 take = m_bodyBytesLeft < available ? (UInt32) m_bodyBytesLeft : available;
                OnResponseBody( data + offset, take );
                offset += take;
                m_bodyBytesLeft -= take;
                if ( 0 == m_bodyBytesLeft )
                    m_chunkState = CHUNK_DATA_END;
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::ProcessBodyData( const char* data, UInt32 dataSize )
{GUCEF_TRACE;

    switch ( m_framing )
    {
        case BODY_CONTENT_LENGTH:
        {
            UInt32 take = m_bodyBytesLeft < dataSize ? (UInt32) m_bodyBytesLeft : dataSize;
            OnResponseBody( data, take );
            m_bodyBytesLeft -= take;
            if ( 0 == m_bodyBytesLeft )
                Finish( true, CString::Empty );
            break;
        }
        case BODY_CHUNKED:
        {
            ProcessChunkedData( data, dataSize );
            break;
        }
        case BODY_UNTIL_CLOSE:
        {
            OnResponseBody( data, dataSize );
            break;
        }
        default:
        {
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::ProcessReceivedData( const char* data, UInt32 dataSize )
{GUCEF_TRACE;

    OnDataReceived( dataSize );

    while ( !m_finalHeaderReceived && dataSize > 0 && !IsFinished() )
    {
        // Gather the header, it can be split over any number of reads
        size_t searchFrom = m_headerBuffer.size() > 3 ? m_headerBuffer.size() - 3 : 0;
        m_headerBuffer.append( data, dataSize );
        size_t headerEnd = m_headerBuffer.find( "\r\n\r\n", searchFrom );
        size_t terminatorSize = 4;
        if ( std::string::npos == headerEnd )
        {
            headerEnd = m_headerBuffer.find( "\n\n", searchFrom );
            terminatorSize = 2;
        }
        if ( std::string::npos == headerEnd )
        {
            if ( m_headerBuffer.size() > MaxHeaderSize )
                Finish( false, "The response header is too large" );
            return;
        }

        std::string headerBlock = m_headerBuffer.substr( 0, headerEnd );
        size_t bodyStartInBuffer = headerEnd + terminatorSize;
        size_t alreadyBuffered = m_headerBuffer.size() - dataSize;   // bytes from earlier reads
        UInt32 consumedFromThisRead = (UInt32) ( bodyStartInBuffer - alreadyBuffered );
        m_headerBuffer.clear();

        if ( !ParseResponseHeader( headerBlock ) )
        {
            Finish( false, "The server did not reply with a valid HTTP response" );
            return;
        }

        data += consumedFromThisRead;
        dataSize -= consumedFromThisRead;

        if ( m_response.statusCode >= 100 && m_response.statusCode < 200 )
        {
            // An interim response such as 100 Continue, the actual response follows
            continue;
        }

        m_finalHeaderReceived = true;
        OnResponseHeadersComplete();
        if ( BODY_NONE == m_framing )
        {
            Finish( true, CString::Empty );
            return;
        }
    }

    if ( m_finalHeaderReceived && dataSize > 0 && !IsFinished() )
        ProcessBodyData( data, dataSize );
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnSocketConnected( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsFinished() )
        return;

    // For TLS this is after the handshake. Through a proxy it is the connection to the proxy, which is asked for the tunnel first
    OnConnected();
    if ( TUNNEL_CONNECTING == m_tunnelState )
        SendConnect();
    else
        SendRequest();
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnSocketHandshakeCompleted( CORE::CNotifier* notifier    ,
                                                      const CORE::CEvent& eventId  ,
                                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsFinished() )
        return;

    // Only the handshake inside a tunnel is of interest here: a direct one is followed by ConnectedEvent
    if ( TUNNEL_HANDSHAKE == m_tunnelState )
    {
        m_tunnelState = TUNNEL_DONE;
        SendRequest();
    }
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnSocketTlsError( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsFinished() )
        return;

    COMCORE::TTlsErrorEventData* errorData = static_cast< COMCORE::TTlsErrorEventData* >( eventData );
    if ( GUCEF_NULL != errorData )
        OnTlsError( errorData->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnSocketDataReceived( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsFinished() )
        return;

    COMCORE::CTCPClientSocket::TDataRecievedEventData* receivedData = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventData );
    if ( GUCEF_NULL != receivedData )
    {
        const CORE::CDynamicBuffer& buffer = receivedData->GetData();
        if ( TUNNEL_WAIT_RESPONSE == m_tunnelState )
            ProcessProxyResponse( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
        else
            ProcessReceivedData( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
    }
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnSocketDataSent( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsFinished() )
        return;

    // What goes to the proxy to open the tunnel is not part of the request
    if ( TUNNEL_NONE == m_tunnelState || TUNNEL_DONE == m_tunnelState )
    {
        COMCORE::CTCPClientSocket::TDataSentEventData* sentData = static_cast< COMCORE::CTCPClientSocket::TDataSentEventData* >( eventData );
        if ( GUCEF_NULL != sentData )
            OnDataSent( sentData->GetData().GetDataSize() );
        if ( !m_uploadComplete && !IsFinished() )
            SendMoreRequestBody();
    }
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnSocketDisconnected( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsFinished() )
        return;

    if ( TUNNEL_NONE != m_tunnelState && TUNNEL_DONE != m_tunnelState )
    {
        m_transferInfo.errorClass = HTTPERRORCLASS_CONNECT;
        Finish( false, "The proxy closed the connection before the tunnel was open" );
    }
    else
    if ( m_finalHeaderReceived && BODY_UNTIL_CLOSE == m_framing )
    {
        // A body that ends at the end of the connection is only complete when the end was announced: without the close
        // notification of TLS anyone on the path could have cut it
        if ( m_secure && !m_socket.GetSessionInfo().peerClosedCleanly )
        {
            m_transferInfo.errorClass = HTTPERRORCLASS_CONNECT;
            Finish( false, "The server closed the connection without the TLS close notification, the response may be incomplete" );
        }
        else
            Finish( true, CString::Empty );
    }
    else
        Finish( false, "The connection was closed before the response was complete" );
}

/*-------------------------------------------------------------------------*/

void
CNativeHttpClientRequest::OnSocketError( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsFinished() )
        return;

    COMCORE::CTCPClientSocket::TSocketErrorEventData* errorData = static_cast< COMCORE::CTCPClientSocket::TSocketErrorEventData* >( eventData );
    Int32 errorCode = GUCEF_NULL != errorData ? errorData->GetData() : -1;
    m_transferInfo.errorClass = HTTPERRORCLASS_CONNECT;
    Finish( false, "Socket error " + CORE::ToString( errorCode ), errorCode );
}

/*-------------------------------------------------------------------------*/

CNativeHttpClientBackend::CNativeHttpClientBackend( void )
    : CIHttpClientBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNativeHttpClientBackend::~CNativeHttpClientBackend()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CNativeHttpClientBackend::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

bool
CNativeHttpClientBackend::IsHttpsAvailable( CString& whyNot )
{GUCEF_TRACE;

    return COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNot );
}

/*-------------------------------------------------------------------------*/

bool
CNativeHttpClientBackend::SupportsScheme( const CString& scheme ) const
{GUCEF_TRACE;

    if ( scheme.Equals( "http", false ) )
        return true;

    // https needs a TLS provider, which this system may or may not have at this moment
    if ( scheme.Equals( "https", false ) )
    {
        CString whyNot;
        return IsHttpsAvailable( whyNot );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CIHttpClientBackendRequestPtr
CNativeHttpClientBackend::StartRequest( const CHttpRequestData& request          ,
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
        transferInfo.errorClass = HTTPERRORCLASS_NO_BACKEND;
        transferInfo.errorDescription = "The native HTTP client backend does not support \"" + request.requestProtocol + "\"";
        return CIHttpClientBackendRequestPtr();
    }

    CNativeHttpClientRequest* nativeRequest = GUCEF_NEW CNativeHttpClientRequest( request, options, response, transferInfo, pulseGenerator, observer );
    CIHttpClientBackendRequestPtr requestPtr( nativeRequest );
    if ( !nativeRequest->Start() )
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
