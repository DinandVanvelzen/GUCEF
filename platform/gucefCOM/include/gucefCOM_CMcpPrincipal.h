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

#ifndef GUCEF_COM_CMCPPRINCIPAL_H
#define GUCEF_COM_CMCPPRINCIPAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

/**
 *  Who is asking. A transport sets it on its channel, an in-process caller passes it: it is never taken from a message.
 *  The access policy, the request state seal and the rate limit are keyed by the subject.
 */
class GUCEF_COM_PUBLIC_CPP CMcpPrincipal
{
    public:

    static const CORE::CString InProcessTransportName;     /**< "in-process": calls made through the CallTool() family */

    CORE::CString transport;                    /**< "stdio", "tcp", "streamable-http", "in-process" */
    CORE::CString subject;                      /**< "local:parent-process", "bearer:<label>", "anonymous" and the like */
    bool isAuthenticated;
    CORE::CString::StringVector scopes;

    CMcpPrincipal( void );

    CMcpPrincipal( const CORE::CString& transportName, const CORE::CString& subjectName, bool authenticated );

    bool HasScope( const CORE::CString& scope ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPPRINCIPAL_H ? */
