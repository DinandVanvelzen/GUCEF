/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#define GUCEF_COMCORE_CRPCPRINCIPAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Who the peer of a session is (gucefCOMCORE RPC design 10.7). Set by the transport, never taken from a frame,
 *  and pinned to the session at HELLO.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcPrincipal
{
    public:

    CORE::CString transport;                    /**< "loopback", "tcp", "drn", ... */
    CORE::CString subject;                      /**< "drn:<userId>", "local:in-process", "tcp:<address>" */
    bool isAuthenticated;
    CORE::CString::StringSet roles;
    CORE::CValueList attributes;                /**< transport specific: "drn.publicKey", "drn.nodeId", ... */

    CRpcPrincipal( void );
    CRpcPrincipal( const CORE::CString& transportType, const CORE::CString& subjectName, bool authenticated );
    CRpcPrincipal( const CRpcPrincipal& src );
    ~CRpcPrincipal();
    CRpcPrincipal& operator=( const CRpcPrincipal& src );

    bool HasRole( const CORE::CString& role ) const;

    /**
     *  True when a session established with this principal may be resumed by the other (8.2). An authenticated
     *  principal: the same subject, the same public key attribute where one is present, and no step down to an
     *  unauthenticated one. An unauthenticated principal names no identity (its subject is an address that changes
     *  with every connection): the resume proof, the possession of the resume token, is what identifies the peer
     */
    bool MayResumeAs( const CRpcPrincipal& other ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */
