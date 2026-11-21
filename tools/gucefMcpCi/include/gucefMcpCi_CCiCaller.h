/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CCICALLER_H
#define GUCEF_MCPCI_CCICALLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Who asks the service for something: the machine and what it may do. On a gateway both come from the token of the
 *  request (the tokens file of the lablink hub); in direct mode from the options of the server. Scopes:
 *
 *    build     everything that builds and reads: submit, status, reports, images, the release status
 *    deploy    also deploy and roll back
 */
class CCiCaller
{
    public:

    static const char* ScopeBuild;      /**< "build" */
    static const char* ScopeDeploy;     /**< "deploy" */

    GUCEF::CORE::CString machineId;
    GUCEF::CORE::CString::StringSet scopes;

    CCiCaller( void );

    CCiCaller( const GUCEF::CORE::CString& machine, const GUCEF::CORE::CString& scopeList );

    bool HasScope( const char* scope ) const;

    /** The scopes, comma separated */
    GUCEF::CORE::CString ListScopes( void ) const;

    /**
     *  The known scopes of a comma separated list; an empty list is "build". Unknown names are left out and listed
     *  in 'unknown' (comma separated) for the log of the caller.
     */
    static GUCEF::CORE::CString::StringSet ParseScopes( const GUCEF::CORE::CString& scopeList, GUCEF::CORE::CString& unknown );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCICALLER_H ? */
