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

#include "AncestryBackup_CDiscoveryJournal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDiscoveryJournal::CDiscoveryJournal( void )
    : m_ownRedactor( CEndpointCatalog::MakeSettings( true ) )
    , m_journal()
{GUCEF_TRACE;

    // the recorder applies the DNA rule as it records; what reaches the journal directly is written as it is given
    m_journal.SetRedactor( &m_ownRedactor );
}

/*-------------------------------------------------------------------------*/

CDiscoveryJournal::~CDiscoveryJournal()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryJournal::GetNextSequenceOf( const CORE::CString& path )
{GUCEF_TRACE;

    return WEB::CSiteDiscoveryJournal::GetNextSequenceOf( path );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::Open( const CDataLayout& layout )
{GUCEF_TRACE;

    return m_journal.Open( layout.GetDiscoveryJournalPath() );
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryJournal::Close( void )
{GUCEF_TRACE;

    m_journal.Close();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::IsOpen( void ) const
{GUCEF_TRACE;

    return m_journal.IsOpen();
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryJournal::GetNextSequence( void ) const
{GUCEF_TRACE;

    return m_journal.GetNextSequence();
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryJournal::SetSharedRedactor( WEB::CSiteDiscoveryRedactor* redactor )
{GUCEF_TRACE;

    m_journal.SetRedactor( GUCEF_NULL != redactor ? redactor : &m_ownRedactor );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::Append( const SObservedRequest& observation )
{GUCEF_TRACE;

    return m_journal.Append( CEndpointCatalog::ToObservation( observation, GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::AppendObservation( const WEB::CObservedWebRequest& observation )
{GUCEF_TRACE;

    return m_journal.Append( observation );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::AppendAction( const WEB::CObservedUserAction& action )
{GUCEF_TRACE;

    return m_journal.AppendAction( action );
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryJournal::Load( const CDataLayout& layout, CEndpointCatalog& catalog, UInt32* nextSequence )
{GUCEF_TRACE;

    // read with the rules of the catalog, so the shapes of saved responses are described the way it wants them
    WEB::CSiteDiscoveryJournal reader;
    reader.SetRedactor( &catalog.GetRedactor() );
    return reader.Load( layout.GetDiscoveryJournalPath(), catalog, nextSequence );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
