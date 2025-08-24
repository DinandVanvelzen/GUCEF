/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_COMCORE_CDNSCACHEREFRESHTASKCONSUMER_H
#include "gucefCOMCORE_CDnsCacheRefreshTaskConsumer.h"
#define GUCEF_COMCORE_CDNSCACHEREFRESHTASKCONSUMER_H
#endif /* GUCEF_COMCORE_CDNSCACHEREFRESHTASKCONSUMER_H ? */

#include "gucefCOMCORE_CDnsCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_DEFAULT_DNSCACHE_ASYNC_REFRESH_INTERVAL_IN_MS     300000   // default to every 5 mins

const CORE::CEvent CDnsCache::DnsCacheRefreshStartedEvent = "GUCEF::COMCORE::CDnsCache::DnsCacheRefreshStartedEvent";
const CORE::CEvent CDnsCache::DnsCacheRefreshStoppedEvent = "GUCEF::COMCORE::CDnsCache::DnsCacheRefreshStoppedEvent";

const CORE::CString CGlobalDnsCache::ClassTypeName = "GUCEF::COMCORE::CGlobalDnsCache";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDnsCache::RegisterEvents( void )
{GUCEF_TRACE;

    DnsCacheRefreshStartedEvent.Initialize();
    DnsCacheRefreshStoppedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDnsCache::CDnsCache( void )
    : CORE::CTSGNotifier()
    , CORE::CTSharedObjCreator< CDnsCache, MT::CMutex >( this )
    , m_dnsEntryCache()
    , m_asyncRefreshIntervalInMs( GUCEF_DEFAULT_DNSCACHE_ASYNC_REFRESH_INTERVAL_IN_MS )
    , m_lock( true )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDnsCache::~CDnsCache()
{GUCEF_TRACE;

    ClearCache();
}

/*-------------------------------------------------------------------------*/

void
CDnsCache::SetAsyncRefreshIntervalInMs( UInt32 intervalInMs )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );
    m_asyncRefreshIntervalInMs = intervalInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CDnsCache::GetAsyncRefreshIntervalInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readLock( m_lock );
    return m_asyncRefreshIntervalInMs;
}

/*-------------------------------------------------------------------------*/

void
CDnsCache::ClearCache( void )
{GUCEF_TRACE;

    MT::CScopeWriterLock lock( m_lock );
    m_dnsEntryCache.clear();
}

/*-------------------------------------------------------------------------*/

CDnsCacheEntryPtr
CDnsCache::GetOrAddCacheEntryForDns( const CString& dns )
{GUCEF_TRACE;

    MT::CScopeReaderLock readLock( m_lock );

    TDnsCacheEntryMap::iterator i = m_dnsEntryCache.find( dns );
    if ( i != m_dnsEntryCache.end() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "DnsCache:GetOrAddCacheEntryForDns: Found cache entry in dns cache for " + dns );
        return (*i).second;
    }

    MT::CScopeWriterLock writeLock( readLock );

    CDnsCacheEntryPtr dnsEntry = ( GUCEF_NEW CDnsCacheEntry() )->CreateSharedPtr();
    dnsEntry->SetPulseGenerator( GetPulseGenerator() );
    if ( dnsEntry->SetDnsNameAndResolve( dns ) )
    {
        m_dnsEntryCache.insert( TDnsCacheEntryMap::value_type( dns, dnsEntry ) );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "DnsCache:GetOrAddCacheEntryForDns: Added entry to dns cache for " + dns );
        return dnsEntry;
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DnsCache:GetOrAddCacheEntryForDns: Failed to resolve entry dns for " + dns );
    }

    return CDnsCacheEntryPtr();
}

/*-------------------------------------------------------------------------*/

CDnsCacheEntryPtr
CDnsCache::AddOrUpdateCacheEntryForDns( const CString& dns )
{GUCEF_TRACE;

    MT::CScopeWriterLock writeLock( m_lock );
    
    CDnsCacheEntryPtr dnsEntry = ( GUCEF_NEW CDnsCacheEntry() )->CreateSharedPtr();
    dnsEntry->SetPulseGenerator( GetPulseGenerator() );
    if ( dnsEntry->SetDnsNameAndResolve( dns ) )
    {
        m_dnsEntryCache.insert( TDnsCacheEntryMap::value_type( dns, dnsEntry ) );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "DnsCache:AddOrUpdateCacheEntryForDns: Added entry to dns cache for " + dns );
        return dnsEntry;
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DnsCache:AddOrUpdateCacheEntryForDns: Failed to resolve entry dns for " + dns );
    }

    return CDnsCacheEntryPtr();
}

/*-------------------------------------------------------------------------*/

