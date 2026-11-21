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

#ifndef GUCEF_WEB_COBSERVEDUSERACTION_H
#define GUCEF_WEB_COBSERVEDUSERACTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Something the user did in a watched tab, after which requests followed: see CUserActionTracker for the kinds and
 *  how requests are grouped by them.
 */
class GUCEF_WEB_PUBLIC_CPP CObservedUserAction
{
    public:

    static const CORE::CString KindNavigation;          /**< "navigation": a document request of a tab's main frame */
    static const CORE::CString KindInPageNavigation;    /**< "in-page-navigation": a route change of a single page application */
    static const CORE::CString KindClick;               /**< "click", reported by the optional click listener */
    static const CORE::CString KindSubmit;              /**< "submit", reported by the optional click listener */
    static const CORE::CString KindPause;               /**< "activity-after-pause": requests that followed a quiet time without another action */

    UInt32 id;                      /**< 1 and up, in the order of the actions */
    Float64 timestamp;              /**< seconds on the recorder's clock */
    CORE::CString kind;
    CORE::CString label;            /**< what the clicked element says, redacted, digit runs as {n}; empty for other kinds */
    CORE::CString page;             /**< the page it happened on as scheme://host/path-template */

    CObservedUserAction( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_COBSERVEDUSERACTION_H ? */
