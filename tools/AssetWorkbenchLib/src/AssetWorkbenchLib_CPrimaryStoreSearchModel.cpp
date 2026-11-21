/*
 *  AssetWorkbenchLib: the asset sources of the workbench: online stores, the primary store and the transfers
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_GUI_CTRANSFERQUEUEFORM_H
#include "gucefGUI_CTransferQueueForm.h"
#define GUCEF_GUI_CTRANSFERQUEUEFORM_H
#endif /* GUCEF_GUI_CTRANSFERQUEUEFORM_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

#ifndef ASSETWORKBENCHLIB_CPRIMARYSTORESEARCHMODEL_H
#include "AssetWorkbenchLib_CPrimaryStoreSearchModel.h"
#define ASSETWORKBENCHLIB_CPRIMARYSTORESEARCHMODEL_H
#endif /* ASSETWORKBENCHLIB_CPRIMARYSTORESEARCHMODEL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


const CORE::CString CPrimaryStoreSearchModel::PackKeyPrefix = "pack:";

namespace {

const char* const GroupKind = "kind";
const char* const GroupCategory = "category";
const char* const GroupSource = "source";
const char* const GroupLicence = "licence";
const char* const GroupFormat = "format";
const char* const GroupFlags = "flags";
const char* const GroupAllows = "allows";
const char* const RangeSize = "megabytes";
const char* const RangeTriangles = "triangles";
const char* const RangeTexture = "texture";

const Float64 BytesPerMegabyte = 1048576.0;

struct SAllowed
{
    const char* value;
    const char* label;
    const char* flag;
};

const SAllowed Allowed[] =
{
    { "machineLearning", "Machine learning", "noMachineLearning" },
    { "noAttribution", "No attribution needed", "attributionRequired" },
    { "commercial", "Commercial use", "noCommercialUse" },
    { "redistribution", "Redistribution", "noRedistribution" }
};

struct SFlag
{
    const char* value;
    const char* label;
};

const SFlag Flags[] = { { "rigged", "Rigged" }, { "animated", "Animated" }, { "lods", "Has LODs" }, { "collision", "Has collision" } };

const char* const FormatIds[] = { "gltf", "glb", "fbx", "obj", "usd", "blend", "dae", "stl", "unreal", "unity" };

CORE::CString
NiceName( const CORE::CString& id )
{
    CORE::CString text = id.ReplaceChar( '-', ' ' ).ReplaceChar( '/', ' ' );
    if ( text.Length() > 0 )
        text = text.SubstrToIndex( 1, true ).Uppercase() + text.CutChars( 1, true );
    return text;
}

void
AddSet( GUI::CSearchQuery& query, const char* group, const CORE::CString::StringSet& values )
{
    for ( CORE::CString::StringSet::const_iterator i = values.begin(); i != values.end(); ++i )
        query.AddChoice( group, *i );
}

void
GetSet( const GUI::CSearchQuery& query, const char* group, CORE::CString::StringSet& values )
{
    values.clear();
    GUI::CSearchQuery::TChoiceMap::const_iterator i = query.choices.find( group );
    if ( i != query.choices.end() )
        values = (*i).second;
}

void
SetRangeOf( GUI::CSearchQuery& query, const char* group, Int64 minimum, Int64 maximum, Float64 divisor )
{
    GUI::CSearchQuery::TRange range;
    if ( minimum >= 0 )
    {
        range.hasMinimum = true;
        range.minimum = (Float64) minimum / divisor;
    }
    if ( maximum >= 0 )
    {
        range.hasMaximum = true;
        range.maximum = (Float64) maximum / divisor;
    }
    query.SetRange( group, range );
}

void
GetRangeOf( const GUI::CSearchQuery& query, const char* group, Int64& minimum, Int64& maximum, Float64 factor )
{
    const GUI::CSearchQuery::TRange range = query.GetRange( group );
    minimum = range.hasMinimum ? (Int64) ( range.minimum * factor ) : -1;
    maximum = range.hasMaximum ? (Int64) ( range.maximum * factor ) : -1;
}

}

/*-------------------------------------------------------------------------*/

