/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CRemoteShareSessionHolds.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteShareSessionHolds::CRemoteShareSessionHolds( void )
    : m_lock()
    , m_holds()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteShareSessionHolds::~CRemoteShareSessionHolds()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRemoteShareSessionHolds::Created( const DRN::CDRNSession* session )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_holds[ session ] = 1;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareSessionHolds::Join( const DRN::CDRNSession* session )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    GUCEF::map< const DRN::CDRNSession*, UInt32 >::iterator i = m_holds.find( session );
    if ( i == m_holds.end() )
        return false;
    ++(*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteShareSessionHolds::Release( const DRN::CDRNSession* session )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    GUCEF::map< const DRN::CDRNSession*, UInt32 >::iterator i = m_holds.find( session );
    if ( i == m_holds.end() )
        return false;
    if ( (*i).second > 1 )
    {
        --(*i).second;
        return false;
    }
    m_holds.erase( i );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CRemoteShareSessionHolds::GetCount( const DRN::CDRNSession* session ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    GUCEF::map< const DRN::CDRNSession*, UInt32 >::const_iterator i = m_holds.find( session );
    return i == m_holds.end() ? 0 : (*i).second;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
