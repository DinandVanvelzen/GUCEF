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

#ifndef GUCEF_CORE_CTBASICSHAREDPTR_H
#include "CTBasicSharedPtr.h"
#define GUCEF_CORE_CTBASICSHAREDPTR_H
#endif /* GUCEF_CORE_CTBASICSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

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

class CILogger;
class CMultiLogger;
class CLoggingTask;

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CLogManager : public MT::CILockable
{
    public:

    typedef CTAbstractFactory< CString, CILoggingFormatter, MT::CMutex > TAbstractLoggingFormatterFactory;
    typedef TAbstractLoggingFormatterFactory::TConcreteFactory TLoggingFormatterFactory;
    typedef TAbstractLoggingFormatterFactory::TProductPtr TLoggingFormatterPtr;

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

    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    );

    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    ,
              const UInt32 threadId        ,
              const CDateTime& timestamp   );

    void SetMinLogLevel( const Int32 logLevel );

    Int32 GetMinLogLevel( void ) const;

    void FlushLogs( void );

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
    
    virtual bool Lock( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CLogManager( void );

    virtual ~CLogManager();

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
        CDateTime timestamp;
    };
    typedef struct SBootstrapLogEntry TBootstrapLogEntry;
    typedef std::vector< TBootstrapLogEntry > TBootstrapLogVector;
    typedef CTBasicSharedPtr< CLoggingTask, MT::CMutex > CLoggingTaskBasePtr;

    CMultiLogger* m_loggers;
    CLoggingTaskBasePtr m_loggingTask;
    bool m_useLogThread;
    TBootstrapLogVector m_bootstrapLog;
    bool m_busyLogging;
    bool m_redirectToLogQueue;
    TAbstractLoggingFormatterFactory m_logFormatterFactory;
    CString m_defaultLogFormatter;
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
