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

#ifndef GUCEF_COM_CMCPSCHEMABUILDER_H
#include "gucefCOM_CMcpSchemaBuilder.h"
#define GUCEF_COM_CMCPSCHEMABUILDER_H
#endif /* GUCEF_COM_CMCPSCHEMABUILDER_H ? */

#ifndef GUCEF_COM_CMCPJOBINFO_H
#include "gucefCOM_CMcpJobInfo.h"
#define GUCEF_COM_CMCPJOBINFO_H
#endif /* GUCEF_COM_CMCPJOBINFO_H ? */

#include "gucefASSETSTORE_CAssetToolCatalogue.h"

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

typedef COM::CMcpSchemaBuilder S;
typedef CAssetToolCatalogue C;

const char* JobNote = " Runs as a job: answers with the final result when it ends within waitMs, else with the job (poll job_status, read job_result). Cancelling the call only stops the wait; job_cancel stops the job. requestKey makes a repeat safe.";

/** The tools in the order of tools/list */
const C::SEntry g_tools[] =
{
    // pack inspection (7.1)
    { "pack_list", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "List converter packs",
      "Lists the converter packs (manifests) with their licence decision, whether their content may be shown (contentExposure), whether they can run through MCP, and their last run." },
    { "pack_get", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Get a converter pack",
      "The effective manifest of a pack, its licence summary, cook profiles and last run, with links to its manifest, licence record, report and activity resources." },
    { "pack_assets", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "List the assets of a pack",
      "The assets of a pack from its last report: state, counts of errors, warnings and approximations, triangles, vertices, materials, duration. Filter by state, glob, importer or kind; paged with cursor." },
    { "asset_report", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Report of one asset",
      "Report items (messages are untrusted text), files written, sidecar summary, effective settings and their layers, the last activity entry and the cache entry of one asset." },
    { "pack_activity", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Activity of a run",
      "The activity log of a run of a pack (0 = the last run): per asset the action, the reason and a detail. Paged with cursor." },
    { "asset_explain_reimport", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Why an asset would reimport",
      "What the last run did with an asset and whether a run now would import it again, with the reasons and the settings that changed (JSON pointers)." },
    { "pack_cook_outputs", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Cooked files of a pack",
      "The cooked files of a profile with their own and inclusive sizes and their references. Paged with cursor." },
    { "pipeline_plugins", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Converter plugins",
      "The importers (with their default settings), exporters, cookers, post-processors and validators that are loaded." },
    { "licence_check", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Check the licence of a pack",
      "Whether the licence record of a pack (packId) or of a source (sourcePath) allows conversion and whether content may be shown, with the restrictions and the missing fields." },
    { "licence_draft_record", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Draft a licence record",
      "A draft licence record for the owner to review and save; the decisions (flags and restrictions) are left unset and listed. Writes nothing." },
    // conversion (7.2)
    { "convert_plan", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Plan a conversion",
      "What a conversion of the pack would do now, without doing it: the licence decision, counts per planned action and per asset the reason (a paged sample; the full plan is a resource)." },
    { "convert_pack", "convert", C::FAMILY_JOB_START, COM::MCP_TOOLEFFECT_ADDITIVE, true, false, "Convert a pack",
      "Converts a pack (optionally only the assets matching the globs) to the intermediate and cooks it." },
    { "convert_file", "convert", C::FAMILY_JOB_START, COM::MCP_TOOLEFFECT_ADDITIVE, true, false, "Convert one file",
      "Converts one source file through a scratch pack under the work root; needs the id of an existing licence record." },
    { "cook_pack", "convert", C::FAMILY_JOB_START, COM::MCP_TOOLEFFECT_ADDITIVE, true, false, "Cook a pack",
      "Cooks a pack from its existing intermediate; refused when there is none." },
    { "validate_pack", "read", C::FAMILY_JOB_START, COM::MCP_TOOLEFFECT_READ_ONLY, false, false, "Validate a pack",
      "Imports and validates a pack (or the assets matching the globs) without writing anything; the result groups the findings per validator and severity." },
    { "compare_routes", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Compare conversion routes",
      "Compares the same asset converted through different packs (routes) from their sidecars: counts, bounds, orientation; converts nothing. render adds contact sheets where the licence and the service allow." },
    { "job_status", "read", C::FAMILY_JOB, COM::MCP_TOOLEFFECT_READ_ONLY, false, false, "Status of a job",
      "The job object: state, progress and times. Poll until the state is completed, failed, cancelled or interrupted." },
    { "job_result", "read", C::FAMILY_JOB, COM::MCP_TOOLEFFECT_READ_ONLY, false, false, "Result of a job",
      "The result of a job: a bounded summary (counts, failure and approximation groups, links), or the sections failures, approximations and assets paged with cursor." },
    { "job_list", "read", C::FAMILY_JOB, COM::MCP_TOOLEFFECT_READ_ONLY, false, false, "List jobs",
      "The jobs of the caller, newest first, filtered by state, kind or pack." },
    { "job_cancel", "convert", C::FAMILY_JOB, COM::MCP_TOOLEFFECT_ADDITIVE, true, false, "Cancel a job",
      "Asks a job to stop; assets that run finish, nothing more starts. The run can be resumed by starting it again." },
    // visual verification (7.3)
    { "render_sheet", "read", C::FAMILY_RENDER, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Render contact sheets",
      "Contact sheets (front, side, top, three quarter) of up to 4 assets. Packs whose licence withholds content give no picture. Answers unsupported when the service has no renderer." },
    // authoring (7.4)
    { "manifest_propose", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Propose a manifest",
      "Looks at a source folder or archive and proposes a manifest (importer, globs, folders, cook targets) with notes. Writes nothing." },
    { "manifest_create", "author", C::FAMILY_PIPELINE_CHANGE, COM::MCP_TOOLEFFECT_ADDITIVE, false, false, "Create a manifest",
      "Writes a new manifest. Without apply: a preview with a diff and a changeToken. With apply and the changeToken: the write." },
    { "manifest_edit", "author", C::FAMILY_PIPELINE_CHANGE, COM::MCP_TOOLEFFECT_DESTRUCTIVE, false, false, "Edit a manifest",
      "Edits a manifest with JSON pointer operations. Without apply: a preview with a diff and a changeToken. With apply and the changeToken: the write. id, licenceRecord and licenceFolder cannot change." },
    { "override_write", "author", C::FAMILY_PIPELINE_CHANGE, COM::MCP_TOOLEFFECT_DESTRUCTIVE, false, false, "Write an asset override",
      "Writes the import settings override of an asset. Without apply: a preview with the diff, the effective settings and unknown keys. With apply and the changeToken: the write." },
    { "override_remove", "author", C::FAMILY_PIPELINE_CHANGE, COM::MCP_TOOLEFFECT_DESTRUCTIVE, false, false, "Remove an asset override",
      "Removes the import settings override of an asset (kept in the history). Preview first, then apply with the changeToken." },
    { "material_map_suggest", "read", C::FAMILY_PIPELINE_QUERY, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Suggest a shader property map",
      "Gathers the source materials of a shader from the sidecars and proposes a shaderPropertyMaps fragment for manifest_edit." },
    // primary store (7.5)
    { "store_search", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Search the primary store",
      "Searches the assets or packs of the primary store with facets. The enumerations come from the vocabulary of the store." },
    { "store_asset", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "An asset of the store",
      "An asset record of the primary store with its typed properties, relations, pack and licence summary and availability." },
    { "store_pack", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "A pack of the store",
      "A pack record of the primary store (the description is untrusted text), its licence, asset counts per kind, ingest history and conversion state." },
    { "store_licence", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "A licence of the store",
      "The class, name, URL and restrictions of a licence (by licenceId or storePackId); its text as untrusted text, capped, with a link to the full text." },
    { "store_locations", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Storage locations",
      "The storage locations of the primary store: id, name, whether they are online, and their root without credentials." },
    { "store_vocabulary", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Vocabulary of the store",
      "The kinds, categories, themes, styles, licence classes and restriction flags of the primary store." },
    { "store_saved_searches", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Saved searches",
      "The saved searches of the primary store with their queries." },
    { "store_unclassified", "read", C::FAMILY_STORE_READ, COM::MCP_TOOLEFFECT_READ_ONLY, false, false, "Unclassified items",
      "Packs or assets without a category, with their names, keywords and source categories (untrusted)." },
    { "store_save_search", "store", C::FAMILY_STORE_WRITE, COM::MCP_TOOLEFFECT_ADDITIVE, false, false, "Save a search",
      "Saves a query of store_search under a name." },
    { "store_delete_search", "store", C::FAMILY_STORE_WRITE, COM::MCP_TOOLEFFECT_DESTRUCTIVE, false, false, "Delete a saved search",
      "Deletes a saved search of the user." },
    { "store_classify", "store", C::FAMILY_STORE_WRITE, COM::MCP_TOOLEFFECT_DESTRUCTIVE, false, false, "Classify items",
      "Sets the category, themes and art style of assets or packs. Without apply: a preview of current and new values with a changeToken. With apply and the changeToken: the write." },
    { "store_edit_rules", "store", C::FAMILY_STORE_WRITE, COM::MCP_TOOLEFFECT_DESTRUCTIVE, false, false, "Edit classification rules",
      "Upserts or removes category rules and category map rows. Without apply: a preview with a changeToken. With apply and the changeToken: the write." },
    { "store_reclassify", "store", C::FAMILY_STORE_WRITE, COM::MCP_TOOLEFFECT_ADDITIVE, true, false, "Reclassify packs",
      "Applies the rules and maps again to the packs (all when none are named)." },
    { "store_ingest", "store", C::FAMILY_JOB_START, COM::MCP_TOOLEFFECT_ADDITIVE, false, false, "Ingest into the store",
      "Ingests a pack folder or archive (a rooted path in a source or staging root) into the primary store." },
    { "store_ingest_resume", "store", C::FAMILY_JOB_START, COM::MCP_TOOLEFFECT_ADDITIVE, false, false, "Resume an ingest",
      "Resumes an ingest that stopped." },
    // online stores and transfers (7.6)
    { "online_stores", "read", C::FAMILY_ONLINE, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Online stores",
      "The online stores the service knows and whether they are signed in." },
    { "online_library", "read", C::FAMILY_ONLINE, COM::MCP_TOOLEFFECT_READ_ONLY, true, true, "Library of an online store",
      "The items the owner has in an online store; read only. A store that is not signed in answers not-signed-in: ask the owner to sign in through the viewer." },
    { "online_item", "read", C::FAMILY_ONLINE, COM::MCP_TOOLEFFECT_READ_ONLY, true, true, "An item of an online store",
      "The details of an item of an online store: formats and files, licence, categories and description (untrusted)." },
    { "transfer_queue", "read", C::FAMILY_ONLINE, COM::MCP_TOOLEFFECT_READ_ONLY, false, false, "Transfer queue",
      "The downloads, ingests and conversions of the service with their state and progress." },
    // the library sync (library sync design 7.3): metadata only; a run that downloads is not started through these tools
    { "sync_plan", "store", C::FAMILY_JOB_START, COM::MCP_TOOLEFFECT_ADDITIVE, false, true, "Plan a library sync",
      "Plans the sync of the library of an online store with the profile the service keeps for it, without downloading anything: a dry run job that writes only the observations of the items. The result has the counts (missing, stale, current, unavailable), the estimate of the download and the free space of the library." },
    { "sync_status", "read", C::FAMILY_ONLINE, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "State of a library sync",
      "The state of a library sync or plan (by jobId, or the last one of a store by storeId): the job, the counts of the plan and the run, and a page of the lines of the plan (item, title, state, reason; titles are untrusted)." },
    { "sync_marks", "read", C::FAMILY_ONLINE, COM::MCP_TOOLEFFECT_READ_ONLY, true, false, "Marks of a library sync",
      "The ignore and pin marks the owner set on items of an online store, with their reasons (untrusted)." }
};

