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

#ifndef GUCEF_COMCORE_CDNSCACHE_H
#define GUCEF_COMCORE_CDNSCACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* often used gucefCOMCORE macros */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

#ifndef GUCEF_COMCORE_CDNSCACHEENTRY_H
#include "gucefCOMCORE_CDnsCacheEntry.h"
#define GUCEF_COMCORE_CDNSCACHEENTRY_H
#endif /* GUCEF_COMCORE_CDNSCACHEENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class representing an in-proc cache of DNS addresses and their resolved IPs
 *  Helps avoid redundant DNS resolution which can be expensive plus provides an easy
 *  method of ensure the DNS provided information does not become stale via the refresh
 *  mechanisms and associated event messages.
 * 
 *  This class is threadsafe 
 */
class GUCEF_COMCORE_EXPORT_CPP CDnsCache : public CORE::CTSGNotifier          ,
                                           public CORE::CTSharedObjCreator< CDnsCache, MT::CMutex >
{
    public:

    static const CORE::CEvent DnsCacheRefreshStartedEvent;
    static const CORE::CEvent DnsCacheRefreshStoppedEvent;

    static void RegisterEvents( void );

    CDnsCache( void );

    virtual ~CDnsCache();

    CDnsCacheEntryPtr GetOrAddCacheEntryForDns( const CString& dns );

    CDnsCacheEntryPtr AddOrUpdateCacheEntryForDns( const CString& dns );

    void RemoveCacheEntryForDns( const CString& dns );

    void ClearCache( void );

    void Refresh( void );

    bool StartRefreshingASync( void );

    void SetAsyncRefreshIntervalInMs( UInt32 intervalInMs );

    UInt32 GetAsyncRefreshIntervalInMs( void ) const;
    
    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus ReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    CDnsCache( const CDnsCache& src );              /**< not implemented, not wanted */
    CDnsCache& operator=( const CDnsCache& src );   /**< not implemented, not wanted */

    typedef std::pair< const CString, CDnsCacheEntryPtr >   TDnsCacheEntryMapPair;
    typedef GUCEF::map< CString, CDnsCacheEntryPtr, std::less< CString >, gucef_allocator< TDnsCacheEntryMapPair > > TDnsCacheEntryMap;
    
    TDnsCacheEntryMap m_dnsEntryCache;
    UInt32 m_asyncRefreshIntervalInMs;
    MT::CReadWriteLock m_lock;
};

/*-------------------------------------------------------------------------*/

/**
 *  Same as regular DNS Cache but set up as a globally configurable
 *  Used internally as part of ComCoreGlobal
 * 
 *  This class is threadsafe
 */
class GUCEF_HIDDEN CGlobalDnsCache : public CDnsCache                   ,
                                     public CORE::CGloballyConfigurable 
{
    public:

    static const CORE::CString ClassTypeName;

    CGlobalDnsCache( void );

    virtual ~CGlobalDnsCache();

    /**
     *      @param cfg the config data you wish to store the config
     *      @return wheter storing all the config information to the provided tree was successfull
     */
    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      @param cfg node that is to act as root of the relevant config data
     *      @return whether loading required/mandatory settings from the given config was successfull
     */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool m_asyncUpdateCache;
};

/*-------------------------------------------------------------------------*/

typedef CDnsCache::TSharedPtrType CDnsCachePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDNSCACHE_H ? */
