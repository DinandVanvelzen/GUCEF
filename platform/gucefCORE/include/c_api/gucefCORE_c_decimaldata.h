/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_C_DECIMALDATA_H
#define GUCEF_CORE_C_DECIMALDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stddef.h>         /* offsetof */

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"           /* include the GUCEF common config */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Number of payload bytes of a decimal: the 8 byte unscaled value followed by the 1 byte scale */
#define GUCEF_DECIMAL_PAYLOAD_SIZE      9

/** Largest decimal scale: value = unscaled * 10^-scale with scale in 0..18 */
#define GUCEF_DECIMAL_MAX_SCALE         18

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Exact decimal value: value = unscaled * 10^-scale
 *  The unscaled value is never INT64_MIN, the value range is symmetric.
 *  Only the first GUCEF_DECIMAL_PAYLOAD_SIZE bytes carry data, any padding after the scale is not part of the value.
 */
struct SDecimalData
{
    Int64 unscaled;     /**< two's complement, byte order as named by the type id, never INT64_MIN */
    UInt8 scale;        /**< 0..18 */
};
typedef struct SDecimalData TDecimalData;

/*
 *  Compile time checks of the layout: the payload is contiguous with the scale directly after the unscaled value
 *  C++98 compatible negative array size idiom
 */
typedef char GUCEF_CORE_DecimalDataScaleOffsetCheck[ ( offsetof( TDecimalData, scale ) == 8 ) ? 1 : -1 ];
typedef char GUCEF_CORE_DecimalDataSizeCheck[ ( sizeof( TDecimalData ) >= GUCEF_DECIMAL_PAYLOAD_SIZE ) ? 1 : -1 ];

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_DECIMALDATA_H ? */
