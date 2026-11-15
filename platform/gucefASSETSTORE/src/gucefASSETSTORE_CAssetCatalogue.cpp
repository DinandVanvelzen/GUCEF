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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */
#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */
#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */
#ifndef GUCEF_CORE_CDATATABLEROWCURSOR_H
#include "gucefCORE_CDataTableRowCursor.h"
#define GUCEF_CORE_CDATATABLEROWCURSOR_H
#endif /* GUCEF_CORE_CDATATABLEROWCURSOR_H ? */
#ifndef GUCEF_CORE_CDATATABLEWRITERESULT_H
#include "gucefCORE_CDataTableWriteResult.h"
#define GUCEF_CORE_CDATATABLEWRITERESULT_H
#endif /* GUCEF_CORE_CDATATABLEWRITERESULT_H ? */
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#include "gucefASSETSTORE_CAssetVocabulary.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
#include "gucefASSETSTORE_CAssetCatalogue.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 RowsPerWrite = 5000;
const UInt32 ValuesPerLookup = 500;

/**
 *  Adds a table: columns as "name:type" separated by ',' (types utf8, int64, float64, bool; a '!' after the type: not
 *  nullable), the primary key as names separated by ',', index hints as "name=column+column" separated by ';'
 */
void
AddTable( CAssetCatalogue::TDefinitionVector& definitions, const char* name, const char* columns, const char* primaryKey, const char* indexes )
{
    CORE::CDataTableDefinition definition;
    definition.name = name;
    const CORE::CString::StringVector columnSpecs = CORE::CString( columns ).ParseElements( ',', false );
    for ( size_t c=0; c<columnSpecs.size(); ++c )
    {
        const CORE::CString columnName = columnSpecs[ c ].SubstrToChar( ':', true );
        CORE::CString typeText = columnSpecs[ c ].SubstrToChar( ':', false );
        bool nullable = true;
        if ( typeText.HasChar( '!' ) >= 0 )
        {
            nullable = false;
            typeText = typeText.RemoveChar( '!' );
        }
        CORE::TDataTableLogicalType type = CORE::DATATABLE_TYPE_UTF8;
        if ( typeText == "int64" )
            type = CORE::DATATABLE_TYPE_INT64;
        else if ( typeText == "float64" )
            type = CORE::DATATABLE_TYPE_FLOAT64;
        else if ( typeText == "bool" )
            type = CORE::DATATABLE_TYPE_BOOL;
        definition.schema.AddColumn( CORE::CDataTableColumnDef( columnName, type, nullable ) );
    }
    definition.schema.SetPrimaryKey( CORE::CString( primaryKey ).ParseElements( ',', false ) );
    const CORE::CString::StringVector indexSpecs = CORE::CString( indexes ).ParseElements( ';', false );
    for ( size_t i=0; i<indexSpecs.size(); ++i )
    {
        CORE::CDataTableIndexDef index;
        index.name = indexSpecs[ i ].SubstrToChar( '=', true );
        index.columns = indexSpecs[ i ].SubstrToChar( '=', false ).ParseElements( '+', false );
        definition.indexes.push_back( index );
    }
    definitions.push_back( definition );
}

