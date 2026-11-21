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

#ifndef GUCEF_WEB_COBSERVEDRESPONSEBODY_H
#define GUCEF_WEB_COBSERVEDRESPONSEBODY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  The body of a response the page itself requested, as a CWebResponseTap hands it to a CIWebResponseSink.
 *  It refers to the tap's buffer: valid only for the duration of CIWebResponseSink::OnResponseBody(), after which the
 *  buffer is overwritten with zeros. A sink that needs the data later copies what it needs, and it is the sink's data
 *  to protect: it never goes to disk, a journal, a report or a log by way of this facility.
 */
class GUCEF_WEB_PUBLIC_CPP CObservedResponseBody
{
    public:

    CORE::CString method;               /**< upper case */
    CORE::CString host;                 /**< lower case, with the port when the address has one */
    CORE::CString pathTemplate;         /**< as CUrlPathTemplate makes it, never the query */
    Int32 status;
    CORE::CString mimeType;
    UInt32 requestSeq;                  /**< the tap's count of matching requests, in the order the page made them */
    UInt32 tapId;                       /**< the tap it is delivered for */
    CORE::CString::StringMap requestHeaders;    /**< the headers of the request, for a tap that keeps them (CWebResponseTap::SetTapKeepsRequestHeaders()); they may carry the session of the page: never logged or stored */
    CORE::CString::StringMap queryValues;   /**< the query parameters of the request that the tap keeps (CWebResponseTap::SetTapQueryKeys()), as the address has them; empty for a tap that keeps none */
    const CORE::CDynamicBuffer& body;   /**< decoded, the bytes the page received */

    explicit CObservedResponseBody( const CORE::CDynamicBuffer& responseBody );

    CObservedResponseBody( const CObservedResponseBody& src );

    private:

    CObservedResponseBody& operator=( const CObservedResponseBody& src );       /**< not implemented: the body is a reference */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_COBSERVEDRESPONSEBODY_H ? */
