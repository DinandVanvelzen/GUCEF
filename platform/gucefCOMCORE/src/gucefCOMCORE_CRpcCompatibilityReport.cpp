/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#include "gucefCOMCORE_CRpcCompatibilityReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcCompatibilityReport::SEntry::SEntry( void )
    : path()
    , ruleId()
    , message()
    , isBreaking( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcCompatibilityReport::CRpcCompatibilityReport( void )
    : m_entries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRpcCompatibilityReport::Add( const CORE::CString& path, const CORE::CString& ruleId, const CORE::CString& message, bool isBreaking )
{GUCEF_TRACE;

    SEntry entry;
    entry.path = path;
    entry.ruleId = ruleId;
    entry.message = message;
    entry.isBreaking = isBreaking;
    m_entries.push_back( entry );
}

/*-------------------------------------------------------------------------*/

void
CRpcCompatibilityReport::Clear( void )
{GUCEF_TRACE;

    m_entries.clear();
}

/*-------------------------------------------------------------------------*/

const CRpcCompatibilityReport::TEntryVector&
CRpcCompatibilityReport::GetEntries( void ) const
{GUCEF_TRACE;

    return m_entries;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcCompatibilityReport::GetBreakingCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].isBreaking )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CRpcCompatibilityReport::HasRule( const CORE::CString& ruleId ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].ruleId == ruleId )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcCompatibilityReport::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    for ( UInt32 i=0; i<m_entries.size(); ++i )
    {
        const SEntry& e = m_entries[ i ];
        text += ( e.isBreaking ? "BREAKING " : "ok " ) + e.ruleId + " " + e.path + ": " + e.message + "\n";
    }
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
