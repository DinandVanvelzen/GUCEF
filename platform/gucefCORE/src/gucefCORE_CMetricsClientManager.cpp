/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_CMetricsClientManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMetricsClientManager::AddMetricsClient( const CString& name, CIMetricsSystemClientPtr client )
{GUCEF_TRACE;

    MT::CScopeWriterLock writerLock( m_rwDataLock );
    m_clients[ name ] = client;
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::RemoveMetricsClient( CIMetricsSystemClientPtr client )
{GUCEF_TRACE;

    if ( client.IsNULL() )
        return;
    MT::CScopeWriterLock writerLock( m_rwDataLock );
    m_clients.erase( client->GetName() );
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::RemoveMetricsClient( const CString& name )
{GUCEF_TRACE;

    MT::CScopeWriterLock writerLock( m_rwDataLock );
    m_clients.erase( name );
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::ClearMetricsClients( void )
{GUCEF_TRACE;

    m_clients.clear();
}

/*-------------------------------------------------------------------------*/

CMetricsClientManager::CMetricsClientManager( void )
    : CIMetricsSystemClient()
    , m_clients()
    , m_rwDataLock( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMetricsClientManager::~CMetricsClientManager()
{GUCEF_TRACE;

    MT::CScopeWriterLock writerLock( m_rwDataLock );
    ClearMetricsClients();
}

/*-------------------------------------------------------------------------*/

bool
CMetricsClientManager::Open( void )
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    bool totalSuccess = true;
    TMetricsSystemClientPtrMap::iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        CIMetricsSystemClientPtr client = (*i).second;
        totalSuccess &= client->Open();
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CMetricsClientManager::Close( void )
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    bool totalSuccess = true;
    TMetricsSystemClientPtrMap::iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        CIMetricsSystemClientPtr client = (*i).second;
        totalSuccess &= client->Close();
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CMetricsClientManager::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    bool totalSuccess = true;
    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        totalSuccess &= (*i).second->SaveConfig( tree );
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CMetricsClientManager::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    bool totalSuccess = true;
    TMetricsSystemClientPtrMap::iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        CIMetricsSystemClientPtr client = (*i).second;
        totalSuccess &= client->LoadConfig( treeroot );
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMetricsClientManager::GetName( void ) const
{GUCEF_TRACE;

    return GetClassTypeName();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMetricsClientManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::CORE::CMetricsClientManager";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Increment( const CString& key, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Increment( key, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Decrement( const CString& key, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Decrement( key, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Count( const CString& key, const Int32 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Count( key, delta, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Count( const CString& key, const Int64 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Count( key, delta, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Count( const CString& key, const UInt64 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Count( key, delta, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Count( const CString& key, const UInt32 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Count( key, delta, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Count( const CString& key, const CVariant& delta, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Count( key, delta, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Gauge( const CString& key, const Int32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Gauge( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Gauge( const CString& key, const UInt32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Gauge( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Gauge( const CString& key, const Int64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Gauge( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Gauge( const CString& key, const UInt64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Gauge( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Gauge( const CString& key, const Float32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Gauge( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Gauge( const CString& key, const Float64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Gauge( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Timing( const CString& key, const UInt32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Timing( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Timing( const CString& key, const UInt64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Timing( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Timing( const CString& key, const Float32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Timing( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CMetricsClientManager::Timing( const CString& key, const Float64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readerLock( m_rwDataLock );

    TMetricsSystemClientPtrMap::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        (*i).second->Timing( key, value, frequency );
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
