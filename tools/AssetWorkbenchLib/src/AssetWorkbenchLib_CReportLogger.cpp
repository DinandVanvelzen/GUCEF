/*
 *  AssetWorkbenchLib: a logger that collects the warnings and errors of the workbench for its report view
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "AssetWorkbenchLib_CReportLogger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

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
    , m_foldCount()
    , m_foldFirst()
    , m_foldNewSincePump( false )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) FOLD_COUNT; ++i )
        m_foldCount[ i ] = 0;
}

/*-------------------------------------------------------------------------*/

CReportLogger::~CReportLogger()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CReportLogger::EFold
CReportLogger::ClassifyMessage( const CORE::CString& logMessage, CORE::CString& name )
{GUCEF_TRACE;

    static const char* const notInPack = "Vfs: Unable to locate a mount which can provide the file";
    const Int32 at = logMessage.HasSubstr( notInPack );
    if ( at >= 0 )
    {
        // "...the file: <path>" and "...the file using the provided meta-data constraints: <path>"
        const Int32 colon = logMessage.HasSubstr( CORE::CString( ": " ), at + (Int32) strlen( notInPack ), true );
        name = colon >= 0 ? logMessage.CutChars( (UInt32) colon + 2, true ) : CORE::CString::Empty;
        return FOLD_FILE_NOT_IN_PACK;
    }
    // "Ogre: Error: preparing texture 'x.psd'. Texture layer will be blank: Cannot locate resource x.psd in resource group g": the same thing, for Ogre
    const Int32 blank = logMessage.HasSubstr( CORE::CString( "Texture layer will be blank" ) );
    if ( blank >= 0 )
    {
        const Int32 open = logMessage.HasSubstr( CORE::CString( "preparing texture '" ) );
        const Int32 close = open >= 0 ? logMessage.HasSubstr( CORE::CString( "'" ), open + 19, true ) : -1;
        name = open >= 0 && close > open + 19 ? logMessage.SubstrFromRange( (UInt32) open + 19, (UInt32) close ) : CORE::CString::Empty;
        return FOLD_FILE_NOT_IN_PACK;
    }
    // "CModelView: The picture x.png of material m is not in the mount: the material is shown without it": the same, for the model view
    if ( logMessage.HasSubstr( CORE::CString( " is not in the mount: the material is shown without it" ) ) >= 0 )
    {
        const Int32 open = logMessage.HasSubstr( CORE::CString( "The picture " ) );
        const Int32 close = open >= 0 ? logMessage.HasSubstr( CORE::CString( " of material " ), open + 12, true ) : -1;
        name = open >= 0 && close > open + 12 ? logMessage.SubstrFromRange( (UInt32) open + 12, (UInt32) close ) : CORE::CString::Empty;
        return FOLD_FILE_NOT_IN_PACK;
    }
    static const char* const skipping = "Skipping '";
    const Int32 skip = logMessage.HasSubstr( skipping );
    if ( skip >= 0 && logMessage.HasSubstr( CORE::CString( "' because it already exists in resource group" ) ) > skip )
    {
        const UInt32 start = (UInt32) skip + 10;
        const Int32 end = logMessage.HasSubstr( CORE::CString( "' because it already exists" ), (Int32) start, true );
        name = end > (Int32) start ? logMessage.SubstrFromRange( start, (UInt32) end ) : CORE::CString::Empty;
        return FOLD_NAME_IN_MORE_FOLDERS;
    }
    // "Ogre: Warning: the mesh 'x' includes vertices with more than 4 bone assignments. The lowest weighted ..."
    if ( logMessage.HasSubstr( CORE::CString( "includes vertices with more than 4 bone assignments" ) ) >= 0 )
    {
        const Int32 open = logMessage.HasSubstr( CORE::CString( "the mesh '" ) );
        const Int32 close = open >= 0 ? logMessage.HasSubstr( CORE::CString( "'" ), open + 10, true ) : -1;
        name = open >= 0 && close > open + 10 ? logMessage.SubstrFromRange( (UInt32) open + 10, (UInt32) close ) : CORE::CString::Empty;
        return FOLD_WEIGHTS_REDUCED;
    }
    return FOLD_NONE;
}

