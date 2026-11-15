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

#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#include "gucefASSETSTORE_CAssetVocabulary.h"
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */
#include "gucefASSETSTORE_CAssetDiskPackNormalizer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetDiskPackNormalizer::CAssetDiskPackNormalizer( void )
    : CAssetStoreItemNormalizer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetDiskPackNormalizer::~CAssetDiskPackNormalizer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetDiskPackNormalizer::GetName( void ) const
{GUCEF_TRACE;

    return "diskPack";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetDiskPackNormalizer::GetVersion( void ) const
{GUCEF_TRACE;

    return "1";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetDiskPackNormalizer::TitleFromName( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString title = CAssetNameHints::IsArchiveExtension( CAssetNameHints::GetExtension( name ) ) ? CAssetNameHints::GetBaseName( name ) : name;
    const Int32 slash = title.ReplaceChar( '\\', '/' ).HasChar( '/', false );
    if ( slash >= 0 )
        title = title.CutChars( (UInt32) slash + 1, true );
    title = title.ReplaceChar( '_', ' ' ).ReplaceChar( '-', ' ' ).ReplaceChar( '.', ' ' ).CompactRepeatingChar( ' ' );
    return title.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

void
CAssetDiskPackNormalizer::ApplyRules( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack )
{GUCEF_TRACE;

    CORE::CString::StringSet keywords;
    for ( size_t k=0; k<pack.keywords.size(); ++k )
        if ( pack.keywords[ k ].field != "description" )
            keywords.insert( pack.keywords[ k ].keyword );
    CORE::CString::StringVector names;
    names.push_back( pack.title );
    names.push_back( input.packName );
    for ( size_t f=0; f<input.fileNames.size(); ++f )
        names.push_back( CAssetNameHints::GetBaseName( input.fileNames[ f ] ) );
    CAssetClassification classification;
    if ( !vocabulary.ApplyRules( keywords, names, pack.seller, classification ) )
        return;
    if ( pack.categories.empty() )
        pack.categories.insert( classification.categories.begin(), classification.categories.end() );
    pack.themes.insert( classification.themes.begin(), classification.themes.end() );
    pack.artStyles.insert( classification.artStyles.begin(), classification.artStyles.end() );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetDiskPackNormalizer::Normalize( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack )
{GUCEF_TRACE;

    // a pack.json, or the name, as the item of a store would be
    CAssetNormalizationInput itemInput( input );
    CAssetStoreItem& item = itemInput.item;
    if ( item.title.IsNULLOrEmpty() )
        item.title = TitleFromName( input.packName );
    if ( input.hasPackJson )
    {
        const CORE::CDataNode& json = input.packJson;
        const CORE::CString title = json.GetAttributeValueOrChildValueByName( "title" ).AsString();
        if ( !title.IsNULLOrEmpty() )
            item.title = title;
        item.seller = json.GetAttributeValueOrChildValueByName( "seller" ).AsString( json.GetAttributeValueOrChildValueByName( "author" ).AsString() );
        item.description = json.GetAttributeValueOrChildValueByName( "description" ).AsString();
        item.url = json.GetAttributeValueOrChildValueByName( "url" ).AsString();
        item.version = json.GetAttributeValueOrChildValueByName( "version" ).AsString();
        json.GetValuesOfChildByName( "tags", item.tags );
        json.GetValuesOfChildByName( "categories", item.categories );
        const CORE::CDataNode* licence = json.FindChild( "licence" );
        if ( GUCEF_NULL != licence )
        {
            item.licenceName = licence->GetAttributeValueOrChildValueByName( "name" ).AsString();
            item.licenceUrl = licence->GetAttributeValueOrChildValueByName( "url" ).AsString();
            item.licenceText = licence->GetAttributeValueOrChildValueByName( "text" ).AsString();
            licence->GetValuesOfChildByName( "restrictions", item.licenceRestrictions );
        }
    }
    const CORE::CString itemId = item.itemId;
    MapFields( item, pack );
    pack.sourceItemId = itemId;
    MapCategories( input.sourceId, vocabulary, pack );
    MapLicence( item, vocabulary, pack );
    MakeKeywords( vocabulary, pack );
    ApplyRules( itemInput, vocabulary, pack );
    pack.normalizer = GetName() + "/" + GetVersion();
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
