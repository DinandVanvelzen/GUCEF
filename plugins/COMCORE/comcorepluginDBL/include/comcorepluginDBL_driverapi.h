/*
 *  comcorepluginDBL: Generic GUCEF COMCORE plugin for providing access to the DBL network driver
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

#ifndef COMCOREPLUGIN_DBL_DRIVERAPI_H
#define COMCOREPLUGIN_DBL_DRIVERAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdint.h>

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef COMCOREPLUGIN_DBL_MACROS_H
#include "comcorepluginDBL_macros.h"
#define COMCOREPLUGIN_DBL_MACROS_H
#endif /* COMCOREPLUGIN_DBL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace COMCOREPLUGIN {
namespace DBL {
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/

COMCOREPLUGIN_DBL_PLUGIN_PUBLIC_C int GUCEF_PLUGIN_CALLSPEC_PREFIX
dblwrapper_init( char failureIsOk ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

COMCOREPLUGIN_DBL_PLUGIN_PUBLIC_C int GUCEF_PLUGIN_CALLSPEC_PREFIX
dblwrapper_shutdown( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

/**
 *  If you have multiple DBL cards this helps to address the correct card
 *  You can use the card index to attain the needed product IDs which is in turn
 *  needed for the other management API functions
 * 
 *  @return the error code on failure and 0 on success
 */
COMCOREPLUGIN_DBL_PLUGIN_PUBLIC_C int32_t GUCEF_PLUGIN_CALLSPEC_PREFIX
dblwrapper_mal_translate_to_product( int64_t nicIndex        , 
                                     char* productIdPartA    , 
                                     int32_t* productIdPartB ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

/**
 *  Opens management access to the specified DBL card
 * 
 *  @return the error code on failure and 0 on success
 */
COMCOREPLUGIN_DBL_PLUGIN_PUBLIC_C int32_t GUCEF_PLUGIN_CALLSPEC_PREFIX
dblwrapper_mal_open( uint32_t productIdPartB , 
                     uint32_t v2             , 
                     void** deviceContext    , 
                     int32_t optionFlags     , 
                     int32_t productIdVar2   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

/**
 *  Closes management access to the specified DBL card
 *  Should be used after a succesfull dblwrapper_mal_open()
 */
COMCOREPLUGIN_DBL_PLUGIN_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
dblwrapper_mal_close( void* deviceContext) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

COMCOREPLUGIN_DBL_PLUGIN_PUBLIC_C int32_t GUCEF_PLUGIN_CALLSPEC_PREFIX
dblwrapper_mal_nic_id_to_str( const char** outStr , 
                              int64_t nicId       , 
                              char flag           ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

#define MAL_IOCTL_INFOTYPEID_NIC                 0x222037
#define MAL_IOCTL_INFOTYPEID_PORTS               0x222033
#define MAL_IOCTL_INFOTYPEID_COUNTERS_METADATA   0x22207b
#define MAL_IOCTL_INFOTYPEID_COUNTERS_STRINGS    0x22207f
#define MAL_IOCTL_INFOTYPEID_COUNTERS            0x222097
#define MAL_IOCTL_INFOTYPEID_COUNTERS_NOCLASSES  0x222083
#define MAL_IOCTL_INFOTYPEID_CLEAR_COUNTERS      0x22208f
#define MAL_IOCTL_INFOTYPEID_IRQ                 0x222087


#define MAL_IOCTL_PROPERTYID_INT32_COUNT      4
#define MAL_IOCTL_PROPERTYID_INT32_ID         0x10

/**
 *  Device control access and query access
 * 
 *  @return the error code on failure and 0 on success
 */
COMCOREPLUGIN_DBL_PLUGIN_PUBLIC_C int32_t GUCEF_PLUGIN_CALLSPEC_PREFIX
dblwrapper_mal_ioctl( void* deviceContext ,
                      int32_t infoTypeId  ,
                      void* inAndOutData  ,
                      char propertyId     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/                 

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace DBL */
}; /* namespace COMCOREPLUGIN */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/

#endif /* COMCOREPLUGIN_DBL_DRIVERAPI_H ? */