TAssetStoreStatus
StatusOf( CORE::TDataTableStatus status )
{
    switch ( status )
    {
        case CORE::DATATABLE_STATUS_OK:
        case CORE::DATATABLE_STATUS_END: return ASSETSTORE_STATUS_OK;
        case CORE::DATATABLE_STATUS_NOT_FOUND: return ASSETSTORE_STATUS_NOT_FOUND;
        case CORE::DATATABLE_STATUS_INVALID_ARGUMENT:
        case CORE::DATATABLE_STATUS_TYPE_MISMATCH: return ASSETSTORE_STATUS_INVALID_ARGUMENT;
        case CORE::DATATABLE_STATUS_UNSUPPORTED: return ASSETSTORE_STATUS_UNSUPPORTED;
        case CORE::DATATABLE_STATUS_IO_ERROR: return ASSETSTORE_STATUS_IO_ERROR;
        default: return ASSETSTORE_STATUS_DATABASE_ERROR;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetCatalogue::CAssetCatalogue( void )
    : m_client()
    , m_tables()
    , m_schemaVersion( 0 )
    , m_lastError()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetCatalogue::~CAssetCatalogue()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

void
CAssetCatalogue::GetTableDefinitions( UInt32 schemaVersion, TDefinitionVector& definitions )
{GUCEF_TRACE;

    definitions.clear();
    if ( schemaVersion < 1 )
        return;

    AddTable( definitions, "schema_version", "version:int64!,applied_ms:int64,description:utf8", "version", "" );
    AddTable( definitions, "store_locations", "location_id:utf8!,name:utf8,vfs_root:utf8,is_default:bool,online:bool,checked_ms:int64", "location_id", "" );
    AddTable( definitions, "sources", "source_id:utf8!,ingest_mode:utf8,keep:utf8,location_id:utf8", "source_id", "" );
    AddTable( definitions, "packs",
              "pack_id:utf8!,source_id:utf8,source_item_id:utf8,source_version:utf8,source_url:utf8,title:utf8,seller:utf8,description:utf8,"
              "source_categories:utf8,source_tags:utf8,categories:utf8,unmapped_categories:utf8,kinds:utf8,themes:utf8,art_styles:utf8,"
              "pbr_roles:utf8,licence_id:utf8,licence_class:utf8,licence_restrictions:utf8,formats_original:utf8,formats_converted:utf8,"
              "keep:utf8,location_id:utf8,root_path:utf8,size_bytes:int64,file_count:int64,asset_count:int64,ingested_ms:int64,"
              "normalizer:utf8,analyzers:utf8,state:utf8,state_reason:utf8",
              "pack_id", "packs_source_item=source_id+source_item_id;packs_state=state;packs_location=location_id;packs_seller=seller" );
    AddTable( definitions, "assets",
              "asset_id:utf8!,pack_id:utf8,kind:utf8,name:utf8,original_path:utf8,converted_path:utf8,source_format:utf8,"
              "converted_format:utf8,size_bytes:int64,digest:utf8,category_id:utf8",
              "asset_id", "assets_pack=pack_id;assets_digest=digest;assets_kind=kind;assets_category=category_id;assets_format=source_format" );
    AddTable( definitions, "asset_model_props",
              "asset_id:utf8!,pack_id:utf8,triangles:int64,vertices:int64,sub_meshes:int64,bones:int64,animations:int64,lods:int64,"
              "bounds_x:float64,bounds_y:float64,bounds_z:float64,up_axis:utf8,unit:utf8,rigged:bool,animated:bool,has_collision:bool",
              "asset_id", "model_props_pack=pack_id;model_props_triangles=triangles" );
    AddTable( definitions, "asset_texture_props",
              "asset_id:utf8!,pack_id:utf8,width:int64,height:int64,resolution:int64,channels:int64,bit_depth:int64,colour_space:utf8,"
              "compression:utf8,role:utf8",
              "asset_id", "texture_props_pack=pack_id;texture_props_resolution=resolution" );
    AddTable( definitions, "asset_material_props", "asset_id:utf8!,pack_id:utf8,shading_model:utf8,texture_count:int64,pbr_roles:utf8", "asset_id", "material_props_pack=pack_id" );
    AddTable( definitions, "asset_animation_props", "asset_id:utf8!,pack_id:utf8,length_s:float64,frame_rate:float64,skeleton:utf8", "asset_id", "animation_props_pack=pack_id" );
    AddTable( definitions, "asset_audio_props", "asset_id:utf8!,pack_id:utf8,length_s:float64,channels:int64,sample_rate:int64", "asset_id", "audio_props_pack=pack_id" );
    AddTable( definitions, "asset_files", "asset_id:utf8!,path:utf8!,pack_id:utf8,size_bytes:int64,digest:utf8,role:utf8", "asset_id,path", "asset_files_pack=pack_id;asset_files_digest=digest" );
    AddTable( definitions, "relations", "from_asset:utf8!,kind:utf8!,to_asset:utf8!,pack_id:utf8", "from_asset,kind,to_asset", "relations_pack=pack_id;relations_to=to_asset" );
    AddTable( definitions, "keywords", "keyword:utf8!,object_id:utf8!,object_type:utf8,pack_id:utf8,field:utf8,weight:int64,original:utf8", "keyword,object_id", "keywords_pack=pack_id" );
    AddTable( definitions, "categories", "category_id:utf8!,axis:utf8,parent_id:utf8,name:utf8,sort_order:int64", "category_id", "categories_axis=axis" );
    AddTable( definitions, "category_map", "source_id:utf8!,source_category:utf8!,category_id:utf8,theme:utf8,kind:utf8", "source_id,source_category", "" );
    AddTable( definitions, "synonyms", "word:utf8!,replacement:utf8", "word", "" );
    AddTable( definitions, "category_rules", "rule_id:utf8!,match_on:utf8,pattern:utf8,category_id:utf8,kind:utf8,themes:utf8,art_styles:utf8,priority:int64", "rule_id", "" );
    AddTable( definitions, "licence_map", "rule_id:utf8!,match_on:utf8,pattern:utf8,licence_class:utf8,restrictions:utf8,priority:int64", "rule_id", "" );
    AddTable( definitions, "facets", "object_id:utf8!,facet:utf8!,value:utf8!,object_type:utf8,pack_id:utf8", "object_id,facet,value", "facets_value=facet+value;facets_pack=pack_id" );
    AddTable( definitions, "saved_searches", "search_id:utf8!,name:utf8,query_json:utf8,precanned:bool,sort_order:int64", "search_id", "" );
    AddTable( definitions, "licences", "licence_id:utf8!,licence_class:utf8,name:utf8,url:utf8,text:utf8,restrictions:utf8", "licence_id", "licences_class=licence_class" );
    AddTable( definitions, "thumbnails", "asset_id:utf8!,size:int64!,pack_id:utf8,vfs_path:utf8", "asset_id,size", "thumbnails_pack=pack_id" );
    AddTable( definitions, "ingest_history",
              "entry_id:utf8!,ingest_id:utf8,pack_id:utf8,source_id:utf8,origin:utf8,policy:utf8,started_ms:int64,duration_ms:int64,"
              "outcome:utf8,step:utf8,problems:utf8",
              "entry_id", "ingest_history_pack=pack_id;ingest_history_ingest=ingest_id" );
    if ( schemaVersion >= 3 )
    {
        AddTable( definitions, "pack_conversions",
                  "pack_id:utf8!,profile:utf8,converters:utf8,outcome:utf8,reason:utf8,report_path:utf8,converted_ms:int64,converted_count:int64,"
                  "approximated_count:int64,cached_count:int64,skipped_count:int64,failed_count:int64",
                  "pack_id", "pack_conversions_outcome=outcome" );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetCatalogue::GetPackChildTables( CORE::CString::StringVector& tableNames )
{GUCEF_TRACE;

    // the order of writing: the assets before what refers to them, the keywords and facets last
    tableNames.clear();
    tableNames.push_back( "assets" );
    tableNames.push_back( "asset_model_props" );
    tableNames.push_back( "asset_texture_props" );
    tableNames.push_back( "asset_material_props" );
    tableNames.push_back( "asset_animation_props" );
    tableNames.push_back( "asset_audio_props" );
    tableNames.push_back( "asset_files" );
    tableNames.push_back( "relations" );
    tableNames.push_back( "thumbnails" );
    tableNames.push_back( "facets" );
    tableNames.push_back( "keywords" );
    tableNames.push_back( "pack_conversions" );
}

/*-------------------------------------------------------------------------*/

void
CAssetCatalogue::MakeConversionRow( const CORE::CString& packId, const CAssetConversionRecord& record, TRow& row )
{GUCEF_TRACE;

    row.clear();
    row[ "pack_id" ] = CORE::CVariant( packId );
    row[ "profile" ] = CORE::CVariant( record.profile );
    row[ "converters" ] = CORE::CVariant( JoinSet( record.converters ) );
    row[ "outcome" ] = CORE::CVariant( record.outcome );
    row[ "reason" ] = CORE::CVariant( record.reason );
    row[ "report_path" ] = CORE::CVariant( record.reportPath );
    row[ "converted_ms" ] = CORE::CVariant( record.convertedAtMs );
    row[ "converted_count" ] = CORE::CVariant( (Int64) record.convertedCount );
    row[ "approximated_count" ] = CORE::CVariant( (Int64) record.approximatedCount );
    row[ "cached_count" ] = CORE::CVariant( (Int64) record.cachedCount );
    row[ "skipped_count" ] = CORE::CVariant( (Int64) record.skippedCount );
    row[ "failed_count" ] = CORE::CVariant( (Int64) record.failedCount );
}

/*-------------------------------------------------------------------------*/

void
CAssetCatalogue::ConversionFromRow( const TRow& row, CAssetConversionRecord& record )
{GUCEF_TRACE;

    record = CAssetConversionRecord();
    record.profile = GetText( row, "profile" );
    record.converters = SplitSet( GetText( row, "converters" ) );
    record.outcome = GetText( row, "outcome" );
    record.reason = GetText( row, "reason" );
    record.reportPath = GetText( row, "report_path" );
    record.convertedAtMs = GetInt( row, "converted_ms" );
    record.convertedCount = (UInt32) GetInt( row, "converted_count" );
    record.approximatedCount = (UInt32) GetInt( row, "approximated_count" );
    record.cachedCount = (UInt32) GetInt( row, "cached_count" );
    record.skippedCount = (UInt32) GetInt( row, "skipped_count" );
    record.failedCount = (UInt32) GetInt( row, "failed_count" );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::Fail( TAssetStoreStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_lastError = message;
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetCatalogue: " << message );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCatalogue::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastError;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCatalogue::TryGetTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TTableMap::const_iterator i = m_tables.find( tableName );
    if ( i == m_tables.end() )
        return false;
    table = (*i).second;
    return !table.IsNULL();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::EnsureTables( UInt32 schemaVersion )
{GUCEF_TRACE;

    TDefinitionVector definitions;
    GetTableDefinitions( schemaVersion, definitions );
    for ( size_t d=0; d<definitions.size(); ++d )
    {
        CORE::CIDataTablePtr table;
        if ( TryGetTable( definitions[ d ].name, table ) )
            continue;
        CORE::TDataTableStatus status = m_client->CreateTable( definitions[ d ], table );
        if ( CORE::DATATABLE_STATUS_ALREADY_EXISTS == status )
            status = m_client->OpenTable( definitions[ d ].name, table );
        if ( CORE::DATATABLE_STATUS_OK != status || table.IsNULL() )
            return Fail( StatusOf( status ), "The table " + definitions[ d ].name + " cannot be made: " + m_client->GetLastErrorMessage() );
        MT::CScopeMutex lock( m_lock );
        m_tables[ definitions[ d ].name ] = table;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::Open( const COMCORE::CIDatabaseClientPtr& client, CAssetVocabulary* seedSource, UInt32 targetVersion )
{GUCEF_TRACE;

    Close();
    if ( client.IsNULL() || !client->IsOpen() )
        return Fail( ASSETSTORE_STATUS_INVALID_ARGUMENT, "The database client is not open" );
    m_client = client;

    // the tables there are
    CORE::CString::StringSet tableNames;
    CORE::TDataTableStatus status = m_client->ListTables( tableNames );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( StatusOf( status ), "The tables of the catalogue cannot be listed: " + client->GetLastErrorMessage() );
    TDefinitionVector definitions;
    GetTableDefinitions( CurrentSchemaVersion, definitions );
    for ( size_t d=0; d<definitions.size(); ++d )
    {
        if ( tableNames.end() == tableNames.find( definitions[ d ].name ) )
            continue;
        CORE::CIDataTablePtr table;
        status = m_client->OpenTable( definitions[ d ].name, table );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( StatusOf( status ), "The table " + definitions[ d ].name + " cannot be opened: " + client->GetLastErrorMessage() );
        MT::CScopeMutex lock( m_lock );
        m_tables[ definitions[ d ].name ] = table;
    }

    // the migrations that ran
    m_schemaVersion = 0;
    TRowVector versions;
    if ( tableNames.end() != tableNames.find( "schema_version" ) )
    {
        if ( ASSETSTORE_STATUS_OK != SelectAll( "schema_version", versions ) )
            return Fail( ASSETSTORE_STATUS_DATABASE_ERROR, "The schema version of the catalogue cannot be read" );
        for ( size_t v=0; v<versions.size(); ++v )
        {
            const UInt32 version = (UInt32) GetInt( versions[ v ], "version" );
            if ( version > m_schemaVersion )
                m_schemaVersion = version;
        }
    }
    if ( m_schemaVersion > CurrentSchemaVersion )
    {
        const UInt32 found = m_schemaVersion;
        Close();
        return Fail( ASSETSTORE_STATUS_UNSUPPORTED, "The catalogue has schema version " + CORE::ToString( found ) + ", this build knows up to " + CORE::ToString( CurrentSchemaVersion ) );
    }

    for ( UInt32 version=m_schemaVersion+1; version<=targetVersion && version<=CurrentSchemaVersion; ++version )
    {
        const TAssetStoreStatus migrated = RunMigration( version, seedSource );
        if ( ASSETSTORE_STATUS_OK != migrated )
            return migrated;
        m_schemaVersion = version;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetCatalogue: Opened the catalogue on backend " << client->GetBackendName() << ", schema version " << m_schemaVersion );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::RunMigration( UInt32 version, CAssetVocabulary* seedSource )
{GUCEF_TRACE;

    CORE::CString description;
    TAssetStoreStatus status = ASSETSTORE_STATUS_OK;
    switch ( version )
    {
        case 1:
        {
            description = "the tables of the catalogue";
            status = EnsureTables( 1 );
            break;
        }
        case 2:
        {
            description = "the seed data of the vocabulary and the precanned searches";
            if ( GUCEF_NULL == seedSource )
                return Fail( ASSETSTORE_STATUS_INVALID_ARGUMENT, "Migration 2 needs the seed data of the vocabulary" );
            status = seedSource->WriteSeedRows( *this );
            break;
        }
        case 3:
        {
            description = "the table of the conversions of the packs";
            status = EnsureTables( 3 );
            break;
        }
        default:
        {
            return Fail( ASSETSTORE_STATUS_UNSUPPORTED, "There is no migration to version " + CORE::ToString( version ) );
        }
    }
    if ( ASSETSTORE_STATUS_OK != status )
        return Fail( status, "Migration " + CORE::ToString( version ) + " (" + description + ") failed: " + GetLastErrorMessage() );

    TRowVector rows( 1 );
    rows[ 0 ][ "version" ] = CORE::CVariant( (Int64) version );
    rows[ 0 ][ "applied_ms" ] = CORE::CVariant( (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() );
    rows[ 0 ][ "description" ] = CORE::CVariant( description );
    status = Write( "schema_version", rows );
    if ( ASSETSTORE_STATUS_OK == status )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetCatalogue: Migrated the catalogue to schema version " << version << ": " << description );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

void
CAssetCatalogue::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_tables.clear();
    m_client.Unlink();
    m_schemaVersion = 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCatalogue::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_client.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetCatalogue::GetSchemaVersion( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_schemaVersion;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCatalogue::HasTransactions( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_client.IsNULL() && 0 != ( m_client->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_TRANSACTIONS );
}

/*-------------------------------------------------------------------------*/

COMCORE::CIDatabaseClientPtr
CAssetCatalogue::GetClient( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_client;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::Write( const CORE::CString& tableName, const TRowVector& rows )
{GUCEF_TRACE;

    if ( rows.empty() )
        return ASSETSTORE_STATUS_OK;
    CORE::CIDataTablePtr table;
    if ( !TryGetTable( tableName, table ) )
        return Fail( ASSETSTORE_STATUS_NOT_FOUND, "The catalogue has no table " + tableName );

    const CORE::CDataTableSchemaPtr schema = table->GetSchema();
    const UInt32 columnCount = schema->GetColumnCount();
    GUCEF::vector< CORE::CVariant > values( columnCount );
    for ( size_t first=0; first<rows.size(); first+=RowsPerWrite )
    {
        CORE::CDataTableBatchBuilder builder( schema, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
        const size_t last = ( first + RowsPerWrite < rows.size() ) ? first + RowsPerWrite : rows.size();
        for ( size_t r=first; r<last; ++r )
        {
            for ( UInt32 c=0; c<columnCount; ++c )
            {
                TRow::const_iterator cell = rows[ r ].find( schema->GetColumn( c ).name );
                values[ c ] = ( cell == rows[ r ].end() ) ? CORE::CVariant() : (*cell).second;
                if ( values[ c ].IsInitialized() && CORE::DATATABLE_TYPE_UTF8 != schema->GetColumn( c ).type && values[ c ].IsString() && values[ c ].AsString().IsNULLOrEmpty() )
                    values[ c ] = CORE::CVariant();
            }
            if ( !builder.AppendRow( &values[ 0 ], columnCount ) )
                return Fail( ASSETSTORE_STATUS_INVALID_ARGUMENT, "A row does not fit the table " + tableName + ": " + CORE::DataTableStatusToString( builder.GetLastStatus() ) );
        }
        CORE::CDataTableBatchPtr batch;
        CORE::CString error;
        if ( CORE::DATATABLE_STATUS_OK != builder.Build( batch, error ) )
            return Fail( ASSETSTORE_STATUS_INVALID_ARGUMENT, "The rows of " + tableName + " cannot be put together: " + error );
        CORE::CDataTableWriteResult result;
        const CORE::TDataTableStatus status = table->Write( *batch, CORE::DATATABLE_WRITE_UPSERT, result );
        if ( CORE::DATATABLE_STATUS_OK != status || CORE::DATATABLE_STATUS_OK != result.GetOverallStatus() )
            return Fail( StatusOf( CORE::DATATABLE_STATUS_OK != status ? status : result.GetOverallStatus() ), "Writing to " + tableName + " failed: " + table->GetLastErrorMessage() );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::Select( const CORE::CString& tableName, const CORE::CDataTableQuery& query, TRowVector& rows ) const
{GUCEF_TRACE;

    rows.clear();
    CORE::CIDataTablePtr table;
    if ( !TryGetTable( tableName, table ) )
        return Fail( ASSETSTORE_STATUS_NOT_FOUND, "The catalogue has no table " + tableName );
    CORE::CDataTableScanPtr scan;
    CORE::TDataTableStatus status = table->Scan( query, scan );
    if ( CORE::DATATABLE_STATUS_OK != status || scan.IsNULL() )
        return Fail( StatusOf( status ), "Reading " + tableName + " failed: " + table->GetLastErrorMessage() );

    for ( ;; )
    {
        CORE::CDataTableBatchPtr batch;
        status = scan->NextBatch( batch );
        if ( CORE::DATATABLE_STATUS_END == status )
            break;
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( StatusOf( status ), "Reading " + tableName + " failed: " + scan->GetLastErrorMessage() );
        if ( batch.IsNULL() )
            continue;
        const CORE::CDataTableSchemaPtr& schema = batch->GetSchema();
        const UInt32 columnCount = batch->GetColumnCount();
        CORE::CString::StringVector names( columnCount );
        for ( UInt32 c=0; c<columnCount; ++c )
            names[ c ] = schema->GetColumn( batch->GetSchemaColumnIndex( c ) ).name;
        CORE::CDataTableRowCursor cursor( batch );
        while ( CORE::DATATABLE_STATUS_OK == cursor.Next() )
        {
            TRow row;
            for ( UInt32 c=0; c<columnCount; ++c )
            {
                CORE::CVariant value;
                if ( cursor.GetValue( c, value, false ) )
                    row[ names[ c ] ] = value;
            }
            rows.push_back( row );
        }
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::SelectAll( const CORE::CString& tableName, TRowVector& rows ) const
{GUCEF_TRACE;

    return Select( tableName, CORE::CDataTableQuery(), rows );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::SelectIn( const CORE::CString& tableName, const CORE::CString& columnName, const CORE::CString::StringSet& values, TRowVector& rows ) const
{GUCEF_TRACE;

    rows.clear();
    CORE::CVariant::VariantVector batchValues;
    CORE::CString::StringSet::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        batchValues.push_back( CORE::CVariant( (*i) ) );
        ++i;
        if ( batchValues.size() >= ValuesPerLookup || i == values.end() )
        {
            CORE::CDataTableQuery query;
            query.WhereIn( columnName, batchValues );
            TRowVector found;
            const TAssetStoreStatus status = Select( tableName, query, found );
            if ( ASSETSTORE_STATUS_OK != status )
                return status;
            rows.insert( rows.end(), found.begin(), found.end() );
            batchValues.clear();
        }
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::Delete( const CORE::CString& tableName, const CORE::CDataTableQuery& predicates )
{GUCEF_TRACE;

    CORE::CIDataTablePtr table;
    if ( !TryGetTable( tableName, table ) )
        return Fail( ASSETSTORE_STATUS_NOT_FOUND, "The catalogue has no table " + tableName );
    CORE::CDataTableWriteResult result;
    const CORE::TDataTableStatus status = table->DeleteRows( predicates, result );
    if ( CORE::DATATABLE_STATUS_OK != status && CORE::DATATABLE_STATUS_NOT_FOUND != status )
        return Fail( StatusOf( status ), "Deleting from " + tableName + " failed: " + table->GetLastErrorMessage() );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetCatalogue::DeleteWhereEquals( const CORE::CString& tableName, const CORE::CString& columnName, const CORE::CVariant& value )
{GUCEF_TRACE;

    CORE::CDataTableQuery query;
    query.WhereEquals( columnName, value );
    return Delete( tableName, query );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCatalogue::GetText( const TRow& row, const CORE::CString& column )
{GUCEF_TRACE;

    TRow::const_iterator i = row.find( column );
    if ( i == row.end() || !(*i).second.IsInitialized() || (*i).second.IsNULL() )
        return CORE::CString::Empty;
    return (*i).second.AsString();
}

/*-------------------------------------------------------------------------*/

Int64
CAssetCatalogue::GetInt( const TRow& row, const CORE::CString& column, Int64 defaultValue )
{GUCEF_TRACE;

    TRow::const_iterator i = row.find( column );
    if ( i == row.end() || !(*i).second.IsInitialized() || (*i).second.IsNULL() )
        return defaultValue;
    return (*i).second.AsInt64( defaultValue );
}

/*-------------------------------------------------------------------------*/

Float64
CAssetCatalogue::GetFloat( const TRow& row, const CORE::CString& column, Float64 defaultValue )
{GUCEF_TRACE;

    TRow::const_iterator i = row.find( column );
    if ( i == row.end() || !(*i).second.IsInitialized() || (*i).second.IsNULL() )
        return defaultValue;
    return (*i).second.AsFloat64( defaultValue );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCatalogue::GetBool( const TRow& row, const CORE::CString& column, bool defaultValue )
{GUCEF_TRACE;

    TRow::const_iterator i = row.find( column );
    if ( i == row.end() || !(*i).second.IsInitialized() || (*i).second.IsNULL() )
        return defaultValue;
    return (*i).second.AsBool( defaultValue );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCatalogue::JoinSet( const CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CString text;
    CORE::CString::StringSet::const_iterator i = values.begin();
    while ( i != values.end() )
    {
        if ( !(*i).IsNULLOrEmpty() )
        {
            if ( !text.IsNULLOrEmpty() )
                text += ',';
            text += (*i).ReplaceChar( ',', ' ' );
        }
        ++i;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet
CAssetCatalogue::SplitSet( const CORE::CString& text )
{GUCEF_TRACE;

    return text.ParseUniqueElements( ',', false );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCatalogue::JoinLines( const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<values.size(); ++i )
    {
        if ( i > 0 )
            text += '\n';
        text += values[ i ].ReplaceChar( '\n', ' ' );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CAssetCatalogue::SplitLines( const CORE::CString& text )
{GUCEF_TRACE;

    return text.ParseElements( '\n', false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
