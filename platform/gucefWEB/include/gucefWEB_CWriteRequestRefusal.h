/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CWRITEREQUESTREFUSAL_H
#define GUCEF_WEB_CWRITEREQUESTREFUSAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A request other than GET, HEAD or OPTIONS that a CWriteRequestGuard failed before it left the browser. Holds the
 *  address with the path as a template and the values of the query replaced (the names stay), never a body.
 */
class GUCEF_WEB_PUBLIC_CPP CWriteRequestRefusal
{
    public:

    static const CORE::CString ReasonMethodNotAllowed;  /**< "method-not-allowed": no rule names the method */
    static const CORE::CString ReasonSchemeNotAllowed;  /**< "scheme-not-allowed": rules name method and host, but with the other scheme */
    static const CORE::CString ReasonHostNotAllowed;    /**< "host-not-allowed": no rule names method and host */
    static const CORE::CString ReasonAmbiguousPath;     /**< "ambiguous-path": the path holds escapes, dot or empty segments or other characters no rule can match */
    static const CORE::CString ReasonPathNotAllowed;    /**< "path-not-allowed": no rule of method and host names the path */
    static const CORE::CString ReasonQueryNotAllowed;   /**< "query-not-allowed": a rule names method, host and path, none the query */
    static const CORE::CString ReasonMalformed;         /**< "malformed": not an http(s) address, or one with user info */

    CORE::CString method;           /**< as the browser sent it */
    CORE::CString url;              /**< scheme://host/path-template?name={redacted}&... */
    CORE::CString host;             /**< with the port when the address has one */
    CORE::CString pathTemplate;
    CORE::CString resourceType;     /**< as the browser names it: XHR, Fetch, Ping, Document, Other ... */
    CORE::CString frameId;
    bool isMainFrame;
    bool redirected;                /**< a later leg of a redirect */
    CORE::CString reason;

    CWriteRequestRefusal( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CWriteRequestRefusal >             TWriteRequestRefusalVector;
typedef CORE::CTCloneableObj< CWriteRequestRefusal >      TWriteRequestRefusalEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWRITEREQUESTREFUSAL_H ? */
