/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_C_VARIANTMAP_H
#define GUCEF_CORE_C_VARIANTMAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_VARIANTDATA_H
#include "gucefCORE_c_variantdata.h"
#define GUCEF_CORE_C_VARIANTDATA_H
#endif /* GUCEF_CORE_C_VARIANTDATA_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_insert ) ( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_set ) ( void* privdata, TVariantData* key, TVariantData* value, UInt8 linkIfPossible ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_clear ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef size_t ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_size ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_erase ) ( void* privdata, TVariantData* searchKey ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_at_key ) ( void* privdata, TVariantData* searchKey, TVariantData* outValue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_at_index ) ( void* privdata, UInt32 index, TVariantData* outValue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef Int8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_is_const ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_type_id_of_key_elements ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_byte_size_of_key_element_type ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_type_id_of_value_elements ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TVariantMapFunc_byte_size_of_value_element_type ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *  Structure that hold all the function pointers to the event handlers
 *  for read events. Should be called by plugins as needed.
 */
struct SVariantMapApi
{
    /**
     *  The following are function pointers for functions which mimic the std::map interface
     *  These work with a read-only map
     */
    TVariantMapFunc_at_key       at_key;   /**< attempts to retrieve an existing key's value pair from the map. It does not create new entries if no such key exists */
    TVariantMapFunc_at_index     at_index; /**< attempts to retrieve an existing map entry's value from the map. It does not create new entries if the offset is invalid */
    TVariantMapFunc_size         size;     /**< returns the number of key-value pairs in the map */

    /**
     *  The following are function pointers for functions which mimic the std::map interface
     *  These require a writeable map
     */    
    TVariantMapFunc_insert       insert;   /**< attempts to insert a key-value pair into the map. It does not overwrite thus will fail if the key exists */
    TVariantMapFunc_set          set;      /**< attempts to set an existing key's value pair into the map. It does not overwrite thus will fail if the key exists */
    TVariantMapFunc_clear        clear;    /**< clears the map */
    TVariantMapFunc_erase        erase;    /**< attempts to erase a key-value pair from the map, if the key doesnt exists the operation fails and is a no-op */

    /**
     *  The following are function pointers for functions which are custom helpers
     *  keeping in mind that not information is as easily available across the C API boundary
     *  These work with a read-only map
     */
    TVariantMapFunc_is_const                        is_const;
    TVariantMapFunc_type_id_of_key_elements         type_id_of_key_elements;
    TVariantMapFunc_byte_size_of_key_element_type   byte_size_of_key_element_type;
    TVariantMapFunc_type_id_of_value_elements       type_id_of_value_elements;    
    TVariantMapFunc_byte_size_of_value_element_type byte_size_of_value_element_type;
    
    void* privateData;
};
typedef struct SVariantMapApi TVariantMapApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_VARIANTMAP_H ? */
