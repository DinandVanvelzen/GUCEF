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
                      const UInt32 threadId        ,
                      const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The logging backend should format the info given
     *  as appropriote and output it.
     *  This variant accepts a CVariantStream for structured log data.
     */
    virtual void Log( const TLogMsgType logMsgType     ,
                      const Int32 logLevel             ,
                      const CVariantStream& logMessage ,
                      const UInt32 threadId            ,
                      const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The logging backend should output the logMessage string as given
     */
    virtual void LogWithoutFormatting( const TLogMsgType logMsgType ,
                                       const Int32 logLevel         ,
                                       const CString& logMessage    ,
                                       const UInt32 threadId        ,
                                       const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The logging backend should output the logMessage as given.
     *  This variant accepts a CVariantStream for structured log data.
     */
    virtual void LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                       const Int32 logLevel             ,
                                       const CVariantStream& logMessage ,
                                       const UInt32 threadId            ,
                                       const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void AddLogger( CILogger* loggerImp );

    void RemoveLogger( CILogger* loggerImp );

    void ClearLoggers( void );

    UInt32 GetLoggerCount( void ) const;

    bool IsLoggingEnabled( const TLogMsgType logMsgType ,
                           const Int32 logLevel         ) const;

    private:

    typedef GUCEF::set< CILogger* > TLoggerList;

    TLoggerList m_loggers;
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
