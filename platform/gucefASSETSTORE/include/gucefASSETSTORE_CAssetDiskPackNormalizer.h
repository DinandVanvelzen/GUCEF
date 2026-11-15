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

#ifndef GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H
#define GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H
#include "gucefASSETSTORE_CAssetStoreItemNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H ? */
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
 *  The metadata normaliser of packs from disk (section 7.2): the name of the folder or archive is the title (with
 *  separators made blanks), unless a pack.json at the root of the pack says more:
 *  @code
 *  { "title": "", "seller": "", "description": "", "url": "", "version": "", "tags": [], "categories": [],
 *    "licence": { "name": "", "url": "", "text": "", "restrictions": [ "noMachineLearning" ] } }
 *  @endcode
 *  Its categories go through the category map of the source "disk"; the names of the files take part in the category
 *  rules. The rest is as CAssetStoreItemNormalizer does it.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetDiskPackNormalizer : public CAssetStoreItemNormalizer
{
    public:

    CAssetDiskPackNormalizer( void );
    virtual ~CAssetDiskPackNormalizer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TAssetStoreStatus Normalize( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack ) GUCEF_VIRTUAL_OVERRIDE;

    /** A title from a folder or archive name: the extension off, '_', '-' and '.' made blanks */
    static CORE::CString TitleFromName( const CORE::CString& name );

    protected:

    virtual void ApplyRules( const CAssetNormalizationInput& input, const CAssetVocabulary& vocabulary, CAssetPackRecord& pack ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H ? */
