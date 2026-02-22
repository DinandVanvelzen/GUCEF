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

#ifndef GUCEF_CORE_STRINGLOGGINGMACROS_H
#define GUCEF_CORE_STRINGLOGGINGMACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_NO_LOGGING

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

#ifndef GUCEF_CORE_LOGLEVELS_H
#include "gucefCORE_LogLevels.h"
#define GUCEF_CORE_LOGLEVELS_H
#endif /* GUCEF_CORE_LOGLEVELS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Lightweight logging helper class for string-based logging.
 *  Used by the LOGSTR macros to avoid including the full logging infrastructure.
 *  Implementation routes calls to CLoggingGlobal.
 */
class GUCEF_CORE_PUBLIC_CPP CStringLogHelper
{
    public:

    static void LogStr( const TLogMsgType logMsgType ,
                        const Int32 logLevel         ,
                        const CString& logMessage    );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

#endif /* GUCEF_NO_LOGGING ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  String-based logging macros with minimal dependencies.
 *  Use these in headers that cannot include the full logging infrastructure.
 */
#ifndef GUCEF_NO_LOGGING

/*-------------------------------------------------------------------------*/

#undef GUCEF_ERROR_LOGSTR
#define GUCEF_ERROR_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_ERROR, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_LOGSTR
#define GUCEF_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_STANDARD, logLevel, logMessage );
#define GUCEF_STANDARD_LOGSTR GUCEF_LOGSTR

/*-------------------------------------------------------------------------*/

#undef GUCEF_USER_LOGSTR
#define GUCEF_USER_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_USER, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_SYSTEM_LOGSTR
#define GUCEF_SYSTEM_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_SYSTEM, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_DEV_LOGSTR
#define GUCEF_DEV_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_DEV, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_SERVICE_LOGSTR
#define GUCEF_SERVICE_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_SERVICE, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_PROTECTED_LOGSTR
#define GUCEF_PROTECTED_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_PROTECTED, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_CALLSTACK_LOGSTR
#define GUCEF_CALLSTACK_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_CALLSTACK, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_EXCEPTION_LOGSTR
#define GUCEF_EXCEPTION_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_EXCEPTION, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_CONSOLE_LOGSTR
#define GUCEF_CONSOLE_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_CONSOLE, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_WARNING_LOGSTR
#define GUCEF_WARNING_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_WARNING, logLevel, logMessage );

/*-------------------------------------------------------------------------*/

#undef GUCEF_DEBUG_LOGSTR
#ifdef GUCEF_DEBUG_MODE
  #define GUCEF_DEBUG_LOGSTR( logLevel, logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_DEBUG, logLevel, logMessage );
  #ifdef GUCEF_DEBUG_MODE_VERBOSE
    #define GUCEF_DEBUG_LOGSTR_EVERYTHING( logMessage ) ::GUCEF::CORE::CStringLogHelper::LogStr( ::GUCEF::CORE::LOG_DEBUG, LOGLEVEL_EVERYTHING, logMessage );
  #else
    #define GUCEF_DEBUG_LOGSTR_EVERYTHING( logMessage )
  #endif
#else
  #define GUCEF_DEBUG_LOGSTR( logLevel, logMessage )
  #define GUCEF_DEBUG_LOGSTR_EVERYTHING( logMessage )
#endif

/*-------------------------------------------------------------------------*/

#else /* GUCEF_NO_LOGGING ? */

#define GUCEF_ERROR_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_STANDARD_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_USER_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_SYSTEM_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_DEV_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_SERVICE_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_PROTECTED_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_CALLSTACK_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_EXCEPTION_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_CONSOLE_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_WARNING_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_DEBUG_LOGSTR( logLevel, logMessage ) { }
#define GUCEF_DEBUG_LOGSTR_EVERYTHING( logMessage ) { }

#endif /* GUCEF_NO_LOGGING ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_STRINGLOGGINGMACROS_H ? */
