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

#ifndef GUCEF_CORE_LOGTYPES_H
#define GUCEF_CORE_LOGTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"    
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_LOGTYPES_H
#include "gucefCORE_c_logtypes.h"
#define GUCEF_CORE_C_LOGTYPES_H
#endif /* GUCEF_CORE_C_LOGTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum ELogMsgType
{
    LOG_UNKNOWNTYPE = GUCEF_LOGMSGTYPE_UNKNOWNTYPE, /**< reserved for initialization of a ELogMsgType variable with a debug value */
    
    LOG_ERROR = GUCEF_LOGMSGTYPE_ERROR,             /**< I strongly suggest you use this for logging serious errors only */
    LOG_WARNING = GUCEF_LOGMSGTYPE_WARNING,         /**< used for something that could be a problem but not necessarily so */
    LOG_STANDARD = GUCEF_LOGMSGTYPE_STANDARD,       /**< can be anything */
    LOG_USER = GUCEF_LOGMSGTYPE_USER,               /**< typically info that relates to user actions */
    LOG_SYSTEM = GUCEF_LOGMSGTYPE_SYSTEM,           /**< typically info that relates to the state of of the system */
    LOG_DEV = GUCEF_LOGMSGTYPE_DEV,                 /**< typically info that will be of interest to developers */
    LOG_DEBUG = GUCEF_LOGMSGTYPE_DEBUG,             /**< typically info that wont be logged in a release build, just for debugging */
    LOG_SERVICE = GUCEF_LOGMSGTYPE_SERVICE,         /**< typically info useful to a service engineer */
    LOG_PROTECTED = GUCEF_LOGMSGTYPE_PROTECTED,     /**< typically used for logging info that should be encrypted before being stored */
    LOG_CALLSTACK = GUCEF_LOGMSGTYPE_CALLSTACK,     /**< typically used for logging the call stack of a running application for debugging purposes */
    LOG_EXCEPTION = GUCEF_LOGMSGTYPE_EXCEPTION,     /**< typically used for logging exception details just before throwing an exception, see exception macros */
    LOG_CONSOLE = GUCEF_LOGMSGTYPE_CONSOLE          /**< typically used for logging input/output of the system console, intended to use log interface for a CLI */  
};
typedef enum ELogMsgType TLogMsgType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_LOGTYPES_H ? */

