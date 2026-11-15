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

#include "gucefCOMCORE_CServiceCandidate.h"

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

CServiceCandidate::CServiceCandidate( void )
    : announcement()
    , host()
    , lastSeen()
    , sourceClientNames()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceCandidate::CServiceCandidate( const CServiceCandidate& src )
    : announcement( src.announcement )
    , host( src.host )
    , lastSeen( src.lastSeen )
    , sourceClientNames( src.sourceClientNames )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceCandidate::~CServiceCandidate()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceCandidate&
CServiceCandidate::operator=( const CServiceCandidate& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        announcement = src.announcement;
        host = src.host;
        lastSeen = src.lastSeen;
        sourceClientNames = src.sourceClientNames;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CServiceCandidate::FromEntry( const CIDiscoveryEntry& entry )
{GUCEF_TRACE;

    if ( !announcement.FromEntry( entry ) )
        return false;
    host = CHostAddress();
    entry.GetHost( host );
    lastSeen = entry.GetLastSeen();
    sourceClientNames.clear();
    if ( !entry.GetSourceClientName().IsNULLOrEmpty() )
        sourceClientNames.insert( entry.GetSourceClientName() );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceCandidate::GetKey( void ) const
{GUCEF_TRACE;

    return announcement.GetKey();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceCandidate::GetFirstEndpoint( void ) const
{GUCEF_TRACE;

    return announcement.endpoints.empty() ? CORE::CString::Empty : announcement.endpoints[ 0 ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
