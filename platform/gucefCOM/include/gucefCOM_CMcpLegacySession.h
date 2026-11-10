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

#ifndef GUCEF_COM_CMCPLEGACYSESSION_H
#define GUCEF_COM_CMCPLEGACYSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPIMPLEMENTATIONINFO_H
#include "gucefCOM_CMcpImplementationInfo.h"
#define GUCEF_COM_CMCPIMPLEMENTATIONINFO_H
#endif /* GUCEF_COM_CMCPIMPLEMENTATIONINFO_H ? */

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
 *  The 2025-11-25 session of a stream channel (stdio, one TCP connection), opened by initialize. It holds what the
 *  handshake negotiated, which legacy requests on the channel use. A second initialize replaces it.
 *  Lives on the protocol thread of the server only.
 */
class GUCEF_COM_PUBLIC_CPP CMcpLegacySession
{
    public:

    CORE::CString protocolVersion;                  /**< the negotiated version */
    CORE::CDataNode clientCapabilities;             /**< the ClientCapabilities of initialize, an object node */
    bool hasClientInfo;
    CMcpImplementationInfo clientInfo;              /**< as reported, never verified */
    bool isOperational;                             /**< notifications/initialized arrived */
    CORE::CString::StringSet resourceSubscriptions; /**< the URIs of resources/subscribe */

    CMcpLegacySession( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPLEGACYSESSION_H ? */
