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

#ifndef GUCEF_WEB_CIWEBRESPONSESINK_H
#define GUCEF_WEB_CIWEBRESPONSESINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_COBSERVEDRESPONSEBODY_H
#include "gucefWEB_CObservedResponseBody.h"
#define GUCEF_WEB_COBSERVEDRESPONSEBODY_H
#endif /* GUCEF_WEB_COBSERVEDRESPONSEBODY_H ? */

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
 *  Receives the response bodies of a CWebResponseTap. Called on the thread that runs CWebResponseTap::Process(); the
 *  response, and its body, are only valid during the call.
 */
class GUCEF_WEB_PUBLIC_CPP CIWebResponseSink
{
    public:

    CIWebResponseSink( void );

    virtual ~CIWebResponseSink();

    virtual void OnResponseBody( const CObservedResponseBody& response ) = 0;

    private:

    CIWebResponseSink( const CIWebResponseSink& src );                  /**< not implemented */
    CIWebResponseSink& operator=( const CIWebResponseSink& src );       /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIWEBRESPONSESINK_H ? */
