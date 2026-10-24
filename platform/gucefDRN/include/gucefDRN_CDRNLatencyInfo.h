/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNLATENCYINFO_H
#define GUCEF_DRN_CDRNLATENCYINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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
 *  Latency figures of a link. All times are in microseconds.
 */
class GUCEF_DRN_EXPORT_CPP CDRNLatencyInfo
{
    public:

    UInt32 smoothedRtt;         /**< EWMA 1/8 of the round trip time */
    UInt32 rttVariance;         /**< EWMA 1/4 of the deviation from the smoothed RTT: the jitter */
    UInt32 minRtt;              /**< lowest RTT of the last 10 seconds */
    UInt32 lastRtt;             /**< the most recent sample */
    Float32 lossRatio;          /**< share of the last 256 packets from the peer that were lost */
    Int64 clockOffset;          /**< peer link clock minus local link clock */
    bool hasClockOffset;        /**< false until the first sample */
    UInt64 sampleTime;          /**< local link time of the most recent sample */
    UInt32 sampleCount;         /**< number of RTT samples taken */

    CDRNLatencyInfo( void );

    void Clear( void );

    Float64 GetSmoothedRttInMs( void ) const;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTCloneableObj< CDRNLatencyInfo > CDRNLatencyInfoEventData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNLATENCYINFO_H ? */
