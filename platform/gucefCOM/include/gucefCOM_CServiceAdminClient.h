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

#ifndef GUCEF_COM_CSERVICEADMINCLIENT_H
#define GUCEF_COM_CSERVICEADMINCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

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
 *  The client of a service's loopback admin endpoint (CServiceAdminEndpoint): what "<Service> --admin <command>" runs.
 *  It reads the token file of the state directory, connects to 127.0.0.1:port over the RPC tcp transport, calls
 *  gucef.service.admin.Admin/Run and returns the answer. The blocking form runs on the calling thread and pumps only a
 *  pulse generator that it made for itself (a process of its own: the admin client never uses the service's state).
 */
class GUCEF_COM_PUBLIC_CPP CServiceAdminClient
{
    public:

    typedef CORE::CString::StringMap TArgumentMap;

    /**
     *  Runs one command. True when the service answered (status says how: OK, or the failure with its message);
     *  false when it could not be reached or did not answer within timeoutMs (status UNAVAILABLE or DEADLINE_EXCEEDED)
     */
    static bool Run( UInt16 port                     ,
                     const CORE::CString& tokenHex   ,
                     const CORE::CString& command    ,
                     const TArgumentMap& arguments   ,
                     CORE::CDataNode& result         ,
                     COMCORE::CRpcStatus& status     ,
                     UInt32 timeoutMs = 30000        );

    /** Run() with the token read from the token file of the state directory */
    static bool RunWithTokenFile( UInt16 port                        ,
                                  const CORE::CString& stateDirectory ,
                                  const CORE::CString& command        ,
                                  const TArgumentMap& arguments       ,
                                  CORE::CDataNode& result             ,
                                  COMCORE::CRpcStatus& status         ,
                                  UInt32 timeoutMs = 30000            );

    /**
     *  Parses "<command> [--name value | --flag] ..." (what follows --admin on a command line) into the command and its
     *  arguments ("--flag" alone gives "true"); false with the reason for an empty command or a value without a name
     */
    static bool ParseCommandLine( const CORE::CString::StringVector& words, CORE::CString& command, TArgumentMap& arguments, CORE::CString& error );

    private:

    CServiceAdminClient( void );        /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSERVICEADMINCLIENT_H ? */
