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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */
#include "gucefASSETSTORE_CAssetVocabulary.h"

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

CORE::CString
TextOf( const CORE::CDataNode& node, const char* name )
{
    return node.GetAttributeValueOrChildValueByName( name ).AsString();
}

CORE::CString
SetOf( const CORE::CDataNode& node, const char* name )
{
    CORE::CString::StringSet values;
    node.GetValuesOfChildByName( name, values );
    return CAssetCatalogue::JoinSet( values );
}

CORE::CString
JoinPath( const CORE::CString& parent, const CORE::CString& id )
{
    return parent.IsNULLOrEmpty() ? id : parent + "/" + id;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetVocabulary::CAssetVocabulary( void )
    : m_seedRows()
    , m_normalizer()
    , m_mappings()
    , m_categoryRules()
    , m_licenceRules()
    , m_classRestrictions()
    , m_terms()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetVocabulary::~CAssetVocabulary()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetVocabulary::NormalizeStoreCategory( const CORE::CString& storeCategory )
{GUCEF_TRACE;

    const CORE::CString::StringVector parts = CAssetKeywordNormalizer::Fold( storeCategory ).ReplaceChar( '>', '/' ).ReplaceChar( '\\', '/' ).ParseElements( '/', false );
    CORE::CString result;
    for ( size_t p=0; p<parts.size(); ++p )
    {
        const CORE::CString part = parts[ p ].Trim( true ).Trim( false );
        if ( part.IsNULLOrEmpty() )
            continue;
        if ( !result.IsNULLOrEmpty() )
            result += '/';
        result += part;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

void
CAssetVocabulary::AddCategoryRows( const CORE::CDataNode& node, const CORE::CString& parentId, const CORE::CString& axis, CAssetCatalogue::TRowVector& rows, Int64& order ) const
{GUCEF_TRACE;

    const CORE::CString id = JoinPath( parentId, TextOf( node, "id" ) );
    CAssetCatalogue::TRow row;
    row[ "category_id" ] = CORE::CVariant( id );
    row[ "axis" ] = CORE::CVariant( axis );
    row[ "parent_id" ] = CORE::CVariant( parentId );
    row[ "name" ] = CORE::CVariant( TextOf( node, "name" ) );
    row[ "sort_order" ] = CORE::CVariant( order++ );
    rows.push_back( row );

    const CORE::CDataNode* children = node.FindChild( "children" );
    if ( GUCEF_NULL == children )
        return;
    CORE::CDataNode::const_iterator i = children->ConstBegin();
    while ( i != children->ConstEnd() )
    {
        AddCategoryRows( *(*i), id, axis, rows, order );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetVocabulary::AddTermRows( const CORE::CDataNode* list, const CORE::CString& axis, CAssetCatalogue::TRowVector& rows ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == list )
        return;
    Int64 order = 0;
    CORE::CDataNode::const_iterator i = list->ConstBegin();
    while ( i != list->ConstEnd() )
    {
        CAssetCatalogue::TRow row;
        row[ "category_id" ] = CORE::CVariant( axis + ":" + TextOf( *(*i), "id" ) );
        row[ "axis" ] = CORE::CVariant( axis );
        row[ "parent_id" ] = CORE::CVariant( CORE::CString::Empty );
        row[ "name" ] = CORE::CVariant( TextOf( *(*i), "name" ) );
        row[ "sort_order" ] = CORE::CVariant( order++ );
        rows.push_back( row );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetVocabulary::LoadSeed( const CORE::CString& seedVfsFolder, CORE::CString& error )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDataNode vocabulary;
    CORE::CDataNode categoryMap;
    CORE::CDataNode savedSearches;
    const CORE::CString folder = seedVfsFolder.ReplaceChar( '\\', '/' );
    if ( !vfs.LoadFile( vocabulary, folder + "/vocabulary.json", "json" ) )
    {
        error = "the seed file " + folder + "/vocabulary.json cannot be read";
        return false;
    }
    if ( !vfs.LoadFile( categoryMap, folder + "/category_map.json", "json" ) )
    {
        error = "the seed file " + folder + "/category_map.json cannot be read";
        return false;
    }
    if ( !vfs.LoadFile( savedSearches, folder + "/saved_searches.json", "json" ) )
    {
        error = "the seed file " + folder + "/saved_searches.json cannot be read";
        return false;
    }

    TSeedRowMap rows;

    // the terms of every axis
    CAssetCatalogue::TRowVector& categories = rows[ "categories" ];
    const CORE::CDataNode* tree = vocabulary.FindChild( "categories" );
    Int64 order = 0;
    if ( GUCEF_NULL != tree )
    {
        CORE::CDataNode::const_iterator i = tree->ConstBegin();
        while ( i != tree->ConstEnd() )
        {
            AddCategoryRows( *(*i), CORE::CString::Empty, "category", categories, order );
            ++i;
        }
    }
    AddTermRows( vocabulary.FindChild( "kinds" ), "kind", categories );
    AddTermRows( vocabulary.FindChild( "themes" ), "theme", categories );
    AddTermRows( vocabulary.FindChild( "artStyles" ), "style", categories );

    // licence classes and licence rules
    CAssetCatalogue::TRowVector& licenceMap = rows[ "licence_map" ];
    const CORE::CDataNode* classes = vocabulary.FindChild( "licenceClasses" );
    if ( GUCEF_NULL != classes )
    {
        CORE::CDataNode::const_iterator i = classes->ConstBegin();
        while ( i != classes->ConstEnd() )
        {
            CAssetCatalogue::TRow row;
            const CORE::CString id = TextOf( *(*i), "id" );
            row[ "rule_id" ] = CORE::CVariant( "class:" + id );
            row[ "match_on" ] = CORE::CVariant( CORE::CString( "class" ) );
            row[ "pattern" ] = CORE::CVariant( id );
            row[ "licence_class" ] = CORE::CVariant( id );
            row[ "restrictions" ] = CORE::CVariant( SetOf( *(*i), "restrictions" ) );
            row[ "priority" ] = CORE::CVariant( (Int64) 0 );
            licenceMap.push_back( row );
            ++i;
        }
    }
    const CORE::CDataNode* licenceRules = vocabulary.FindChild( "licenceMap" );
    if ( GUCEF_NULL != licenceRules )
    {
        CORE::CDataNode::const_iterator i = licenceRules->ConstBegin();
        while ( i != licenceRules->ConstEnd() )
        {
            CAssetCatalogue::TRow row;
            row[ "rule_id" ] = CORE::CVariant( TextOf( *(*i), "id" ) );
            row[ "match_on" ] = CORE::CVariant( TextOf( *(*i), "matchOn" ).Lowercase() );
            row[ "pattern" ] = CORE::CVariant( TextOf( *(*i), "pattern" ).Lowercase() );
            row[ "licence_class" ] = CORE::CVariant( TextOf( *(*i), "class" ) );
            row[ "restrictions" ] = CORE::CVariant( SetOf( *(*i), "restrictions" ) );
            row[ "priority" ] = CORE::CVariant( (*i)->GetAttributeValueOrChildValueByName( "priority" ).AsInt64( 0, true ) );
            licenceMap.push_back( row );
            ++i;
        }
    }

    // stop words and synonyms
    CAssetCatalogue::TRowVector& synonyms = rows[ "synonyms" ];
    CORE::CString::StringSet stopWords;
    vocabulary.GetValuesOfChildByName( "stopWords", stopWords );
    CORE::CString::StringSet::const_iterator s = stopWords.begin();
    while ( s != stopWords.end() )
    {
        CAssetCatalogue::TRow row;
        row[ "word" ] = CORE::CVariant( CAssetKeywordNormalizer::Fold( (*s) ) );
        row[ "replacement" ] = CORE::CVariant( CORE::CString::Empty );
        synonyms.push_back( row );
        ++s;
    }
    const CORE::CDataNode* synonymNode = vocabulary.FindChild( "synonyms" );
    if ( GUCEF_NULL != synonymNode )
    {
        for ( UInt32 a=0; a<synonymNode->GetAttCount(); ++a )
        {
            const CORE::CDataNode::TKeyValuePair* pair = synonymNode->GetAttribute( a );
            CAssetCatalogue::TRow row;
            row[ "word" ] = CORE::CVariant( CAssetKeywordNormalizer::Fold( pair->first ) );
            row[ "replacement" ] = CORE::CVariant( CAssetKeywordNormalizer::Fold( pair->second.AsString() ) );
            synonyms.push_back( row );
        }
        CORE::CDataNode::const_iterator i = synonymNode->ConstBegin();
        while ( i != synonymNode->ConstEnd() )
        {
            CAssetCatalogue::TRow row;
            row[ "word" ] = CORE::CVariant( CAssetKeywordNormalizer::Fold( (*i)->GetName() ) );
            row[ "replacement" ] = CORE::CVariant( CAssetKeywordNormalizer::Fold( (*i)->GetValue().AsString() ) );
            synonyms.push_back( row );
            ++i;
        }
    }

    // category rules
    CAssetCatalogue::TRowVector& categoryRules = rows[ "category_rules" ];
    const CORE::CDataNode* rulesNode = vocabulary.FindChild( "categoryRules" );
    if ( GUCEF_NULL != rulesNode )
    {
        CORE::CDataNode::const_iterator i = rulesNode->ConstBegin();
        while ( i != rulesNode->ConstEnd() )
        {
            CAssetCatalogue::TRow row;
            row[ "rule_id" ] = CORE::CVariant( TextOf( *(*i), "id" ) );
            row[ "match_on" ] = CORE::CVariant( TextOf( *(*i), "matchOn" ).Lowercase() );
            row[ "pattern" ] = CORE::CVariant( TextOf( *(*i), "pattern" ).Lowercase() );
            row[ "category_id" ] = CORE::CVariant( TextOf( *(*i), "category" ) );
            row[ "kind" ] = CORE::CVariant( TextOf( *(*i), "kind" ) );
            row[ "themes" ] = CORE::CVariant( SetOf( *(*i), "themes" ) );
            row[ "art_styles" ] = CORE::CVariant( SetOf( *(*i), "artStyles" ) );
            row[ "priority" ] = CORE::CVariant( (*i)->GetAttributeValueOrChildValueByName( "priority" ).AsInt64( 0, true ) );
            categoryRules.push_back( row );
            ++i;
        }
    }

    // the mapping of store categories
    CAssetCatalogue::TRowVector& mappings = rows[ "category_map" ];
    const CORE::CDataNode* mappingNode = categoryMap.FindChild( "mappings" );
    if ( GUCEF_NULL != mappingNode )
    {
        CORE::CDataNode::const_iterator i = mappingNode->ConstBegin();
        while ( i != mappingNode->ConstEnd() )
        {
            CAssetCatalogue::TRow row;
            row[ "source_id" ] = CORE::CVariant( TextOf( *(*i), "source" ).Lowercase() );
            row[ "source_category" ] = CORE::CVariant( NormalizeStoreCategory( TextOf( *(*i), "category" ) ) );
            row[ "category_id" ] = CORE::CVariant( TextOf( *(*i), "to" ) );
            row[ "theme" ] = CORE::CVariant( TextOf( *(*i), "theme" ) );
            row[ "kind" ] = CORE::CVariant( TextOf( *(*i), "kind" ) );
            mappings.push_back( row );
            ++i;
        }
    }

    // the precanned searches
    CAssetCatalogue::TRowVector& searches = rows[ "saved_searches" ];
    const CORE::CDataNode* searchNode = savedSearches.FindChild( "searches" );
    if ( GUCEF_NULL != searchNode )
    {
        Int64 searchOrder = 0;
        CORE::CDataNode::const_iterator i = searchNode->ConstBegin();
        while ( i != searchNode->ConstEnd() )
        {
            const CORE::CDataNode* queryNode = (*i)->FindChild( "query" );
            CORE::CString queryJson = "{}";
            if ( GUCEF_NULL != queryNode && !CAssetStoreJson::ToText( *queryNode, queryJson ) )
            {
                error = "a precanned search cannot be written as JSON";
                return false;
            }
            CAssetCatalogue::TRow row;
            row[ "search_id" ] = CORE::CVariant( TextOf( *(*i), "id" ) );
            row[ "name" ] = CORE::CVariant( TextOf( *(*i), "name" ) );
            row[ "query_json" ] = CORE::CVariant( queryJson );
            row[ "precanned" ] = CORE::CVariant( true );
            row[ "sort_order" ] = CORE::CVariant( searchOrder++ );
            searches.push_back( row );
            ++i;
        }
    }

    {
        MT::CScopeMutex lock( m_lock );
        m_seedRows = rows;
    }
    BuildFromRows( rows );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetVocabulary::HasSeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_seedRows.empty();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetVocabulary::WriteSeedRows( CAssetCatalogue& catalogue )
{GUCEF_TRACE;

    TSeedRowMap rows;
    {
        MT::CScopeMutex lock( m_lock );
        rows = m_seedRows;
    }
    if ( rows.empty() )
        return ASSETSTORE_STATUS_NOT_FOUND;
    TSeedRowMap::const_iterator i = rows.begin();
    while ( i != rows.end() )
    {
        const TAssetStoreStatus status = catalogue.Write( (*i).first, (*i).second );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        ++i;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetVocabulary::LoadFromCatalogue( const CAssetCatalogue& catalogue )
{GUCEF_TRACE;

    static const char* const tables[] = { "categories", "category_map", "synonyms", "category_rules", "licence_map" };
    TSeedRowMap rows;
    for ( UInt32 t=0; t<sizeof( tables ) / sizeof( tables[ 0 ] ); ++t )
    {
        const TAssetStoreStatus status = catalogue.SelectAll( tables[ t ], rows[ tables[ t ] ] );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
    }
    BuildFromRows( rows );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CAssetVocabulary::SRule
CAssetVocabulary::MakeRule( const CAssetCatalogue::TRow& row ) const
{GUCEF_TRACE;

    SRule rule;
    rule.ruleId = CAssetCatalogue::GetText( row, "rule_id" );
    rule.matchOn = CAssetCatalogue::GetText( row, "match_on" ).Lowercase();
    rule.pattern = CAssetCatalogue::GetText( row, "pattern" ).Lowercase();
    rule.categoryId = CAssetCatalogue::GetText( row, "category_id" );
    rule.kind = CAssetCatalogue::GetText( row, "kind" );
    rule.themes = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "themes" ) );
    rule.artStyles = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "art_styles" ) );
    rule.licenceClass = CAssetCatalogue::GetText( row, "licence_class" );
    rule.restrictions = CAssetCatalogue::SplitSet( CAssetCatalogue::GetText( row, "restrictions" ) );
    rule.priority = CAssetCatalogue::GetInt( row, "priority" );
    if ( rule.matchOn == "keyword" )
        CAssetKeywordNormalizer::SplitWords( rule.pattern, rule.patternWords );
    return rule;
}

/*-------------------------------------------------------------------------*/

void
CAssetVocabulary::BuildFromRows( const TSeedRowMap& rows )
{GUCEF_TRACE;

    CORE::CString::StringSet stopWords;
    CORE::CString::StringMap synonyms;
    TSeedRowMap::const_iterator synonymRows = rows.find( "synonyms" );
    if ( synonymRows != rows.end() )
    {
        for ( size_t r=0; r<(*synonymRows).second.size(); ++r )
        {
            const CORE::CString word = CAssetCatalogue::GetText( (*synonymRows).second[ r ], "word" );
            const CORE::CString replacement = CAssetCatalogue::GetText( (*synonymRows).second[ r ], "replacement" );
            if ( replacement.IsNULLOrEmpty() )
                stopWords.insert( word );
            else
                synonyms[ word ] = replacement;
        }
    }
    m_normalizer.SetStopWords( stopWords );
    m_normalizer.SetSynonyms( synonyms );

    MT::CScopeMutex lock( m_lock );
    m_terms.clear();
    TSeedRowMap::const_iterator categoryRows = rows.find( "categories" );
    if ( categoryRows != rows.end() )
        for ( size_t r=0; r<(*categoryRows).second.size(); ++r )
            m_terms[ CAssetCatalogue::GetText( (*categoryRows).second[ r ], "category_id" ) ] = CAssetCatalogue::GetText( (*categoryRows).second[ r ], "axis" );

    m_mappings.clear();
    TSeedRowMap::const_iterator mappingRows = rows.find( "category_map" );
    if ( mappingRows != rows.end() )
    {
        for ( size_t r=0; r<(*mappingRows).second.size(); ++r )
        {
            const CAssetCatalogue::TRow& row = (*mappingRows).second[ r ];
            SMapping mapping;
            mapping.categoryId = CAssetCatalogue::GetText( row, "category_id" );
            mapping.theme = CAssetCatalogue::GetText( row, "theme" );
            mapping.kind = CAssetCatalogue::GetText( row, "kind" );
            m_mappings[ CAssetCatalogue::GetText( row, "source_id" ) + "\n" + CAssetCatalogue::GetText( row, "source_category" ) ] = mapping;
        }
    }

    m_categoryRules.clear();
    TSeedRowMap::const_iterator ruleRows = rows.find( "category_rules" );
    if ( ruleRows != rows.end() )
        for ( size_t r=0; r<(*ruleRows).second.size(); ++r )
            m_categoryRules.push_back( MakeRule( (*ruleRows).second[ r ] ) );

    m_licenceRules.clear();
    m_classRestrictions.clear();
    TSeedRowMap::const_iterator licenceRows = rows.find( "licence_map" );
    if ( licenceRows != rows.end() )
    {
        for ( size_t r=0; r<(*licenceRows).second.size(); ++r )
        {
            SRule rule = MakeRule( (*licenceRows).second[ r ] );
            if ( rule.matchOn == "class" )
                m_classRestrictions[ rule.licenceClass ] = rule.restrictions;
            else
                m_licenceRules.push_back( rule );
        }
    }
}

/*-------------------------------------------------------------------------*/

const CAssetKeywordNormalizer&
CAssetVocabulary::GetKeywordNormalizer( void ) const
{GUCEF_TRACE;

    return m_normalizer;
}

/*-------------------------------------------------------------------------*/

bool
CAssetVocabulary::MapStoreCategory( const CORE::CString& sourceId, const CORE::CString& storeCategory, CAssetClassification& classification ) const
{GUCEF_TRACE;

    CORE::CString path = NormalizeStoreCategory( storeCategory );
    const CORE::CString source = sourceId.Lowercase();
    MT::CScopeMutex lock( m_lock );
    while ( !path.IsNULLOrEmpty() )
    {
        TMappingMap::const_iterator i = m_mappings.find( source + "\n" + path );
        if ( i != m_mappings.end() )
        {
            if ( !(*i).second.categoryId.IsNULLOrEmpty() )
                classification.categories.insert( (*i).second.categoryId );
            if ( !(*i).second.theme.IsNULLOrEmpty() )
                classification.themes.insert( (*i).second.theme );
            if ( !(*i).second.kind.IsNULLOrEmpty() && classification.kind.IsNULLOrEmpty() )
                classification.kind = (*i).second.kind;
            return true;
        }
        const Int32 slash = path.HasChar( '/', false );
        path = slash > 0 ? path.SubstrToIndex( (UInt32) slash, true ) : CORE::CString::Empty;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetVocabulary::ApplyRules( const CORE::CString::StringSet& keywords, const CORE::CString::StringVector& names, const CORE::CString& seller, CAssetClassification& classification ) const
{GUCEF_TRACE;

    const CORE::CString lowerSeller = CAssetKeywordNormalizer::Fold( seller );
    bool matched = false;
    const SRule* best = GUCEF_NULL;
    MT::CScopeMutex lock( m_lock );
    for ( size_t r=0; r<m_categoryRules.size(); ++r )
    {
        const SRule& rule = m_categoryRules[ r ];
        bool matches = false;
        if ( rule.matchOn == "keyword" )
        {
            matches = keywords.end() != keywords.find( rule.pattern );
            if ( !matches && rule.patternWords.size() > 1 )
            {
                matches = true;
                for ( size_t w=0; w<rule.patternWords.size() && matches; ++w )
                {
                    const CORE::CString word = m_normalizer.NormalizeWord( rule.patternWords[ w ] );
                    matches = !word.IsNULLOrEmpty() && keywords.end() != keywords.find( word );
                }
            }
        }
        else
        if ( rule.matchOn == "name" )
        {
            for ( size_t n=0; n<names.size() && !matches; ++n )
                matches = CAssetKeywordNormalizer::Fold( names[ n ] ).WildcardEquals( rule.pattern, '*', true );
        }
        else
        if ( rule.matchOn == "seller" )
        {
            matches = !lowerSeller.IsNULLOrEmpty() && lowerSeller.WildcardEquals( rule.pattern, '*', true );
        }
        if ( !matches )
            continue;
        matched = true;
        classification.themes.insert( rule.themes.begin(), rule.themes.end() );
        classification.artStyles.insert( rule.artStyles.begin(), rule.artStyles.end() );
        if ( !rule.kind.IsNULLOrEmpty() && classification.kind.IsNULLOrEmpty() )
            classification.kind = rule.kind;
        if ( rule.categoryId.IsNULLOrEmpty() )
            continue;
        if ( GUCEF_NULL == best || rule.priority > best->priority ||
             ( rule.priority == best->priority && rule.categoryId.GetCharacterCount( '/' ) > best->categoryId.GetCharacterCount( '/' ) ) )
            best = &rule;
    }
    if ( GUCEF_NULL != best )
        classification.categories.insert( best->categoryId );
    return matched;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet
CAssetVocabulary::GetClassRestrictions( const CORE::CString& licenceClass ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString::StringMapSet::const_iterator i = m_classRestrictions.find( licenceClass );
    return i == m_classRestrictions.end() ? CORE::CString::StringSet() : (*i).second;
}

/*-------------------------------------------------------------------------*/

void
CAssetVocabulary::ClassifyLicence( CAssetLicence& licence ) const
{GUCEF_TRACE;

    const CORE::CString name = CAssetKeywordNormalizer::Fold( licence.name );
    const CORE::CString text = CAssetKeywordNormalizer::Fold( licence.text );
    const bool needsClass = licence.licenceClass.IsNULLOrEmpty() || licence.licenceClass == "unknown";
    const SRule* best = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t r=0; r<m_licenceRules.size(); ++r )
        {
            const SRule& rule = m_licenceRules[ r ];
            const CORE::CString& subject = rule.matchOn == "text" ? text : name;
            if ( subject.IsNULLOrEmpty() || !subject.WildcardEquals( rule.pattern, '*', true ) )
                continue;
            licence.restrictions.insert( rule.restrictions.begin(), rule.restrictions.end() );
            if ( !rule.licenceClass.IsNULLOrEmpty() && ( GUCEF_NULL == best || rule.priority > best->priority ) )
                best = &rule;
        }
        if ( needsClass )
            licence.licenceClass = GUCEF_NULL != best ? best->licenceClass : CORE::CString( "unknown" );
    }
    const CORE::CString::StringSet classRestrictions = GetClassRestrictions( licence.licenceClass );
    licence.restrictions.insert( classRestrictions.begin(), classRestrictions.end() );
    if ( licence.licenceId.IsNULLOrEmpty() )
        licence.licenceId = licence.MakeLicenceId();
}

/*-------------------------------------------------------------------------*/

bool
CAssetVocabulary::IsKnownCategory( const CORE::CString& categoryId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString::StringMap::const_iterator i = m_terms.find( categoryId );
    return i != m_terms.end() && (*i).second == "category";
}

/*-------------------------------------------------------------------------*/

void
CAssetVocabulary::GetTerms( const CORE::CString& axis, CORE::CString::StringVector& ids ) const
{GUCEF_TRACE;

    ids.clear();
    MT::CScopeMutex lock( m_lock );
    CORE::CString::StringMap::const_iterator i = m_terms.begin();
    while ( i != m_terms.end() )
    {
        if ( (*i).second == axis )
            ids.push_back( axis == "category" ? (*i).first : (*i).first.CutChars( axis.Length() + 1, true ) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetVocabulary::GetMappingCount( const CORE::CString& sourceId ) const
{GUCEF_TRACE;

    const CORE::CString prefix = sourceId.Lowercase() + "\n";
    UInt32 count = 0;
    MT::CScopeMutex lock( m_lock );
    TMappingMap::const_iterator i = m_mappings.begin();
    while ( i != m_mappings.end() )
    {
        if ( 0 == (*i).first.HasSubstr( prefix, true ) )
            ++count;
        ++i;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetVocabulary::AddCategoryMapping( CAssetCatalogue& catalogue, const CORE::CString& sourceId, const CORE::CString& storeCategory,
                                      const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind )
{GUCEF_TRACE;

    CAssetCatalogue::TRowVector rows( 1 );
    rows[ 0 ][ "source_id" ] = CORE::CVariant( sourceId.Lowercase() );
    rows[ 0 ][ "source_category" ] = CORE::CVariant( NormalizeStoreCategory( storeCategory ) );
    rows[ 0 ][ "category_id" ] = CORE::CVariant( categoryId );
    rows[ 0 ][ "theme" ] = CORE::CVariant( theme );
    rows[ 0 ][ "kind" ] = CORE::CVariant( kind );
    const TAssetStoreStatus status = catalogue.Write( "category_map", rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    SMapping mapping;
    mapping.categoryId = categoryId;
    mapping.theme = theme;
    mapping.kind = kind;
    MT::CScopeMutex lock( m_lock );
    m_mappings[ sourceId.Lowercase() + "\n" + NormalizeStoreCategory( storeCategory ) ] = mapping;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetVocabulary::AddSynonym( CAssetCatalogue& catalogue, const CORE::CString& word, const CORE::CString& replacement )
{GUCEF_TRACE;

    CAssetCatalogue::TRowVector rows( 1 );
    rows[ 0 ][ "word" ] = CORE::CVariant( CAssetKeywordNormalizer::Fold( word ) );
    rows[ 0 ][ "replacement" ] = CORE::CVariant( CAssetKeywordNormalizer::Fold( replacement ) );
    const TAssetStoreStatus status = catalogue.Write( "synonyms", rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    return LoadFromCatalogue( catalogue );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetVocabulary::AddCategoryRule( CAssetCatalogue& catalogue, const CORE::CString& ruleId, const CORE::CString& matchOn,
                                   const CORE::CString& pattern, const CORE::CString& categoryId, const CORE::CString& kind,
                                   const CORE::CString::StringSet& themes, const CORE::CString::StringSet& artStyles, Int64 priority )
{GUCEF_TRACE;

    CAssetCatalogue::TRowVector rows( 1 );
    rows[ 0 ][ "rule_id" ] = CORE::CVariant( ruleId );
    rows[ 0 ][ "match_on" ] = CORE::CVariant( matchOn.Lowercase() );
    rows[ 0 ][ "pattern" ] = CORE::CVariant( pattern.Lowercase() );
    rows[ 0 ][ "category_id" ] = CORE::CVariant( categoryId );
    rows[ 0 ][ "kind" ] = CORE::CVariant( kind );
    rows[ 0 ][ "themes" ] = CORE::CVariant( CAssetCatalogue::JoinSet( themes ) );
    rows[ 0 ][ "art_styles" ] = CORE::CVariant( CAssetCatalogue::JoinSet( artStyles ) );
    rows[ 0 ][ "priority" ] = CORE::CVariant( priority );
    const TAssetStoreStatus status = catalogue.Write( "category_rules", rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    return LoadFromCatalogue( catalogue );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetVocabulary::AddLicenceRule( CAssetCatalogue& catalogue, const CORE::CString& ruleId, const CORE::CString& matchOn,
                                  const CORE::CString& pattern, const CORE::CString& licenceClass,
                                  const CORE::CString::StringSet& restrictions, Int64 priority )
{GUCEF_TRACE;

    CAssetCatalogue::TRowVector rows( 1 );
    rows[ 0 ][ "rule_id" ] = CORE::CVariant( ruleId );
    rows[ 0 ][ "match_on" ] = CORE::CVariant( matchOn.Lowercase() );
    rows[ 0 ][ "pattern" ] = CORE::CVariant( matchOn.Lowercase() == "class" ? pattern : pattern.Lowercase() );
    rows[ 0 ][ "licence_class" ] = CORE::CVariant( licenceClass );
    rows[ 0 ][ "restrictions" ] = CORE::CVariant( CAssetCatalogue::JoinSet( restrictions ) );
    rows[ 0 ][ "priority" ] = CORE::CVariant( priority );
    const TAssetStoreStatus status = catalogue.Write( "licence_map", rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    return LoadFromCatalogue( catalogue );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
