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

#ifndef GUCEF_LOGSERVICELIB_CLOGSVCSERVERFILELOGGER_H
#define GUCEF_LOGSERVICELIB_CLOGSVCSERVERFILELOGGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H
#include "GucefLogServiceLib_CILogSvcServerLogger.h"
#define GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H
#endif /* GUCEF_LOGSERVICELIB_CILOGSVCSERVERLOGGER_H ? */

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

class GUCEF_LOGSERVICELIB_EXPORT_CPP CLogSvcServerFileLogger : public CILogSvcServerLogger
{
    public:

    CLogSvcServerFileLogger( void );
    
    CLogSvcServerFileLogger( const CORE::CString& outputDir );
                                            
    virtual ~CLogSvcServerFileLogger() GUCEF_VIRTUAL_OVERRIDE;

    virtual void StartOfLoggingForClient( const TClientInfo& clientInfo ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void EndOfLoggingForClient( const TClientInfo& clientInfo ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void Log( const CLogSvcServer::TClientInfo& clientInfo ,
                      const TLogMsgType logMsgType                 ,
                      const CORE::Int32 logLevel                   ,
                      const CORE::CString& logMessage              ,
                      const CORE::UInt32 threadId                  ,
                      const CORE::CTimestamp& timestamp            ) GUCEF_VIRTUAL_OVERRIDE;
                      
    virtual void FlushLog( const TClientInfo& clientInfo ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetMinimalLogLevel( const CORE::Int32 minimalLogLevel ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void SetWhetherAppNameIsLogged( bool logAppName ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool GetWhetherAppNameIsLogged( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetWhetherProcessNameIsLogged( bool logProcessName ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool GetWhetherProcessNameIsLogged( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void SetWhetherProcessIdIsLogged( bool logProcessId ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool GetWhetherProcessIdIsLogged( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    void SetOutputDir( const CORE::CString& logOutputDir );
    
    const CORE::CString& GetOutputDir( void ) const;

    void SetUseSeperateLogPerApp( bool logPerApp );
    
    bool GetUseSeperateLogPerApp( void ) const;

    void SetUseSeperateLogPerProcessName( bool logPerProcessName );
    
    bool GetUseSeperateLogPerProcessName( void ) const;
    
    void CloseOutput( void );
    
    private:
                                         
    typedef CORE::CTSharedPtr< CORE::CFileAccess, MT::CMutex >      TFileAccessPtr;
    typedef CORE::TLoggingFormatterPtr                               TLoggingFormatterPtr;
    
    CLogSvcServerFileLogger( const CLogSvcServerFileLogger& src );             /** <- not implemented */
    CLogSvcServerFileLogger& operator=( const CLogSvcServerFileLogger& src );  /** <- not implemented */

    TFileAccessPtr GetFileAccess( const TClientInfo& clientInfo );
    
    CORE::CString CreateRelOutputFilePath( const TClientInfo& clientInfo );
    
    CORE::CString CreateAbsOutputFilePath( const CORE::CString& relPath );
    
    private:
    
    typedef std::map< CORE::CString, TFileAccessPtr > TOutputMap;
    
    CORE::CString m_outputDir;
    CORE::Int32 m_minimalLogLevel;
    TLoggingFormatterPtr m_logFormatter;
    bool m_logAppName;
    bool m_logProcessName;
    bool m_logProcessId;
    bool m_seperateLogPerApp;
    bool m_seperateLogPerProcessName;
    TOutputMap m_clientMap;
    TOutputMap m_outputMap;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICELIB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LOGSERVICELIB_CLOGSVCSERVERFILELOGGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/
