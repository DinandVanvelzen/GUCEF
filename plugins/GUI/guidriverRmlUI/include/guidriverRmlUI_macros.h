/*
 *  guidriverRmlUI: GUI backend using RmlUI
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

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#define GUCEF_guidriverRmlUI_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverRmlUI_ETYPES_H
#include "guidriverRmlUI_ETypes.h"
#define GUCEF_guidriverRmlUI_ETYPES_H
#endif /* GUCEF_guidriverRmlUI_ETYPES_H ? */

#ifndef GUCE_MYGUIOGRE_CONFIG_H
#include "guidriverRmlUI_config.h"        /* Module build configuration */
#define GUCE_MYGUIOGRE_CONFIG_H
#endif /* GUCE_MYGUIOGRE_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Macros for dynamic linking or static linking. Use the switches in the
 *      config file to control the export type.
 */
#undef GUCEF_EXPORTSPEC
#ifdef GUCEF_guidriverRmlUI_BUILD_MODULE
  #define GUCEF_EXPORTSPEC GUCEF_EXPORT
#else
  #define GUCEF_EXPORTSPEC GUCEF_IMPORT
#endif /* GUCE_MYGUIOGRE_BUILD_MODULE ? */

#undef GUCEF_guidriverRmlUI_PUBLIC_CPP
#ifdef GUCEF_guidriverRmlUI_PUBLIC_CPP_CODE
  #define GUCEF_guidriverRmlUI_PUBLIC_CPP GUCEF_EXPORTSPEC
#else
  #define GUCEF_guidriverRmlUI_PUBLIC_CPP GUCEF_HIDDEN
#endif /* GUCEF_guidriverRmlUI_PUBLIC_CPP_CODE */

#undef GUCEF_guidriverRmlUI_PRIVATE_CPP
#define GUCEF_guidriverRmlUI_PRIVATE_CPP GUCEF_HIDDEN

#undef GUCEF_guidriverRmlUI_PUBLIC_C
#ifdef GUCEF_guidriverRmlUI_PUBLIC_C_CODE
  #define GUCEF_guidriverRmlUI_PUBLIC_C GUCEF_EXPORTSPEC
#else
  #define GUCEF_guidriverRmlUI_PUBLIC_C GUCEF_HIDDEN
#endif /* GUCEF_guidriverRmlUI_PUBLIC_C_CODE */

#undef GUCEF_guidriverRmlUI_PRIVATE_C
#define GUCEF_guidriverRmlUI_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-03-2007 :
        - Dinand: re-added this header

-----------------------------------------------------------------------------*/

