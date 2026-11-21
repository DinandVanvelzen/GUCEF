/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefWEB_CSiteDiscoverySession.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSiteDiscoverySession::CSiteDiscoverySession( void )
    : m_settings()
    , m_redactor()
    , m_journal()
    , m_catalog()
    , m_browser()
    , m_givenPulseGenerator()
    , m_pulseGenerator()
    , m_client( GUCEF_NULL )
    , m_recorder( GUCEF_NULL )
    , m_extraSinks()
    , m_open( false )
{GUCEF_TRACE;

    m_journal.SetRedactor( &m_redactor );
    m_catalog.SetRedactor( &m_redactor );
}

/*-------------------------------------------------------------------------*/

CSiteDiscoverySession::~CSiteDiscoverySession()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoverySession::SetSettings( const CSiteDiscoverySettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
    m_redactor.SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::AddHostFilterTerm( const CORE::CString& term )
{GUCEF_TRACE;

    const CORE::CString lower = term.Trim( true ).Trim( false ).Lowercase();
    if ( lower.IsNULLOrEmpty() || CSiteDiscoveryRedactor::MatchesHostFilter( m_settings.hostFilter, lower ) )
        return false;       // an empty filter records every host already, and a host the filter covers is recorded
    for ( size_t i=0; i<m_settings.hostFilter.size(); ++i )
        if ( m_settings.hostFilter[ i ].Lowercase() == lower )
            return false;
    // the registry of the redactor stays: SetSettings() keeps the registered values
    m_settings.hostFilter.push_back( lower );
    m_redactor.SetSettings( m_settings );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteDiscoverySession: Recording the hosts that contain " << lower << " from now on" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoverySession::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    m_givenPulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

const CSiteDiscoverySettings&
CSiteDiscoverySession::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoverySession::GetJournalPath( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_settings.outputDirectory, m_settings.journalFileName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoverySession::GetReportPath( void ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_settings.outputDirectory, m_settings.reportFileName );
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoverySession::AddSink( CIWebObservationSink* sink )
{GUCEF_TRACE;

    if ( GUCEF_NULL == sink )
        return;
    m_extraSinks.push_back( sink );
    if ( GUCEF_NULL != m_recorder )
        m_recorder->AddSink( sink );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::Open( CORE::CString& problem )
{GUCEF_TRACE;

    Release();
    if ( m_settings.outputDirectory.IsNULLOrEmpty() )
    {
        problem = "The settings name no output folder";
        return false;
    }
    if ( !CORE::DirExists( m_settings.outputDirectory ) )
    {
        if ( !CORE::CreateDirs( m_settings.outputDirectory ) )
        {
            problem = "The output folder " + m_settings.outputDirectory + " cannot be made";
            return false;
        }
        CORE::SetOwnerOnlyAccess( m_settings.outputDirectory );
    }

    // what earlier runs recorded is part of the report
    m_catalog.Clear();
    const CORE::CString journalPath = GetJournalPath();
    if ( m_settings.continueJournal && CORE::FileExists( journalPath ) )
    {
        const UInt32 earlier = m_journal.Load( journalPath, m_catalog );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SiteDiscoverySession: The journal holds " << earlier << " requests of earlier runs" );
    }
    if ( !m_journal.Open( journalPath ) )
    {
        problem = "The journal " + journalPath + " cannot be opened";
        return false;
    }

    // a pulse generator of our own, driven by this thread, so waiting for the browser never pumps one of another thread
    if ( m_givenPulseGenerator )
        m_pulseGenerator = m_givenPulseGenerator;
    else
    {
        m_pulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
        m_pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    }

    m_browser.Configure( m_settings );
    m_browser.SetPulseGenerator( m_pulseGenerator );
    if ( CSiteDiscoverySettings::BROWSER_RUNMODE_ATTACH == m_settings.browserRunMode )
    {
        if ( !m_browser.Attach() )
        {
            problem = "No running browser allows remote debugging: start Chrome with --remote-debugging-port=" + CORE::UInt32ToString( m_settings.browserDebugPort ) +
                      " or allow it in chrome://inspect/#remote-debugging";
            Release();
            return false;
        }
    }
    else
    {
        if ( m_settings.browserProfileDirectory.IsNULLOrEmpty() )
            m_browser.SetProfileDir( CORE::CombinePath( m_settings.outputDirectory, "browser-profile" ) );
        // a browser of an earlier recorded session that still runs on the port is used as it is
        if ( !m_browser.Attach() && !m_browser.Launch( m_settings.browserStartUrl ) )
        {
            problem = "The browser could not be started";
            Release();
            return false;
        }
    }

    m_client = GUCEF_NEW CChromeDevToolsClient( m_pulseGenerator );
    if ( !m_browser.ConnectToBrowser( *m_client, m_settings.browserConnectTimeoutMs ) )
    {
        problem = "No connection to the browser: was the connection allowed?";
        Release();
        return false;
    }

    m_recorder = GUCEF_NEW CChromeNetworkRecorder( *m_client );
    m_recorder->SetRedactor( &m_redactor );
    m_recorder->SetBodyDirectory( m_settings.outputDirectory );
    m_recorder->AddSink( &m_journal );
    m_recorder->AddSink( &m_catalog );
    for ( size_t i=0; i<m_extraSinks.size(); ++i )
        m_recorder->AddSink( m_extraSinks[ i ] );
    m_recorder->SetNextSequence( m_journal.GetNextSequence() );
    if ( !m_recorder->Start() )
    {
        problem = "The browser did not let the recorder watch its tabs";
        Release();
        return false;
    }
    m_open = true;
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteDiscoverySession: Recording. Browse the site by hand; the journal is " << journalPath );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::IsBrowserConnected( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_client && m_client->IsConnected();
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteDiscoverySession::Process( void )
{GUCEF_TRACE;

    if ( !m_open )
        return 0;
    m_pulseGenerator->ForceDirectPulse();
    return m_recorder->Process();
}

/*-------------------------------------------------------------------------*/

void
CSiteDiscoverySession::Release( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_recorder )
    {
        m_recorder->Stop();
        GUCEF_DELETE m_recorder;
        m_recorder = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_client )
    {
        m_client->Disconnect();
        GUCEF_DELETE m_client;
        m_client = GUCEF_NULL;
    }
    if ( m_settings.closeLaunchedBrowser && m_browser.WasLaunched() )
        m_browser.Shutdown();
    m_journal.Close();
    m_pulseGenerator = CORE::PulseGeneratorPtr();
    m_open = false;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::Close( CORE::CString& problem )
{GUCEF_TRACE;

    if ( m_open )
    {
        // what was still loading
        for ( UInt32 i=0; i<20 && IsBrowserConnected(); ++i )
        {
            Process();
            MT::ThreadDelay( 10 );
        }
    }
    Release();
    return WriteReport( problem );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::WriteReport( CORE::CString& problem )
{GUCEF_TRACE;

    // which registered values the journal holds and where: a value in a large share of the lines is a public one
    if ( m_redactor.GetSecretInfoCount() > 0 )
    {
        CORE::CString journalText;
        if ( CORE::FileExists( GetJournalPath() ) && CSiteDiscoveryJournal::ReadWholeFile( GetJournalPath(), journalText ) )
            m_redactor.AssessJournal( journalText );
    }

    CORE::CString report;
    if ( !m_catalog.RenderReport( report ) )
    {
        problem = "The report was refused: the finished report would contain a secret value";
        return false;
    }
    const CORE::CString path = GetReportPath();
    if ( CORE::FileExists( path ) && !CORE::DeleteFile( path ) )
    {
        problem = "The report " + path + " cannot be replaced";
        return false;
    }
    if ( !CORE::CreateOwnerOnlyFile( path, report.C_String(), report.IsNULLOrEmpty() ? 0 : report.ByteSize() - 1, problem ) )
        return false;
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteDiscoverySession: Wrote the report of " << m_catalog.GetRequestCount() << " requests to " << path );
    const CORE::CString notice = GetRedactionNotice();
    if ( !notice.IsNULLOrEmpty() )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteDiscoverySession: " << notice );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::RebuildReportFromJournal( CORE::CString& problem )
{GUCEF_TRACE;

    if ( m_open )
    {
        problem = "The session is recording: close it first";
        return false;
    }
    if ( m_settings.outputDirectory.IsNULLOrEmpty() )
    {
        problem = "The settings name no output folder";
        return false;
    }
    const CORE::CString journalPath = GetJournalPath();
    if ( !CORE::FileExists( journalPath ) )
    {
        problem = "There is no journal at " + journalPath;
        return false;
    }
    m_catalog.Clear();
    const UInt32 loaded = m_journal.Load( journalPath, m_catalog );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteDiscoverySession: Read " << loaded << " requests from the journal " << journalPath );
    if ( 0 == loaded )
    {
        problem = "The journal " + journalPath + " holds no requests, or the json codec plugin is not loaded";
        return false;
    }
    return WriteReport( problem );
}

/*-------------------------------------------------------------------------*/

bool
CSiteDiscoverySession::ScrubJournal( CSiteDiscoveryJournal::SScrubResult& result, CORE::CString& problem )
{GUCEF_TRACE;

    result = CSiteDiscoveryJournal::SScrubResult();
    if ( m_open )
    {
        problem = "The session is recording: close it first";
        return false;
    }
    if ( m_settings.outputDirectory.IsNULLOrEmpty() )
    {
        problem = "The settings name no output folder";
        return false;
    }
    const CORE::CString journalPath = GetJournalPath();
    if ( !m_journal.Scrub( journalPath, result, problem ) )
        return false;
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteDiscoverySession: Scrubbed the journal " << journalPath << ": " << result.stubbedRequests << " request line(s) reduced, " <<
               result.droppedRequests << " dropped, " << result.stubbedActions << " action line(s) cleared, " << result.maskedRequests << " request and " <<
               result.maskedActions << " action line(s) rewritten by the value rules; the journal as it was is at " << result.backupPath );
    return RebuildReportFromJournal( problem );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteDiscoverySession::GetRedactionNotice( void ) const
{GUCEF_TRACE;

    const UInt32 parts = m_catalog.GetRedactedEntryCount();
    UInt32 suspected = 0;
    for ( UInt32 i=0; i<m_redactor.GetSecretInfoCount(); ++i )
        if ( m_redactor.GetSecretInfo( i ).suspectedPublic )
            ++suspected;
    if ( 0 == parts && 0 == m_catalog.GetStubCount() && 0 == suspected )
        return CORE::CString::Empty;

    CORE::CString notice;
    if ( parts > 0 )
    {
        notice += CORE::UInt32ToString( parts ) + " part(s) of the report were left out and replaced by a placeholder (secret classes: " + m_catalog.GetRedactionSummary() +
                  "), each because a value seen during recording in a field with a secret name would have appeared in it; the journal holds every line. The values, never shown themselves: ";
        const CSiteEndpointCatalog::TValueCountMap& byValue = m_catalog.GetRedactedValueCounts();
        for ( CSiteEndpointCatalog::TValueCountMap::const_iterator v = byValue.begin(); v != byValue.end(); ++v )
            notice += ( v == byValue.begin() ? "" : "; " ) + CORE::UInt32ToString( v->second ) + " part(s): " + m_redactor.DescribeSecretUse( v->first );
        notice += ". To see those parts: discover-report rebuilds the report from the journal without any registered value, "
                  "or add the field to ignoredSecretFields. ";
    }
    if ( suspected > 0 )
    {
        notice += CORE::UInt32ToString( suspected ) + " value(s) from fields with a secret looking name were in more than " +
                  CORE::UInt32ToString( m_settings.tripwireMaxLineShare ) + " percent of the journal lines and were taken for public values (tripwireMaxLineShare): ";
        bool first = true;
        for ( UInt32 i=0; i<m_redactor.GetSecretInfoCount(); ++i )
        {
            if ( !m_redactor.GetSecretInfo( i ).suspectedPublic )
                continue;
            notice += ( first ? "" : "; " ) + m_redactor.DescribeSecretUse( i );
            first = false;
        }
        notice += ". ";
    }
    if ( m_catalog.GetStubCount() > 0 )
        notice += CORE::UInt32ToString( m_catalog.GetStubCount() ) + " journal line(s) are stubs that an older recorder wrote in place of the line (secret classes: " + m_catalog.GetStubSummary() +
                  "): what they held was never written and cannot be recovered; record again for those requests. ";
    return notice;
}

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor& CSiteDiscoverySession::GetRedactor( void ) {GUCEF_TRACE; return m_redactor; }
CSiteDiscoveryJournal& CSiteDiscoverySession::GetJournal( void ) {GUCEF_TRACE; return m_journal; }
CSiteEndpointCatalog& CSiteDiscoverySession::GetCatalog( void ) {GUCEF_TRACE; return m_catalog; }
CDevToolsBrowser& CSiteDiscoverySession::GetBrowser( void ) {GUCEF_TRACE; return m_browser; }
CChromeNetworkRecorder* CSiteDiscoverySession::GetRecorder( void ) {GUCEF_TRACE; return m_recorder; }
CChromeDevToolsClient* CSiteDiscoverySession::GetClient( void ) {GUCEF_TRACE; return m_client; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
