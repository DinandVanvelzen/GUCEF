/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

/* 
 *  This header provides a basic C interface to Data Storage (DStore) codec plugins
 *  The purpose of these functions is to provide a way to link against the
 *  C++ module and start a sequence of events (trigger use-cases) without being
 *  tied to C++ linking. Instead you can do dynamic linking against these C
 *  functions to provide a more resiliant bridge to C++ modules, isolating you
 *  from changes within the C++ code which would normally require re-linking.
 */
 
#ifndef GUCEF_CORE_C_DSTORE_PLUGIN_API_H
#define GUCEF_CORE_C_DSTORE_PLUGIN_API_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_C_API_H
#include "gucefCORE_c_api.h"
#define GUCEF_CORE_C_API_H
#endif /* GUCEF_CORE_C_API_H ? */

#ifndef GUCEF_CORE_C_VARIANTDATA_H
#include "gucefCORE_c_variantdata.h"
#define GUCEF_CORE_C_VARIANTDATA_H
#endif /* GUCEF_CORE_C_VARIANTDATA_H ? */

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

#ifndef GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H
#include "DStoreCodecPluginStructs.h"   /* shared structs */
#define GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H
#endif /* GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H ? */

#ifndef GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H
#include "gucefCORE_c_datadriven_dstorecodec_meta.h"
#define GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H
#endif /* GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Init )                ( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Shutdown )              ( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Name )           ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Copyright )      ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const TVersion* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Version )    ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Type )           ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Type_Is_Data_Driven )  ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Create_Data_Driven_Codec ) ( const void* plugdata                  ,
                                                                                           TDataDrivenDStoreCodecMeta* codecMeta ,
                                                                                           TVariantMapApi* loadedResources       ,
                                                                                           void** dataDrivenCodecPrivateData     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt8 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Destroy_Data_Driven_Codec ) ( const void* plugdata                  ,
                                                                                            TDataDrivenDStoreCodecMeta* codecMeta ,
                                                                                            void** dataDrivenCodecPrivateData     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Dest_File_Open )      ( void** plugdata, void** filedata, TIOAccess* file ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Dest_File_Close )       ( void** plugdata, void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Begin_Node_Store )      ( void** plugdata, void** filedata, const char* nodename, int nodeType, UInt32 attscount, UInt32 haschildren ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_End_Node_Store )        ( void** plugdata, void** filedata, const char* nodename, UInt32 attscount, UInt32 haschildren ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Store_Node_Att )        ( void** plugdata, void** filedata, const char* nodename, UInt32 attscount, UInt32 attindex, const char* attname, const TVariantData* attvalue, UInt32 haschildren ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Begin_Node_Children )   ( void** plugdata, void** filedata, const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_End_Node_Children )     ( void** plugdata, void** filedata, const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Src_File_Open )       ( void** plugdata, void** codecdata, void** filedata, TIOAccess* file ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Src_File_Close )        ( void** plugdata, void** codecdata, void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Set_Read_Handlers )     ( void** plugdata, void** codecdata, void** filedata, const TReadHandlers* rhandlers, void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_Start_Reading )       ( void** plugdata, void** codecdata, void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

/**
 *  Structure that hold all the function pointers to the C API functions for a DStore Codec plugin
 */
struct SGucefCoreCDStorePluginApi
{
    TDSTOREPLUGFPTR_Init Init;                                             /**< Function to initialize the plugin, e.g. allocate memory, etc. */
    TDSTOREPLUGFPTR_Shutdown Shutdown;                                     /**< Function to shutdown the plugin, e.g. free memory, etc. */
    TDSTOREPLUGFPTR_Name Name;                                             /**< Function to get the name of the plugin, e.g. "My Awesome DStore codec plugin" */
    TDSTOREPLUGFPTR_Copyright Copyright;                                   /**< Function to get the copyright of the plugin, e.g. "Licensed under the Apache License, Version 2.0" */
    TDSTOREPLUGFPTR_Version Version;                                       /**< Function to get the semantic version of the plugin, e.g. "1.0.0" */
    TDSTOREPLUGFPTR_Type Type;                                             /**< Function to get the type of the plugin, e.g. "protobuf", "json", etc. */

    TDSTOREPLUGFPTR_Type_Is_Data_Driven Type_Is_Data_Driven;               /**< Optional function to determine if the plugin is data driven, default false (0) if not present */
    TDSTOREPLUGFPTR_Create_Data_Driven_Codec Create_Data_Driven_Codec;     /**< Optional function to create a data driven codec, Mandatory for data driven codec plugins if Type_Is_Data_Driven returns true (1). Factory function returns 0 or 1 to indicate success */
    TDSTOREPLUGFPTR_Destroy_Data_Driven_Codec Destroy_Data_Driven_Codec;   /**< Optional function to destroy a data driven codec, Mandatory for data driven codec plugins if Type_Is_Data_Driven returns true (1). returns 0 or 1 to indicate success */

    TDSTOREPLUGFPTR_Dest_File_Open Dest_File_Open;
    TDSTOREPLUGFPTR_Dest_File_Close Dest_File_Close;
    TDSTOREPLUGFPTR_Begin_Node_Store Begin_Node_Store;
    TDSTOREPLUGFPTR_End_Node_Store End_Node_Store;
    TDSTOREPLUGFPTR_Store_Node_Att Store_Node_Att;
    TDSTOREPLUGFPTR_Begin_Node_Children Begin_Node_Children;
    TDSTOREPLUGFPTR_End_Node_Children End_Node_Children;
    
    TDSTOREPLUGFPTR_Src_File_Open Src_File_Open;
    TDSTOREPLUGFPTR_Src_File_Close Src_File_Close;
    TDSTOREPLUGFPTR_Set_Read_Handlers Set_Read_Handlers;
    TDSTOREPLUGFPTR_Start_Reading Start_Reading;

};
typedef struct SGucefCoreCDStorePluginApi TGucefCoreCDStorePluginApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/                 

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_DSTORE_PLUGIN_API_H ? */