CPrimaryStoreSearchModel::CPrimaryStoreSearchModel( ASSETSTORE::CIAssetStoreService& service )
    : GUI::CISearchModel()
    , m_service( service )
    , m_sourceIds()
    , m_vocabulary()
    , m_haveVocabulary( false )
    , m_vocabularyFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_savedSearches()
    , m_savedSearchesFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_savedSearchChanges()
    , m_suggestions()
    , m_suggestionOrder()
    , m_suggestRequests()
    , m_pending()
    , m_hits()
    , m_remoteThumbnails( GUCEF_NULL )
    , m_nextRequestId( 1 )
{GUCEF_TRACE;

    TEventCallback callback( this, &CPrimaryStoreSearchModel::OnCatalogueChanged );
    SubscribeTo( &m_service, ASSETSTORE::CIAssetStoreService::CatalogueChangedEvent, callback );
    if ( !m_service.IsInProcess() )
    {
        // nothing waits on the GUI thread: what the form asks synchronously is cached from the start
        RequestVocabulary();
        RequestSavedSearches();
    }

}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::SetRemoteThumbnails( CRemoteThumbnailSource* source )
{GUCEF_TRACE;

    m_remoteThumbnails = source;
}

/*-------------------------------------------------------------------------*/

CPrimaryStoreSearchModel::~CPrimaryStoreSearchModel()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::SetSourceIds( const CORE::CString::StringVector& sourceIds )
{GUCEF_TRACE;

    if ( sourceIds == m_sourceIds )
        return;
    m_sourceIds = sourceIds;
    NotifyObservers( FilterDescriptionChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::ToStoreQuery( const GUI::CSearchQuery& query, ASSETSTORE::CAssetSearchQuery& storeQuery )
{GUCEF_TRACE;

    storeQuery.Clear();
    // what the form does not edit comes from the extra
    CORE::CDataNodeSerializableSettings serializerSettings;
    if ( query.extra.HasChildren() || query.extra.AttributeBegin() != query.extra.AttributeEnd() )
        storeQuery.Deserialize( query.extra, serializerSettings );

    storeQuery.text = query.text;
    storeQuery.prefixLastWord = true;
    GetSet( query, GroupKind, storeQuery.kinds );
    GetSet( query, GroupCategory, storeQuery.categories );
    GetSet( query, GroupSource, storeQuery.sources );
    GetSet( query, GroupLicence, storeQuery.licenceClasses );
    GetSet( query, GroupFormat, storeQuery.formats );
    CORE::CString::StringSet flags;
    GetSet( query, GroupFlags, flags );
    storeQuery.rigged = flags.find( "rigged" ) != flags.end() ? 1 : -1;
    storeQuery.animated = flags.find( "animated" ) != flags.end() ? 1 : -1;
    storeQuery.hasLods = flags.find( "lods" ) != flags.end() ? 1 : -1;
    storeQuery.hasCollision = flags.find( "collision" ) != flags.end() ? 1 : -1;
    CORE::CString::StringSet allows;
    GetSet( query, GroupAllows, allows );
    storeQuery.excludeLicenceRestrictions.clear();
    for ( UInt32 i=0; i<sizeof( Allowed ) / sizeof( Allowed[ 0 ] ); ++i )
        if ( allows.find( Allowed[ i ].value ) != allows.end() )
            storeQuery.excludeLicenceRestrictions.insert( Allowed[ i ].flag );
    GetRangeOf( query, RangeSize, storeQuery.minAssetBytes, storeQuery.maxAssetBytes, BytesPerMegabyte );
    GetRangeOf( query, RangeTriangles, storeQuery.minTriangles, storeQuery.maxTriangles, 1.0 );
    GetRangeOf( query, RangeTexture, storeQuery.minTextureResolution, storeQuery.maxTextureResolution, 1.0 );
    if ( !query.sort.IsNULLOrEmpty() )
        storeQuery.sort = query.sort;
    storeQuery.pageIndex = query.pageIndex;
    storeQuery.pageSize = query.pageSize > 0 ? query.pageSize : 50;
    storeQuery.withFacets = true;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::FromStoreQuery( const ASSETSTORE::CAssetSearchQuery& storeQuery, GUI::CSearchQuery& query )
{GUCEF_TRACE;

    query.Clear();
    query.text = storeQuery.text;
    AddSet( query, GroupKind, storeQuery.kinds );
    AddSet( query, GroupCategory, storeQuery.categories );
    AddSet( query, GroupSource, storeQuery.sources );
    AddSet( query, GroupLicence, storeQuery.licenceClasses );
    AddSet( query, GroupFormat, storeQuery.formats );
    if ( 1 == storeQuery.rigged )
        query.AddChoice( GroupFlags, "rigged" );
    if ( 1 == storeQuery.animated )
        query.AddChoice( GroupFlags, "animated" );
    if ( 1 == storeQuery.hasLods )
        query.AddChoice( GroupFlags, "lods" );
    if ( 1 == storeQuery.hasCollision )
        query.AddChoice( GroupFlags, "collision" );
    for ( UInt32 i=0; i<sizeof( Allowed ) / sizeof( Allowed[ 0 ] ); ++i )
        if ( storeQuery.excludeLicenceRestrictions.find( Allowed[ i ].flag ) != storeQuery.excludeLicenceRestrictions.end() )
            query.AddChoice( GroupAllows, Allowed[ i ].value );
    SetRangeOf( query, RangeSize, storeQuery.minAssetBytes, storeQuery.maxAssetBytes, BytesPerMegabyte );
    SetRangeOf( query, RangeTriangles, storeQuery.minTriangles, storeQuery.maxTriangles, 1.0 );
    SetRangeOf( query, RangeTexture, storeQuery.minTextureResolution, storeQuery.maxTextureResolution, 1.0 );
    query.sort = storeQuery.sort;
    query.pageSize = (UInt32) storeQuery.pageSize;
    CORE::CDataNodeSerializableSettings serializerSettings;
    storeQuery.Serialize( query.extra, serializerSettings );
    query.extra.SetName( "extra" );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::GetFilterDescription( GUI::CSearchFilterDescription& description ) const
{GUCEF_TRACE;

    description.Clear();
    ASSETSTORE::CAssetVocabularyInfo vocabulary;
    if ( m_service.IsInProcess() )
        m_service.GetVocabulary( vocabulary );
    else
        vocabulary = m_vocabulary;

    CORE::CString::StringVector terms;
    description.AddChoiceGroup( GroupKind, "Kind" );
    terms = vocabulary.kinds;
    for ( size_t i=0; i<terms.size(); ++i )
        description.AddChoice( GroupKind, terms[ i ], NiceName( terms[ i ] ) );

    description.AddChoiceGroup( GroupCategory, "Category" );
    terms = vocabulary.categories;
    for ( size_t i=0; i<terms.size(); ++i )
        if ( terms[ i ].HasChar( '/', true ) < 0 )
            description.AddChoice( GroupCategory, terms[ i ], NiceName( terms[ i ] ) );

    description.AddChoiceGroup( GroupSource, "Source" );
    description.AddChoice( GroupSource, "disk", "Disk" );
    for ( size_t i=0; i<m_sourceIds.size(); ++i )
        if ( m_sourceIds[ i ] != "disk" )
            description.AddChoice( GroupSource, m_sourceIds[ i ], NiceName( m_sourceIds[ i ] ) );

    description.AddChoiceGroup( GroupFormat, "Format" );
    for ( UInt32 i=0; i<sizeof( FormatIds ) / sizeof( FormatIds[ 0 ] ); ++i )
        description.AddChoice( GroupFormat, FormatIds[ i ], CORE::CString( FormatIds[ i ] ).Uppercase() );

    description.AddChoiceGroup( GroupFlags, "Model" );
    for ( UInt32 i=0; i<sizeof( Flags ) / sizeof( Flags[ 0 ] ); ++i )
        description.AddChoice( GroupFlags, Flags[ i ].value, Flags[ i ].label );

    description.AddChoiceGroup( GroupLicence, "Licence class" );
    terms = vocabulary.licenceClasses;
    if ( terms.empty() )
    {
        terms.push_back( "public-domain" );
        terms.push_back( "attribution" );
        terms.push_back( "royalty-free" );
        terms.push_back( "noncommercial" );
        terms.push_back( "proprietary" );
        terms.push_back( "unknown" );
    }
    for ( size_t i=0; i<terms.size(); ++i )
        description.AddChoice( GroupLicence, terms[ i ], NiceName( terms[ i ] ) );

    description.AddChoiceGroup( GroupAllows, "The licence allows" ).tooltip = "Leaves out the packs whose licence has the restriction";
    for ( UInt32 i=0; i<sizeof( Allowed ) / sizeof( Allowed[ 0 ] ); ++i )
        description.AddChoice( GroupAllows, Allowed[ i ].value, Allowed[ i ].label );

    description.AddRangeGroup( RangeSize, "Size of the file", "MB" );
    description.AddRangeGroup( RangeTriangles, "Polygons", "triangles" );
    description.AddRangeGroup( RangeTexture, "Texture size", "px" );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::GetResultColumns( TColumnVector& columns ) const
{GUCEF_TRACE;

    columns.clear();
    columns.push_back( TColumn( "title", "Name", 40 ) );
    columns.push_back( TColumn( "kind", "Kind", 20 ) );
    columns.push_back( TColumn( "pack", "Pack", 28 ) );
    columns.push_back( TColumn( "size", "Size", 12 ) );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::GetSortChoices( TSortChoiceVector& choices ) const
{GUCEF_TRACE;

    choices.clear();
    choices.push_back( TSortChoice( "relevance", "Relevance" ) );
    choices.push_back( TSortChoice( "name", "Name" ) );
    choices.push_back( TSortChoice( "size", "Size" ) );
    choices.push_back( TSortChoice( "triangles", "Polygons" ) );
    choices.push_back( TSortChoice( "newest", "Newest" ) );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::GetSuggestions( const CORE::CString& prefix, UInt32 maxCount, CORE::CString::StringVector& suggestions ) const
{GUCEF_TRACE;

    suggestions.clear();
    if ( !m_service.IsCatalogueAvailable() || prefix.IsNULLOrEmpty() )
        return;
    if ( m_service.IsInProcess() )
    {
        ASSETSTORE::CAssetServiceStringList keywords;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK == m_service.Suggest( prefix, maxCount, keywords ) )
            suggestions = keywords.values;
        return;
    }

    // a service that is not in process: the cache answers, a prefix it does not have is asked for in the background
    TSuggestionMap::const_iterator cached = m_suggestions.find( prefix );
    if ( cached != m_suggestions.end() )
    {
        for ( size_t i=0; i<(*cached).second.size() && i<maxCount; ++i )
            suggestions.push_back( (*cached).second[ i ] );
        for ( size_t i=0; i<m_suggestionOrder.size(); ++i )
        {
            if ( m_suggestionOrder[ i ] == prefix )
            {
                m_suggestionOrder.erase( m_suggestionOrder.begin() + i );
                break;
            }
        }
        m_suggestionOrder.push_back( prefix );
        return;
    }
    for ( size_t i=0; i<m_suggestRequests.size(); ++i )
        if ( m_suggestRequests[ i ].prefix == prefix )
            return;
    m_suggestRequests.push_back( SSuggestRequest( prefix, m_service.SuggestAsync( prefix, 64 ) ) );
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryStoreSearchModel::SupportsSavedSearches( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::GetSavedSearches( GUI::TSavedSearchVector& searches ) const
{GUCEF_TRACE;

    searches.clear();
    if ( !m_service.IsInProcess() )
    {
        ToGenericSavedSearches( m_savedSearches, searches );
        return;
    }
    ASSETSTORE::CAssetSavedSearchPage stored;
    if ( !m_service.IsCatalogueAvailable() || ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.GetSavedSearches( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries, stored ) )
        return;
    ToGenericSavedSearches( stored.entries, searches );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::ToGenericSavedSearches( const ASSETSTORE::TAssetSavedSearchVector& stored, GUI::TSavedSearchVector& searches ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<stored.size(); ++i )
    {
        GUI::CSavedSearch saved;
        saved.id = stored[ i ].searchId;
        saved.name = stored[ i ].name;
        saved.builtIn = stored[ i ].precanned;
        FromStoreQuery( stored[ i ].query, saved.query );
        searches.push_back( saved );
    }
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryStoreSearchModel::SaveSearch( const GUI::CSavedSearch& search )
{GUCEF_TRACE;

    if ( !m_service.IsCatalogueAvailable() )
        return false;
    ASSETSTORE::CAssetSavedSearch stored;
    // a search that is new gets an id of its own from the time and a counter
    static UInt32 counter = 0;
    stored.searchId = search.id.IsNULLOrEmpty() ? "user-" + CORE::UInt64ToString( (UInt64) MT::PrecisionTickCount() ) + "-" + CORE::UInt32ToString( ++counter ) : search.id;
    stored.sortOrder = 10000;
    stored.name = search.name;
    stored.precanned = false;
    ToStoreQuery( search.query, stored.query );
    stored.query.pageIndex = 0;
    if ( !m_service.IsInProcess() )
    {
        // the change is shown at once; the list of the service follows when it refused
        bool replaced = false;
        for ( size_t i=0; i<m_savedSearches.size(); ++i )
        {
            if ( m_savedSearches[ i ].searchId == stored.searchId )
            {
                m_savedSearches[ i ] = stored;
                replaced = true;
            }
        }
        if ( !replaced )
            m_savedSearches.push_back( stored );
        m_savedSearchChanges.push_back( m_service.SaveSearchAsync( CORE::CString::Empty, stored ) );
        NotifyObservers( SavedSearchesChangedEvent );
        return true;
    }
    ASSETSTORE::CAssetServiceResult result;
    const ASSETSTORE::TAssetStoreStatus status = m_service.SaveSearch( CORE::CString::Empty, stored, result );
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CPrimaryStoreSearchModel: The search \"" << search.name << "\" cannot be saved: " << ASSETSTORE::AssetStoreStatusToString( status ) << ", " << result.message );
        return false;
    }
    NotifyObservers( SavedSearchesChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryStoreSearchModel::DeleteSavedSearch( const CORE::CString& searchId )
{GUCEF_TRACE;

    if ( !m_service.IsCatalogueAvailable() )
        return false;
    if ( !m_service.IsInProcess() )
    {
        for ( size_t i=0; i<m_savedSearches.size(); ++i )
        {
            if ( m_savedSearches[ i ].searchId == searchId )
            {
                m_savedSearches.erase( m_savedSearches.begin() + i );
                break;
            }
        }
        m_savedSearchChanges.push_back( m_service.DeleteSavedSearchAsync( CORE::CString::Empty, searchId ) );
        NotifyObservers( SavedSearchesChangedEvent );
        return true;
    }
    ASSETSTORE::CAssetServiceResult result;
    if ( ASSETSTORE::ASSETSTORE_STATUS_OK != m_service.DeleteSavedSearch( CORE::CString::Empty, searchId, result ) )
        return false;
    NotifyObservers( SavedSearchesChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CPrimaryStoreSearchModel::RequestSearch( const GUI::CSearchQuery& query )
{GUCEF_TRACE;

    const UInt32 requestId = m_nextRequestId++;
    ASSETSTORE::CAssetSearchQuery storeQuery;
    ToStoreQuery( query, storeQuery );
    if ( !m_service.IsCatalogueAvailable() )
    {
        TSearchCompletedEventData data;
        data.GetData().requestId = requestId;
        data.GetData().succeeded = false;
        data.GetData().message = "The primary store is not open";
        NotifyObservers( SearchCompletedEvent, &data );
        return requestId;
    }
    m_pending.push_back( SPending( requestId, m_service.SearchAsync( storeQuery ) ) );
    return requestId;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::Complete( UInt32 requestId, const CORE::CFutureResult& future )
{GUCEF_TRACE;

    TSearchCompletedEventData data;
    TSearchCompletion& completion = data.GetData();
    completion.requestId = requestId;
    CORE::CTaskPtr task = future.GetResult( 0 );
    const ASSETSTORE::CAssetSearchResultPage* page = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetSearchResultPage >( task->GetTaskData() );
    if ( GUCEF_NULL == page || CORE::TASKSTATUS_FINISHED != task->GetTaskStatus() )
    {
        completion.succeeded = false;
        completion.message = CORE::CString( "The search failed: " ) + ASSETSTORE::AssetStoreStatusToString( ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( future ) );
        NotifyObservers( SearchCompletedEvent, &data );
        return;
    }
    completion.succeeded = true;
    completion.page.total = page->total;
    completion.page.pageIndex = (UInt32) page->pageIndex;
    completion.page.pageSize = (UInt32) page->pageSize;
    m_hits.clear();
    for ( size_t i=0; i<page->hits.size(); ++i )
    {
        const ASSETSTORE::CAssetSearchHit& hit = page->hits[ i ];
        GUI::CSearchResultPage::TRow row;
        row.key = hit.assetId.IsNULLOrEmpty() ? PackKeyPrefix + hit.packId : hit.assetId;
        row.title = hit.name;
        row.typeName = hit.kind.IsNULLOrEmpty() ? CORE::CString( "pack" ) : hit.kind;
        row.subtitle = row.typeName + ( hit.packTitle.IsNULLOrEmpty() ? CORE::CString::Empty : " - " + hit.packTitle );
        row.tooltip = hit.packTitle + ( hit.seller.IsNULLOrEmpty() ? CORE::CString::Empty : " by " + hit.seller );
        row.thumbnailKey = GUCEF_NULL != m_remoteThumbnails ? m_remoteThumbnails->ToClientPath( hit.vfsPath ) : hit.vfsPath;
        if ( GUCEF_NULL != m_remoteThumbnails )
            m_remoteThumbnails->NoteAsset( row.thumbnailKey, hit.assetId.IsNULLOrEmpty() ? "pack:" + hit.packId : hit.assetId );
        row.available = hit.available;
        row.values[ "kind" ] = row.typeName;
        row.values[ "pack" ] = hit.packTitle;
        row.values[ "size" ] = GUI::CTransferQueueForm::FormatSize( hit.sizeBytes > 0 ? (UInt64) hit.sizeBytes : 0 );
        completion.page.rows.push_back( row );
        m_hits[ row.key ] = hit;
    }
    // the groups of the facets of the store have other ids than the groups of the form
    ASSETSTORE::CAssetSearchResultPage::TFacetMap::const_iterator f;
    for ( f = page->facets.begin(); f != page->facets.end(); ++f )
    {
        const CORE::CString group = "licenceClass" == (*f).first ? CORE::CString( GroupLicence ) : (*f).first;
        completion.page.facets[ group ] = (*f).second;
    }
    NotifyObservers( SearchCompletedEvent, &data );
}

/*-------------------------------------------------------------------------*/

UInt32
CPrimaryStoreSearchModel::Update( void )
{GUCEF_TRACE;

    UInt32 answered = 0;
    size_t i = 0;
    while ( i < m_pending.size() )
    {
        const SPending pending = m_pending[ i ];
        if ( pending.future.IsReady() || pending.future.HasNoFuture() )
        {
            m_pending.erase( m_pending.begin() + i );
            Complete( pending.requestId, pending.future );
            ++answered;
        }
        else
            ++i;
    }
    TakeBackgroundAnswers();
    return answered;
}

/*-------------------------------------------------------------------------*/

UInt32
CPrimaryStoreSearchModel::TakeBackgroundAnswers( void )
{GUCEF_TRACE;

    UInt32 taken = 0;

    // the vocabulary
    if ( m_vocabularyFuture.IsReady() )
    {
        ASSETSTORE::CAssetVocabularyInfo vocabulary;
        if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_vocabularyFuture, vocabulary ) )
        {
            m_vocabulary = vocabulary;
            m_haveVocabulary = true;
            NotifyObservers( FilterDescriptionChangedEvent );
        }
        m_vocabularyFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        ++taken;
    }

    // the saved searches, and the changes of them that were sent
    bool refused = false;
    size_t c = 0;
    while ( c < m_savedSearchChanges.size() )
    {
        const CORE::CFutureResult change = m_savedSearchChanges[ c ];
        if ( change.IsReady() || change.HasNoFuture() )
        {
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK != ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( change ) )
            {
                refused = true;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CPrimaryStoreSearchModel: The service refused a change of the saved searches: " << ASSETSTORE::CIAssetStoreService::GetMessageOfFuture( change ) );
            }
            m_savedSearchChanges.erase( m_savedSearchChanges.begin() + c );
            ++taken;
        }
        else
            ++c;
    }
    if ( refused )
        RequestSavedSearches();
    if ( m_savedSearchesFuture.IsReady() )
    {
        ASSETSTORE::CAssetSavedSearchPage page;
        if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( m_savedSearchesFuture, page ) )
        {
            m_savedSearches = page.entries;
            NotifyObservers( SavedSearchesChangedEvent );
        }
        m_savedSearchesFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
        ++taken;
    }

    // the suggestions of prefixes that were not in the cache
    bool arrived = false;
    size_t r = 0;
    while ( r < m_suggestRequests.size() )
    {
        const SSuggestRequest request = m_suggestRequests[ r ];
        if ( request.future.IsReady() || request.future.HasNoFuture() )
        {
            ASSETSTORE::CAssetServiceStringList keywords;
            if ( ASSETSTORE::CIAssetStoreService::TryGetFutureResult( request.future, keywords ) )
            {
                if ( m_suggestions.find( request.prefix ) == m_suggestions.end() )
                    m_suggestionOrder.push_back( request.prefix );
                m_suggestions[ request.prefix ] = keywords.values;
                while ( m_suggestionOrder.size() > SuggestionCacheSize )
                {
                    m_suggestions.erase( m_suggestionOrder.front() );
                    m_suggestionOrder.erase( m_suggestionOrder.begin() );
                }
                arrived = true;
            }
            m_suggestRequests.erase( m_suggestRequests.begin() + r );
            ++taken;
        }
        else
            ++r;
    }
    if ( arrived )
        NotifyObservers( SuggestionsChangedEvent );
    return taken;
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::RequestVocabulary( void )
{GUCEF_TRACE;

    m_vocabularyFuture = m_service.GetVocabularyAsync();
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::RequestSavedSearches( void )
{GUCEF_TRACE;

    m_savedSearchesFuture = m_service.GetSavedSearchesAsync( CORE::CString::Empty, ASSETSTORE::CIAssetStoreService::MaxPageEntries );
}

/*-------------------------------------------------------------------------*/

void
CPrimaryStoreSearchModel::OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a service in this process is asked when the form needs something: nothing is cached that could go stale
    if ( m_service.IsInProcess() )
        return;
    const ASSETSTORE::CAssetCatalogueChange* change = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetCatalogueChange >( eventData );
    if ( GUCEF_NULL == change )
        return;
    if ( ASSETSTORE::ASSETCHANGE_VOCABULARY == change->kind || ASSETSTORE::ASSETCHANGE_RESYNC == change->kind )
    {
        m_suggestions.clear();
        m_suggestionOrder.clear();
        RequestVocabulary();
    }
    if ( ASSETSTORE::ASSETCHANGE_SAVED_SEARCHES == change->kind || ASSETSTORE::ASSETCHANGE_RESYNC == change->kind )
        RequestSavedSearches();
}

/*-------------------------------------------------------------------------*/

UInt32
CPrimaryStoreSearchModel::GetCachedSuggestionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_suggestions.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CPrimaryStoreSearchModel::GetPendingSuggestionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_suggestRequests.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CPrimaryStoreSearchModel::GetPendingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

bool
CPrimaryStoreSearchModel::FindHit( const CORE::CString& key, ASSETSTORE::CAssetSearchHit& hit ) const
{GUCEF_TRACE;

    THitMap::const_iterator i = m_hits.find( key );
    if ( i == m_hits.end() )
        return false;
    hit = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPrimaryStoreSearchModel::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CPrimaryStoreSearchModel";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
