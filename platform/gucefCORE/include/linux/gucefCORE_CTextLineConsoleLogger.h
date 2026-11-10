/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CTEXTLINECONSOLELOGGER_H
#define GUCEF_CORE_CTEXTLINECONSOLELOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONSOLELOGGER_H
#include "gucefCORE_CIConsoleLogger.h"
#define GUCEF_CORE_CICONSOLELOGGER_H
#endif /* GUCEF_CORE_CICONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

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
 *  Base for console loggers which turn every log message into one line of text. Takes care of the log level
 *  filtering and the formatting, descendants only deal with where the lines go by implementing WriteLine().
 */
class GUCEF_CORE_PUBLIC_CPP CTextLineConsoleLogger : public CIConsoleLogger
{
    public:

    CTextLineConsoleLogger( void );

    virtual ~CTextLineConsoleLogger() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Log( const TLogMsgType logMsgType ,
                      const Int32 logLevel         ,
                      const CString& logMessage    ,
                      const UInt32 threadId        ,
                      const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Log( const TLogMsgType logMsgType     ,
                      const Int32 logLevel             ,
                      const CVariantStream& logMessage ,
                      const UInt32 threadId            ,
                      const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void LogWithoutFormatting( const TLogMsgType logMsgType ,
                                       const Int32 logLevel         ,
                                       const CString& logMessage    ,
                                       const UInt32 threadId        ,
                                       const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void LogWithoutFormatting( const TLogMsgType logMsgType     ,
                                       const Int32 logLevel             ,
                                       const CVariantStream& logMessage ,
                                       const UInt32 threadId            ,
                                       const CTimestamp& timestamp      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetMinimalLogLevel( const Int32 minimalLogLevel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetFormatAsConsoleUI( bool formatForUiPurpose ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetFormatAsConsoleUI( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Delivers a line of text, without end of line, to wherever it is supposed to go. Must never block for long.
     *  Called from any thread that logs, a descendant has to do its own locking.
     */
    virtual void WriteLine( const CString& line ) = 0;

    /** Gives descendants the chance to do something when logs are flushed */
    virtual void OnFlushLog( void );

    private:

    CTextLineConsoleLogger& operator=( const CTextLineConsoleLogger& src );
    CTextLineConsoleLogger( const CTextLineConsoleLogger& src );

    private:

    Int32 m_minimalLogLevel;
    bool m_formatForUiPurpose;
    TLoggingFormatterPtr m_logFormatter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTEXTLINECONSOLELOGGER_H ? */
