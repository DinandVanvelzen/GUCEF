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

#ifndef GUCEF_ASSETSTORE_CASSETVOCABULARY_H
#define GUCEF_ASSETSTORE_CASSETVOCABULARY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCLASSIFICATION_H
#include "gucefASSETSTORE_CAssetClassification.h"
#define GUCEF_ASSETSTORE_CASSETCLASSIFICATION_H
#endif /* GUCEF_ASSETSTORE_CASSETCLASSIFICATION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H
#include "gucefASSETSTORE_CAssetKeywordNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H ? */
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

/**
 *  The vocabulary of the primary store (sections 7.2 and 9.5) and what normalisation looks up in it: the kinds, the
 *  category tree, the themes and the art styles; the mapping of store categories onto them (category_map); the rules
 *  for items without a mappable store category (category_rules); the licence classes with their restriction flags
 *  and the rules that give a licence its class and flags (licence_map); the stop words and synonyms of the keywords
 *  (synonyms: a stop word is a word with an empty replacement).
 *
 *  The vocabulary is data: the seed is JSON in the data folder of the module (vocabulary.json, category_map.json,
 *  saved_searches.json), written into the tables by migration 2 of the catalogue, and the tables are what is used and
 *  what the owner edits. Ids in the categories table: category ids as paths ("weapons/melee"), the other axes with
 *  their axis as prefix ("kind:texture", "theme:fantasy", "style:low-poly").
 *  Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetVocabulary
{
    public:

    CAssetVocabulary( void );
    ~CAssetVocabulary();

    /** Reads the seed files from the VFS folder; false with the reason when one of them cannot be read */
    bool LoadSeed( const CORE::CString& seedVfsFolder, CORE::CString& error );

    bool HasSeed( void ) const;

    /** Writes the seed into the tables of the catalogue: migration 2 */
    TAssetStoreStatus WriteSeedRows( CAssetCatalogue& catalogue );

    /** Takes the vocabulary from the tables of the catalogue */
    TAssetStoreStatus LoadFromCatalogue( const CAssetCatalogue& catalogue );

    /** The keyword normaliser with the stop words and synonyms of the vocabulary */
    const CAssetKeywordNormalizer& GetKeywordNormalizer( void ) const;

    /**
     *  The mapping of a store category, or of the nearest of its parents that has one ("3D/Weapons & Combat/Melee",
     *  then "3D/Weapons & Combat", then "3D"); false when none has a mapping
     */
    bool MapStoreCategory( const CORE::CString& sourceId, const CORE::CString& storeCategory, CAssetClassification& classification ) const;

    /**
     *  The category rules applied to the keywords (normalised), the names (of the pack, its folders and files) and the
     *  seller: the best category of the rules that match (highest priority, then the deepest category), and the themes,
     *  art styles and kind of every rule that matches. False when no rule matches
     */
    bool ApplyRules( const CORE::CString::StringSet& keywords, const CORE::CString::StringVector& names, const CORE::CString& seller, CAssetClassification& classification ) const;

    /**
     *  Gives a licence its class (when it has none or "unknown") and restriction flags from the licence map: the rules on
     *  its name and its text, and the flags of its class. Flags the licence has already are kept. Sets the licence id
     */
    void ClassifyLicence( CAssetLicence& licence ) const;

    /** The restriction flags of a licence class */
    CORE::CString::StringSet GetClassRestrictions( const CORE::CString& licenceClass ) const;

    bool IsKnownCategory( const CORE::CString& categoryId ) const;

    /** The ids of an axis: "category", "kind", "theme", "style"; categories as paths, the others without their prefix */
    void GetTerms( const CORE::CString& axis, CORE::CString::StringVector& ids ) const;

    /** The source categories that a mapping knows for the source */
    UInt32 GetMappingCount( const CORE::CString& sourceId ) const;

    /** A store category as the mapping keys it: lower case, parts separated by '/' without blanks around them */
    static CORE::CString NormalizeStoreCategory( const CORE::CString& storeCategory );

    /** Adds or replaces a mapping, in the catalogue and here */
    TAssetStoreStatus AddCategoryMapping( CAssetCatalogue& catalogue, const CORE::CString& sourceId, const CORE::CString& storeCategory,
                                          const CORE::CString& categoryId, const CORE::CString& theme, const CORE::CString& kind );

    /** Adds or replaces a synonym; an empty replacement makes the word a stop word */
    TAssetStoreStatus AddSynonym( CAssetCatalogue& catalogue, const CORE::CString& word, const CORE::CString& replacement );

    /** Adds or replaces a category rule; matchOn is "keyword", "name" or "seller" */
    TAssetStoreStatus AddCategoryRule( CAssetCatalogue& catalogue, const CORE::CString& ruleId, const CORE::CString& matchOn,
                                       const CORE::CString& pattern, const CORE::CString& categoryId, const CORE::CString& kind,
                                       const CORE::CString::StringSet& themes, const CORE::CString::StringSet& artStyles, Int64 priority );

    /** Adds or replaces a licence rule; matchOn is "name", "text" or "class" (the flags of a class) */
    TAssetStoreStatus AddLicenceRule( CAssetCatalogue& catalogue, const CORE::CString& ruleId, const CORE::CString& matchOn,
                                      const CORE::CString& pattern, const CORE::CString& licenceClass,
                                      const CORE::CString::StringSet& restrictions, Int64 priority );

    private:

    struct SMapping
    {
        CORE::CString categoryId;
        CORE::CString theme;
        CORE::CString kind;
    };

    struct SRule
    {
        CORE::CString ruleId;
        CORE::CString matchOn;
        CORE::CString pattern;
        CORE::CString::StringVector patternWords;
        CORE::CString categoryId;
        CORE::CString kind;
        CORE::CString::StringSet themes;
        CORE::CString::StringSet artStyles;
        CORE::CString licenceClass;
        CORE::CString::StringSet restrictions;
        Int64 priority;
    };

    typedef GUCEF::map< CORE::CString, SMapping > TMappingMap;
    typedef GUCEF::vector< SRule > TRuleVector;
    typedef GUCEF::map< CORE::CString, CAssetCatalogue::TRowVector > TSeedRowMap;

    void BuildFromRows( const TSeedRowMap& rows );
    void AddCategoryRows( const CORE::CDataNode& node, const CORE::CString& parentId, const CORE::CString& axis, CAssetCatalogue::TRowVector& rows, Int64& order ) const;
    void AddTermRows( const CORE::CDataNode* list, const CORE::CString& axis, CAssetCatalogue::TRowVector& rows ) const;
    SRule MakeRule( const CAssetCatalogue::TRow& row ) const;

    CAssetVocabulary( const CAssetVocabulary& src );              /**< not implemented */
    CAssetVocabulary& operator=( const CAssetVocabulary& src );   /**< not implemented */

    private:

    TSeedRowMap m_seedRows;
    CAssetKeywordNormalizer m_normalizer;
    TMappingMap m_mappings;
    TRuleVector m_categoryRules;
    TRuleVector m_licenceRules;
    CORE::CString::StringMapSet m_classRestrictions;
    CORE::CString::StringMap m_terms;               /**< table id to axis */
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

#endif /* GUCEF_ASSETSTORE_CASSETVOCABULARY_H ? */
