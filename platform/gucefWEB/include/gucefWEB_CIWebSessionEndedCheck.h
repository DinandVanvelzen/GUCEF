/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CIWEBSESSIONENDEDCHECK_H
#define GUCEF_WEB_CIWEBSESSIONENDEDCHECK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */
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
 *  Tells from an answer of a store whether its web session ended, so CWebSessionSignIn::CheckResponse() can forget
 *  the session and the store can sign in again. A store plugin gives its own when the default (CWebSessionEndedCheck:
 *  status codes, a redirect to the login page) does not fit, for example a page that says so in its body.
 *  Called on the thread that made the request; an implementation must be thread safe.
 */
class GUCEF_WEB_PUBLIC_CPP CIWebSessionEndedCheck
{
    public:

    CIWebSessionEndedCheck( void );
    virtual ~CIWebSessionEndedCheck();

    /** Whether the answer to a request made with the session means that the session ended */
    virtual bool IsSessionEnded( const WEB::CHttpResponseData& response ) const = 0;

    private:

    CIWebSessionEndedCheck( const CIWebSessionEndedCheck& src );                /**< not implemented */
    CIWebSessionEndedCheck& operator=( const CIWebSessionEndedCheck& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIWEBSESSIONENDEDCHECK_H ? */
