/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H
#define DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTBASE_H
#include "gucefCOMCORE_CDatabaseClientBase.h"
#define GUCEF_COMCORE_CDATABASECLIENTBASE_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTBASE_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H
#include "dbpluginVFSTABLES_CVfsTablesStore.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESSTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The "vfstables" database backend (section 14 of the data tables design): tables are partition files in the VFS
 *  below settings.database, read and written through any registered table codec, with their definitions in
 *  "_tables/<name>.json" (DataNode JSON, which needs the JSON DStore codec at run time).
 *
 *  A read-write open needs a writeable mount and removes the temporary files its writer id left behind; one
 *  read-write client per root in a process (a second is refused); other processes open the root read only. Backend
 *  options: CVfsTablesSettings. The async calls run the sync calls on a pool (CDatabaseClientBase).
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CVfsTablesDatabaseClient : public COMCORE::CDatabaseClientBase
{
    public:

    static const CORE::CString BackendName;

    CVfsTablesDatabaseClient( void );

    /** Waits for the async calls that are queued or running, then closes */
    virtual ~CVfsTablesDatabaseClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Open( const COMCORE::CDatabaseClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ListTables( CORE::CString::StringSet& tableNames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;

    /** Removes the definition; with deleteData also every partition file of the table first */
    virtual CORE::TDataTableStatus DropTable( const CORE::CString& tableName, bool deleteData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The storage counters of the open database; false when it is not open */
    bool GetStorageStats( CVfsTablesStats& stats ) const;

    void ResetStorageStats( void );

    /** The store of the open database, NULL when it is not open */
    TVfsTablesStorePtr GetStore( void ) const;

    /** Clients and tables of the module that are alive, described for the log of an unload that finds them */
    static void GetLiveObjects( CORE::CString::StringVector& descriptions );

    /** Bookkeeping of the module: a client or table came or went */
    static void OnObjectCreated( const CORE::CString& description );
    static void OnObjectDestroyed( const CORE::CString& description );

    private:

    typedef GUCEF::map< TThreadId, CORE::CString > TErrorMap;

    CVfsTablesDatabaseClient( const CVfsTablesDatabaseClient& src );              /**< not implemented */
    CVfsTablesDatabaseClient& operator=( const CVfsTablesDatabaseClient& src );   /**< not implemented */

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const;
    CORE::TDataTableStatus Succeed( void ) const;
    TVfsTablesStorePtr GetOpenStore( CORE::TDataTableStatus& status ) const;
    void ReleaseStore( bool notify );
    CORE::CString GetDefinitionPath( TVfsTablesStorePtr store, const CORE::CString& tableName ) const;
    CORE::TDataTableStatus LoadDefinition( TVfsTablesStorePtr store, const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) const;
    CORE::TDataTableStatus StoreDefinition( TVfsTablesStorePtr store, const CORE::CDataTableDefinition& definition ) const;
    CORE::TDataTableStatus LookupCodec( const CORE::CString& codecType, CVfsTablesStore::TTableCodecPtr& codec ) const;
    CORE::TDataTableStatus MakeTable( TVfsTablesStorePtr store, const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table ) const;

    mutable MT::CMutex m_lock;
    TVfsTablesStorePtr m_store;
    CORE::CString m_exclusiveRoot;          /**< the root this client holds for writing, empty for a read only client */
    mutable TErrorMap m_errors;             /**< per thread */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIDatabaseClient, CVfsTablesDatabaseClient, MT::CMutex > TVfsTablesDatabaseClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H ? */
