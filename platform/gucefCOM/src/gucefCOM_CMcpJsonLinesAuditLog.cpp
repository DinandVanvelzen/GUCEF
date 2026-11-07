/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefCOM_CMcpJsonLinesAuditLog.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpJsonLinesAuditLog::CMcpJsonLinesAuditLog( void )
    : CORE::CObservingNotifier()
    , m_lock()
    , m_output()
    , m_flushEveryRecord( true )
    , m_restrictToOwner( true )
    , m_recordsWritten( 0 )
    , m_writeFailures( 0 )
    , m_rollingPath()
    , m_rolloverBytes( 0 )
    , m_maxFiles( 2 )
    , m_fileBytes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpJsonLinesAuditLog::~CMcpJsonLinesAuditLog()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();
    Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpJsonLinesAuditLog::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpJsonLinesAuditLog";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJsonLinesAuditLog::Attach( CMcpServer& server )
{GUCEF_TRACE;

    typedef CORE::CTEventHandlerFunctor< CMcpJsonLinesAuditLog > TEventCallback;
    TEventCallback callback( this, &CMcpJsonLinesAuditLog::OnAuditEvent );
    SubscribeTo( &server, CMcpServer::AuditEvent, callback );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpJsonLinesAuditLog::Detach( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CMcpJsonLinesAuditLog::SetOutput( const CORE::IOAccessPtr& output )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !output.IsNULL() && !output->IsWriteable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpJsonLinesAuditLog: The output is not writeable" );
        return false;
    }
    m_output = output;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJsonLinesAuditLog::OpenRollingFile( const CORE::CString& path, UInt64 rolloverBytes, UInt32 maxFiles )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() || 0 == rolloverBytes )
        return false;

    CORE::CString dir = CORE::StripFilename( path );
    if ( !dir.IsNULLOrEmpty() && !CORE::CreateDirs( dir ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpJsonLinesAuditLog: Cannot create the directory of the audit log" );
        return false;
    }

    MT::CScopeMutex lock( m_lock );
    m_rollingPath = path;
    m_rolloverBytes = rolloverBytes;
    m_maxFiles = maxFiles < 2 ? 2 : ( maxFiles > 1000 ? 1000 : maxFiles );
    if ( !OpenRollingFileUnlocked() )
    {
        m_rollingPath.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJsonLinesAuditLog::OpenRollingFileUnlocked( void )
{GUCEF_TRACE;

    if ( m_restrictToOwner )
    {
        // the file is made for its owner only before anything is in it; one that is there already is restricted
        CORE::CString problem;
        if ( !CORE::FileExists( m_rollingPath ) )
        {
            if ( !CORE::CreateOwnerOnlyFile( m_rollingPath, "", 0, problem ) && !CORE::FileExists( m_rollingPath ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpJsonLinesAuditLog: Cannot make the audit log file: " << problem );
                return false;
            }
        }
        if ( !CORE::SetOwnerOnlyAccess( m_rollingPath ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpJsonLinesAuditLog: The audit log file cannot be restricted to its owner" );
    }
    CORE::CFileAccess* file = GUCEF_NEW CORE::CFileAccess( m_rollingPath, "ab" );
    if ( !file->IsValid() || !file->Opened() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpJsonLinesAuditLog: Cannot open the audit log file for writing" );
        GUCEF_DELETE file;
        return false;
    }
    m_output = CORE::IOAccessPtr( file );
    m_fileBytes = CORE::FileSize( m_rollingPath );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJsonLinesAuditLog::Rollover( void )
{GUCEF_TRACE;

    // "audit.jsonl" becomes "audit.1.jsonl", "audit.1.jsonl" becomes "audit.2.jsonl" and so on; what would become file number maxFiles goes
    if ( !m_output.IsNULL() )
    {
        m_output->Flush();
        m_output->Close();
        m_output = CORE::IOAccessPtr();
    }

    CORE::CString dir = CORE::StripFilename( m_rollingPath );
    CORE::CString name = CORE::ExtractFilename( m_rollingPath );
    CORE::CString extension = name.SubstrToChar( '.', false );
    CORE::CString stem = extension.IsNULLOrEmpty() ? name : name.CutChars( extension.Length() + 1, false );
    if ( !extension.IsNULLOrEmpty() )
        extension = '.' + extension;
    CORE::CString oldest = CORE::CombinePath( dir, stem + '.' + CORE::UInt32ToString( m_maxFiles - 1 ) + extension );
    if ( CORE::FileExists( oldest ) )
        CORE::DeleteFile( oldest );
    for ( UInt32 n=m_maxFiles-1; n>1; --n )
    {
        CORE::CString from = CORE::CombinePath( dir, stem + '.' + CORE::UInt32ToString( n - 1 ) + extension );
        CORE::CString to = CORE::CombinePath( dir, stem + '.' + CORE::UInt32ToString( n ) + extension );
        if ( CORE::FileExists( from ) )
            CORE::MoveFile( from, to, true );
    }
    if ( CORE::FileExists( m_rollingPath ) )
        CORE::MoveFile( m_rollingPath, CORE::CombinePath( dir, stem + ".1" + extension ), true );
    return OpenRollingFileUnlocked();
}

/*-------------------------------------------------------------------------*/

void
CMcpJsonLinesAuditLog::SetRestrictToOwner( bool restrictToOwner )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_restrictToOwner = restrictToOwner;
}

/*-------------------------------------------------------------------------*/

void
CMcpJsonLinesAuditLog::SetFlushEveryRecord( bool flushEveryRecord )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_flushEveryRecord = flushEveryRecord;
}

/*-------------------------------------------------------------------------*/

bool
CMcpJsonLinesAuditLog::WriteRecord( const CMcpAuditRecord& record )
{GUCEF_TRACE;

    CORE::CString line;
    if ( !record.ToJsonLine( line ) )
    {
        MT::CScopeMutex lock( m_lock );
        ++m_writeFailures;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpJsonLinesAuditLog: An audit record could not be turned into JSON" );
        return false;
    }
    line += '\n';

    MT::CScopeMutex lock( m_lock );
    if ( m_output.IsNULL() || !m_output->Opened() )
    {
        ++m_writeFailures;
        return false;
    }
    const UInt32 bytes = ( line.ByteSize() > 0 ? line.ByteSize() - 1 : 0 );
    if ( !m_rollingPath.IsNULLOrEmpty() && m_fileBytes > 0 && m_fileBytes + bytes > m_rolloverBytes )
    {
        if ( !Rollover() )
        {
            ++m_writeFailures;
            return false;
        }
    }
    // one call per line, so a record is never split
    const UInt32 written = m_output->Write( line, false );
    if ( written != bytes )
    {
        ++m_writeFailures;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpJsonLinesAuditLog: An audit record could not be written" );
        return false;
    }
    m_fileBytes += bytes;
    if ( m_flushEveryRecord )
        m_output->Flush();
    ++m_recordsWritten;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpJsonLinesAuditLog::Flush( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_output.IsNULL() && m_output->Opened() )
        m_output->Flush();
}

/*-------------------------------------------------------------------------*/

void
CMcpJsonLinesAuditLog::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_output.IsNULL() )
    {
        if ( m_output->Opened() )
        {
            m_output->Flush();
            m_output->Close();
        }
        m_output = CORE::IOAccessPtr();
    }
    m_rollingPath.Clear();
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpJsonLinesAuditLog::GetRecordsWritten( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_recordsWritten;
}

/*-------------------------------------------------------------------------*/

UInt64
CMcpJsonLinesAuditLog::GetWriteFailures( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_writeFailures;
}

/*-------------------------------------------------------------------------*/

void
CMcpJsonLinesAuditLog::OnAuditEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const TMcpAuditEventData* data = static_cast< const TMcpAuditEventData* >( eventData );
    if ( GUCEF_NULL != data )
        WriteRecord( data->GetData() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
