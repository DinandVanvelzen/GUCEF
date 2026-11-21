/*
 *  assetpluginUNREAL: The Oodle provider that asks an external tool in a child process
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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#include "assetpluginUNREAL_CUnrealCapabilities.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H
#include "assetpluginUNREAL_COodleToolProvider.h"
#define GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_COODLETOOLPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The longest answer line of the tool that is accepted: a longer one is not the protocol */
static const size_t MaxAnswerLineBytes = 4096;

/** The poll interval while waiting for an answer, in milliseconds: how fast a cancellation is noticed */
static const UInt32 PollIntervalMs = 50;

/*-------------------------------------------------------------------------*/

COodleToolProvider::COodleToolProvider( void )
    : CIUnrealOodleProvider()
    , m_toolPath( CUnrealCapabilities::GetExpectedOodleToolPath() )
    , m_workFolder()
    , m_process()
    , m_running( false )
    , m_requestTimeoutSecs( 120 )
    , m_startCount( 0 )
    , m_requestCounter( 0 )
    , m_answerBuffer()
    , m_cancellation( GUCEF_NULL )
{GUCEF_TRACE;

    m_process.SetIsolateFromParent( true );
}

/*-------------------------------------------------------------------------*/

COodleToolProvider::COodleToolProvider( const CORE::CString& toolPath )
    : CIUnrealOodleProvider()
    , m_toolPath( toolPath )
    , m_workFolder()
    , m_process()
    , m_running( false )
    , m_requestTimeoutSecs( 120 )
    , m_startCount( 0 )
    , m_requestCounter( 0 )
    , m_answerBuffer()
    , m_cancellation( GUCEF_NULL )
{GUCEF_TRACE;

    m_process.SetIsolateFromParent( true );
}

/*-------------------------------------------------------------------------*/

COodleToolProvider::~COodleToolProvider()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

void
COodleToolProvider::SetRequestTimeoutSecs( UInt32 seconds )
{GUCEF_TRACE;

    m_requestTimeoutSecs = seconds;
}

/*-------------------------------------------------------------------------*/

UInt32
COodleToolProvider::GetStartCount( void ) const
{GUCEF_TRACE;

    return m_startCount;
}

/*-------------------------------------------------------------------------*/

