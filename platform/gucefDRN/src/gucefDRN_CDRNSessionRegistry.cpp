/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#include "gucefDRN_CDRNSessionRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNSessionRegistry::CDRNSessionRegistry( void )
    : CORE::CObserver()
    , m_lock()
    , m_sessions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSessionRegistry::~CDRNSessionRegistry()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRegistry::Register( CDRNSession& session )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        for ( UInt32 i=0; i<(UInt32) m_sessions.size(); ++i )
            if ( m_sessions[ i ].session == &session )
                return false;
        CEntry entry;
        entry.session = &session;
        entry.notifier = &session;
        m_sessions.push_back( entry );
    }

    // the session is told to the registry when it goes: only its standard events are wanted
    SubscribeTo( &session, CDRNSession::SessionClosedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNSessionRegistry::Unregister( CDRNSession& session )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        TEntryVector::iterator i = m_sessions.begin();
        while ( i != m_sessions.end() )
        {
            if ( (*i).session == &session )
            {
                m_sessions.erase( i );
                break;
            }
            ++i;
        }
    }
    UnsubscribeFrom( session );
}

/*-------------------------------------------------------------------------*/

CDRNSession*
CDRNSessionRegistry::Find( const CORE::CString& sessionName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32) m_sessions.size(); ++i )
        if ( m_sessions[ i ].session->GetName() == sessionName )
            return m_sessions[ i ].session;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNSessionRegistry::GetSessionNames( CORE::CString::StringSet& names ) const
{GUCEF_TRACE;

    names.clear();
    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<(UInt32) m_sessions.size(); ++i )
        if ( !m_sessions[ i ].session->GetName().IsNULLOrEmpty() )
            names.insert( m_sessions[ i ].session->GetName() );
}

/*-------------------------------------------------------------------------*/

void
CDRNSessionRegistry::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent != eventid )
        return;

    MT::CScopeMutex lock( m_lock );
    TEntryVector::iterator i = m_sessions.begin();
    while ( i != m_sessions.end() )
    {
        if ( (*i).notifier == notifier )
        {
            m_sessions.erase( i );
            return;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
