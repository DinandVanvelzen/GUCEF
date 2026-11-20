/*
 *  gucefMcpCi_TestApp: tests of the gucefMcpCi MCP server
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

#ifndef GUCEF_MCPCI_TESTAPP_TESTCIGATEWAY_H
#define GUCEF_MCPCI_TESTAPP_TESTCIGATEWAY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MCPCITEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The gucefMcpCi server in mode gateway as a child process on a free port of 127.0.0.1. Its log goes to a file of
 *  its own, read after it stopped, so that a test can check that no token is in it.
 */
class CTestCiGateway
{
    public:

    CTestCiGateway( void );

    ~CTestCiGateway();

    /**
     *  Starts the gateway with the arguments and --listen 127.0.0.1:<port> (or --listen-tls with 'tls'), --log-file,
     *  and waits until it accepts connections.
     */
    bool Start( const GUCEF::CORE::CProcess::TStringVector& args, const GUCEF::CORE::CString& logFile, bool tls = false );

    /** Stops it the way a service manager does (SIGTERM). @return whether it ended */
    bool Stop( void );

    GUCEF::UInt16 GetPort( void ) const;

    /** http://127.0.0.1:<port>, or https://localhost:<port> with TLS */
    GUCEF::CORE::CString GetUrl( void ) const;

    /** The log file as it is now */
    GUCEF::CORE::CString GetLog( void ) const;

    private:

    CTestCiGateway( const CTestCiGateway& src );                /**< not implemented */
    CTestCiGateway& operator=( const CTestCiGateway& src );     /**< not implemented */

    GUCEF::CORE::CProcess m_process;
    GUCEF::UInt16 m_port;
    bool m_tls;
    bool m_started;
    GUCEF::CORE::CString m_logFile;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One request to the gateway API with the bearer token (none when empty).
 *  @return the HTTP status, 0 when no answer came
 */
GUCEF::Int32 GatewayRequest( const GUCEF::CORE::CString& baseUrl        ,
                             const GUCEF::CORE::CString& token          ,
                             const GUCEF::CORE::CString& method         ,
                             const GUCEF::CORE::CString& pathAndQuery   ,
                             const GUCEF::CORE::CDynamicBuffer& body    ,
                             GUCEF::CORE::CString& answer               ,
                             const GUCEF::CORE::CString& caBundle = GUCEF::CORE::CString::Empty ,
                             GUCEF::UInt32 timeoutInMs = 60000          );

/** As GatewayRequest() with a text body */
GUCEF::Int32 GatewayRequest( const GUCEF::CORE::CString& baseUrl        ,
                             const GUCEF::CORE::CString& token          ,
                             const GUCEF::CORE::CString& method         ,
                             const GUCEF::CORE::CString& pathAndQuery   ,
                             const GUCEF::CORE::CString& body           ,
                             GUCEF::CORE::CString& answer               );

/**
 *  Starts a GET on the thread pool of the HTTP client, which must stay until the request is done; GatewayResult()
 *  gives its status once it is ready
 */
GUCEF::CORE::CFutureResult GatewayRequestAsync( GUCEF::WEB::CHTTPClient& client          ,
                                                const GUCEF::CORE::CString& baseUrl      ,
                                                const GUCEF::CORE::CString& token        ,
                                                const GUCEF::CORE::CString& pathAndQuery ,
                                                GUCEF::UInt32 timeoutInMs                );

/** The status of a request of GatewayRequestAsync(), waiting at most timeoutInMs; 0 when none came */
GUCEF::Int32 GatewayResult( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutInMs );

/** The value of a JSON attribute in an answer, as text; empty when it is not there */
GUCEF::CORE::CString JsonValue( const GUCEF::CORE::CString& json, const char* name );

/** The SHA-256 of bytes as 64 lower case hexadecimal digits */
GUCEF::CORE::CString Sha256Hex( const void* data, GUCEF::UInt32 size );

/** A line of a tokens file for a token: its SHA-256, a TAB, the machine */
GUCEF::CORE::CString TokenLine( const GUCEF::CORE::CString& token, const GUCEF::CORE::CString& machine );

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TESTAPP_TESTCIGATEWAY_H ? */
