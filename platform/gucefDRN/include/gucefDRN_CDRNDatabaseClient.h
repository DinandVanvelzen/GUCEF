/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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


#ifndef GUCEF_DRN_CDRNDATABASECLIENT_H
#define GUCEF_DRN_CDRNDATABASECLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTBASE_H
#include "gucefCOMCORE_CDatabaseClientBase.h"
#define GUCEF_COMCORE_CDATABASECLIENTBASE_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTBASE_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H
#include "gucefDRN_CDRNDataGridTableSettings.h"
#define GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H
#endif /* GUCEF_DRN_CDRNDATAGRIDTABLESETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;

/**
 *  The "drn" database backend: the data grids of a session as the tables of a database, so that any code that works
 *  with CIDatabaseClient (browsers, exports, tools) reaches replicated state without DRN specific code. An optional
 *  way in: the grids and their classes are used as before, with or without a client.
 *
 *  The client is bound to a session: either by name, settings.database naming a session that the application
 *  registered with CDRNSessionRegistry (CDRNGlobal::GetSessionRegistry()), or by Attach() for code that holds the
 *  session. The tables are the grids of the session, those it published and those announced to it. OpenTable()
 *  subscribes to a grid that is announced and not yet subscribed, and returns when its snapshot has arrived:
 *  OpenTable() waits on the calling thread for that without pumping a pulse generator, so the node must be driven by
 *  another thread (ownThread, or a thread pool: OpenTableAsync()); OpenTableAsync() is for the thread that drives the node.
 *  Grids that already hold their data open at once.
 *
 *  CreateTable() publishes a grid of this member: the first two columns of the definition are key and value and give
 *  the declared types, the backend options "authority", "reliable", "sendRateHz", "capacity", "notify" and "priority"
 *  give the grid settings. DropTable() withdraws a grid this member published.
 *
 *  Backend options of the settings (defaults for the tables opened): "openTimeoutMs" (wait of OpenTable for a
 *  snapshot, default 10000, 0 waits without end), "scanChunkKeys", "includeRemoved", "snapshotCacheMs", "writeTimeoutMs".
 *  The declared types of a replicated grid are not sent by the publisher: SetTableSettings() or the options
 *  "<table>.keyType" and "<table>.valueType" name them. Capabilities: write (unless opened read only), async and
 *  predicate push down; no partitions.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDatabaseClient : public COMCORE::CDatabaseClientBase
{
    public:

    static const CORE::CString BackendName;

    static const UInt32 DefaultOpenTimeoutMs = 10000;

    CDRNDatabaseClient( void );

    /** Waits for the async calls that are queued or running, then closes */
    virtual ~CDRNDatabaseClient() GUCEF_VIRTUAL_OVERRIDE;

    /** Binds the client to a session held by the caller; the settings given to Open() then need not name it */
    bool Attach( CDRNSession& session );

    /** The options of the tables this client opens for the grid of the name; replaces the defaults of the backend options */
    void SetTableSettings( const CORE::CString& tableName, const CDRNDataGridTableSettings& settings );

    virtual bool Open( const COMCORE::CDatabaseClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ListTables( CORE::CString::StringSet& tableNames ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;

    /** Withdraws a grid this member published; the data is the grid itself, deleteData changes nothing */
    virtual CORE::TDataTableStatus DropTable( const CORE::CString& tableName, bool deleteData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CSessionWatcher : public CORE::CObserver
    {
        public:
        CDRNDatabaseClient* m_client;
        CORE::CNotifier* m_session;
        CSessionWatcher( void ) : CORE::CObserver(), m_client( GUCEF_NULL ), m_session( GUCEF_NULL ) {}
        virtual ~CSessionWatcher() GUCEF_VIRTUAL_OVERRIDE { SignalUpcomingObserverDestruction(); }
        protected:
        virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;
    };
    friend class CSessionWatcher;

    typedef GUCEF::map< UInt32, CORE::CString >                         TErrorMap;
    typedef GUCEF::map< CORE::CString, CDRNDataGridTableSettings >      TTableSettingsMap;

    CDRNDatabaseClient( const CDRNDatabaseClient& src );                /**< not implemented */
    CDRNDatabaseClient& operator=( const CDRNDatabaseClient& src );     /**< not implemented */

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const;
    CORE::TDataTableStatus Succeed( void ) const;
    CDRNSession* GetOpenSession( CORE::TDataTableStatus& status ) const;
    CDRNDataGridTableSettings GetTableSettings( const CORE::CString& tableName ) const;
    void ReleaseSession( bool notify );
    void OnSessionDestroyed( void );

    mutable MT::CMutex m_lock;
    CDRNSession* m_session;
    bool m_attached;                        /**< the session was given with Attach() */
    CSessionWatcher m_sessionWatcher;
    TTableSettingsMap m_tableSettings;      /**< set by SetTableSettings() */
    mutable TErrorMap m_errors;             /**< per thread */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIDatabaseClient, CDRNDatabaseClient, MT::CMutex >  TDRNDatabaseClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATABASECLIENT_H ? */
