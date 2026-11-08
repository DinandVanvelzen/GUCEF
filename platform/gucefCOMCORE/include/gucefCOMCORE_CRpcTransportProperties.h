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

#ifndef GUCEF_COMCORE_CRPCTRANSPORTPROPERTIES_H
#define GUCEF_COMCORE_CRPCTRANSPORTPROPERTIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  What a transport offers the RPC session (gucefCOMCORE RPC design 10.2)
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcTransportProperties
{
    public:

    enum ELatencyClass
    {
        LATENCY_LOCAL             = 0 ,
        LATENCY_LAN               = 1 ,
        LATENCY_WAN               = 2 ,
        LATENCY_STORE_AND_FORWARD = 3
    };
    typedef enum ELatencyClass TLatencyClass;

    bool isReliable;                    /**< the transport never loses a frame while connected */
    bool isOrdered;                     /**< frames arrive in the order sent */
    bool isSecure;                      /**< confidentiality and integrity of every frame (TLS, DRN AEAD, in process) */
    bool hasFrameProtectionClasses;     /**< honours a protection class per frame (drn with item protection) */
    UInt32 maxFrameSize;                /**< largest frame the transport carries in one piece */
    TLatencyClass latencyClass;
    CORE::CString profileName;          /**< "loopback", "tcp", "drn", "udp", "mail", ... see CRpcTransportProfile */

    CRpcTransportProperties( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCTRANSPORTPROPERTIES_H ? */
