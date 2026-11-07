/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CInAppMatchAlertSink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CInAppMatchAlertSink::MatchAlertAddedEvent = "GUCEF::MULTIPLAY::CInAppMatchAlertSink::MatchAlertAddedEvent";
const CORE::CEvent CInAppMatchAlertSink::MatchAlertClearedEvent = "GUCEF::MULTIPLAY::CInAppMatchAlertSink::MatchAlertClearedEvent";
const CORE::CEvent CInAppMatchAlertSink::PendingCountChangedEvent = "GUCEF::MULTIPLAY::CInAppMatchAlertSink::PendingCountChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CInAppMatchAlertSink::RegisterEvents( void )
{GUCEF_TRACE;

    MatchAlertAddedEvent.Initialize();
    MatchAlertClearedEvent.Initialize();
    PendingCountChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CInAppMatchAlertSink::CInAppMatchAlertSink( void )
    : CORE::CObservingNotifier()
    , CIMatchAlertSink()
    , m_pending()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CInAppMatchAlertSink::~CInAppMatchAlertSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CInAppMatchAlertSink::RaiseCount( void )
{GUCEF_TRACE;

    TPendingCountEventData count( (UInt32) m_pending.size() );
    NotifyObservers( PendingCountChangedEvent, &count );
}

/*-------------------------------------------------------------------------*/

void
CInAppMatchAlertSink::Deliver( const CMatchAlert& alert )
{GUCEF_TRACE;

    // one per match: a newer revision takes the place of the older one
    if ( !alert.matchId.IsNULLOrEmpty() )
    {
        for ( size_t i=0; i<m_pending.size(); ++i )
        {
            if ( m_pending[ i ].matchId == alert.matchId )
            {
                m_pending.erase( m_pending.begin() + i );
                break;
            }
        }
    }
    if ( m_pending.size() >= MaxPending )
        m_pending.erase( m_pending.begin() );
    m_pending.push_back( alert );

    CMatchAlert added( alert );
    NotifyObservers( MatchAlertAddedEvent, &added );
    RaiseCount();
}

/*-------------------------------------------------------------------------*/

void
CInAppMatchAlertSink::Clear( const CORE::CString& matchId )
{GUCEF_TRACE;

    bool removed = false;
    for ( size_t i=0; i<m_pending.size(); )
    {
        if ( m_pending[ i ].matchId == matchId )
        {
            m_pending.erase( m_pending.begin() + i );
            removed = true;
        }
        else
            ++i;
    }
    if ( removed )
    {
        TMatchAlertClearedEventData cleared( matchId );
        NotifyObservers( MatchAlertClearedEvent, &cleared );
        RaiseCount();
    }
}

/*-------------------------------------------------------------------------*/

bool
CInAppMatchAlertSink::IsInterrupting( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CInAppMatchAlertSink::GetSinkName( void ) const
{GUCEF_TRACE;

    static const CORE::CString sinkName = "in-app";
    return sinkName;
}

/*-------------------------------------------------------------------------*/

void
CInAppMatchAlertSink::GetPending( TMatchAlertVector& pending ) const
{GUCEF_TRACE;

    pending.insert( pending.end(), m_pending.begin(), m_pending.end() );
}

/*-------------------------------------------------------------------------*/

UInt32
CInAppMatchAlertSink::GetPendingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

void
CInAppMatchAlertSink::ClearAll( void )
{GUCEF_TRACE;

    if ( m_pending.empty() )
        return;
    TMatchAlertVector cleared;
    cleared.swap( m_pending );
    for ( size_t i=0; i<cleared.size(); ++i )
    {
        TMatchAlertClearedEventData data( cleared[ i ].matchId );
        NotifyObservers( MatchAlertClearedEvent, &data );
    }
    RaiseCount();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CInAppMatchAlertSink::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CInAppMatchAlertSink";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
