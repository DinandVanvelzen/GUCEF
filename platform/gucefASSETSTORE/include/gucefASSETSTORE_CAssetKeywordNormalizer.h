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

#ifndef GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H
#define GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETKEYWORD_H
#include "gucefASSETSTORE_CAssetKeyword.h"
#define GUCEF_ASSETSTORE_CASSETKEYWORD_H
#endif /* GUCEF_ASSETSTORE_CASSETKEYWORD_H ? */
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

typedef GUCEF::vector< CAssetKeyword > TAssetKeywordVector;

/**
 *  Turns text into the keywords of the keyword index (section 7.2), the same way for what is indexed and for what is
 *  searched:
 *    - case folding, and accents removed from the Latin letters (é to e, ß to ss, æ to ae): text is UTF-8; letters of
 *      other scripts are kept as they are
 *    - split on everything that is not a letter or a digit, on camel case (MetalCrate: metal, crate; HTTPServer:
 *      http, server) and between letters and digits (Wall01: wall)
 *    - words of digits only, words of one letter and stop words are dropped (the stop words of the vocabulary hold the
 *      English filler words and the name hint prefixes such as sm and sk)
 *    - light English stemming: plurals in s and es (crates: crate, boxes: box, bodies: body; glass, bus and axis stay)
 *    - the synonym table: a word with a synonym becomes the synonym (bld: building), before and after stemming
 *  "SM_Bld_Apartment_01" gives "building" and "apartment". The words as they were are kept next to the keywords.
 *  Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetKeywordNormalizer
{
    public:

    CAssetKeywordNormalizer( void );

    void SetStopWords( const CORE::CString::StringSet& stopWords );
    void SetSynonyms( const CORE::CString::StringMap& synonyms );
    void AddSynonym( const CORE::CString& word, const CORE::CString& synonym );

    /**
     *  Adds the keywords of the text to 'keywords' with the field and the weight; a keyword that is there already for
     *  the field keeps the higher weight. Returns the number added
     */
    UInt32 Normalize( const CORE::CString& text, const CORE::CString& field, Int64 weight, TAssetKeywordVector& keywords ) const;

    /** The keywords of a query, in their order, without repeats */
    void NormalizeQuery( const CORE::CString& text, CORE::CString::StringVector& keywords ) const;

    /** One word: folded, stemmed and replaced by its synonym; empty when it is dropped */
    CORE::CString NormalizeWord( const CORE::CString& word ) const;

    /** The words of the text as split, before folding (for tests and suggestions) */
    static void SplitWords( const CORE::CString& text, CORE::CString::StringVector& words );

    /** Lower case without accents */
    static CORE::CString Fold( const CORE::CString& text );

    /** The light English stem of a folded word */
    static CORE::CString Stem( const CORE::CString& foldedWord );

    private:

    CORE::CString ApplySynonym( const CORE::CString& word ) const;

    private:

    CORE::CString::StringSet m_stopWords;
    CORE::CString::StringMap m_synonyms;
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

#endif /* GUCEF_ASSETSTORE_CASSETKEYWORDNORMALIZER_H ? */
