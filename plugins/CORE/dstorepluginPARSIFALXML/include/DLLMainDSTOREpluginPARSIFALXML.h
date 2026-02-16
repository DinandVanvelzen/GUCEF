/*
 *  dstorepluginPARSIFALXML: Plugin adding the ability to use Parsifal XML
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
 
#ifndef DLLMAINDSTOREPLUGINPARSIFALXML_H
#define DLLMAINDSTOREPLUGINPARSIFALXML_H

/*
 *      API for a DSTORE codec plugin module
 */
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 
 
#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"                   /* media independant recource I/O */
#define GUCEF_CORE_C_IOACCESS_H
#endif /* IOACCESS_H ? */

#ifndef GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H
#include "DStoreCodecPluginStructs.h"   /* structs specific for this type of plugin */
#define GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H
#endif /* GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"                   /* gucefCORE structures, needed for TVersion */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"           /* gucefCORE macros, used here for the export and callspec macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */ 

#ifndef GUCEF_CORE_C_VARIANTDATA_H
#include "gucefCORE_c_variantdata.h"
#define GUCEF_CORE_C_VARIANTDATA_H
#endif /* GUCEF_CORE_C_VARIANTDATA_H ? */

#ifndef GUCEF_CORE_C_API_H
#include "gucefCORE_c_api.h"
#define GUCEF_CORE_C_API_H
#endif /* GUCEF_CORE_C_API_H ? */
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef DSTOREPLUGINPARSIFALXML_BUILD_MODULE
  #define DSTOREPLUG_EXPORT_C GUCEF_EXPORT
#else
  #define DSTOREPLUG_EXPORT_C
#endif 

#undef MODULE_API_FUNCTION
#ifdef MODULE_API_PREFIX
  #define MODULE_API_FUNCTION( func ) GUCEF_CONCAT( MODULE_API_PREFIX, func )
#else
  #define MODULE_API_FUNCTION( func ) func
#endif

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

DSTOREPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( DSTOREPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( DSTOREPLUG_Shutdown )( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

DSTOREPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Dest_File_Open )( void** plugdata      ,
                                                  void** codecdata     ,
                                                  void** filedata      ,
                                                  TIOAccess* outFile   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                           
/*---------------------------------------------------------------------------*/
                           
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Dest_File_Close )( void** plugdata  , 
                                                   void** codecdata ,
                                                   void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                            
/*---------------------------------------------------------------------------*/
                            
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Begin_Node_Store )( void** plugdata      , 
                                                    void** codecdata     ,
                                                    void** filedata      , 
                                                    const char* nodename , 
                                                    Int32 nodeType       ,
                                                    UInt32 attscount     ,
                                                    UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/
                             
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_End_Node_Store )( void** plugdata      , 
                                                  void** codecdata     ,
                                                  void** filedata      , 
                                                  const char* nodename ,
                                                  UInt32 attscount     ,
                                                  UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                           
/*---------------------------------------------------------------------------*/
                           
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Store_Node_Att )( void** plugdata              , 
                                                  void** codecdata             ,
                                                  void** filedata              , 
                                                  const char* nodename         ,
                                                  UInt32 attscount             , 
                                                  UInt32 attindex              , 
                                                  const char* attname          , 
                                                  const TVariantData* attvalue ,
                                                  UInt32 haschildren           ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                           
/*---------------------------------------------------------------------------*/
                           
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Begin_Node_Children )( void** plugdata      , 
                                                       void** codecdata     ,
                                                       void** filedata      , 
                                                       const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX; 
                                
/*---------------------------------------------------------------------------*/
                                
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_End_Node_Children )( void** plugdata      , 
                                                     void** codecdata     ,
                                                     void** filedata      , 
                                                     const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                                  
/*---------------------------------------------------------------------------*/                                  
                                  
DSTOREPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Src_File_Open )( void** plugdata  , 
                                                 void** codecdata ,
                                                 void** filedata  ,
                                                 TIOAccess* file  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                          
/*---------------------------------------------------------------------------*/
                          
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Src_File_Close )( void** plugdata  , 
                                                  void** codecdata ,
                                                  void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                           
/*---------------------------------------------------------------------------*/
                           
DSTOREPLUG_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Set_Read_Handlers )( void** plugdata                , 
                                                     void** codecdata               ,
                                                     void** filedata                , 
                                                     const TReadHandlers* rhandlers , 
                                                     void* privdata                 ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                              
/*---------------------------------------------------------------------------*/
                             
DSTOREPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
MODULE_API_FUNCTION( DSTOREPLUG_Start_Reading )( void** plugdata  , 
                                                 void** codecdata ,
                                                 void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                  
/*---------------------------------------------------------------------------*/

DSTOREPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( DSTOREPLUG_Type )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
                  
/*---------------------------------------------------------------------------*/

DSTOREPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( DSTOREPLUG_Name )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

DSTOREPLUG_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( DSTOREPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/

DSTOREPLUG_EXPORT_C const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( DSTOREPLUG_Version )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*---------------------------------------------------------------------------*/                 

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*--------------------------------------------------------------------------*/

#endif /* DLLMAINDSTOREPLUGINPARSIFALXML_H ? */
