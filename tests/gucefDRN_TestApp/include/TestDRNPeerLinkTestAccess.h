/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNPEERLINKTESTACCESS_H
#define GUCEF_DRN_TESTAPP_TESTDRNPEERLINKTESTACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The friend of CDRNPeerLink that the tests of the receiver's enforcement use: it queues a message in exactly the
 *  given packet kind, without the resolution CDRNPeerLink::Send() applies. Defined by the test application only.
 */
class CDRNPeerLinkTestAccess
{
    public:

    static bool SendAsPacketKind( CDRNPeerLink& link, UInt8 lane, UInt8 msgClass, UInt32 opcode, const void* payload, UInt32 payloadSize, UInt8 packetKind )
    {
        return link.SendAsPacketKind( lane, msgClass, opcode, payload, payloadSize, packetKind );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNPEERLINKTESTACCESS_H ? */
