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

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

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
 *  The contracts a process knows (gucefCOMCORE RPC design 4): built in, loaded from files, found in the disk cache and
 *  fetched from peers. Owned by CComCoreGlobal (GetRpcContractRegistry()). Thread safe: one mutex, lookups return
 *  CRpcContractPtr copies.
 *
 *  FindLatest() and FindAtLeast() only consider BUILTIN and FILE sources: a contract fetched from a peer is used by the
 *  session that fetched it and later only when a peer names exactly its hash, so a peer cannot plant a newer minor.
 *
 *  Disk cache: <dir>/<name>/<major>.<minor>/<hash hex>.bin with exactly the schema bytes, and <hash hex>.docs.bin when
 *  the documentation was fetched. Written atomically; on load the SHA-256 must equal the name, else the file is
 *  deleted and logged. PEER and FILE contracts are cached, BUILTIN ones are not. Pruned oldest-modified first above
 *  the maximum; a hit touches the file. A missing or read-only directory degrades to fetching.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcContractRegistry : public CORE::CTSGNotifier ,
                                                      public CORE::CIConfigurable
{
    public:

    static const CORE::CEvent ContractRegisteredEvent;      /**< CRpcContractEventData */

    static void RegisterEvents( void );

    enum EContractSource
    {
        CONTRACTSOURCE_BUILTIN = 0 ,
        CONTRACTSOURCE_FILE    = 1 ,
        CONTRACTSOURCE_CACHE   = 2 ,
        CONTRACTSOURCE_PEER    = 3
    };
    typedef enum EContractSource TContractSource;

    static const UInt64 DefaultCacheMaxBytes = 64ULL * 1024 * 1024;

    CRpcContractRegistry( void );
    virtual ~CRpcContractRegistry() GUCEF_VIRTUAL_OVERRIDE;

    bool Register( const CRpcContractPtr& contract, TContractSource source );
    bool RegisterFromDataNode( const CORE::CDataNode& source, CRpcContractPtr* registered, CORE::CString& error );

    /** A file path; the json dstore codec must be available */
    bool RegisterFromJsonFile( const CORE::CString& path, CRpcContractPtr* registered, CORE::CString& error );

    /** JSON text compiled into the application; the json dstore codec must be available */
    bool RegisterFromJsonText( const CORE::CString& jsonText, CRpcContractPtr* registered, CORE::CString& error );

    bool RegisterFromSchema( const void* schema, UInt32 size, TContractSource source, CRpcContractPtr* registered,
                             CORE::CString& error, const void* docs = GUCEF_NULL, UInt32 docsSize = 0 );

    /** Memory, then the disk cache */
    CRpcContractPtr FindByHash( const CRpcContractHash& hash ) const;

    /** The highest minor of BUILTIN and FILE sources */
    CRpcContractPtr FindLatest( const CORE::CString& name, UInt32 major ) const;
    CRpcContractPtr FindAtLeast( const CORE::CString& name, UInt32 major, UInt32 minMinor ) const;
    void GetContracts( TRpcContractPtrVector& contracts ) const;
    bool GetSource( const CRpcContractHash& hash, TContractSource& source ) const;

    /** Removes a contract from memory (not from the cache); built-in contracts stay */
    bool Unregister( const CRpcContractHash& hash );

    bool StoreInCache( const CRpcContractPtr& contract, const CORE::CDynamicBuffer* docs );

    /** Default "$CACHEDIR$/gucef/rpc-contracts"; variables are resolved */
    void SetCacheDirectory( const CORE::CString& dir );
    CORE::CString GetCacheDirectory( void ) const;

    /** Default 64 MB, 0 = no maximum */
    void SetCacheMaxBytes( UInt64 maxBytes );
    UInt64 GetCacheMaxBytes( void ) const;

    /** The cache file of a contract */
    CORE::CString GetCacheFilePath( const CRpcContractPtr& contract ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SEntry
    {
        CRpcContractPtr contract;
        TContractSource source;
    };
    typedef GUCEF::map< CRpcContractHash, SEntry > TEntryMap;

    CRpcContractPtr LoadFromCache( const CRpcContractHash& hash ) const;
    void PruneCache( void ) const;
    bool RegisterLocked( const CRpcContractPtr& contract, TContractSource source, bool notify );

    CRpcContractRegistry( const CRpcContractRegistry& src );                /**< not implemented */
    CRpcContractRegistry& operator=( const CRpcContractRegistry& src );     /**< not implemented */

    mutable MT::CMutex m_lock;
    mutable TEntryMap m_entries;
    CORE::CString m_cacheDir;
    UInt64 m_cacheMaxBytes;
};

/*-------------------------------------------------------------------------*/

/** Event data of CRpcContractRegistry::ContractRegisteredEvent */
class GUCEF_COMCORE_EXPORT_CPP CRpcContractEventData : public CORE::CICloneable
{
    public:

    CRpcContractPtr contract;
    CRpcContractRegistry::TContractSource source;

    CRpcContractEventData( const CRpcContractPtr& registered, CRpcContractRegistry::TContractSource from );
    CRpcContractEventData( const CRpcContractEventData& src );
    virtual ~CRpcContractEventData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */
