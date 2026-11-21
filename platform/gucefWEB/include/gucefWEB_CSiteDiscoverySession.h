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

#ifndef GUCEF_WEB_CSITEDISCOVERYSESSION_H
#define GUCEF_WEB_CSITEDISCOVERYSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#include "gucefWEB_CSiteDiscoverySettings.h"
#define GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#endif /* GUCEF_WEB_CSITEDISCOVERYSETTINGS_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#include "gucefWEB_CSiteDiscoveryRedactor.h"
#define GUCEF_WEB_CSITEDISCOVERYREDACTOR_H
#endif /* GUCEF_WEB_CSITEDISCOVERYREDACTOR_H ? */

#ifndef GUCEF_WEB_CSITEDISCOVERYJOURNAL_H
#include "gucefWEB_CSiteDiscoveryJournal.h"
#define GUCEF_WEB_CSITEDISCOVERYJOURNAL_H
#endif /* GUCEF_WEB_CSITEDISCOVERYJOURNAL_H ? */

#ifndef GUCEF_WEB_CSITEENDPOINTCATALOG_H
#include "gucefWEB_CSiteEndpointCatalog.h"
#define GUCEF_WEB_CSITEENDPOINTCATALOG_H
#endif /* GUCEF_WEB_CSITEENDPOINTCATALOG_H ? */

#ifndef GUCEF_WEB_CDEVTOOLSBROWSER_H
#include "gucefWEB_CDevToolsBrowser.h"
#define GUCEF_WEB_CDEVTOOLSBROWSER_H
#endif /* GUCEF_WEB_CDEVTOOLSBROWSER_H ? */

#ifndef GUCEF_WEB_CCHROMENETWORKRECORDER_H
#include "gucefWEB_CChromeNetworkRecorder.h"
#define GUCEF_WEB_CCHROMENETWORKRECORDER_H
#endif /* GUCEF_WEB_CCHROMENETWORKRECORDER_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A recorded browsing session, ready made: the user browses a site by hand in Chrome while this records which
 *  endpoints the site uses, into a journal and an endpoint report in the output folder (see CSiteDiscoverySettings).
 *
 *      CSiteDiscoverySettings settings;
 *      settings.LoadConfig( config );                  // or set the members
 *      CSiteDiscoverySession session;
 *      session.SetSettings( settings );
 *      CORE::CString problem;
 *      if ( !session.Open( problem ) ) ...
 *      while ( !done && session.IsBrowserConnected() )
 *      {
 *          session.Process();
 *          MT::ThreadDelay( 10 );
 *      }
 *      session.Close( problem );                       // writes the report
 *
 *  Run modes: attach to a browser the user runs with remote debugging allowed, or launch one with a profile folder of its
 *  own (recorded session). In both the user signs in in the browser, never in the program, and the recorder only listens.
 *
 *  Threading: the session makes a pulse generator of its own for its DevTools connection and declares the thread that
 *  calls Open() its driver. Call every member on that thread; Process() pumps that pulse generator only, so the session
 *  never pumps a pulse generator of another thread (or use SetPulseGenerator()). For asynchronous use run the whole
 *  session in a pool task.
 */
class GUCEF_WEB_PUBLIC_CPP CSiteDiscoverySession
{
    public:

    CSiteDiscoverySession( void );

    ~CSiteDiscoverySession();

    /** Before Open() */
    void SetSettings( const CSiteDiscoverySettings& settings );

    /**
     *  Before Open(): run the DevTools connection on this pulse generator instead of one of the session's own, for a
     *  program whose thread already drives one (its application pulse generator). Open() and every other member must then
     *  be called on the thread that drives it
     */
    void SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator );

    const CSiteDiscoverySettings& GetSettings( void ) const;

    /** Reads the journal of earlier runs (continueJournal), opens the journal, finds or starts the browser, connects and starts recording */
    bool Open( CORE::CString& problem );

    bool IsOpen( void ) const;

    bool IsBrowserConnected( void ) const;

    /** Pumps the session's pulse generator once and lets the recorder do its queued work. @return the things done */
    UInt32 Process( void );

    /** Stops recording, disconnects, closes the journal and writes the report. A launched browser is closed with closeLaunchedBrowser only */
    bool Close( CORE::CString& problem );

    /**
     *  Writes the report of what the catalog holds now, owner-only. A part of the report that would hold a secret value is
     *  left out with a placeholder and the rest is written (see CSiteEndpointCatalog::RenderReport()); the number of
     *  parts left out and the secret classes are logged and given by GetRedactionNotice(). False when the report could not
     *  be made or written
     */
    bool WriteReport( CORE::CString& problem );

    /**
     *  Rebuilds the report from the journal of the output folder, without a browser and without a network: reads
     *  the journal into a fresh catalog and writes the report as WriteReport() does. The journal only holds what the
     *  recorder already redacted, a journal line that was reduced to a stub is counted and not catalogued. The journal is
     *  only read. Needs the json codec plugin. False, with the problem, when the session is recording, there is no
     *  journal, or the report could not be written
     */
    bool RebuildReportFromJournal( CORE::CString& problem );

    /**
     *  Applies the never-record rules of the settings to the journal of the output folder (see CSiteDiscoveryJournal::Scrub(),
     *  which first copies the journal to a backup) and rebuilds the report from the scrubbed journal. No browser, no
     *  network. False, with the problem, when the session is recording, there is no journal, the backup or the rewrite
     *  failed; the result then holds what was done up to there. Needs the json codec plugin
     */
    bool ScrubJournal( CSiteDiscoveryJournal::SScrubResult& result, CORE::CString& problem );

    /**
     *  What the last WriteReport() and the journal left out because it would have held a secret value, one sentence:
     *  the numbers and the secret classes, never a value; empty when nothing was left out
     */
    CORE::CString GetRedactionNotice( void ) const;

    CORE::CString GetJournalPath( void ) const;

    CORE::CString GetReportPath( void ) const;

    /** Adds a sink of the program's own, which then also sees the requests and actions */
    void AddSink( CIWebObservationSink* sink );

    CSiteDiscoveryRedactor& GetRedactor( void );
    CSiteDiscoveryJournal& GetJournal( void );
    CSiteEndpointCatalog& GetCatalog( void );
    CDevToolsBrowser& GetBrowser( void );

    /** GUCEF_NULL while the session is not open */
    CChromeNetworkRecorder* GetRecorder( void );

    /** GUCEF_NULL while the session is not open */
    CChromeDevToolsClient* GetClient( void );

    private:

    CSiteDiscoverySession( const CSiteDiscoverySession& src );              /**< not implemented */
    CSiteDiscoverySession& operator=( const CSiteDiscoverySession& src );   /**< not implemented */

    void Release( void );

    typedef GUCEF::vector< CIWebObservationSink* > TSinkVector;

    CSiteDiscoverySettings m_settings;
    CSiteDiscoveryRedactor m_redactor;
    CSiteDiscoveryJournal m_journal;
    CSiteEndpointCatalog m_catalog;
    CDevToolsBrowser m_browser;
    CORE::PulseGeneratorPtr m_givenPulseGenerator;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CChromeDevToolsClient* m_client;
    CChromeNetworkRecorder* m_recorder;
    TSinkVector m_extraSinks;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CSITEDISCOVERYSESSION_H ? */
