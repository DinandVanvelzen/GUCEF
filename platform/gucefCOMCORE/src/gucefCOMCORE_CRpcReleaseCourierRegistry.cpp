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

#include "gucefCOMCORE_CRpcReleaseCourierRegistry.h"

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

CRpcReleaseCourierRegistry::CRpcReleaseCourierRegistry( void )
    : m_lock()
    , m_couriers()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcReleaseCourierRegistry::~CRpcReleaseCourierRegistry()
{GUCEF_TRACE;

    TCourierMap couriers;
    {
        MT::CScopeMutex lock( m_lock );
        couriers.swap( m_couriers );
    }
    couriers.clear();
}

/*-------------------------------------------------------------------------*/

CRpcReleaseCourierPtr
CRpcReleaseCourierRegistry::GetCourier( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    if ( pulseGenerator.IsNULL() )
        return CRpcReleaseCourierPtr();

    // couriers nobody else uses and that hold nothing go, outside the lock (they unsubscribe from their pulse generator)
    GUCEF::vector< CRpcReleaseCourierPtr > idle;
    CRpcReleaseCourierPtr courier;
    {
        MT::CScopeMutex lock( m_lock );
        TCourierMap::iterator i = m_couriers.begin();
        while ( i != m_couriers.end() )
        {
            if ( (*i).first != pulseGenerator.GetPointerAlways() && 1 == (*i).second.GetReferenceCount() && (*i).second->IsIdle() )
            {
                idle.push_back( (*i).second );
                m_couriers.erase( i++ );
            }
            else
            {
                ++i;
            }
        }
        TCourierMap::iterator c = m_couriers.find( pulseGenerator.GetPointerAlways() );
        if ( c != m_couriers.end() )
            courier = (*c).second;
    }
    idle.clear();
    if ( !courier.IsNULL() )
        return courier;

    // made outside the lock: a new courier subscribes to the pulse generator, whose lock a thread in its pulse holds
    // while it may make an endpoint (and ask for a courier here)
    CRpcReleaseCourierPtr made( GUCEF_NEW CRpcReleaseCourier( pulseGenerator ) );
    {
        MT::CScopeMutex lock( m_lock );
        TCourierMap::iterator c = m_couriers.find( pulseGenerator.GetPointerAlways() );
        if ( c != m_couriers.end() )
        {
            courier = (*c).second;
        }
        else
        {
            m_couriers[ pulseGenerator.GetPointerAlways() ] = made;
            courier = made;
        }
    }
    // another thread made one meanwhile: this one goes, outside the lock
    made.Unlink();
    return courier;
}

/*-------------------------------------------------------------------------*/

UInt32
CRpcReleaseCourierRegistry::GetCourierCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_couriers.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
