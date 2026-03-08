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

#ifndef GUCEF_DRGUP_CONFIG_H
#define GUCEF_DRGUP_CONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"      /* GUCEF platform build configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD DEFINES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Are we building the module itself or are we simply linking it ?
 *  If possible you can also set this define in your project instead of altering the switch below
 */
/* #define GUCEF_DRGUP_BUILD_MODULE */

/*
 *      If we are building or using a DLL then it is sometimes desireable to
 *      export only C code or perhaps only C++ code. You can do this with the
 *      following defines
 */
#undef GUCEF_DRGUP_PUBLIC_CPP_CODE /* do not change this line */
#undef GUCEF_DRGUP_PUBLIC_C_CODE   /* do not change this line */
#define GUCEF_DRGUP_PUBLIC_CPP_CODE /* do you want to enable the C++ exports ? */
#define GUCEF_DRGUP_PUBLIC_C_CODE   /* do you want to enable the C exports ? */

/*
 *      Switch htat activates or deactivates the redirection of OLE's 
 *      memory management APIs
 */
#undef MEMCHECK_OLEAPI  /* do not change this line */
#define MEMCHECK_OLEAPI

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRGUP_CONFIG_H */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-10-2004 :
       - Dinand: Initial version of this file.

-----------------------------------------------------------------------------*/

