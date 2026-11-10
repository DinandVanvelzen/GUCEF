/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CIMCPTRANSPORT_H
#define GUCEF_COM_CIMCPTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMcpServer;

/**
 *  Carries MCP messages between clients and a CMcpServer. A transport makes a CMcpChannel per conversation scope
 *  (the stdio process, a TCP connection, an HTTP exchange), opens it with CMcpServer::OpenChannel(), hands it the
 *  messages it receives on the protocol thread of the server and closes it with CMcpServer::CloseChannel().
 *  The principal of a channel is set by the transport, never taken from a message.
 */
class GUCEF_COM_PUBLIC_CPP CIMcpTransport : public CORE::CObservingNotifier ,
                                            public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent StartedEvent;
    static const CORE::CEvent StoppedEvent;
    static const CORE::CEvent TransportErrorEvent;              /**< TTransportErrorEventData: a description of the problem */

    typedef CORE::TCloneableString TTransportErrorEventData;

    static const CORE::CString StdioTransportName;              /**< "stdio" */
    static const CORE::CString TcpTransportName;                /**< "tcp" */
    static const CORE::CString StreamableHttpTransportName;     /**< "streamable-http", registered by gucefWEB */

    static void RegisterEvents( void );

    CIMcpTransport( void );

    virtual ~CIMcpTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** The name the transport is registered with in the factory */
    virtual const CORE::CString& GetTransportName( void ) const = 0;

    /** Starts serving the server, on its protocol thread */
    virtual bool Start( CMcpServer& server ) = 0;

    virtual void Stop( bool graceful ) = 0;

    virtual bool IsActive( void ) const = 0;

    /** "stdio", "tcp://127.0.0.1:47123", "http://127.0.0.1:47124/mcp" */
    virtual CORE::CString GetEndpointDescription( void ) const = 0;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    CIMcpTransport( const CIMcpTransport& src );
    CIMcpTransport& operator=( const CIMcpTransport& src );
};

typedef CORE::CTAbstractFactory< CORE::CString, CIMcpTransport, MT::CMutex >   CMcpTransportFactory;
typedef CMcpTransportFactory::TProductPtr                                       CIMcpTransportPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMCPTRANSPORT_H ? */
