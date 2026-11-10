/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_CUDPDISCOVERYCODEC_H
#define GUCEF_COM_CUDPDISCOVERYCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYENTRY_H
#include "gucefCOMCORE_CDiscoveryEntry.h"
#define GUCEF_COMCORE_CDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CDISCOVERYENTRY_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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
 *  The beacon of the broadcast and multicast discovery clients:
 *
 *      ['G','D','S','C'][u8 version][u8 kind][string service type][string instance id][string instance name]
 *      [u16 ttl][u16 endpoint count][endpoint strings][u16 attribute count][attribute key and value strings]
 *
 *  Strings are a u16 length followed by that many bytes of UTF-8, integers are big endian.
 *  A query only has the service type, the strings and numbers after it are empty and 0.
 */
class GUCEF_COM_PUBLIC_CPP CUdpDiscoveryCodec
{
    public:

    enum EBeaconKind
    {
        BEACONKIND_QUERY    = 1 ,
        BEACONKIND_ANNOUNCE = 2 ,
        BEACONKIND_GOODBYE  = 3
    };
    typedef enum EBeaconKind TBeaconKind;

    static const UInt8 ProtocolVersion;

    /** The largest beacon written: it has to fit in one datagram without fragmentation */
    static const UInt32 MaxBeaconSizeInBytes;

    /**
     *  Writes a query for the given service type, replacing the content of the buffer
     */
    static bool EncodeQuery( const CORE::CString& serviceType ,
                             CORE::CDynamicBuffer& beacon     );

    /**
     *  Writes an announcement or goodbye for the given entry, replacing the content of the buffer.
     *  The instance id is the application instance id of the entry.
     *
     *  @return false when the entry does not fit in MaxBeaconSizeInBytes
     */
    static bool EncodeEntry( TBeaconKind kind                         ,
                             const COMCORE::CIDiscoveryEntry& entry   ,
                             CORE::CDynamicBuffer& beacon             );

    /**
     *  Reads a beacon. For a query only the service type of the entry is set.
     *
     *  @return false if the data is not a valid beacon
     */
    static bool Decode( const void* data                    ,
                        UInt32 dataSize                     ,
                        TBeaconKind& kind                   ,
                        COMCORE::CDiscoveryEntry& entry     );

    /** Quick check whether the data starts like a beacon */
    static bool IsBeacon( const void* data, UInt32 dataSize );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CUDPDISCOVERYCODEC_H ? */
