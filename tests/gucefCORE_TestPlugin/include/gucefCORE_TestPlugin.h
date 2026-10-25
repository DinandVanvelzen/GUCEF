/*
 *  gucefCORE_TestPlugin: generic plugin used by the gucefCORE tests of the plugin system
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_TESTPLUGIN_H
#define GUCEF_CORE_TESTPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_CORE_TESTPLUGIN_BUILD_MODULE
  #define GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF_EXPORT
#else
  #define GUCEF_CORE_TESTPLUGIN_EXPORT_C
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  A generic plugin that does nothing but what the tests ask of it through environment variables:
 *  GUCEF_TESTPLUGIN_LOAD_THROWS=1 and GUCEF_TESTPLUGIN_UNLOAD_THROWS=1 make GUCEFPlugin_Load() and GUCEFPlugin_Unload()
 *  throw, as a broken plugin would. GUCEF_TESTPLUGIN_LOAD_FAILS=1 makes GUCEFPlugin_Load() return that it failed, and
 *  GUCEF_TESTPLUGIN_LOAD_ADDS_FAILING_CONFIGURABLE=1 makes it bring along a global configurable that fails to load its config,
 *  as the globals of a module that a plugin starts can.
 *  It counts the calls of Load and Unload in GUCEF_TESTPLUGIN_LOADS and GUCEF_TESTPLUGIN_UNLOADS.
 */

#ifdef __cplusplus
extern "C" {
#endif

GUCEF_CORE_TESTPLUGIN_EXPORT_C GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( GUCEF::CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( GUCEF::CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

GUCEF_CORE_TESTPLUGIN_EXPORT_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

#ifdef __cplusplus
}
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTPLUGIN_H ? */
