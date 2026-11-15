/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CSERVICEADMINENDPOINT_H
#define GUCEF_COM_CSERVICEADMINENDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#include "gucefCOMCORE_CIRpcTransportListener.h"
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_COM_CISERVICEADMINHANDLER_H
#include "gucefCOM_CIServiceAdminHandler.h"
#define GUCEF_COM_CISERVICEADMINHANDLER_H
#endif /* GUCEF_COM_CISERVICEADMINHANDLER_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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

/**
 *  The loopback admin endpoint of a service: an RPC host with one tcp listener on 127.0.0.1 (never another address)
 *  serving gucef.service.admin (CServiceAdminContracts), for the admin client mode of the service's own executable run
 *  inside its container (kubectl exec) or on its machine.
 *  Authentication: Open() makes the token file "admin.token" in the state directory with 32 random bytes as 64
 *  hexadecimal characters, owner only (CreateOwnerOnlyFile, mode 0600), after removing a previous one; possession of
 *  the state directory is the admin credential. Every request carries the token; it is compared in constant time, and
 *  a refused request is logged with the address only, never with a value. Close() removes the file.
 *  The commands it knows are those given with SetCommands() (empty: any command goes to the handler); what to do is the
 *  handler's decision. The handler runs on the pulse generator given at construction.
 *  Open() fails (and leaves an existing token file of another running service untouched) when the port is not free.
 */
class GUCEF_COM_PUBLIC_CPP CServiceAdminEndpoint
{
    public:

    static const char* const TokenFileName;     /**< "admin.token" */

    CServiceAdminEndpoint( CIServiceAdminHandler& handler, const CORE::PulseGeneratorPtr& pulseGenerator );
    ~CServiceAdminEndpoint();

    /** The commands the endpoint accepts; others answer NOT_FOUND. Empty (the default): every command */
    void SetCommands( const CORE::CString::StringSet& commands );

    /** Listens on 127.0.0.1:port ('port' 0: the system chooses) and then makes the token file; false with the reason */
    bool Open( const CORE::CString& stateDirectory, UInt16 port, CORE::CString& error );

    /** Stops listening, ends the sessions, removes the token file */
    void Close( void );

    bool IsOpen( void ) const;
    UInt16 GetBoundPort( void ) const;

    /** Requests refused for a missing or wrong token since Open() */
    UInt32 GetRefusedCount( void ) const;

    /** The token a client sends, from the token file of a state directory; false with the reason */
    static bool ReadTokenFile( const CORE::CString& stateDirectory, CORE::CString& tokenHex, CORE::CString& error );

    private:

    typedef COMCORE::CTRpcMethodHandler< CServiceAdminEndpoint > THandler;

    CServiceAdminEndpoint( const CServiceAdminEndpoint& src );              /**< not implemented */
    CServiceAdminEndpoint& operator=( const CServiceAdminEndpoint& src );   /**< not implemented */

    COMCORE::TRpcHandlerResult OnRun( COMCORE::CRpcServerCall& call );

    CIServiceAdminHandler& m_handler;
    CORE::PulseGeneratorPtr m_pulse;
    COMCORE::CRpcHost* m_host;
    COMCORE::CIRpcTransportListenerPtr m_listener;
    COMCORE::CRpcServiceImpl m_service;
    CORE::CString::StringSet m_commands;
    CORE::CString m_tokenHex;
    CORE::CString m_tokenFilePath;
    UInt16 m_boundPort;
    UInt32 m_refused;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSERVICEADMINENDPOINT_H ? */
