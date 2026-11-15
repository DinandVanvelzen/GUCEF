/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */
#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */
#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */
#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#include "gucefCOMCORE_CIDatabaseClient.h"
#define GUCEF_COMCORE_CIDATABASECLIENT_H
#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H
#include "gucefASSETSTORE_CAssetConversionRecord.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetVocabulary;

/**
 *  The catalogue of the primary store in a database (section 9): the tables of section 9.2 through a
 *  COMCORE::CIDatabaseClient, made and migrated when the catalogue opens, and the reading and writing of their rows.
 *
 *  Tables, all with a primary key and index hints for the backends that keep indexes: store_locations, sources,
 *  packs, assets, asset_model_props, asset_texture_props, asset_material_props, asset_animation_props,
 *  asset_audio_props, asset_files, relations, keywords, categories, category_map, synonyms, category_rules,
 *  licence_map, facets, saved_searches, licences, thumbnails, ingest_history, pack_conversions and schema_version. Every table of rows
 *  that belong to a pack has a pack_id column, so a pack is taken out with one delete per table. Times are INT64 UTC
 *  milliseconds since the Unix epoch; sets of names are text with the names joined by ','.
 *
 *  Migrations: schema_version holds a row per migration that ran. Version 1 makes the tables, version 2 writes the
 *  seed data of the vocabulary (CAssetVocabulary), version 3 adds the table pack_conversions (what the last conversion
 *  of a pack did, section 7.4). Open() runs those the database has not had up to the target
 *  version; a catalogue of a newer version than this build knows is refused.
 *
 *  A row is a map of column name to value; columns a row does not have are null. Thread safe once open.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetCatalogue
{
    public:

    static const UInt32 CurrentSchemaVersion = 3;

    typedef GUCEF::map< CORE::CString, CORE::CVariant > TRow;
    typedef GUCEF::vector< TRow > TRowVector;
    typedef GUCEF::vector< CORE::CDataTableDefinition > TDefinitionVector;

    CAssetCatalogue( void );
    ~CAssetCatalogue();

    /**
     *  Opens the catalogue on an open database client: opens the tables there are and runs the migrations up to the
     *  target version. The vocabulary gives the seed data of migration 2 and may be GUCEF_NULL when the target is 1
     */
    TAssetStoreStatus Open( const COMCORE::CIDatabaseClientPtr& client         ,
                            CAssetVocabulary* seedSource                       ,
                            UInt32 targetVersion = CurrentSchemaVersion        );

    void Close( void );

    bool IsOpen( void ) const;

    /** The highest migration that ran, 0 for an empty database */
    UInt32 GetSchemaVersion( void ) const;

    /** Whether the backend has transactions: a pack is then indexed in one transaction */
    bool HasTransactions( void ) const;

    COMCORE::CIDatabaseClientPtr GetClient( void ) const;

    /** Adds or replaces rows by their primary key */
    TAssetStoreStatus Write( const CORE::CString& tableName, const TRowVector& rows );

    /** The rows that match the query; the columns of the selection of the query, every column without one */
    TAssetStoreStatus Select( const CORE::CString& tableName, const CORE::CDataTableQuery& query, TRowVector& rows ) const;

    /** Every row of the table */
    TAssetStoreStatus SelectAll( const CORE::CString& tableName, TRowVector& rows ) const;

    /** The rows whose column has one of the values; the values are looked up in batches */
    TAssetStoreStatus SelectIn( const CORE::CString& tableName, const CORE::CString& columnName, const CORE::CString::StringSet& values, TRowVector& rows ) const;

    /** Takes out the rows that match the predicates */
    TAssetStoreStatus Delete( const CORE::CString& tableName, const CORE::CDataTableQuery& predicates );

    /** Takes out the rows whose column equals the value */
    TAssetStoreStatus DeleteWhereEquals( const CORE::CString& tableName, const CORE::CString& columnName, const CORE::CVariant& value );

    /** The message of the last failure */
    CORE::CString GetLastErrorMessage( void ) const;

    /** The definitions of the tables of the given schema version (section 9.2) */
    static void GetTableDefinitions( UInt32 schemaVersion, TDefinitionVector& definitions );

    /** The tables of rows that belong to a pack, the packs table not included, in the order a pack is written */
    static void GetPackChildTables( CORE::CString::StringVector& tableNames );

    /** The row of the table pack_conversions for a pack, and the record back from such a row */
    static void MakeConversionRow( const CORE::CString& packId, const CAssetConversionRecord& record, TRow& row );
    static void ConversionFromRow( const TRow& row, CAssetConversionRecord& record );

    /** Helpers for rows: text of a cell, a number of a cell, a set kept as text */
    static CORE::CString GetText( const TRow& row, const CORE::CString& column );
    static Int64 GetInt( const TRow& row, const CORE::CString& column, Int64 defaultValue = 0 );
    static Float64 GetFloat( const TRow& row, const CORE::CString& column, Float64 defaultValue = 0.0 );
    static bool GetBool( const TRow& row, const CORE::CString& column, bool defaultValue = false );
    static CORE::CString JoinSet( const CORE::CString::StringSet& values );
    static CORE::CString::StringSet SplitSet( const CORE::CString& text );
    static CORE::CString JoinLines( const CORE::CString::StringVector& values );
    static CORE::CString::StringVector SplitLines( const CORE::CString& text );

    private:

    TAssetStoreStatus Fail( TAssetStoreStatus status, const CORE::CString& message ) const;
    TAssetStoreStatus RunMigration( UInt32 version, CAssetVocabulary* seedSource );
    TAssetStoreStatus EnsureTables( UInt32 schemaVersion );
    bool TryGetTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table ) const;

    CAssetCatalogue( const CAssetCatalogue& src );              /**< not implemented */
    CAssetCatalogue& operator=( const CAssetCatalogue& src );   /**< not implemented */

    private:

    typedef GUCEF::map< CORE::CString, CORE::CIDataTablePtr > TTableMap;

    COMCORE::CIDatabaseClientPtr m_client;
    TTableMap m_tables;
    UInt32 m_schemaVersion;
    mutable CORE::CString m_lastError;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
