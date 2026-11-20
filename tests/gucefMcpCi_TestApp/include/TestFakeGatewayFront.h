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

#ifndef GUCEF_MCPCI_TESTAPP_TESTFAKEGATEWAYFRONT_H
#define GUCEF_MCPCI_TESTAPP_TESTFAKEGATEWAYFRONT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

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
 *  A server on 127.0.0.1 in front of a gateway, for what a network does to a proxy: it passes requests on to the
 *  gateway, and makes chunks of uploads fail on request (after passing them on, as when the answer is lost, or
 *  without, as when the chunk is lost). In garbage mode it answers every request with something that is not the API.
 */
class CFakeGatewayFront : public GUCEF::MT::CActiveObject
{
    public:

    enum EMode
    {
        MODE_FORWARD = 0,
        MODE_GARBAGE
    };

    /** @param targetUrl    the gateway, http://127.0.0.1:<port>; empty for garbage mode only */
    explicit CFakeGatewayFront( const GUCEF::CORE::CString& targetUrl );

    virtual ~CFakeGatewayFront();

    bool Start( void );

    void Stop( void );

    /** http://127.0.0.1:<port> */
    GUCEF::CORE::CString GetUrl( void ) const;

    void SetMode( EMode mode );

    /**
     *  Makes chunk 'number' (the n-th PUT, from 1) fail with 502.
     *  @param passOn   true: the gateway gets the chunk and only its answer is lost; false: the chunk is lost
     */
    void FailChunk( GUCEF::UInt32 number, bool passOn );

    /** The chunks that came by, failed ones included */
    GUCEF::UInt32 GetChunkCount( void ) const;

    GUCEF::UInt32 GetFailedChunkCount( void ) const;

    void Handle( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

    virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< GUCEF::UInt32, bool > TFailureMap;

    CFakeGatewayFront( const CFakeGatewayFront& src );                /**< not implemented */
    CFakeGatewayFront& operator=( const CFakeGatewayFront& src );     /**< not implemented */

    void PassOn( const GUCEF::WEB::CHttpRequestData& request, GUCEF::WEB::CHttpResponseData& response );

    GUCEF::CORE::CString m_targetUrl;
    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::CORE::PulseGeneratorPtr m_pulseGenerator;
    GUCEF::WEB::CHTTPServer* m_server;
    void* m_factory;
    bool m_running;
    EMode m_mode;
    TFailureMap m_failures;
    GUCEF::UInt32 m_chunkCount;
    GUCEF::UInt32 m_failedCount;
};

/*-------------------------------------------------------------------------*/

}; /* namespace MCPCITEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_TESTAPP_TESTFAKEGATEWAYFRONT_H ? */
