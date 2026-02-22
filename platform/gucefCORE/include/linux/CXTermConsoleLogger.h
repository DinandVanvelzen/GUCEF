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

#ifndef GUCEF_CORE_CXTERMCONSOLELOGGER_H
#define GUCEF_CORE_CXTERMCONSOLELOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#ifndef GUCEF_CORE_CICONSOLELOGGER_H
#include "gucefCORE_CIConsoleLogger.h"
#define GUCEF_CORE_CICONSOLELOGGER_H
#endif /* GUCEF_CORE_CICONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifdef GUCEF_LINUX_BUILD
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
 *  GUCEF logger implementation for Linux's X11 environment which
 *  displays log output in a console window.
 */
class GUCEF_CORE_PUBLIC_CPP CXTermConsoleLogger : public CIConsoleLogger
{
    public:

    CXTermConsoleLogger( bool initialize = true );

    virtual ~CXTermConsoleLogger() GUCEF_VIRTUAL_OVERRIDE;

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

    bool Initialize( void );

    private:

    CXTermConsoleLogger& operator=( const CXTermConsoleLogger& src );
    CXTermConsoleLogger( const CXTermConsoleLogger& src );

    private:

    Int32 m_minimalLogLevel;
    bool m_formatForUiPurpose;
    int m_xtermpid;
    int m_masterfd;
    int m_slavefd;
    FILE* m_slaveFptr;
    CLogManager::TLoggingFormatterPtr m_logFormatter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
#endif /* GUCEF_LINUX_BUILD ? */

#endif /* GUCEF_CORE_CXTERMCONSOLELOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 19-05-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
