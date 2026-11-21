/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#include <time.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CDeployAudit.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CDeployAudit::FileName = "deploy.jsonl";

static const UInt32 KeptRotations = 3;
static const UInt32 MaxReasonChars = 2000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
NowIsoUtc( void )
{
    char buffer[ 32 ];
    time_t now = ::time( GUCEF_NULL );
    struct tm parts;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    ::gmtime_s( &parts, &now );
    #else
    ::gmtime_r( &now, &parts );
    #endif
    ::strftime( buffer, sizeof( buffer ), "%Y-%m-%dT%H:%M:%SZ", &parts );
    return buffer;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDeployAudit::CDeployAudit( void )
    : m_directory()
    , m_maxFileBytes( (UInt64) 4 * 1024 * 1024 )
    , m_redactor( CORE::REDACT_TEXT_AUTH_SCHEMES | CORE::REDACT_TEXT_KEY_VALUES )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDeployAudit::SetDirectory( const CORE::CString& directory )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_directory = directory;
}

/*-------------------------------------------------------------------------*/

void
CDeployAudit::SetMaxFileBytes( UInt64 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxFileBytes = maxBytes > 4096 ? maxBytes : 4096;
}

/*-------------------------------------------------------------------------*/

void
CDeployAudit::AddSecret( const CORE::CString& secret )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !secret.IsNULLOrEmpty() )
        m_redactor.AddSecret( secret );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDeployAudit::GetFilePath( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_directory.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CombinePath( m_directory, FileName );
}

/*-------------------------------------------------------------------------*/

void
CDeployAudit::RotateIfNeeded( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !CORE::FileExists( path ) || CORE::FileSize( path ) < m_maxFileBytes )
        return;
    // deploy.jsonl.3 goes, .2 becomes .3, .1 becomes .2, the file becomes .1
    const CORE::CString oldest = path + "." + CORE::UInt32ToString( KeptRotations );
    if ( CORE::FileExists( oldest ) )
        CORE::DeleteFile( oldest );
    for ( UInt32 i=KeptRotations; i>1; --i )
    {
        const CORE::CString from = path + "." + CORE::UInt32ToString( i-1 );
        if ( CORE::FileExists( from ) )
            CORE::MoveFile( from, path + "." + CORE::UInt32ToString( i ), true );
    }
    CORE::MoveFile( path, path + ".1", true );
}

/*-------------------------------------------------------------------------*/

bool
CDeployAudit::Record( const CCiDeployResult& result, const CORE::CString& callerMachineId )
{GUCEF_TRACE;

    CORE::CDataNode line( GUCEF_DATATYPE_OBJECT );
    line.SetAttribute( "time", NowIsoUtc() );
    line.SetAttribute( "action", result.action );
    line.SetAttribute( "caller", callerMachineId );
    line.SetAttribute( "ci", result.ciId );
    line.SetAttribute( "image", result.image );
    line.SetAttribute( "target", result.target );
    line.SetAttribute( "namespace", result.nameSpace );
    line.SetAttribute( "deployment", result.deployment );
    line.SetAttribute( "run", result.runId );
    line.SetAttribute( "commit", result.commit );
    line.SetAttribute( "digest", result.digest );
    line.SetAttribute( "oldDigest", result.previousDigest );
    line.SetAttribute( "result", result.outcome );
    line.SetAttribute( "code", result.code );
    CORE::CString reason;
    {
        MT::CScopeMutex lock( m_lock );
        reason = m_redactor.RedactText( result.message );
    }
    line.SetAttribute( "reason", CCiUtils::Bound( reason, MaxReasonChars ) );
    CORE::CString text;
    if ( !CCiUtils::ToJson( line, text ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: Audit: A line could not be written as JSON" );
        return false;
    }
    // one object per line: a line end can only be white space between the members, inside strings it is escaped
    text = CCiUtils::Trimmed( text.ReplaceSubstr( "\r", "" ).ReplaceSubstr( "\n", "" ) ) + "\n";

    MT::CScopeMutex lock( m_lock );
    if ( m_directory.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: Audit: No directory is set, the line is only logged: " << text );
        return false;
    }
    CORE::CreateDirs( m_directory );
    const CORE::CString path = CORE::CombinePath( m_directory, FileName );
    RotateIfNeeded( path );
    CORE::CFileAccess file;
    if ( !file.Open( path, "ab" ) || !file.Opened() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: Audit: " << path << " cannot be written; the line is only logged: " << text );
        return false;
    }
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    const bool written = file.Write( text.C_String(), 1, size ) == size;
    file.Close();
    if ( !written )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "gucefMcpCi: Audit: " << path << " could not be written completely" );
    return written;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
