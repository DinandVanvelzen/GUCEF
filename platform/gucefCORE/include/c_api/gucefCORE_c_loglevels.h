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

#ifndef GUCEF_CORE_C_LOGLEVELS_H
#define GUCEF_CORE_C_LOGLEVELS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"    
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 * A number of optional constants that can be used to define log levels.
 * These levels are used by the library itself. The lower the importance of the log level
 * the more likely it will be filtered out by most logging backends
 */
#define GUCEF_LOGLEVEL_CRITICAL         (GUCEF_INT32MAX - 1)
#define GUCEF_LOGLEVEL_VERY_IMPORTANT   (250000)
#define GUCEF_LOGLEVEL_IMPORTANT        (100000)
#define GUCEF_LOGLEVEL_NORMAL           (50000)
#define GUCEF_LOGLEVEL_BELOW_NORMAL     (25000)
#define GUCEF_LOGLEVEL_EVERYTHING       (0)

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_LOGLEVELS_H ? */
