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

#ifndef GUCEF_CORE_C_LOGTYPES_H
#define GUCEF_CORE_C_LOGTYPES_H

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

#define GUCEF_LOGMSGTYPE_UNKNOWNTYPE    0     /**< reserved for initialization of a ELogMsgType variable with a debug value */

#define GUCEF_LOGMSGTYPE_ERROR          1     /**< I strongly suggest you use this for logging serious errors only */
#define GUCEF_LOGMSGTYPE_WARNING        2     /**< used for something that could be a problem but not necessarily so */
#define GUCEF_LOGMSGTYPE_STANDARD       3     /**< can be anything */
#define GUCEF_LOGMSGTYPE_USER           4     /**< typically info that relates to user actions */
#define GUCEF_LOGMSGTYPE_SYSTEM         5     /**< typically info that relates to the state of of the system */
#define GUCEF_LOGMSGTYPE_DEV            6     /**< typically info that will be of interest to developers */
#define GUCEF_LOGMSGTYPE_DEBUG          7     /**< typically info that wont be logged in a release build, just for debugging */
#define GUCEF_LOGMSGTYPE_SERVICE        8     /**< typically info useful to a service engineer */
#define GUCEF_LOGMSGTYPE_PROTECTED      9     /**< typically used for logging info that should be encrypted before being stored */
#define GUCEF_LOGMSGTYPE_CALLSTACK      10    /**< typically used for logging the call stack of a running application for debugging purposes */
#define GUCEF_LOGMSGTYPE_EXCEPTION      11    /**< typically used for logging exception details just before throwing an exception, see exception macros */
#define GUCEF_LOGMSGTYPE_CONSOLE        12    /**< typically used for logging input/output of the system console, intended to use log interface for a CLI */    


/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_LOGTYPES_H ? */