bool
COodleToolProvider::IsAvailable( void )
{GUCEF_TRACE;

    return CORE::FileExists( m_toolPath );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COodleToolProvider::GetUnavailableReason( void )
{GUCEF_TRACE;

    if ( CORE::FileExists( m_toolPath ) )
        return CORE::CString::Empty;
    return CORE::CString( CUnrealCapabilities::NeedsOodleProvider ) + " at " + m_toolPath;
}

/*-------------------------------------------------------------------------*/

void
COodleToolProvider::SetCancellationSource( const ASSETCONVERTER::CConversionReport* report )
{GUCEF_TRACE;

    m_cancellation = report;
}

/*-------------------------------------------------------------------------*/

bool
COodleToolProvider::IsCancelled( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_cancellation && m_cancellation->IsCancellationRequested();
}

/*-------------------------------------------------------------------------*/

void
COodleToolProvider::StopChild( void )
{GUCEF_TRACE;

    if ( m_running )
    {
        m_process.CloseStdinPipe();
        // a tool that is waiting for the next request ends by itself when its input ends; one that is busy or hung is forced
        Int32 exitCode = 0;
        if ( !m_process.WaitForExit( 200, exitCode ) )
            m_process.Terminate( false, 1000 );
        m_running = false;
    }
    m_answerBuffer.clear();
}

/*-------------------------------------------------------------------------*/

void
COodleToolProvider::Stop( void )
{GUCEF_TRACE;

    StopChild();
    if ( !m_workFolder.IsNULLOrEmpty() )
    {
        CORE::DeleteDir( m_workFolder, true );
        m_workFolder.Clear();
    }
}

/*-------------------------------------------------------------------------*/

bool
COodleToolProvider::StartChild( CORE::CString& error )
{GUCEF_TRACE;

    if ( m_workFolder.IsNULLOrEmpty() )
    {
        // below the cache folder of the user, one folder per provider; removed again by Stop()
        CORE::CString folder = CORE::ResolveVars( "$CACHEDIR$" );
        CORE::AppendToPath( folder, "GUCEF" );
        CORE::AppendToPath( folder, "UnrealReader" );
        CORE::AppendToPath( folder, "oodle_" + CORE::UInt64ToString( CORE::StringToUInt64( CORE::ResolveVars( "$PID$" ), 0 ) ) + "_" + CORE::UInt64ToString( (UInt64) reinterpret_cast< size_t >( this ) ) );
        if ( !CORE::CreateDirs( folder ) )
        {
            error = "The work folder of the oodle tool cannot be created: " + folder;
            return false;
        }
        m_workFolder = folder;
    }

    CORE::CProcess::TStringVector args;
    args.push_back( "--batch" );
    args.push_back( "--timeout" );
    args.push_back( CORE::UInt32ToString( m_requestTimeoutSecs ) );
    if ( !m_process.LaunchWithPipes( m_toolPath, args, true, true, false ) )
    {
        error = "The oodle tool cannot be started: " + m_toolPath;
        return false;
    }
    m_running = true;
    m_answerBuffer.clear();
    ++m_startCount;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The number at the start of the text (after blanks); 0 when there is none */
static UInt64
ParseNumber( const char* text )
{GUCEF_TRACE;

    while ( ' ' == *text )
        ++text;
    UInt64 value = 0;
    while ( *text >= '0' && *text <= '9' )
    {
        value = value * 10 + (UInt64) ( *text - '0' );
        ++text;
    }
    return value;
}

/*-------------------------------------------------------------------------*/

/** A path as one parameter of a request line: in double quotes, with the quote and the backslash escaped */
static CORE::CString
Quoted( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString text = "\"";
    for ( UInt32 i=0; i<path.Length(); ++i )
    {
        if ( '"' == path[ i ] || '\\' == path[ i ] )
            text += '\\';
        text += path[ i ];
    }
    text += "\"";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
COodleToolProvider::ReadAnswer( CORE::CString& line, CORE::CString& error, bool& cancelled )
{GUCEF_TRACE;

    cancelled = false;
    CORE::CProcessPipe* pipe = m_process.GetStdoutPipe();
    if ( GUCEF_NULL == pipe )
    {
        error = "The oodle tool has no answer channel";
        return false;
    }
    const UInt64 start = MT::PrecisionTickCount();
    const Float64 limitMs = ( (Float64) m_requestTimeoutSecs + 2.0 ) * 1000.0;
    for ( ;; )
    {
        for ( size_t i=0; i<m_answerBuffer.size(); ++i )
            if ( '\n' == m_answerBuffer[ i ] )
            {
                size_t length = i;
                if ( length > 0 && '\r' == m_answerBuffer[ length - 1 ] )
                    --length;
                line = m_answerBuffer.empty() || 0 == length ? CORE::CString::Empty : CORE::CString( &m_answerBuffer[ 0 ], (UInt32) length );
                m_answerBuffer.erase( m_answerBuffer.begin(), m_answerBuffer.begin() + i + 1 );
                return true;
            }
        if ( m_answerBuffer.size() > MaxAnswerLineBytes )
        {
            error = "The answer of the oodle tool is not a line of the protocol";
            return false;
        }
        if ( IsCancelled() )
        {
            cancelled = true;
            error = "cancelled";
            return false;
        }
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > limitMs )
        {
            error = "The oodle tool did not answer within " + CORE::UInt32ToString( m_requestTimeoutSecs ) + " seconds (timeout)";
            return false;
        }
        if ( !pipe->WaitForData( PollIntervalMs ) )
            continue;
        char chunk[ 512 ];
        const UInt32 got = pipe->ReadSome( chunk, sizeof( chunk ) );
        if ( 0 == got )
        {
            error = "The oodle tool ended without an answer";
            return false;
        }
        m_answerBuffer.insert( m_answerBuffer.end(), chunk, chunk + got );
    }
}

/*-------------------------------------------------------------------------*/

COodleToolProvider::EAttempt
COodleToolProvider::TryBlock( const void* compressed, UInt32 compressedSize, UInt32 decompressedSize, void* dest, CORE::CString& error, Int32& toolCode )
{GUCEF_TRACE;

    toolCode = 0;
    if ( !m_running && !StartChild( error ) )
        return ATTEMPT_FAILED;

    const CORE::CString number = CORE::UInt32ToString( ++m_requestCounter );
    CORE::CString inPath = m_workFolder;
    CORE::AppendToPath( inPath, "in_" + number + ".bin" );
    CORE::CString outPath = m_workFolder;
    CORE::AppendToPath( outPath, "out_" + number + ".bin" );
    {
        CORE::CFileAccess file( inPath, "wb" );
        if ( !file.Opened() || file.Write( compressed, 1, compressedSize ) != compressedSize )
        {
            error = "The compressed block cannot be written to " + inPath;
            return ATTEMPT_FAILED;
        }
    }
    const CORE::CString request = "--in " + Quoted( inPath ) + " --offset 0 --size " + CORE::UInt32ToString( compressedSize ) + " --decompressed-size " + CORE::UInt32ToString( decompressedSize ) + " --out " + Quoted( outPath ) + "\n";

    EAttempt result = ATTEMPT_LOST;
    CORE::CProcessPipe* input = m_process.GetStdinPipe();
    if ( GUCEF_NULL == input || input->Write( request.C_String(), 1, request.Length() ) != request.Length() )
    {
        error = "The oodle tool does not accept requests (it ended)";
    }
    else
    {
        CORE::CString line;
        bool cancelled = false;
        if ( !ReadAnswer( line, error, cancelled ) )
        {
            if ( cancelled )
                result = ATTEMPT_FAILED;
            else if ( error.HasSubstr( "timeout" ) >= 0 )
                result = ATTEMPT_FAILED;
        }
        else if ( 0 == line.HasSubstr( "ok " ) )
        {
            const UInt64 bytes = ParseNumber( line.C_String() + 3 );
            const UInt64 onDisk = CORE::FileExists( outPath ) ? CORE::FileSize( outPath ) : 0;
            if ( bytes != decompressedSize || onDisk != decompressedSize )
            {
                error = "The oodle tool delivered " + CORE::UInt64ToString( onDisk ) + " bytes where " + CORE::UInt32ToString( decompressedSize ) + " were expected (truncated output): " + line;
                result = ATTEMPT_FAILED;
            }
            else
            {
                CORE::CFileAccess file( outPath, "rb" );
                if ( file.Opened() && file.Read( dest, 1, decompressedSize ) == decompressedSize )
                    result = ATTEMPT_DONE;
                else
                {
                    error = "The decompressed block cannot be read from " + outPath;
                    result = ATTEMPT_FAILED;
                }
            }
        }
        else if ( 0 == line.HasSubstr( "error " ) )
        {
            const CORE::CString rest = line.SubstrFromRange( 6, line.Length() );
            toolCode = (Int32) ParseNumber( rest.C_String() );
            error = "The oodle tool failed with exit code " + CORE::Int32ToString( toolCode ) + ": " + line;
            result = ATTEMPT_FAILED;
        }
        else
        {
            error = "The oodle tool gave an answer that is not part of its protocol";
            result = ATTEMPT_LOST;
        }
    }
    CORE::DeleteFile( inPath );
    CORE::DeleteFile( outPath );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
COodleToolProvider::DecodeBlock( const void* compressed, UInt32 compressedSize, UInt32 decompressedSize, void* dest, CORE::CString& error, Int32& toolCode )
{GUCEF_TRACE;

    toolCode = 0;
    if ( !IsAvailable() )
    {
        error = GetUnavailableReason();
        return false;
    }
    if ( IsCancelled() )
    {
        error = "cancelled";
        return false;
    }

    // a child that is lost is replaced once for this request
    for ( UInt32 attempt=0; attempt<2; ++attempt )
    {
        const EAttempt result = TryBlock( compressed, compressedSize, decompressedSize, dest, error, toolCode );
        if ( ATTEMPT_DONE == result )
            return true;
        if ( ATTEMPT_FAILED == result )
        {
            // a failure that kills the request also ends a child that may be hung or busy (cancellation, timeout)
            if ( error == "cancelled" || error.HasSubstr( "timeout" ) >= 0 )
                StopChild();
            return false;
        }
        Int32 exitCode = 0;
        if ( m_running && m_process.WaitForExit( 100, exitCode ) )
            error += " (exit code " + CORE::Int32ToString( exitCode ) + ")";
        StopChild();
        if ( 0 == attempt )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "COodleToolProvider: " << error << ": the tool is started again for this block" );
        }
    }
    error += ", also after it was started again";
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
