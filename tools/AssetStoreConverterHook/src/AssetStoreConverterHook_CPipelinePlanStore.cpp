/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#include "AssetStoreConverterHook_CPipelinePlanStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPipelinePlanStore::CPipelinePlanStore( void )
    : m_lock()
    , m_plans()
    , m_order()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPipelinePlanStore::Store( const CORE::CString& packId, const CORE::CDataNode& plan )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_order.size(); ++i )
    {
        if ( m_order[ i ] == packId )
        {
            m_order.erase( m_order.begin() + i );
            break;
        }
    }
    m_order.push_back( packId );
    m_plans[ packId ] = plan;
    while ( m_order.size() > MaxPacks )
    {
        m_plans.erase( m_order.front() );
        m_order.erase( m_order.begin() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPipelinePlanStore::Get( const CORE::CString& packId, CORE::CDataNode& plan ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TPlanMap::const_iterator i = m_plans.find( packId );
    if ( i == m_plans.end() )
        return false;
    plan = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
