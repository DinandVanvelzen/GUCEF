/*
 *  AssetViewerLib: a logger that collects the warnings and errors of the viewer for its report view
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "AssetViewerLib_CReportLogger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** More than this waits for the GUI thread: the oldest go, so that a loop of errors does not eat the memory */
static const UInt32 MaxPendingEntries = 2000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CReportLogger::CReportLogger( void )
    : CORE::CILogger()
    , m_lock()
    , m_pending()
    , m_warnings( 0 )
    , m_errors( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CReportLogger::~CReportLogger()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CReportLogger::Keep( const CORE::TLogMsgType logMsgType, const CORE::CString& logMessage )
{GUCEF_TRACE;

    const bool isError = CORE::LOG_ERROR == logMsgType || CORE::LOG_EXCEPTION == logMsgType;
    const bool isWarning = CORE::LOG_WARNING == logMsgType;
    if ( !isError && !isWarning )
        return;

    MT::CScopeMutex lock( m_lock );
    if ( isError )
        ++m_errors;
    else
        ++m_warnings;
    if ( m_pending.size() >= MaxPendingEntries )
        m_pending.erase( m_pending.begin() );

    // the first line is the message, the rest of a long text is for the details box
    CORE::CString message = logMessage;
    CORE::CString details;
    const Int32 lineEnd = logMessage.HasChar( '\n', true );
    if ( lineEnd > 0 )
    {
        message = logMessage.SubstrFromRange( 0, (UInt32) lineEnd );
        details = logMessage;
    }
    m_pending.push_back( GUI::CReportViewForm::TEntry( isError ? GUI::CReportViewForm::SEVERITY_ERROR : GUI::CReportViewForm::SEVERITY_WARNING, CORE::CString::Empty, message, details ) );
}

/*-------------------------------------------------------------------------*/

void
CReportLogger::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    Keep( logMsgType, logMessage );
}

/*-------------------------------------------------------------------------*/

void
CReportLogger::Log( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    Keep( logMsgType, logMessage.ToString() );
}

/*-------------------------------------------------------------------------*/

void
CReportLogger::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    Keep( logMsgType, logMessage );
}

/*-------------------------------------------------------------------------*/

void
CReportLogger::LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32 logLevel, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp )
{GUCEF_TRACE;

    Keep( logMsgType, logMessage.ToString() );
}

/*-------------------------------------------------------------------------*/

void
CReportLogger::FlushLog( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CReportLogger::SetMinimalLogLevel( const Int32 minimalLogLevel )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Int32
CReportLogger::GetMinimalLogLevel( void ) const
{GUCEF_TRACE;

    return CORE::LOGLEVEL_EVERYTHING;
}

/*-------------------------------------------------------------------------*/

UInt32
CReportLogger::Pump( GUI::CReportViewForm* report )
{GUCEF_TRACE;

    GUCEF::vector< GUI::CReportViewForm::TEntry > entries;
    {
        MT::CScopeMutex lock( m_lock );
        entries.swap( m_pending );
    }
    if ( GUCEF_NULL != report )
        for ( size_t i=0; i<entries.size(); ++i )
            report->AddEntry( entries[ i ] );
    return (UInt32) entries.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CReportLogger::GetPendingCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CReportLogger::GetWarningCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_warnings;
}

/*-------------------------------------------------------------------------*/

UInt32
CReportLogger::GetErrorCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_errors;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
