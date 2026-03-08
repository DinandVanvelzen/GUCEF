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

#ifndef GUCEF_DRGUP_MACROS_H
#define GUCEF_DRGUP_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRGUP_ETYPES_H
#include "gucefDRGUP_ETypes.h"
#define GUCEF_DRGUP_ETYPES_H
#endif /* GUCEF_DRGUP_ETYPES_H ? */

#ifndef GUCEF_DRGUP_CONFIG_H
#include "gucefDRGUP_config.h"        /* Module build configuration */
#define GUCEF_DRGUP_CONFIG_H
#endif /* GUCEF_DRGUP_CONFIG_H ? */

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"           /* include the GUCEF common macros */
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Auto detection of debug mode
 */
#ifndef MANUAL_DEBUG_MODE_DEFINE
  #if !defined( NDEBUG ) && !( defined( _DEBUG ) || defined( __DEBUG ) || defined( __DEBUG__ ) )
    #define _DEBUG
  #endif
  #if defined( _DEBUG ) || defined( __DEBUG ) || defined( __DEBUG__ )
    #define GUCEF_DRGUP_DEBUG_MODE
  #else
    #undef GUCEF_DRGUP_DEBUG_MODE
  #endif /* compiler DEBUG switches */
#endif /* MANUAL_DEBUG_MODE_DEFINE ? */

/*
 *      Macros for dynamic linking or static linking. Use the switches in the
 *      config file to control the export type.
 */

#undef GUCEF_DRGUP_EXPORTSPEC
#if defined( GUCEF_DRGUP_BUILD_MODULE )
  #define GUCEF_DRGUP_EXPORTSPEC GUCEF_EXPORT
#elif defined( GUCEF_DRGUP_EMBED_CODE )
  #define GUCEF_DRGUP_EXPORTSPEC
#else
  #define GUCEF_DRGUP_EXPORTSPEC GUCEF_IMPORT
#endif /* GUCEF_DRGUP_BUILD_MODULE ? */

#undef GUCEF_DRGUP_PUBLIC_CPP
#ifdef GUCEF_DRGUP_PUBLIC_CPP_CODE
  #define GUCEF_DRGUP_PUBLIC_CPP GUCEF_DRGUP_EXPORTSPEC
#else
  #define GUCEF_DRGUP_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_DRGUP_EXPORT_CPP_CODE */

#define GUCEF_DRGUP_PRIVATE_CPP GUCEF_HIDDEN

#undef GUCEF_DRGUP_PUBLIC_C
#ifdef GUCEF_DRGUP_PUBLIC_C_CODE
  #define GUCEF_DRGUP_PUBLIC_C GUCEF_DRGUP_EXPORTSPEC
#else
  #define GUCEF_DRGUP_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_DRGUP_PUBLIC_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRGUP_MACROS_H ? */