void
CDnsCache::RemoveCacheEntryForDns( const CString& dns )
{GUCEF_TRACE;

    MT::CScopeWriterLock writeLock( m_lock );

    TDnsCacheEntryMap::iterator i = m_dnsEntryCache.find( dns );
    if ( i != m_dnsEntryCache.end() )
    {
        m_dnsEntryCache.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

void
CDnsCache::Refresh( void )
{GUCEF_TRACE;

    // we take a read lock since we only need to gaurd against the cache index changing
    // the entries themselves have their own lock
    MT::CScopeReaderLock readLock( m_lock );

    if ( !NotifyObservers( DnsCacheRefreshStartedEvent ) ) return;

    TDnsCacheEntryMap::iterator i = m_dnsEntryCache.begin();
    while ( i != m_dnsEntryCache.end() )
    {
        CDnsCacheEntryPtr& dnsEntry = (*i).second;
        dnsEntry->Refresh();
        ++i;
    }

    NotifyObservers( DnsCacheRefreshStoppedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CDnsCache::StartRefreshingASync( void )
{GUCEF_TRACE;

    CDnsCachePtr thisPtr( CreateSharedPtr() );
    CORE::CFutureResult result = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool()->StartTaskIfNoneExists( CDnsCacheRefreshTaskConsumer::TaskType ,
                                                                                                                         &thisPtr                               );
    return result.HasAFuture();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CDnsCache::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.WriterStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CDnsCache::Unlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.WriterStop() );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CDnsCache::ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.ReaderStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CDnsCache::ReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_lock.ReaderStop() );
}

/*-------------------------------------------------------------------------*/

CGlobalDnsCache::CGlobalDnsCache( void )
    : CDnsCache()
    , CORE::CGloballyConfigurable()
    , m_asyncUpdateCache( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGlobalDnsCache::~CGlobalDnsCache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGlobalDnsCache::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    CORE::CDataNode* cfgNode = cfg.FindOrAddChild( "GlobalDnsCache" );
    if ( GUCEF_NULL != cfgNode )
    {
        bool totalSuccess = true;
        
        totalSuccess = cfgNode->SetAttribute( "asyncRefreshIntervalInMs", m_asyncRefreshIntervalInMs ) && totalSuccess;
        totalSuccess = cfgNode->SetAttribute( "asyncUpdateCache", m_asyncUpdateCache ) && totalSuccess;
        
        CORE::CDataNode* dnsEntriesNode = cfgNode->FindOrAddChild( "DnsEntries", GUCEF_DATATYPE_ARRAY );
        if ( GUCEF_NULL != dnsEntriesNode )
        {
            dnsEntriesNode->DelSubTree();
            
            TDnsCacheEntryMap::const_iterator i = m_dnsEntryCache.begin();
            while ( i != m_dnsEntryCache.end() )
            {
                CORE::CDataNode* dnsEntryNode = cfgNode->AddChild( "DnsEntry", GUCEF_DATATYPE_OBJECT );
                if ( GUCEF_NULL != dnsEntryNode )
                    totalSuccess = dnsEntryNode->SetAttribute( "dnsName", (*i).first ) && totalSuccess;
                ++i;
            }
        }

        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGlobalDnsCache::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    const CORE::CDataNode* cfgNode = cfg.FindChild( "GlobalDnsCache" );
    if ( GUCEF_NULL != cfgNode )
    {
        MT::CScopeWriterLock writeLock( m_lock );

        m_asyncRefreshIntervalInMs = cfgNode->GetAttributeValueOrChildValueByName( "asyncRefreshIntervalInMs" ).AsUInt32( m_asyncRefreshIntervalInMs, true );
        m_asyncUpdateCache = cfgNode->GetAttributeValueOrChildValueByName( "asyncUpdateCache" ).AsBool( m_asyncUpdateCache, true );
                
        const CORE::CDataNode* dnsEntriesNode = cfgNode->FindChild( "DnsEntries" );
        if ( GUCEF_NULL != dnsEntriesNode )
        {
            CORE::CDataNode::const_iterator i = dnsEntriesNode->ConstBegin();
            while ( i != dnsEntriesNode->ConstEnd() )
            {
                const CORE::CDataNode* dnsEntryNode = (*i);
                if ( GUCEF_NULL != dnsEntryNode )
                {
                    CORE::CString dnsName = dnsEntryNode->GetAttributeValueOrChildValueByName( "dnsName" ).AsString( CORE::CString::Empty, true );
                    AddOrUpdateCacheEntryForDns( dnsName );
                }
                ++i;
            }
        }
    }

    if ( m_asyncUpdateCache )
        StartRefreshingASync();

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGlobalDnsCache::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
