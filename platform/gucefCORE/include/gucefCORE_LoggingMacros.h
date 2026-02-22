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

#ifndef GUCEF_CORE_LOGGINGMACROS_H
#define GUCEF_CORE_LOGGINGMACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_NO_LOGGING

#ifndef GUCEF_CORE_STRINGLOGGINGMACROS_H
#include "gucefCORE_StringLoggingMacros.h"
#define GUCEF_CORE_STRINGLOGGINGMACROS_H
#endif /* GUCEF_CORE_STRINGLOGGINGMACROS_H ? */

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#endif /* GUCEF_NO_LOGGING ? */

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

#undef GUCEF_ERROR_LOG
#define GUCEF_ERROR_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_ERROR, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_LOG
#define GUCEF_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_STANDARD, logLevel ).GetStream() << (logMessage); }
#define GUCEF_STANDARD_LOG GUCEF_LOG

/*-------------------------------------------------------------------------*/

#undef GUCEF_USER_LOG
#define GUCEF_USER_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_USER, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_SYSTEM_LOG
#define GUCEF_SYSTEM_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_SYSTEM, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_DEV_LOG
#define GUCEF_DEV_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_DEV, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_SERVICE_LOG
#define GUCEF_SERVICE_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_SERVICE, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_PROTECTED_LOG
#define GUCEF_PROTECTED_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_PROTECTED, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_CALLSTACK_LOG
#define GUCEF_CALLSTACK_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_CALLSTACK, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_EXCEPTION_LOG
#define GUCEF_EXCEPTION_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_EXCEPTION, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_CONSOLE_LOG
#define GUCEF_CONSOLE_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_CONSOLE, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_WARNING_LOG
#define GUCEF_WARNING_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_WARNING, logLevel ).GetStream() << (logMessage); }

/*-------------------------------------------------------------------------*/

#undef GUCEF_DEBUG_LOG
#ifdef GUCEF_DEBUG_MODE
  #define GUCEF_DEBUG_LOG( logLevel, logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_DEBUG, logLevel ).GetStream() << (logMessage); }
  #ifdef GUCEF_DEBUG_MODE_VERBOSE
    #define GUCEF_DEBUG_LOG_EVERYTHING( logMessage ) { ::GUCEF::CORE::CLogStreamScope( ::GUCEF::CORE::LOG_DEBUG, LOGLEVEL_EVERYTHING ).GetStream() << (logMessage); }
  #else
    #define GUCEF_DEBUG_LOG_EVERYTHING( logMessage )
  #endif
#else
  #define GUCEF_DEBUG_LOG( logLevel, logMessage )
  #define GUCEF_DEBUG_LOG_EVERYTHING( logMessage )
#endif

/*-------------------------------------------------------------------------*/

#else /* GUCEF_NO_LOGGING ? */

#define GUCEF_TRACE

#define GUCEF_ERROR_LOG( logLevel, logMessage ) { }
#define GUCEF_LOG( logLevel, logMessage ) { }
#define GUCEF_STANDARD_LOG( logLevel, logMessage ) { }
#define GUCEF_USER_LOG( logLevel, logMessage ) { }
#define GUCEF_SYSTEM_LOG( logLevel, logMessage ) { }
#define GUCEF_DEV_LOG( logLevel, logMessage ) { }
#define GUCEF_SERVICE_LOG( logLevel, logMessage ) { }
#define GUCEF_PROTECTED_LOG( logLevel, logMessage ) { }
#define GUCEF_CALLSTACK_LOG( logLevel, logMessage ) { }
#define GUCEF_EXCEPTION_LOG( logLevel, logMessage ) { }
#define GUCEF_CONSOLE_LOG( logLevel, logMessage ) { }
#define GUCEF_WARNING_LOG( logLevel, logMessage ) { }
#define GUCEF_DEBUG_LOG( logLevel, logMessage ) { }
#define GUCEF_DEBUG_LOG_EVERYTHING( logMessage ) { }

/*
 *  We use a typedef to ensure any logging macro statements that have namespaced
 *  log level variable references are still valid syntax with the macro voided
 *  as seen above. The typedef's are not the same as the actual values but it
 *  is acceptable for the preprocessor as a param since the macro has no content anyway.
 */
typedef int LOGLEVEL_CRITICAL;
typedef int LOGLEVEL_VERY_IMPORTANT;
typedef int LOGLEVEL_IMPORTANT;
typedef int LOGLEVEL_NORMAL;
typedef int LOGLEVEL_BELOW_NORMAL;
typedef int LOGLEVEL_EVERYTHING;

#endif /* GUCEF_NO_LOGGING ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_LOGGINGMACROS_H ? */
