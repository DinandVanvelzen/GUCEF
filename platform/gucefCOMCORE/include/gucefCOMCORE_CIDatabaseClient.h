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

#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#define GUCEF_COMCORE_CIDATABASECLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H
#include "gucefCOMCORE_CDatabaseClientSettings.h"
#define GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Capability flags of a database client, see CIDatabaseClient::GetCapabilities() */
#define GUCEF_DATABASE_CAPABILITY_WRITE                 0x0001  /**< tables can be created and changed */
#define GUCEF_DATABASE_CAPABILITY_PARTITIONED_TABLES    0x0002  /**< tables can be partitioned */
#define GUCEF_DATABASE_CAPABILITY_PREDICATE_PUSHDOWN    0x0004  /**< the predicates of a query are used to prune what is read */
#define GUCEF_DATABASE_CAPABILITY_ASYNC                 0x0008  /**< the async calls run on a thread pool */
#define GUCEF_DATABASE_CAPABILITY_SCHEMA_EVOLUTION      0x0010  /**< columns can be appended to a table */

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
 *  A connection to a database of data tables (section 12 of the data tables design): the way to find the tables of a
 *  database, to open them and to make them. The backend is chosen by name from configuration:
 *
 *      CIDatabaseClientPtr client = CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( settings.backendName );
 *      client->Open( settings );
 *
 *  Backends register with RegisterConcreteFactory() from the plugin that brings them and unregister when the plugin
 *  unloads. A plugin cannot refuse to be unloaded: the application closes its clients before that.
 *
 *  Every call has a sync form that does the work on the calling thread and never pumps a pulse generator, and an async
 *  form that queues the same work on a thread pool (the pool of the settings, "DataTables" by default) and returns a
 *  CFutureResult. Results come with the completed future: CDatabaseTableResult for OpenTableAsync() and
 *  CreateTableAsync(), CDatabaseTableNamesResult for ListTablesAsync(); a failed call ends the future in an error status
 *  with "STATUSNAME: message" as its extra info. Events raised on pool threads reach observers on the thread of the
 *  pulse generator in the settings, as those of gucefAUDIO do; observers subscribe per event with CTEventHandlerFunctor.
 *
 *  A client is thread safe once open; Open() and Close() are called from one thread. The last error message is kept per
 *  calling thread. Settings are stored with SaveConfig() and LoadConfig() as the serialized CDatabaseClientSettings.
 */
class GUCEF_COMCORE_EXPORT_CPP CIDatabaseClient : public CORE::CTSGNotifier ,
                                                  public CORE::CIConfigurable
{
    public:

    typedef CORE::TCloneableString  TDatabaseErrorEventData;
    typedef CORE::TCloneableString  TTableEventData;

    static const CORE::CEvent DatabaseOpenedEvent;
    static const CORE::CEvent DatabaseClosedEvent;
    static const CORE::CEvent DatabaseErrorEvent;      /**< TDatabaseErrorEventData: a description of the problem */
    static const CORE::CEvent TableCreatedEvent;       /**< TTableEventData: the table name */
    static const CORE::CEvent TableDroppedEvent;       /**< TTableEventData: the table name */

    static void RegisterEvents( void );

    CIDatabaseClient( void );

    virtual ~CIDatabaseClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Open( const CDatabaseClientSettings& settings ) = 0;
    virtual CORE::CFutureResult OpenAsync( const CDatabaseClientSettings& settings ) = 0;
    virtual void Close( void ) = 0;
    virtual bool IsOpen( void ) const = 0;

    /** The name the backend is registered with in the factory: "vfstables" */
    virtual const CORE::CString& GetBackendName( void ) const = 0;

    /** GUCEF_DATABASE_CAPABILITY_* */
    virtual UInt32 GetCapabilities( void ) const = 0;

    virtual CORE::TDataTableStatus ListTables( CORE::CString::StringSet& tableNames ) = 0;
    virtual CORE::CFutureResult ListTablesAsync( void ) = 0;

    virtual CORE::TDataTableStatus GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) = 0;

    virtual CORE::TDataTableStatus OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table ) = 0;
    virtual CORE::CFutureResult OpenTableAsync( const CORE::CString& tableName ) = 0;

    virtual CORE::TDataTableStatus CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table ) = 0;
    virtual CORE::CFutureResult CreateTableAsync( const CORE::CDataTableDefinition& definition ) = 0;

    virtual CORE::TDataTableStatus DropTable( const CORE::CString& tableName, bool deleteData ) = 0;
    virtual CORE::CFutureResult DropTableAsync( const CORE::CString& tableName, bool deleteData ) = 0;

    /** The message of the last failed call this thread made on the client */
    virtual CORE::CString GetLastErrorMessage( void ) const = 0;

    /** The settings the client was opened with, or the ones LoadConfig() loaded */
    const CDatabaseClientSettings& GetSettings( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** For the backend, to store the settings it opened with */
    void SetSettings( const CDatabaseClientSettings& settings );

    private:

    CIDatabaseClient( const CIDatabaseClient& src );                /**< not implemented */
    CIDatabaseClient& operator=( const CIDatabaseClient& src );     /**< not implemented */

    private:

    CDatabaseClientSettings m_settings;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIDatabaseClient, MT::CMutex >  CDatabaseClientFactory;
typedef CDatabaseClientFactory::TProductPtr                                      CIDatabaseClientPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */
