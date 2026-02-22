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

#ifndef GUCEF_CORE_CLOGGINGGLOBAL_H
#define GUCEF_CORE_CLOGGINGGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      FORWARD DECLARATIONS                                               //
//                                                                         //
//-------------------------------------------------------------------------*/

class CTimestamp;
class CLogManager;
class CCoreGlobal;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Singleton proxy for the logging manager.
 *  Since logging is so fundamental to the core and is used in many places,
 *  we want to avoid including the logging manager directly in those places
 *  to avoid pulling in its more involved dependencies.
 */
class GUCEF_CORE_PUBLIC_CPP CLoggingGlobal
{
    public:

    static CLoggingGlobal* Instance( void );

    CLogManager& GetLogManager( void );

    /**
     *  Logging proxy call provided here to avoid including the logging manager 
     *  with its more involved dependencies.
     *  Returns a shared pointer to the thread's log stream for streaming log data.
     *  Log entry metadata is written at the start of the stream.
     *  
     *  IMPORTANT: Caller must call WriteSegmentEnd() on the stream when done,
     *  or wrap with CLogStreamScope for automatic segment marking:
     *  
     *  Example:
     *    CLogStreamScope scope( CLoggingGlobal::Instance()->Log( LOG_STANDARD, LOGLEVEL_NORMAL ) );
     *    scope << "User " << userId << " logged in";
     *  // VOID marker automatically written when scope goes out of scope
     */
    CVariantStreamPtr Log( const TLogMsgType logMsgType ,
                           const Int32 logLevel         );

    /**
     *  Logging proxy call provided here to avoid including the logging manager 
     *  with its more involved dependencies
     */
    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    );

    /**
     *  Logging proxy call provided here to avoid including the logging manager 
     *  with its more involved dependencies
     */
    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    ,
              const UInt32 threadId        ,
              const CTimestamp& timestamp  );

    /**
     *  Static provided here to avoid including the full CLogManager header in the logging macros.
     *  It will forward the call provided the core system is available. If not, it will be a no-op.
     */
    void FlushLogs( void );

    private:
    friend class CCoreGlobal;

    static void Deinstance( void );

    private:

    CLoggingGlobal( void );

    ~CLoggingGlobal();

    void Initialize( void );

    private:

    CLogManager* m_logManager;

    static MT::CMutex g_dataLock;
    static CLoggingGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CLOGGINGGLOBAL_H ? */
