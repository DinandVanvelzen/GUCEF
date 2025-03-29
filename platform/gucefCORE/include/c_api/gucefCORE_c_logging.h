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

#ifndef GUCEF_CORE_C_LOGGING_H
#define GUCEF_CORE_C_LOGGING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"    
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_API_H
#include "gucefCORE_c_api.h"
#define GUCEF_CORE_C_API_H
#endif /* GUCEF_CORE_C_API_H ? */

#ifndef GUCEF_CORE_C_LOGLEVELS_H
#include "gucefCORE_c_loglevels.h"
#define GUCEF_CORE_C_LOGLEVELS_H
#endif /* GUCEF_CORE_C_LOGLEVELS_H ? */

#ifndef GUCEF_CORE_C_LOGTYPES_H
#include "gucefCORE_c_logtypes.h"
#define GUCEF_CORE_C_LOGTYPES_H
#endif /* GUCEF_CORE_C_LOGTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  We allow all logging calls to be removed with the define 'GUCEF_NO_LOGGING'
 */
#ifndef GUCEF_NO_LOGGING

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_ERROR_LOG
#define GUCEF_C_ERROR_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_ERROR, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_LOG
#define GUCEF_C_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_STANDARD, logLevel, logMessage ); } }
#define GUCEF_C_STANDARD_LOG GUCEF_C_LOG

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_USER_LOG
#define GUCEF_C_USER_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_USER, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_SYSTEM_LOG
#define GUCEF_C_SYSTEM_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_SYSTEM, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_DEV_LOG
#define GUCEF_C_DEV_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_DEV, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_SERVICE_LOG
#define GUCEF_C_SERVICE_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_SERVICE, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_PROTECTED_LOG
#define GUCEF_C_PROTECTED_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_PROTECTED, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_CALLSTACK_LOG
#define GUCEF_C_CALLSTACK_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_CALLSTACK, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_EXCEPTION_LOG
#define GUCEF_C_EXCEPTION_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_EXCEPTION, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_CONSOLE_LOG
#define GUCEF_C_CONSOLE_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_CONSOLE, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_WARNING_LOG
#define GUCEF_C_WARNING_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_WARNING, logLevel, logMessage ); } }

/*-------------------------------------------------------------------------*/

#undef GUCEF_C_DEBUG_LOG
#ifdef GUCEF_DEBUG_MODE
  #define GUCEF_C_DEBUG_LOG( apiStruct, logLevel, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_DEBUG, logLevel, logMessage ); } }
  #ifdef GUCEF_DEBUG_MODE_VERBOSE
    #define GUCEF_C_DEBUG_LOG_EVERYTHING( apiStruct, logMessage ) { if GUCEF_PREDICT_TRUE( GUCEF_NULL != apiStruct.Log ) { apiStruct.Log( GUCEF_LOGMSGTYPE_DEBUG, GUCEF_LOGLEVEL_EVERYTHING, logMessage ); } }
  #else
    #define GUCEF_C_DEBUG_LOG_EVERYTHING( apiStruct, logMessage )
  #endif
#else
  #define GUCEF_C_DEBUG_LOG( apiStruct, logLevel, logMessage )
  #define GUCEF_C_DEBUG_LOG_EVERYTHING( apiStruct, logMessage )
#endif

/*-------------------------------------------------------------------------*/

#else /* GUCEF_NO_LOGGING ? */

#define GUCEF_C_ERROR_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_STANDARD_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_USER_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_SYSTEM_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_DEV_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_SERVICE_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_PROTECTED_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_CALLSTACK_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_EXCEPTION_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_CONSOLE_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_WARNING_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_DEBUG_LOG( apiStruct, logLevel, logMessage ) { }
#define GUCEF_C_DEBUG_LOG_EVERYTHING( apiStruct, logMessage ) { }

#endif /* GUCEF_NO_LOGGING ? */


/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_LOGGING_H ? */

