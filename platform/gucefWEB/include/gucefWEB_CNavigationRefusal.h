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

#ifndef GUCEF_WEB_CNAVIGATIONREFUSAL_H
#define GUCEF_WEB_CNAVIGATIONREFUSAL_H

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
 *  A navigation a CNavigationGuard refused, or a committed address outside its rules that it could only report.
 *  Holds the address as a template (scheme, host, path template), never the query or the fragment.
 */
class GUCEF_WEB_PUBLIC_CPP CNavigationRefusal
{
    public:

    static const CORE::CString ReasonNotAllowed;    /**< "not-allowed": no allow rule matches */
    static const CORE::CString ReasonDenied;        /**< "denied": a deny rule matches */
    static const CORE::CString ReasonNeverRecord;   /**< "never-record": a sign-in or payment address no allow rule names */
    static const CORE::CString ReasonScheme;        /**< "scheme": not http or https */
    static const CORE::CString ReasonMalformed;     /**< "malformed": not a URL */

    static const CORE::CString SourceNavigate;      /**< "navigate": a Navigate() of the program, nothing was sent */
    static const CORE::CString SourcePage;          /**< "page": a document request of a tab or frame */
    static const CORE::CString SourceRedirect;      /**< "redirect": a later leg of a redirect */
    static const CORE::CString SourceCommitted;     /**< "committed": an address a tab shows, reached without a request (history.pushState) */

    CORE::CString urlTemplate;      /**< scheme://host/path-template, or only the scheme when it is not http(s) */
    CORE::CString host;             /**< with the port when the address has one */
    CORE::CString frameId;          /**< empty for "navigate" */
    bool isMainFrame;
    CORE::CString reason;
    CORE::CString source;
    bool blocked;                   /**< false for "committed": the address was reached and can only be reported */

    CNavigationRefusal( void );
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CNavigationRefusal >             TNavigationRefusalVector;
typedef CORE::CTCloneableObj< CNavigationRefusal >      TNavigationRefusalEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CNAVIGATIONREFUSAL_H ? */
