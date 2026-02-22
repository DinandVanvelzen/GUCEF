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

#ifndef GUCEF_CORE_CANDROIDSYSTEMLOGGER_H
#define GUCEF_CORE_CANDROIDSYSTEMLOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CString;
class CILoggingFormatter;

/*-------------------------------------------------------------------------*/

/**
 *  Logger backend which logs to the Android system log
 */
class GUCEF_CORE_PUBLIC_CPP CAndroidSystemLogger : public CILogger
{
    public:

    CAndroidSystemLogger( void );

    virtual ~CAndroidSystemLogger();

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
     *  The logging backend should output the logMessage string as given
     */
    virtual void LogWithoutFormatting( const TLogMsgType logMsgType ,
                                       const Int32 logLevel         ,
                                       const CString& logMessage    ,
                                       const UInt32 threadId        ,
                                       const CTimestamp& timestamp  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAndroidSystemLogger( const CAndroidSystemLogger& src );              /**< not implemented */
    CAndroidSystemLogger& operator=( const CAndroidSystemLogger& src );   /**< not implemented */

    private:

    CString m_tag;
    void* m_logFunc;
    Int32 m_minimalLogLevel;
    CILoggingFormatter* m_logFormatter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */


#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CANDROIDSYSTEMLOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-10-2010 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
