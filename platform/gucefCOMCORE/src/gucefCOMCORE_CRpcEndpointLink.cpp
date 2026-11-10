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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CRpcEndpointLink.h"

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

CRpcEndpointLink::CScope::CScope( const CRpcEndpointLinkPtr& link )
    : m_link( link )
    , m_endpoint( GUCEF_NULL )
{GUCEF_TRACE;

    if ( m_link.IsNULL() )
        return;
    m_link->m_lock.Lock();
    m_endpoint = m_link->m_endpoint;
    if ( GUCEF_NULL != m_endpoint )
        ++m_link->m_users;
    m_link->m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

CRpcEndpointLink::CScope::~CScope()
{GUCEF_TRACE;

    if ( m_link.IsNULL() || GUCEF_NULL == m_endpoint )
        return;
    m_link->m_lock.Lock();
    --m_link->m_users;
    m_link->m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

CRpcEndpoint*
CRpcEndpointLink::CScope::GetEndpoint( void ) const
{GUCEF_TRACE;

    return m_endpoint;
}

/*-------------------------------------------------------------------------*/

CRpcEndpointLink::CRpcEndpointLink( CRpcEndpoint* endpoint )
    : CORE::CTSharedObjCreator< CRpcEndpointLink, MT::CMutex >( this )
    , m_lock()
    , m_endpoint( endpoint )
    , m_users( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcEndpointLink::~CRpcEndpointLink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcEndpointLinkPtr
CRpcEndpointLink::Create( CRpcEndpoint* endpoint )
{GUCEF_TRACE;

    CRpcEndpointLink* link = GUCEF_NEW CRpcEndpointLink( endpoint );
    return link->CreateSharedPtr();
}

/*-------------------------------------------------------------------------*/

void
CRpcEndpointLink::Detach( void )
{GUCEF_TRACE;

    m_lock.Lock();
    m_endpoint = GUCEF_NULL;
    while ( m_users > 0 )
    {
        m_lock.Unlock();
        MT::PrecisionDelay( 1 );
        m_lock.Lock();
    }
    m_lock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
