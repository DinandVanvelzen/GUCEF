/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H
#define GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H
#include "gucefCOMCORE_CServiceFinderSettings.h"
#define GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H
#endif /* GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How a paired client connects to its remote service (CRemoteServiceConnection), with the values of
 *  gucefASSETSTORE_RemoteNetworking_Design.md 5 and 6 as defaults
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteServiceConnectionSettings
{
    public:

    CORE::CString clientKind;                   /**< told in Hello ("viewer", "worker", "relay", "cli", "probe"); default "client" */
    CORE::CString clientVersion;
    CORE::CString::StringVector capabilities;
    CORE::CString::StringVector wantedContracts;    /**< bound on the control channel besides gucef.drnutils.session (major 1) */
    CORE::CString::StringVector bulkContracts;      /**< bound on the bulk channels (major 1) */
    UInt32 bulkChannels;                        /**< bulk channels (DRN links of their own) attached to the session, 0 to 2; default 0 */
    CORE::CString hostOverride;                 /**< "host:port" tried first instead of the endpoints of the credentials (tests, a second address) */
    UInt32 connectTimeoutMs;                    /**< the blocking Connect(): link, binding and Hello; default 20000 */
    UInt32 linkTimeoutMs;                       /**< DRN link timeout; default 15000 */
    UInt32 echoIntervalMs;                      /**< DRN echo interval; default 1000 */
    UInt32 resumeGraceMs;                       /**< how long the RPC session waits for its resume; default 600000 */
    UInt32 controlWindowBytes;                  /**< RPC receive window of the control channel; default 1 MiB */
    UInt32 controlMaxMessageBytes;              /**< default 4 MiB */
    UInt32 controlSendQueueBytes;               /**< default 4 MiB */
    UInt32 bulkWindowBytes;                     /**< default 8 MiB */
    UInt32 bulkMaxMessageBytes;                 /**< default 1 MiB */
    UInt32 bulkSendQueueBytes;                  /**< default 16 MiB */
    bool autoReconnect;                         /**< reconnect by itself when the link drops; default true */
    UInt32 reconnectFirstMaxMs;                 /**< the first attempt after a lost link within this; default 250 */
    UInt32 reconnectBaseMs;                     /**< full-jitter exponential backoff from this; default 500 */
    UInt32 reconnectMaxMs;                      /**< ... up to this; default 30000 */
    UInt32 reconnectStableResetMs;              /**< the backoff starts over once connected this long; default 60000 */
    UInt32 wakeDetectMs;                        /**< the wall clock ahead of the monotonic clock by more than this between ticks is a sleep of the machine; 0: off; default 5000 */
    UInt32 clockSkewNoticeMs;                   /**< an offset to the service's clock above this raises ClockSkewEvent; default 2000 */
    bool openEventStream;                       /**< open Session/Events after Hello or Resume; default true */
    UInt32 tickIntervalMs;                      /**< how often the connection looks at its links and calls; default 10 */
    bool lanDiscovery;                          /**< when the credentials hold no session endpoint and there is no hostOverride, the blocking Connect() looks on the local network for the paired service (its pinned key fingerprint and service id) and tries what it finds; the configured endpoints always win, a found endpoint is still authenticated by the pinned key; default true */
    COMCORE::CServiceFinderSettings lanDiscoveryFinder; /**< the look: service type (default "_gucef-service._tcp", a product sets its own), findTimeoutMs (default 3000), the discovery clients (default broadcast, multicast) */

    CRemoteServiceConnectionSettings( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESERVICECONNECTIONSETTINGS_H ? */
