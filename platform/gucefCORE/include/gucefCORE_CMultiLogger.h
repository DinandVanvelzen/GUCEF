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

#ifndef GUCEF_CORE_CMULTILOGGER_H
#define GUCEF_CORE_CMULTILOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>
#include <map>

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

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
 *  Class implementing a collection of loggers
 *
 *  Not thread-safe by itself: its owner keeps the calls that deliver (Log(), LogWithoutFormatting(), FlushLog(),
 *  SetMinimalLogLevel()) and the changes of the loggers apart, as the log manager does (see CLogManager::AddLogger()).
 *  A change made from within a delivery on the same thread (a logger that adds or removes a logger while it is called)
 *  is safe when it says so with 'whileLogging': the delivery in progress goes on with the loggers as changed, a
 *  removed logger is not called any more, and its place in the list is given up by a later change made outside a
 *  delivery.
 */
class GUCEF_CORE_PUBLIC_CPP CMultiLogger : public CILogger
{
    public:

    CMultiLogger( void );

    CMultiLogger( const CMultiLogger& src );

    virtual ~CMultiLogger();

    CMultiLogger& operator=( const CMultiLogger& src );

    /**
     *  The logging backend should format the info given
     *  as appropriote and output it.
     */
    virtual void Log( const TLogMsgType logMsgType ,
                      const Int32 logLevel         ,
                      const CString& logMessage    );

    /**
     *  The logging backend should format the info given
     *  as appropriote and output it.
     */
    virtual void Log( const TLogMsgType logMsgType ,
                      const Int32 logLevel         ,
                      const CString& logMessage    ,
                      const TSystemThreadId threadId        ,
                      const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The logging backend should format the info given
     *  as appropriote and output it.
     *  This variant accepts a CVariantStream for structured log data.
     */
    virtual void Log( const TLogMsgType logMsgType     ,
                      const Int32 logLevel             ,
                      const CVariantStream& logMessage ,
                      const TSystemThreadId threadId   ,
                      const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The logging backend should output the logMessage string as given
     */
    virtual void LogWithoutFormatting( const TLogMsgType logMsgType ,
                                       const Int32 logLevel         ,
                                       const CString& logMessage    ,
                                       const TSystemThreadId threadId        ,
                                       const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The logging backend should output the logMessage as given.
     *  This variant accepts a CVariantStream for structured log data.
     */
    virtual void LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                       const Int32 logLevel             ,
                                       const CVariantStream& logMessage ,
                                       const TSystemThreadId threadId   ,
                                       const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Adds a logger, once: a logger that is in the list already is not added again.
     *  @param whileLogging true when the call may come from within a delivery of this object on this thread
     */
    void AddLogger( CILogger* loggerImp, bool whileLogging = false );

    /**
     *  Removes a logger; once this returns the logger is not called any more by this object.
     *  @param whileLogging true when the call may come from within a delivery of this object on this thread
     */
    void RemoveLogger( CILogger* loggerImp, bool whileLogging = false );

    /** @param whileLogging true when the call may come from within a delivery of this object on this thread */
    void ClearLoggers( bool whileLogging = false );

    UInt32 GetLoggerCount( void ) const;

    /** Whether a message of the type and level is passed on to the loggers */
    bool IsLoggingEnabled( const TLogMsgType logMsgType ,
                           const Int32 logLevel         ) const;

    /** Whether messages of a type are passed on */
    bool IsLogMsgTypeEnabled( const TLogMsgType logMsgType ) const;

    private:

    /** Gives up the places of loggers removed while logging; only outside a delivery */
    void CompactLoggers( void );

    /**
     *  In the order they were added. A delivery walks the list by index and reads its size at every step, so that a
     *  logger added from within the delivery does not invalidate it; a logger removed from within a delivery leaves a
     *  NULL in its place (see RemoveLogger()) until CompactLoggers()
     */
    typedef GUCEF::vector< CILogger* > TLoggerList;

    TLoggerList m_loggers;
    UInt32 m_loggerCount;           /**< the loggers in m_loggers, the NULL places left out */
    GUCEF::map< TLogMsgType, bool > m_msgTypeEnablers;
    Int32 m_minLogLevel;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CMULTILOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-02-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