const UInt32 g_toolCount = (UInt32) ( sizeof( g_tools ) / sizeof( g_tools[ 0 ] ) );

CORE::CString::StringVector
List( const char* a, const char* b = GUCEF_NULL, const char* c = GUCEF_NULL, const char* d = GUCEF_NULL, const char* e = GUCEF_NULL,
      const char* f = GUCEF_NULL, const char* g = GUCEF_NULL )
{GUCEF_TRACE;

    CORE::CString::StringVector list;
    const char* all[] = { a, b, c, d, e, f, g };
    for ( UInt32 i=0; i<7; ++i )
        if ( GUCEF_NULL != all[ i ] )
            list.push_back( all[ i ] );
    return list;
}

CORE::CDataNode PackId( void ) { return S::String( 1, 128, "The id of a converter pack (pack_list)" ); }
CORE::CDataNode AssetPath( void ) { return S::String( 1, 512, "The path of an asset relative to its pack" ); }
CORE::CDataNode RootedPath( const char* description ) { return S::String( 3, 1100, description ); }
CORE::CDataNode Globs( void ) { return S::StringList( 64, 512, "Globs or exact pack relative asset paths; only these this time (the manifest is not changed)" ); }
CORE::CDataNode CookTargets( void ) { return S::StringList( 4, 32, "Cook targets (cookers by target name)" ); }
CORE::CDataNode Profiles( void ) { return S::StringList( 8, 64, "Cook profiles of the manifest" ); }
CORE::CDataNode WaitMs( void ) { return S::Integer( 0, 60000, "How long to wait for the end of the job before answering with the job" ); }
CORE::CDataNode RequestKey( void ) { return S::String( 1, 64, "Optional. A repeat with the same key and the same arguments answers with the first outcome instead of acting again" ); }

