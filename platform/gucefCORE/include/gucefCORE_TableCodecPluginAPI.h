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

#ifndef GUCEF_CORE_TABLECODECPLUGINAPI_H
#define GUCEF_CORE_TABLECODECPLUGINAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Version of the C++ interface between gucefCORE and its table codec plugins: ( major << 16 ) | minor.
 *  The plugins implement CITableCodec and its reader, writer and file metadata classes directly, so the layout of
 *  every class of the data table layer is part of this version. A change of those that older plugins cannot cope
 *  with raises the major number. A plugin is accepted when its major number is that of the host and its minor number
 *  is not higher than the one of the host.
 *
 *  Minor versions: 1.1 added CITableFileMetaData::CanVerifySource() and VerifySource(); the host calls them only on
 *  metadata of modules built for 1.1 or later (CTableCodecModuleLifetime::ModuleHasApiMinorVersion).
 */
#define GUCEF_TABLECODECPLUGIN_API_VERSION_MAJOR    1
#define GUCEF_TABLECODECPLUGIN_API_VERSION_MINOR    1
#define GUCEF_TABLECODECPLUGIN_API_VERSION          ( ( GUCEF_TABLECODECPLUGIN_API_VERSION_MAJOR << 16 ) | GUCEF_TABLECODECPLUGIN_API_VERSION_MINOR )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CValueList;
class CITableCodec;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The exports of a table codec plugin module, all with C linkage:
 *
 *      UInt32 TABLECODECPLUGIN_GetApiVersion( void );
 *      Int32 TABLECODECPLUGIN_Init( const CValueList* pluginParams );      0 on success
 *      UInt32 TABLECODECPLUGIN_GetCodecCount( void );
 *      CITableCodec* TABLECODECPLUGIN_CreateCodec( UInt32 index );         the caller owns the codec and deletes it through its virtual destructor
 *      void TABLECODECPLUGIN_Shutdown( void );
 *      const char* TABLECODECPLUGIN_Description( void );
 *      const char* TABLECODECPLUGIN_Copyright( void );
 *      void TABLECODECPLUGIN_Version( TVersion* version );
 */
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_GetApiVersion ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef Int32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_Init ) ( const CValueList* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_GetCodecCount ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef CITableCodec* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_CreateCodec ) ( UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_Shutdown ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_Description ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_Copyright ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFTABLECODECPLUGFPTR_Version ) ( TVersion* version ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TABLECODECPLUGINAPI_H ? */
