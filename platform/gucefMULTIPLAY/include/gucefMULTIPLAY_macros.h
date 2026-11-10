/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_MACROS_H
#define GUCEF_MULTIPLAY_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h" 
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_MULTIPLAY_TYPES_H
#include "gucefMULTIPLAY_Types.h"   /* simple types */
#define GUCEF_MULTIPLAY_TYPES_H
#endif /* GUCEF_MULTIPLAY_TYPES_H ? */

#ifndef GUCEF_MULTIPLAY_CONFIG_H
#include "gucefMULTIPLAY_config.h"  /* Module build configuration */
#define GUCEF_MULTIPLAY_CONFIG_H
#endif /* GUCEF_MULTIPLAY_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Macros for dynamic linking. Use the switches in the
 *      config file to control the export type.
 */

#ifdef GUCEFMULTIPLAY_BUILD_MODULE
  #define GUCEF_MULTIPLAY_BUILD_MODULE
#endif /* GUCEFMULTIPLAY_BUILD_MODULE ? */

#undef GUCEF_MULTIPLAY_EXPORT
#ifdef GUCEF_MULTIPLAY_BUILD_MODULE
  #define GUCEF_MULTIPLAY_EXPORT GUCEF_EXPORT
#else
  #define GUCEF_MULTIPLAY_EXPORT GUCEF_IMPORT
#endif /* GUCEF_MULTIPLAY_BUILD_MODULE ? */

/*
 *  Are we exporting C++ code ?
 */
#undef GUCEF_MULTIPLAY_EXPORT_CPP
#ifdef GUCEF_MULTIPLAY_EXPORT_CPP_CODE
  #define GUCEF_MULTIPLAY_EXPORT_CPP GUCEF_MULTIPLAY_EXPORT
#else
  #define GUCEF_MULTIPLAY_EXPORT_CPP
#endif /* GUCEF_MULTIPLAY_EXPORT_CPP_CODE ? */

/*
 *  Are we exporting C code ?
 */
#undef GUCEF_MULTIPLAY_EXPORT_C
#ifdef GUCEF_MULTIPLAY_EXPORT_C_CODE
  #define GUCEF_MULTIPLAY_EXPORT_C GUCEF_MULTIPLAY_EXPORT
#else
  #define GUCEF_MULTIPLAY_EXPORT_C
#endif /* GUCEF_MULTIPLAY_EXPORT_C_CODE ? */  

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_MACROS_H ? */