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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#include "GucefLogServiceLib_CLogSvcServerFileLogger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace LOGSERVICELIB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CLogSvcServerFileLogger::CLogSvcServerFileLogger( void )
    : CILogSvcServerLogger()                           
    , m_outputDir( "$CURWORKDIR$" )                    
    , m_minimalLogLevel( CORE::LOGLEVEL_BELOW_NORMAL ) 
    , m_logFormatter()
    , m_logAppName( true )                             
    , m_logProcessName( true )                         
    , m_logProcessId( true )                           
    , m_seperateLogPerProcessName( true )              
    , m_clientMap()                                    
    , m_outputMap()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLogSvcServerFileLogger::CLogSvcServerFileLogger( const CORE::CString& outputDir )
    : CILogSvcServerLogger()                           
    , m_outputDir( outputDir )                         
    , m_minimalLogLevel( CORE::LOGLEVEL_BELOW_NORMAL ) 
    , m_logFormatter()
    , m_logAppName( false )                            
    , m_logProcessName( true )                         
    , m_logProcessId( true )                           
    , m_seperateLogPerApp( true )                      
    , m_seperateLogPerProcessName( true )              
    , m_clientMap()                                    
    , m_outputMap()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLogSvcServerFileLogger::~CLogSvcServerFileLogger()
{GUCEF_TRACE;

    CloseOutput();
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::CloseOutput( void )
{GUCEF_TRACE;

    TOutputMap::iterator i = m_outputMap.begin();
    while ( i != m_outputMap.end() )
    {
        TFileAccessPtr& file = (*i).second;
        
        file->Flush();
        file->Close();
            
        m_outputMap.erase( i );
        i = m_outputMap.begin();
    }
    
    m_clientMap.clear();
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::SetUseSeperateLogPerProcessName( bool logPerProcessName )
{GUCEF_TRACE;

    m_seperateLogPerProcessName = logPerProcessName;
}

/*-------------------------------------------------------------------------*/
    
bool
CLogSvcServerFileLogger::GetUseSeperateLogPerProcessName( void ) const
{GUCEF_TRACE;

    return m_seperateLogPerProcessName;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::SetUseSeperateLogPerApp( bool logPerApp )
{GUCEF_TRACE;

    m_seperateLogPerApp = logPerApp;
}

/*-------------------------------------------------------------------------*/
    
bool
CLogSvcServerFileLogger::GetUseSeperateLogPerApp( void ) const
{GUCEF_TRACE;

    return m_seperateLogPerApp;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::StartOfLoggingForClient( const TClientInfo& clientInfo )
{GUCEF_TRACE;
    
    // We don't have to do anything here, GetFileAccess() uses lazy initialization
}

/*-------------------------------------------------------------------------*/
    
void
CLogSvcServerFileLogger::EndOfLoggingForClient( const TClientInfo& clientInfo )
{GUCEF_TRACE;

    TOutputMap::iterator i = m_clientMap.find( clientInfo.addressAndPort );
    if ( i != m_clientMap.end() )
    {
        TFileAccessPtr clientFile = (*i).second;
        
        // Remove the entry for this client
        m_clientMap.erase( i );        
        
        // Check if this was the last client using this log
        if ( 3 == clientFile.GetReferenceCount() )
        {
            // Be nice and close the log now
            clientFile->Flush();
            clientFile->Close();
            
            // Create a key for the output map and erase the file over there as well
            // since this was the last client using it.
            // Note that the last reference is the local shared pointer in this scope.
            CORE::CString filePath = CreateRelOutputFilePath( clientInfo );
            m_outputMap.erase( filePath );
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLogSvcServerFileLogger::CreateRelOutputFilePath( const TClientInfo& clientInfo )
{GUCEF_TRACE;

    CORE::CString filename;
    if ( m_seperateLogPerProcessName && m_seperateLogPerApp )
    {
        filename = "APP_" + clientInfo.appName + "/PROC_" + clientInfo.processName + "/ADDR_" + clientInfo.addressAndPort;
    }
    else
    if ( !m_seperateLogPerProcessName && m_seperateLogPerApp )
    {
        filename = "APP_" + clientInfo.appName + "/ADDR_" + clientInfo.addressAndPort;
    }
    else
    if ( m_seperateLogPerProcessName && !m_seperateLogPerApp )
    {
        filename = "PROC_" + clientInfo.processName + "/ADDR_" + clientInfo.addressAndPort;
    }
    else
    {
        filename = "ADDR_" + clientInfo.addressAndPort;
    }
    
    // Remove chars not allowed in filenames
    filename = filename.RemoveChar( '*' );
    filename = filename.RemoveChar( '?' );
    filename = filename.RemoveChar( ':' );
    filename = filename.RemoveChar( '>' );
    filename = filename.RemoveChar( '<' );
    filename = filename.RemoveChar( '|' );
    
    filename = filename.ReplaceChar( ' ', '_' );    
    filename = filename.ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR );
    
    return filename;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLogSvcServerFileLogger::CreateAbsOutputFilePath( const CORE::CString& relPath )
{GUCEF_TRACE;

    CORE::CString absBasePath = CORE::RelativePath( m_outputDir );
    CORE::AppendToPath( absBasePath, relPath );
    CORE::CString absPath = absBasePath + "_0.txt";

    CORE::UInt32 counter = 1;
    
    while ( CORE::FileExists( absPath ) )
    {
        absPath = absBasePath;
        CORE::AppendToPath( absPath, '_' + CORE::UInt32ToString( counter ) + ".txt" );
        ++counter;
    }
    
    return absPath;
}

/*-------------------------------------------------------------------------*/

CLogSvcServerFileLogger::TFileAccessPtr
CLogSvcServerFileLogger::GetFileAccess( const TClientInfo& clientInfo )
{GUCEF_TRACE;
    
    // First try to find an existing output file
    // Note that flags affecting output file paths do not take effect while there
    // are clients using the relevant logs.
    TOutputMap::iterator i = m_clientMap.find( clientInfo.addressAndPort );
    if ( i != m_clientMap.end() )
    {
        return (*i).second;
    }
    
    // This client does not have any output defined yet.
    // See if we can use an existing file
    CORE::CString relOutputPath = CreateRelOutputFilePath( clientInfo );
    i = m_outputMap.find( relOutputPath );
    if ( i != m_outputMap.end() )
    {
        // With the current settings this client will be mapped to an existing file
        m_clientMap[ clientInfo.addressAndPort ] = (*i).second;
        return (*i).second;
    }

    // Unable to find an existing output file, lets make a new one
    // First determine the output path we want    
    CORE::CString absOutputPath = CreateAbsOutputFilePath( relOutputPath );
    
    // Now make the file and add it to our output map
    CORE::CString pathForFile = CORE::StripFilename( absOutputPath );
    if ( CreateDirs( pathForFile ) )
    {
        //CORE::CString filename = CORE::ExtractFilename( absOutputPath );
    
        TFileAccessPtr filePtr = TFileAccessPtr( new CORE::CFileAccess( absOutputPath, "w" ) );
        m_outputMap[ relOutputPath ] = filePtr;
    
        // Now also make a client entry for this new log file
        m_clientMap[ clientInfo.addressAndPort ] = filePtr;
        return filePtr;   
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "LogSvcServerFileLogger: Failed to create path for logfile: " + pathForFile );
    return TFileAccessPtr();
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::Log( const TClientInfo& clientInfo    ,
                              const TLogMsgType logMsgType     ,
                              const CORE::Int32 logLevel       ,
                              const CORE::CString& logMessage  ,
                              const CORE::UInt32 threadId      ,
                              const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    if ( logLevel >= m_minimalLogLevel )
    {
        CORE::CString actualLogMsg = m_logFormatter->FormatLogMessage( logMsgType       ,
                                                                       logLevel         ,
                                                                       logMessage       ,
                                                                       threadId         ,
                                                                       timestamp        ) + '\n';

        TFileAccessPtr fileAccess = GetFileAccess( clientInfo );        
        if ( 0 != fileAccess )
        {
            fileAccess->Write( actualLogMsg.C_String() ,
                               actualLogMsg.Length()   ,
                               1                       );
        }
    }
}


/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::SetWhetherProcessNameIsLogged( bool logProcessName )
{GUCEF_TRACE;

    m_logProcessName = logProcessName;
}

/*-------------------------------------------------------------------------*/
    
bool
CLogSvcServerFileLogger::GetWhetherProcessNameIsLogged( void ) const
{GUCEF_TRACE;

    return m_logProcessName;
}


/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::SetWhetherAppNameIsLogged( bool logAppName )
{GUCEF_TRACE;

    m_logAppName = logAppName;
}

/*-------------------------------------------------------------------------*/
    
bool
CLogSvcServerFileLogger::GetWhetherAppNameIsLogged( void ) const
{GUCEF_TRACE;

    return m_logAppName;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::SetWhetherProcessIdIsLogged( bool logProcessId )
{GUCEF_TRACE;

    m_logProcessId = logProcessId;
}

/*-------------------------------------------------------------------------*/
    
bool
CLogSvcServerFileLogger::GetWhetherProcessIdIsLogged( void ) const
{GUCEF_TRACE;

    return m_logProcessId;
}

/*-------------------------------------------------------------------------*/
                      
void
CLogSvcServerFileLogger::FlushLog( const TClientInfo& clientInfo )
{GUCEF_TRACE;

    GetFileAccess( clientInfo )->Flush();
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::SetOutputDir( const CORE::CString& logOutputDir )
{GUCEF_TRACE;

    m_outputDir = logOutputDir;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLogSvcServerFileLogger::GetOutputDir( void ) const
{GUCEF_TRACE;

    return m_outputDir;
}

/*-------------------------------------------------------------------------*/

void
CLogSvcServerFileLogger::SetMinimalLogLevel( const CORE::Int32 minimalLogLevel )
{GUCEF_TRACE;

    m_minimalLogLevel = minimalLogLevel;
}

/*-------------------------------------------------------------------------*/

CORE::Int32
CLogSvcServerFileLogger::GetMinimalLogLevel( void ) const
{GUCEF_TRACE;

    return m_minimalLogLevel;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LOGSERVICELIB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