void
AddPaging( CORE::CDataNode& schema, UInt32 maxLimit )
{GUCEF_TRACE;

    S::AddProperty( schema, "cursor", S::String( 1, 256, "The nextCursor of the page before" ) );
    S::AddProperty( schema, "limit", S::Integer( 1, maxLimit, "Entries per page" ) );
}

void
AddWrite( CORE::CDataNode& schema )
{GUCEF_TRACE;

    S::AddProperty( schema, "apply", S::Boolean( "false: preview with a diff and a changeToken; true: write with the changeToken of the preview", true, false ) );
    S::AddProperty( schema, "changeToken", S::String( 1, 64, "The changeToken of the preview (with apply)" ) );
    S::AddProperty( schema, "requestKey", RequestKey() );
}

CORE::CDataNode
AnyValue( const char* description )
{GUCEF_TRACE;

    CORE::CDataNode schema( GUCEF_DATATYPE_OBJECT );
    schema.SetAttribute( "description", description );
    return schema;
}

CORE::CDataNode
Range( void )
{GUCEF_TRACE;

    CORE::CDataNode range = S::Object( "A range: min and max are both optional" );
    S::AddProperty( range, "min", S::UnboundedNumber() );
    S::AddProperty( range, "max", S::UnboundedNumber() );
    return range;
}

