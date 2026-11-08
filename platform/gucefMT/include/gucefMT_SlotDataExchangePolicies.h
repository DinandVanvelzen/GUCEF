/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_MT_SLOTDATAEXCHANGEPOLICIES_H
#define GUCEF_MT_SLOTDATAEXCHANGEPOLICIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_ETYPES_H
#include "gucefMT_ETypes.h"             /* simple types */
#define GUCEF_MT_ETYPES_H
#endif /* GUCEF_MT_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Delivery policy of CTSlotDataExchange: every value that is written is delivered to the single reader
 *  exactly once and in the order it was written. The slots are the burst capacity.
 *  A possible later addition is an overwrite oldest policy, which would drop the oldest unread value
 *  to make room instead of refusing or waiting.
 */
struct SSlotDeliveryAllInOrder {};

/**
 *  Delivery policy of CTSlotDataExchange: readers get the newest complete value, values that were not
 *  read before a newer one arrived are skipped. The writer never waits.
 */
struct SSlotDeliveryLatestOnly {};

/** Full policy of the in order delivery: when no slot is free Write() returns false at once */
struct SSlotFullFail {};

/** Full policy of the in order delivery: when no slot is free Write() waits for one, up to a timeout */
struct SSlotFullBlock {};

/** Timeout value that never expires */
const UInt32 SlotWaitForever = 0xFFFFFFFFU;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_SLOTDATAEXCHANGEPOLICIES_H ? */
