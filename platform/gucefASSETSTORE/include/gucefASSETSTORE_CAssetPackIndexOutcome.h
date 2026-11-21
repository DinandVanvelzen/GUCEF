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


#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H
#define GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

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
 *  The outcome of the indexing of a pack: what a converter hook gives back from CIAssetConverterHook::BuildPackIndex(), and
 *  the result data of CPrimaryAssetStore::IndexPackAsync().
 *
 *  'status' is OK when the document is there and loaded in the catalogue, another status when it is not (the message says
 *  why). 'reused' says that the stored document stood and nothing was built (work once); 'built' that the hook wrote one;
 *  'loaded' that the catalogue rows were written from it; 'derived' that it was made from the rows of the catalogue.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackIndexOutcome : public CORE::CICloneable
{
    public:

    CORE::CString jobId;
    CORE::CString packId;
    TAssetStoreStatus status;
    CORE::CString message;
    CORE::CString documentPath;                     /**< the VFS path of the manifest */
    CORE::CString indexKey;
    CORE::CString documentHash;
    bool reused;                                    /**< the stored document was fresh: nothing was built */
    bool built;                                     /**< the hook built and wrote a document */
    bool loaded;                                    /**< the rows of the catalogue were written from the document */
    bool derived;                                   /**< the document was derived from the catalogue */
    bool locked;                                    /**< another writer holds the lock of the index: the stored document, if any, was used */
    UInt32 resourceCount;
    UInt32 durationMs;

    CAssetPackIndexOutcome( void );
    virtual ~CAssetPackIndexOutcome() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXOUTCOME_H ? */