CORE::CDataNode
VocabularyList( const CORE::CString::StringVector& values, UInt32 maxItems, const char* description )
{GUCEF_TRACE;

    if ( values.empty() )
        return S::StringList( maxItems, 128, description );
    return S::Array( S::Enum( values ), 0, maxItems, description );
}

CORE::CDataNode
LooseObject( const char* description )
{GUCEF_TRACE;

    return S::Object( description, true );
}

CORE::CDataNode
ListOutput( const char* description )
{GUCEF_TRACE;

    CORE::CDataNode schema = LooseObject( description );
    S::AddProperty( schema, "items", S::Array( LooseObject( "An entry" ) ), true );
    S::AddProperty( schema, "nextCursor", S::Nullable( S::String() ) );
    S::AddProperty( schema, "total", S::Nullable( S::UnboundedInteger() ) );
    S::AddProperty( schema, "truncated", S::Boolean() );
    return schema;
}

CORE::CDataNode
WriteOutput( void )
{GUCEF_TRACE;

    CORE::CDataNode schema = LooseObject( "A preview (diff and changeToken) or the outcome of the write" );
    S::AddProperty( schema, "target", S::Nullable( S::String() ) );
    S::AddProperty( schema, "changeToken", S::Nullable( S::String() ) );
    S::AddProperty( schema, "written", S::Boolean() );
    return schema;
}

