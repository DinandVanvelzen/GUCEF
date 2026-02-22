/*
 *  GucefLogServiceLib: Library containing the main logic for the Logging service
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H
#define GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_LOGSERVICELIB_CLOGSVCSERVER_H
#include "GucefLogServiceLib_CLogSvcServer.h"
#define GUCEF_LOGSERVICELIB_CLOGSVCSERVER_H
#endif /* GUCEF_LOGSERVICELIB_CLOGSVCSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace LOGSERVICELIB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_LOGSERVICELIB_EXPORT_CPP CILogSvcServerLogger
{
    public:
    
    typedef CORE::TLogMsgType TLogMsgType;
    typedef CLogSvcServer::TClientInfo TClientInfo;
    
    virtual void StartOfLoggingForClient( const TClientInfo& clientInfo ) = 0;
    
    virtual void EndOfLoggingForClient( const TClientInfo& clientInfo ) = 0;

    virtual void Log( const TClientInfo& clientInfo    ,
                      const TLogMsgType logMsgType     ,
                      const CORE::Int32 logLevel       ,
                      const CORE::CString& logMessage  ,
                      const CORE::UInt32 threadId      ,
                      const CORE::CTimestamp& timestamp ) = 0;
                      
    virtual void FlushLog( const TClientInfo& clientInfo ) = 0;

    virtual void SetMinimalLogLevel( const CORE::Int32 minimalLogLevel ) = 0;

    virtual CORE::Int32 GetMinimalLogLevel( void ) const = 0;
    
    virtual void SetWhetherAppNameIsLogged( bool logAppName ) = 0;
    
    virtual bool GetWhetherAppNameIsLogged( void ) const = 0;

    virtual void SetWhetherProcessNameIsLogged( bool logProcessName ) = 0;
    
    virtual bool GetWhetherProcessNameIsLogged( void ) const = 0;
    
    virtual void SetWhetherProcessIdIsLogged( bool logProcessId ) = 0;
    
    virtual bool GetWhetherProcessIdIsLogged( void ) const = 0;

    //static CORE::CString FormatStdLogMessage( const bool logAppName           ,
    //                                          const bool logProcessName       ,
    //                                          const bool logProcessId         ,
    //                                          const TClientInfo& clientInfo   ,
    //                                          const TLogMsgType logMsgType    ,
    //                                          const CORE::Int32 logLevel      ,
    //                                          const CORE::CString& logMessage ,
    //                                          const CORE::UInt32 threadId     );
    
    CILogSvcServerLogger( void );                                         /**< interface class: no-op */
    virtual ~CILogSvcServerLogger();                                      /**< interface class: no-op */
    CILogSvcServerLogger( const CILogSvcServerLogger& src );              /**< interface class: no-op */
    CILogSvcServerLogger& operator=( const CILogSvcServerLogger& src );   /**< interface class: no-op */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICELIB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/
