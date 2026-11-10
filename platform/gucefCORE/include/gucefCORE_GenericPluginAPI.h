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

#ifndef GUCEF_CORE_GENERICPLUGINAPI_H
#define GUCEF_CORE_GENERICPLUGINAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"         /* often used gucefCORE macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"                 /* often used data structures */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

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

/*
 *  Mandatory generic plugin API functions
 */
typedef Int32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFGENERICPLUGFPTR_Load ) ( UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFGENERICPLUGFPTR_Unload ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFGENERICPLUGFPTR_GetVersion ) ( TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFGENERICPLUGFPTR_GetCopyright ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFGENERICPLUGFPTR_GetDescription ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*
 *  Optional generic plugin API functions
 */

/**
 *  Optional export: returns a comma-separated list of LinkBack module names this plugin
 *  depends on (e.g. "gucefMT,gucefCORE,gucefPUBSUB").
 *  When present, the returned list supplements or overrides any LinkBackModules value
 *  specified in the plugin's metadata configuration.
 *  The returned pointer must remain valid for the lifetime of the loaded module.
 *  No spaces should be used in the list.
 */
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TGUCEFGENERICPLUGFPTR_GetLinkBackModuleDependencies ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

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

#endif /* GUCEF_CORE_GENERICPLUGINAPI_H ? */