/*-------------------------------------------------------------------------*/

bool
CReportLogger::IsImporterDiagnostic( const CORE::CString& logMessage )
{GUCEF_TRACE;

    return logMessage.HasSubstr( CORE::CString( "Assimp: Error, " ) ) >= 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CReportLogger::MakeFoldedMessage( EFold fold, UInt32 count, const CORE::CString& first )
{GUCEF_TRACE;

    const CORE::CString shown = first.IsNULLOrEmpty() ? CORE::CString::Empty : " (the first: " + first + ")";
    switch ( fold )
    {
        case FOLD_FILE_NOT_IN_PACK:
            return CORE::UInt32ToString( count ) + " files that the pack refers to are not in it" + shown + ": the default texture or material is used";
        case FOLD_NAME_IN_MORE_FOLDERS:
            return CORE::UInt32ToString( count ) + " file names are in more than one folder of the pack" + shown + ": a bare name finds the first folder, a name with its folder finds the right one";
        case FOLD_WEIGHTS_REDUCED:
            return CORE::UInt32ToString( count ) + " submeshes have vertices with more than 4 bone influences" + shown + ": the 4 strongest are kept, renormalised (hardware skinning takes 4), so the movement can differ slightly from the source";
        default:
            return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

void
CReportLogger::Keep( const CORE::TLogMsgType logMsgType, const CORE::CString& logMessage )
{GUCEF_TRACE;

    bool isError = CORE::LOG_ERROR == logMsgType || CORE::LOG_EXCEPTION == logMsgType;
    bool isWarning = CORE::LOG_WARNING == logMsgType;
    if ( !isError && !isWarning )
        return;

    MT::CScopeMutex lock( m_lock );

    // the importer calls its own complaints errors; the import goes on, and one that fails is reported by whoever asked for it
    if ( isError && IsImporterDiagnostic( logMessage ) )
    {
        isError = false;
        isWarning = true;
    }

    // not a failure, one line for all of them (see the class)
    CORE::CString name;
    const EFold fold = ClassifyMessage( logMessage, name );
    if ( FOLD_NONE != fold )
    {
        if ( 0 == m_foldCount[ fold ] )
            m_foldFirst[ fold ] = name;
        ++m_foldCount[ fold ];
        m_foldNewSincePump = true;
        return;
    }
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
    m_pending.push_back( GUI::CReportViewForm::TEntry( isError ? GUI::CReportViewForm::SEVERITY_FAILURE : GUI::CReportViewForm::SEVERITY_WARNING, CORE::CString::Empty, message, details ) );
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

        // the folded lines wait until a Pump() sees that no more of them came since the one before
        if ( m_foldNewSincePump )
            m_foldNewSincePump = false;
        else
        {
            for ( UInt32 i=0; i<(UInt32) FOLD_COUNT; ++i )
            {
                if ( 0 == m_foldCount[ i ] )
                    continue;
                const bool info = FOLD_NAME_IN_MORE_FOLDERS == (EFold) i || FOLD_WEIGHTS_REDUCED == (EFold) i;
                entries.push_back( GUI::CReportViewForm::TEntry( info ? GUI::CReportViewForm::SEVERITY_INFO : GUI::CReportViewForm::SEVERITY_WARNING, CORE::CString::Empty,
                                                                 MakeFoldedMessage( (EFold) i, m_foldCount[ i ], m_foldFirst[ i ] ), CORE::CString::Empty ) );
                if ( !info )
                    ++m_warnings;
                m_foldCount[ i ] = 0;
                m_foldFirst[ i ] = CORE::CString::Empty;
            }
        }
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

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
