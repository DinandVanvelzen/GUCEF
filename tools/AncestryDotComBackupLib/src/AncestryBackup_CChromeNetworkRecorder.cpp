/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CURLTEMPLATE_H
#include "AncestryBackup_CUrlTemplate.h"
#define ANCESTRYBACKUP_CURLTEMPLATE_H
#endif /* ANCESTRYBACKUP_CURLTEMPLATE_H ? */

#include "AncestryBackup_CChromeNetworkRecorder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static WEB::CSiteDiscoverySettings
MakeRecorderSettings( bool recordDna )
{
    WEB::CSiteDiscoverySettings settings = CEndpointCatalog::MakeSettings( recordDna );
    settings.saveResponseBodies = true;
    settings.maxResponseBodyBytes = 32ULL * 1024ULL * 1024ULL;
    settings.disableBrowserCache = true;        // a response that comes from the cache is a response we may not see being requested
    return settings;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CChromeNetworkRecorder::CChromeNetworkRecorder( WEB::CChromeDevToolsClient& client ,
                                                const CDataLayout& layout          ,
                                                CEndpointCatalog& catalog          ,
                                                CDiscoveryJournal& journal         )
    : WEB::CIWebObservationSink()
    , m_catalog( catalog )
    , m_journal( journal )
    , m_settings( MakeRecorderSettings( catalog.IsRecordDna() ) )
    , m_redactor( m_settings )
    , m_recorder( client )
    , m_newEndpoints()
{GUCEF_TRACE;

    m_recorder.SetRedactor( &m_redactor );
    m_recorder.SetBodyDirectory( layout.GetDiscoveryDir() );
    m_recorder.AddSink( this );

    // what the recorder sees as secret is known to what writes the journal and the report
    m_catalog.SetSharedRedactor( &m_redactor );
    m_journal.SetSharedRedactor( &m_redactor );
}

/*-------------------------------------------------------------------------*/

CChromeNetworkRecorder::~CChromeNetworkRecorder()
{GUCEF_TRACE;

    Stop();
    m_recorder.RemoveSink( this );
    m_catalog.SetSharedRedactor( GUCEF_NULL );
    m_journal.SetSharedRedactor( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::SetHostFilter( const std::vector< std::string >& terms )
{GUCEF_TRACE;

    m_settings.hostFilter.clear();
    for ( size_t i=0; i<terms.size(); ++i )
        if ( !terms[ i ].empty() )
            m_settings.hostFilter.push_back( CORE::CString( terms[ i ] ).Lowercase() );
    m_redactor.SetSettings( m_settings );
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::SetMaxBodyBytes( UInt64 maxBodyBytes )
{GUCEF_TRACE;

    m_settings.maxResponseBodyBytes = maxBodyBytes;
    m_redactor.SetSettings( m_settings );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::MatchesFilter( const std::vector< std::string >& terms, const std::string& host )
{GUCEF_TRACE;

    CORE::CString::StringVector converted;
    for ( size_t i=0; i<terms.size(); ++i )
        converted.push_back( terms[ i ] );
    return WEB::CSiteDiscoveryRedactor::MatchesHostFilter( converted, host );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::Start( void )
{GUCEF_TRACE;

    m_recorder.SetNextSequence( m_journal.GetNextSequence() );
    return m_recorder.Start();
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::Stop( void )
{GUCEF_TRACE;

    m_recorder.Stop();
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::IsStarted( void ) const
{GUCEF_TRACE;

    return m_recorder.IsStarted();
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::Process( void )
{GUCEF_TRACE;

    return m_recorder.Process();
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::OnWebRequestObserved( const WEB::CObservedWebRequest& observation )
{GUCEF_TRACE;

    m_journal.AppendObservation( observation );
    if ( m_catalog.AddObservation( observation ) )
    {
        SUrlTemplate parsed;
        CUrlTemplate::Parse( observation.url, parsed );
        m_newEndpoints.push_back( ( observation.method + " " + parsed.GetKey() + " (" + observation.resourceType + ")" ).STL_String() );
    }
}

/*-------------------------------------------------------------------------*/

void
CChromeNetworkRecorder::OnUserActionObserved( const WEB::CObservedUserAction& action )
{GUCEF_TRACE;

    m_journal.AppendAction( action );
    m_catalog.OnUserActionObserved( action );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::WantsResponseShape( const WEB::CObservedWebRequest& observation ) const
{GUCEF_TRACE;

    return m_catalog.WantsResponseShape( observation );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::IsNewResponseShape( const WEB::CObservedWebRequest& observation, const WEB::CWebBodyShape& shape ) const
{GUCEF_TRACE;

    return m_catalog.IsNewResponseShape( observation, shape );
}

/*-------------------------------------------------------------------------*/

bool
CChromeNetworkRecorder::PopNewEndpoint( std::string& description )
{GUCEF_TRACE;

    if ( m_newEndpoints.empty() )
        return false;
    description = m_newEndpoints.front();
    m_newEndpoints.erase( m_newEndpoints.begin() );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetRecordedRequestCount( void ) const
{GUCEF_TRACE;

    return m_recorder.GetRecordedRequestCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetSavedBodyCount( void ) const
{GUCEF_TRACE;

    return m_recorder.GetSavedBodyCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CChromeNetworkRecorder::GetSessionCount( void ) const
{GUCEF_TRACE;

    return m_recorder.GetSessionCount();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
