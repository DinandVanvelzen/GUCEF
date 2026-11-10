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

#ifndef GUCEF_CORE_LOGGING_H
#define GUCEF_CORE_LOGGING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_NO_LOGGING

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CSTDLOGGER_H
#include "CStdLogger.h"
#define GUCEF_CORE_CSTDLOGGER_H
#endif /* GUCEF_CORE_CSTDLOGGER_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CROLLINGFILEACCESS_H
#include "gucefCORE_CRollingFileAccess.h"
#define GUCEF_CORE_CROLLINGFILEACCESS_H
#endif /* GUCEF_CORE_CROLLINGFILEACCESS_H ? */

/*
 *  Below are platform specific loggers which can be
 *  used directly however please consider using the
 *  CPlatformNativeConsoleLogger class to create the
 *  correct logger for you which gives you platform
 *  agnostic code.
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

  #ifndef GUCEF_CORE_CMSWINCONSOLELOGGER_H
  #include "CMSWinConsoleLogger.h"
  #define GUCEF_CORE_CMSWINCONSOLELOGGER_H
  #endif /* GUCEF_CORE_CMSWINCONSOLELOGGER_H ? */

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

  #ifndef GUCEF_CORE_CXTERMCONSOLELOGGER_H
  #include "CXTermConsoleLogger.h"
  #define GUCEF_CORE_CXTERMCONSOLELOGGER_H
  #endif /* GUCEF_CORE_CXTERMCONSOLELOGGER_H ? */

#endif

#endif /* GUCEF_NO_LOGGING ? */

#ifndef GUCEF_CORE_LOGGINGMACROS_H
#include "gucefCORE_LoggingMacros.h"
#define GUCEF_CORE_LOGGINGMACROS_H
#endif /* GUCEF_CORE_LOGGINGMACROS_H ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_LOGGING_H ? */
