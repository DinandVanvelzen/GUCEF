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

#ifndef GUCEF_CORE_CLOGMANAGER_H
#define GUCEF_CORE_CLOGMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>
#include <map>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"    
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

#ifndef GUCEF_CORE_LOGLEVELS_H
#include "gucefCORE_LogLevels.h"
#define GUCEF_CORE_LOGLEVELS_H
#endif /* GUCEF_CORE_LOGLEVELS_H ? */

#ifndef GUCEF_CORE_CILOGGINGFORMATTER_H
#include "gucefCORE_CILoggingFormatter.h"
#define GUCEF_CORE_CILOGGINGFORMATTER_H
#endif /* GUCEF_CORE_CILOGGINGFORMATTER_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_CORE_CLOGSTREAM_H
#include "gucefCORE_CLogStream.h"
#define GUCEF_CORE_CLOGSTREAM_H
#endif /* GUCEF_CORE_CLOGSTREAM _H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

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

class CILogger;
class CMultiLogger;
class CLoggingTask;
class CLogStreamScope;
class CThreadLogBuffers;

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CLogManager : public CORE::CObserver
{
    public:

    void AddLogger( CILogger* loggerImp );

    void RemoveLogger( CILogger* loggerImp );

    void ClearLoggers( void );

    bool AddLoggingFormatterFactory( const CString& name                        ,
                                     TLoggingFormatterFactory* formatterFactory ,
                                     bool overrideDefault = true                );

    bool RemoveLoggingFormatterFactory( const CString& name );

    TLoggingFormatterPtr CreateLoggingFormatter( const CString& name );

    TLoggingFormatterPtr CreateDefaultLoggingFormatter( void );

    bool SetDefaultLoggingFormatter( const CString& name );

    void ClearLoggingFormatters( void );

    bool IsLoggingEnabled( const TLogMsgType logMsgType ,
                           const Int32 logLevel         ) const;

    /**
     *  Returns the thread's log stream for streaming log data.
     *  Writes log entry metadata (type, level, threadId, timestamp) at the start.
     *  Use with CLogStreamScope for automatic segment end marking.
     */
    CLogStreamPtr
    Log( const TLogMsgType logMsgType ,
         const Int32 logLevel         );

    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    );

    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    ,
              const UInt32 threadId        ,
              const CTimestamp& timestamp  );

    void Log( const TLogMsgType logMsgType     ,
              const Int32 logLevel             ,
              const CVariantStream& logMessage );

    void Log( const TLogMsgType logMsgType     ,
              const Int32 logLevel             ,
              const CVariantStream& logMessage ,
              const UInt32 threadId            ,
              const CTimestamp& timestamp      );

    void SetMinLogLevel( const Int32 logLevel );

    Int32 GetMinLogLevel( void ) const;

    void FlushLogs( void );

    /**
     *  Flushes all thread stream buffers to the backend loggers.
     *  For each thread buffer:
     *  - If the front buffer is not in use, swaps front/back buffers
     *  - Drains all log segments from the back buffer to the loggers
     *  - Clears the drained back buffer
     *  
     *  Returns the number of thread buffers that were successfully flushed.
     *  Thread buffers where the front buffer was in use are skipped.
     */
    UInt32 FlushThreadStreamBuffers( void );

    void SetUseLoggingThread( bool useLogThread );

    bool GetUseLoggingThread( void ) const;

    /**
     *  About the bootstrap log:
     *  At application startup there will be log messages entered before any logger
     *  is actually attached to the LogManager. Such log messages are stored in the bootstrap
     *  Log which is kept in-memory in the LogManager itself. Log entries will continue to be logged
     *  to the bootstrap log untill the first Logger is registered with the LogManager.
     *
     *  This function will flush all the bootstrap log entries to the attached loggers and clear
     *  the LogManager in-memory bootstrap log.
     */
    void FlushBootstrapLogEntriesToLogs( void );

    /**
     *  Allows you to redirect all logging output back to the bootstrap log queue
     *  Typically only ever needed when you want to fiddle with your logging outputs when the
     *  application is already up and running and already had working loggers attached.
     *  When you turn redirect off the queue is immediatly flushed
     */
    void RedirectToBootstrapLogQueue( bool redirect );

    static const CString& GetLogMsgTypeString( const TLogMsgType logMsgType );

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;


    private:
    friend class CLoggingGlobal;
    friend class CCoreGlobal;

    CLogManager( void );

    virtual ~CLogManager() GUCEF_VIRTUAL_OVERRIDE;

    typedef CTEventHandlerFunctor< CLogManager > TEventCallback;

    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );

    UInt32 FlushThreadStreamBuffer( CLogStreamPtr buffer );

    bool SetPulseGenerator( PulseGeneratorPtr pulseGenerator );

    private:

    CLogManager( const CLogManager& src );              /**< not implemented, don't use */
    CLogManager& operator=( const CLogManager& src );   /**< not implemented, don't use */

    private:

    struct SBootstrapLogEntry
    {
        TLogMsgType logMsgType;
        Int32 logLevel;
        CString logMessage;
        UInt32 threadId;
        CTimestamp timestamp;
    };
    typedef struct SBootstrapLogEntry TBootstrapLogEntry;
    typedef GUCEF::vector< TBootstrapLogEntry > TBootstrapLogVector;
    typedef CTBasicSharedPtr< CLoggingTask, MT::CMutex > CLoggingTaskBasePtr;
    typedef GUCEF::map< UInt32, CThreadLogBuffers* > TThreadBufferMap;

    CThreadLogBuffers* GetOrCreateThreadBuffers( UInt32 threadId );

    CMultiLogger* m_loggers;
    CLoggingTaskBasePtr m_loggingTask;
    bool m_useLogThread;
    TBootstrapLogVector m_bootstrapLog;
    bool m_busyLogging;
    bool m_redirectToLogQueue;
    TAbstractLoggingFormatterFactory m_logFormatterFactory;
    CString m_defaultLogFormatter;
    TThreadBufferMap m_threadBuffers;               /**< Per-thread log buffers */
    PulseGeneratorPtr m_pulseGenerator;             /**< Used to trigger periodic flushing of thread buffers */
    MT::CMutex m_threadBuffersLock;                 /**< Protects thread buffer map */
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------*/

// A number of optional constants that can be used to define log levels.
// These levels are used by the library itself. The lower the importance of the log level
// the more likely it will be filtered out by most logging backends
extern "C"
{

GUCEF_CORE_PUBLIC_C extern const Int32 LOGLEVEL_CRITICAL;
GUCEF_CORE_PUBLIC_C extern const Int32 LOGLEVEL_VERY_IMPORTANT;
GUCEF_CORE_PUBLIC_C extern const Int32 LOGLEVEL_IMPORTANT;
GUCEF_CORE_PUBLIC_C extern const Int32 LOGLEVEL_NORMAL;
GUCEF_CORE_PUBLIC_C extern const Int32 LOGLEVEL_BELOW_NORMAL;
GUCEF_CORE_PUBLIC_C extern const Int32 LOGLEVEL_EVERYTHING;

}

/*-------------------------------------------------------------------------*/

/**
 *  Function that will convert the loglevel values above into a string equivalant
 *  and the rest into numbers.
 */
GUCEF_CORE_PUBLIC_CPP CString
LogLevelToString( const Int32 logLevel );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CLOGMANAGER_H ? */
