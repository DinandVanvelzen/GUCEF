/*
 *  guidriverWin32: module implementing window management for Win32
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUIDRIVERWIN32_PLUGINAPI_H
#define GUIDRIVERWIN32_PLUGINAPI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUIDRIVERWIN32_MACROS_H
#include "GUIDRIVERWIN32_macros.h"
#define GUIDRIVERWIN32_MACROS_H
#endif /* GUIDRIVERWIN32_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace GUIDRIVERWIN32 {
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
#endif  /* __cplusplus */

/*-------------------------------------------------------------------------*/

GUIDRIVERWIN32_PUBLIC_C CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERWIN32_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERWIN32_PUBLIC_C void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERWIN32_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERWIN32_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

GUIDRIVERWIN32_PUBLIC_C const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   };
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace GUIDRIVERWIN32 */
}; /* namespace GUCEF */
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERWIN32_PLUGINAPI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/