CORE::CDataNode
StoreSearchInput( const CAssetVocabularyInfo* vocabulary )
{GUCEF_TRACE;

    static const CAssetVocabularyInfo none;
    const CAssetVocabularyInfo& v = GUCEF_NULL != vocabulary ? *vocabulary : none;
    CORE::CDataNode schema = S::Object();
    S::AddProperty( schema, "text", S::String( 0, 200, "Words to search for" ) );
    S::AddProperty( schema, "savedSearchId", S::String( 1, 128, "Runs a saved search instead of the other members" ) );
    S::AddProperty( schema, "kinds", VocabularyList( v.kinds, 32, "Kinds of assets" ) );
    S::AddProperty( schema, "categories", S::StringList( 32, 256, "Category ids, with their sub categories" ) );
    S::AddProperty( schema, "themes", VocabularyList( v.themes, 32, "Themes" ) );
    S::AddProperty( schema, "artStyles", VocabularyList( v.styles, 32, "Art styles" ) );
    S::AddProperty( schema, "sources", S::StringList( 32, 64, "Store ids, or disk" ) );
    S::AddProperty( schema, "sellers", S::StringList( 32, 128 ) );
    S::AddProperty( schema, "licenceClasses", VocabularyList( v.licenceClasses, 32, "Licence classes" ) );
    S::AddProperty( schema, "requireLicenceRestrictions", VocabularyList( v.restrictionFlags, 16, "Restriction flags a licence must have" ) );
    S::AddProperty( schema, "excludeLicenceRestrictions", VocabularyList( v.restrictionFlags, 16, "Restriction flags a licence must not have" ) );
    S::AddProperty( schema, "formats", S::StringList( 32, 32, "Format ids held" ) );
    S::AddProperty( schema, "triangles", Range() );
    S::AddProperty( schema, "vertices", Range() );
    S::AddProperty( schema, "assetBytes", Range() );
    S::AddProperty( schema, "packBytes", Range() );
    S::AddProperty( schema, "textureResolution", Range() );
    S::AddProperty( schema, "animationSeconds", Range() );
    S::AddProperty( schema, "rigged", S::Boolean() );
    S::AddProperty( schema, "animated", S::Boolean() );
    S::AddProperty( schema, "hasLods", S::Boolean() );
    S::AddProperty( schema, "hasCollision", S::Boolean() );
    S::AddProperty( schema, "pbrRoles", S::StringList( 16, 64 ) );
    S::AddProperty( schema, "unclassified", S::Boolean( "Only items without a category" ) );
    S::AddProperty( schema, "storePackIds", S::StringList( 50, 128, "Only these packs" ) );
    S::AddProperty( schema, "resultKind", S::Enum( List( "assets", "packs" ) ) );
    S::AddProperty( schema, "sort", S::Enum( List( "relevance", "name", "size", "triangles", "newest" ) ) );
    S::AddProperty( schema, "withFacets", S::Boolean( "Counts per kind, category, store and licence class", true, true ) );
    S::AddProperty( schema, "cursor", S::String( 1, 256 ) );
    S::AddProperty( schema, "limit", S::Integer( 1, 50 ) );
    return schema;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CAssetToolCatalogue::GetCount( void )
{GUCEF_TRACE;

    return g_toolCount;
}

/*-------------------------------------------------------------------------*/

const CAssetToolCatalogue::SEntry*
CAssetToolCatalogue::GetEntry( UInt32 index )
{GUCEF_TRACE;

    return index < g_toolCount ? &g_tools[ index ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CAssetToolCatalogue::SEntry*
CAssetToolCatalogue::Find( const CORE::CString& name )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<g_toolCount; ++i )
        if ( name == g_tools[ i ].name )
            return &g_tools[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolCatalogue::GetGroup( const CORE::CString& name )
{GUCEF_TRACE;

    const SEntry* entry = Find( name );
    return GUCEF_NULL != entry ? CORE::CString( entry->group ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolCatalogue::GetGroupOfJobKind( const CORE::CString& kind )
{GUCEF_TRACE;

    const CORE::CString group = GetGroup( kind );
    return group.IsNULLOrEmpty() ? CORE::CString( "store" ) : group;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolCatalogue::IsWriteTool( const CORE::CString& name )
{GUCEF_TRACE;

    const SEntry* entry = Find( name );
    return GUCEF_NULL == entry || COM::MCP_TOOLEFFECT_READ_ONLY != entry->effect;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode
CAssetToolCatalogue::BuildJobSchema( void )
{GUCEF_TRACE;

    CORE::CDataNode schema = COM::CMcpJobInfo::GetSchema();
    S::AddProperty( schema, "packId", S::Nullable( S::String() ) );
    S::AddProperty( schema, "resultUri", S::Nullable( S::String() ) );
    S::AddProperty( schema, "summary", S::Nullable( LooseObject( "The summary of the result of a job that ended" ) ) );
    S::AddProperty( schema, "hint", S::String() );
    return schema;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode
CAssetToolCatalogue::BuildInputSchema( const CORE::CString& name, const CAssetVocabularyInfo* vocabulary )
{GUCEF_TRACE;

    CORE::CDataNode schema = S::Object();
    if ( "pack_list" == name )
    {
        S::AddProperty( schema, "text", S::String( 0, 200, "Words in the id or title" ) );
        S::AddProperty( schema, "root", S::String( 1, 32, "Only manifests in this root" ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "pack_get" == name || "asset_explain_reimport" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        if ( "asset_explain_reimport" == name )
            S::AddProperty( schema, "assetPath", AssetPath(), true );
    }
    else
    if ( "pack_assets" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "state", S::Enum( List( "converted", "cached", "approximated", "failed", "refused", "skipped", "not-run" ) ) );
        S::AddProperty( schema, "glob", S::String( 1, 512 ) );
        S::AddProperty( schema, "importer", S::String( 1, 64 ) );
        S::AddProperty( schema, "kind", S::String( 1, 64 ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "asset_report" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "assetPath", AssetPath(), true );
        S::AddProperty( schema, "sections", S::Array( S::Enum( List( "report", "sidecar", "activity", "cache" ) ), 0, 4 ) );
    }
    else
    if ( "pack_activity" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "runIndex", S::Integer( 0, 1000, "0: the last run" ) );
        S::AddProperty( schema, "action", S::String( 1, 32 ) );
        S::AddProperty( schema, "reason", S::String( 1, 64 ) );
        S::AddProperty( schema, "glob", S::String( 1, 512 ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "pack_cook_outputs" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "profile", S::String( 1, 64 ) );
        S::AddProperty( schema, "assetPath", AssetPath() );
        S::AddProperty( schema, "sort", S::Enum( List( "inclusive-size", "name" ) ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "pipeline_plugins" == name )
    {
        S::AddProperty( schema, "kind", S::Enum( List( "importers", "exporters", "cookers", "postProcessors", "validators" ) ) );
    }
    else
    if ( "licence_check" == name || "licence_draft_record" == name )
    {
        S::AddProperty( schema, "packId", PackId() );
        S::AddProperty( schema, "sourcePath", RootedPath( "A rooted path <rootId>:<relative> in a source or staging root" ) );
        if ( "licence_draft_record" == name )
        {
            S::AddProperty( schema, "storeId", S::String( 1, 64, "An online store whose item gives the product and licence names" ) );
            S::AddProperty( schema, "itemId", S::String( 1, 128 ) );
        }
    }
    else
    if ( "convert_plan" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "assets", Globs() );
        S::AddProperty( schema, "cookTargets", CookTargets() );
        S::AddProperty( schema, "reimport", S::Boolean( "Ignore the cache", true, false ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "convert_pack" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "assets", Globs() );
        S::AddProperty( schema, "cookTargets", CookTargets() );
        S::AddProperty( schema, "profiles", Profiles() );
        S::AddProperty( schema, "force", S::Boolean( "Cook assets whose validation has errors", true, false ) );
        S::AddProperty( schema, "reimport", S::Boolean( "Ignore the cache", true, false ) );
        S::AddProperty( schema, "waitMs", WaitMs() );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "convert_file" == name )
    {
        S::AddProperty( schema, "sourcePath", RootedPath( "A rooted path of a file in a source or staging root" ), true );
        S::AddProperty( schema, "importer", S::String( 1, 64 ) );
        S::AddProperty( schema, "licenceRecordId", S::String( 1, 128, "The id of an existing licence record" ), true );
        S::AddProperty( schema, "cookTargets", CookTargets() );
        S::AddProperty( schema, "waitMs", WaitMs() );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "cook_pack" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "targets", CookTargets() );
        S::AddProperty( schema, "profiles", Profiles() );
        S::AddProperty( schema, "assets", Globs() );
        S::AddProperty( schema, "waitMs", WaitMs() );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "sync_plan" == name )
    {
        S::AddProperty( schema, "storeId", S::String( 1, 64 ), true );
        S::AddProperty( schema, "waitMs", WaitMs() );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "sync_status" == name )
    {
        S::AddProperty( schema, "jobId", S::String( 1, 64 ) );
        S::AddProperty( schema, "storeId", S::String( 1, 64 ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "sync_marks" == name )
    {
        S::AddProperty( schema, "storeId", S::String( 1, 64 ), true );
        AddPaging( schema, 100 );
    }
    else
    if ( "validate_pack" == name )
    {
        CORE::CDataNode validator = S::Object();
        S::AddProperty( validator, "name", S::String( 1, 64 ), true );
        S::AddProperty( validator, "settings", LooseObject( "The settings of the validator" ) );
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "assets", Globs() );
        S::AddProperty( schema, "validators", S::Array( validator, 0, 16, "Replace the validators of the manifest for this run" ) );
        S::AddProperty( schema, "waitMs", WaitMs() );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "compare_routes" == name )
    {
        CORE::CDataNode route = S::Object();
        S::AddProperty( route, "label", S::String( 0, 32 ) );
        S::AddProperty( route, "packId", PackId(), true );
        S::AddProperty( route, "assetPath", AssetPath(), true );
        S::AddProperty( route, "stage", S::Enum( List( "intermediate", "cooked" ) ) );
        S::AddProperty( route, "profile", S::String( 1, 64 ) );
        S::AddProperty( schema, "routes", S::Array( route, 2, 4 ), true );
        S::AddProperty( schema, "tolerance", S::Number( 0.0, 0.5, "Relative tolerance for sizes" ) );
        S::AddProperty( schema, "render", S::Boolean( "Add a contact sheet per route", true, false ) );
    }
    else
    if ( "job_status" == name || "job_cancel" == name )
    {
        S::AddProperty( schema, "jobId", S::String( 1, 64 ), true );
        if ( "job_cancel" == name )
            S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "job_result" == name )
    {
        S::AddProperty( schema, "jobId", S::String( 1, 64 ), true );
        S::AddProperty( schema, "section", S::Enum( List( "summary", "failures", "approximations", "assets" ) ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "job_list" == name )
    {
        S::AddProperty( schema, "state", S::Enum( List( "queued", "working", "completed", "failed", "cancelled", "interrupted" ) ) );
        S::AddProperty( schema, "kind", S::String( 1, 64 ) );
        S::AddProperty( schema, "packId", PackId() );
        AddPaging( schema, 100 );
    }
    else
    if ( "render_sheet" == name )
    {
        CORE::CDataNode target = S::Object();
        S::AddProperty( target, "packId", PackId() );
        S::AddProperty( target, "assetPath", AssetPath() );
        S::AddProperty( target, "stage", S::Enum( List( "source", "intermediate", "cooked" ) ) );
        S::AddProperty( target, "profile", S::String( 1, 64 ) );
        S::AddProperty( target, "storeAssetId", S::String( 1, 128 ) );
        S::AddProperty( schema, "targets", S::Array( target, 1, 4 ), true );
        S::AddProperty( schema, "tile", S::Integer( 128, 512, "Tile size in pixels" ) );
    }
    else
    if ( "manifest_propose" == name )
    {
        S::AddProperty( schema, "sourcePath", RootedPath( "A rooted path of a folder or archive in a source or staging root" ), true );
        S::AddProperty( schema, "packId", PackId() );
        S::AddProperty( schema, "store", S::String( 0, 64 ) );
        S::AddProperty( schema, "title", S::String( 0, 200 ) );
    }
    else
    if ( "manifest_create" == name )
    {
        S::AddProperty( schema, "target", RootedPath( "Where the new manifest goes: a rooted path in a manifests root that does not exist" ), true );
        S::AddProperty( schema, "manifest", LooseObject( "The manifest" ), true );
        AddWrite( schema );
    }
    else
    if ( "manifest_edit" == name )
    {
        CORE::CDataNode operation = S::Object();
        S::AddProperty( operation, "op", S::Enum( List( "set", "remove", "append" ) ), true );
        S::AddProperty( operation, "path", S::String( 1, 512, "A JSON pointer into the manifest" ), true );
        S::AddProperty( operation, "value", AnyValue( "The value for set and append" ) );
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "operations", S::Array( operation, 1, 64 ), true );
        AddWrite( schema );
    }
    else
    if ( "override_write" == name || "override_remove" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "assetPath", AssetPath(), true );
        if ( "override_write" == name )
            S::AddProperty( schema, "settings", LooseObject( "The import settings of the asset" ), true );
        AddWrite( schema );
    }
    else
    if ( "material_map_suggest" == name )
    {
        S::AddProperty( schema, "packId", PackId(), true );
        S::AddProperty( schema, "shaderName", S::String( 1, 128 ) );
        S::AddProperty( schema, "assets", Globs() );
        S::AddProperty( schema, "withTextures", S::Boolean( "Add sheets of the slot textures where the licence and the service allow", true, false ) );
    }
    else
    if ( "store_search" == name )
    {
        return StoreSearchInput( vocabulary );
    }
    else
    if ( "store_asset" == name )
    {
        S::AddProperty( schema, "storeAssetId", S::String( 1, 128 ), true );
    }
    else
    if ( "store_pack" == name )
    {
        S::AddProperty( schema, "storePackId", S::String( 1, 128 ), true );
    }
    else
    if ( "store_licence" == name )
    {
        S::AddProperty( schema, "licenceId", S::String( 1, 128 ) );
        S::AddProperty( schema, "storePackId", S::String( 1, 128 ) );
    }
    else
    if ( "store_vocabulary" == name )
    {
        S::AddProperty( schema, "axis", S::Enum( List( "kind", "category", "theme", "style" ) ) );
    }
    else
    if ( "store_saved_searches" == name || "transfer_queue" == name )
    {
        if ( "transfer_queue" == name )
            S::AddProperty( schema, "state", S::Enum( List( "queued", "running", "paused", "finishing", "done", "failed", "cancelled" ) ) );
        AddPaging( schema, 100 );
    }
    else
    if ( "store_unclassified" == name )
    {
        S::AddProperty( schema, "level", S::Enum( List( "pack", "asset" ) ), true );
        S::AddProperty( schema, "kind", S::String( 1, 64 ) );
        S::AddProperty( schema, "storePackId", S::String( 1, 128 ) );
        AddPaging( schema, 50 );
    }
    else
    if ( "store_save_search" == name )
    {
        S::AddProperty( schema, "name", S::String( 1, 128 ), true );
        S::AddProperty( schema, "query", StoreSearchInput( vocabulary ), true );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "store_delete_search" == name )
    {
        S::AddProperty( schema, "searchId", S::String( 1, 128 ), true );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "store_classify" == name )
    {
        CORE::CDataNode item = S::Object();
        S::AddProperty( item, "storeAssetId", S::String( 1, 128 ) );
        S::AddProperty( item, "storePackId", S::String( 1, 128 ) );
        S::AddProperty( item, "categoryId", S::String( 1, 256 ) );
        S::AddProperty( item, "themes", S::StringList( 16, 64 ) );
        S::AddProperty( item, "artStyle", S::String( 1, 64 ) );
        S::AddProperty( schema, "items", S::Array( item, 1, 200 ), true );
        AddWrite( schema );
    }
    else
    if ( "store_edit_rules" == name )
    {
        CORE::CDataNode operation = S::Object();
        S::AddProperty( operation, "op", S::Enum( List( "upsert", "remove" ) ), true );
        S::AddProperty( operation, "table", S::Enum( List( "category_rules", "category_map" ) ), true );
        S::AddProperty( operation, "key", S::String( 1, 256, "category_rules: the rule id; category_map: <sourceId>/<store category>" ), true );
        S::AddProperty( operation, "value", LooseObject( "category_rules: matchOn, pattern, categoryId, kind, themes, artStyles; category_map: categoryId, theme, kind" ) );
        S::AddProperty( schema, "operations", S::Array( operation, 1, 64 ), true );
        AddWrite( schema );
    }
    else
    if ( "store_reclassify" == name )
    {
        S::AddProperty( schema, "storePackIds", S::StringList( 1000, 128 ) );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "store_ingest" == name )
    {
        S::AddProperty( schema, "path", RootedPath( "A rooted path of a pack folder or archive in a source or staging root" ), true );
        S::AddProperty( schema, "keep", S::Enum( List( "original", "converted", "both" ) ), true );
        S::AddProperty( schema, "locationId", S::String( 1, 128 ) );
        S::AddProperty( schema, "duplicatePolicy", S::Enum( List( "skip", "keep" ) ) );
        S::AddProperty( schema, "waitMs", WaitMs() );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "store_ingest_resume" == name )
    {
        S::AddProperty( schema, "ingestId", S::String( 1, 128 ), true );
        S::AddProperty( schema, "waitMs", WaitMs() );
        S::AddProperty( schema, "requestKey", RequestKey() );
    }
    else
    if ( "online_library" == name )
    {
        S::AddProperty( schema, "storeId", S::String( 1, 64 ), true );
        S::AddProperty( schema, "text", S::String( 0, 200 ) );
        S::AddProperty( schema, "listingType", S::String( 1, 64 ) );
        S::AddProperty( schema, "formatGroup", S::String( 1, 64 ) );
        S::AddProperty( schema, "sort", S::String( 1, 32 ) );
        AddPaging( schema, 50 );
    }
    else
    if ( "online_item" == name )
    {
        S::AddProperty( schema, "storeId", S::String( 1, 64 ), true );
        S::AddProperty( schema, "itemId", S::String( 1, 128 ), true );
    }
    // store_locations and online_stores take no arguments
    return schema;
}

/*-------------------------------------------------------------------------*/

CORE::CDataNode
CAssetToolCatalogue::BuildOutputSchema( const CORE::CString& name )
{GUCEF_TRACE;

    const SEntry* entry = Find( name );
    if ( GUCEF_NULL != entry && FAMILY_JOB_START == entry->family )
        return BuildJobSchema();
    if ( "job_status" == name || "job_cancel" == name )
        return BuildJobSchema();
    if ( GUCEF_NULL != entry && FAMILY_PIPELINE_CHANGE == entry->family )
        return WriteOutput();
    if ( "store_classify" == name || "store_edit_rules" == name )
        return WriteOutput();
    if ( "pack_list" == name || "pack_assets" == name || "pack_activity" == name || "pack_cook_outputs" == name ||
         "job_list" == name || "store_search" == name || "store_saved_searches" == name || "store_unclassified" == name ||
         "store_locations" == name || "online_stores" == name || "online_library" == name || "transfer_queue" == name ||
         "sync_marks" == name )
        return ListOutput( "A page of entries" );
    if ( "job_result" == name )
    {
        CORE::CDataNode schema = LooseObject( "The result of a job: the summary or a page of a section" );
        S::AddProperty( schema, "job", BuildJobSchema(), true );
        S::AddProperty( schema, "section", S::String(), true );
        return schema;
    }
    if ( "render_sheet" == name )
    {
        CORE::CDataNode schema = LooseObject( "Per target the sheet, or why there is none" );
        S::AddProperty( schema, "targets", S::Array( LooseObject( "A target" ) ), true );
        S::AddProperty( schema, "supported", S::Boolean(), true );
        return schema;
    }
    if ( "pack_get" == name || "asset_report" == name || "asset_explain_reimport" == name )
    {
        CORE::CDataNode schema = LooseObject( "The pack or asset" );
        S::AddProperty( schema, "packId", S::String(), true );
        return schema;
    }
    if ( "convert_plan" == name )
    {
        CORE::CDataNode schema = LooseObject( "The plan" );
        S::AddProperty( schema, "packId", S::String(), true );
        S::AddProperty( schema, "counts", LooseObject( "Assets per planned action" ), true );
        return schema;
    }
    if ( "compare_routes" == name )
    {
        CORE::CDataNode schema = LooseObject( "The routes and their differences" );
        S::AddProperty( schema, "routes", S::Array( LooseObject( "A route" ) ), true );
        return schema;
    }
    return LooseObject( "The result" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolCatalogue::BuildDescriptor( const CORE::CString& name, const CAssetVocabularyInfo* vocabulary, COM::CMcpToolDescriptor& descriptor )
{GUCEF_TRACE;

    const SEntry* entry = Find( name );
    if ( GUCEF_NULL == entry )
        return false;
    descriptor = COM::CMcpToolDescriptor();
    descriptor.name = entry->name;
    descriptor.title = entry->title;
    descriptor.description = entry->description;
    if ( FAMILY_JOB_START == entry->family )
        descriptor.description += JobNote;
    descriptor.effect = entry->effect;
    descriptor.idempotent = entry->idempotent;
    descriptor.openWorld = entry->openWorld;
    descriptor.inputSchema = BuildInputSchema( name, vocabulary );
    descriptor.outputSchema = BuildOutputSchema( name );
    descriptor.hasOutputSchema = true;
    descriptor.secretArguments.insert( "changeToken" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
