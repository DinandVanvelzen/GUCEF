/*
 *  AssetWorkbenchLib: what to open for an item of an online store or a pack of the library
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

#ifndef ASSETWORKBENCHLIB_CSTOREITEMOPENLOOKUP_H
#define ASSETWORKBENCHLIB_CSTOREITEMOPENLOOKUP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Finds out, without waiting on the thread of the interface, what opening a row of the Sources tab shows:
 *
 *  - An item of an online store: the pack of the library that the item became (a search of the service for the packs of the
 *    store with the item id as their source item, each candidate checked with its pack record, so that a service that does not
 *    know the filter yet cannot give a wrong pack), then the asset of that pack to show. No pack: the item is not in the library.
 *  - A pack of the library: the asset of the pack to show.
 *
 *  The asset to show is a model when the pack has one (the converted one first, then the first by name), else the first asset.
 *  Every step is an asynchronous call of the service; Update() takes the answers, between two frames.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CStoreItemOpenLookup
{
    public:

    enum EState
    {
        STATE_IDLE = 0          ,
        STATE_FINDING_PACK      ,   /**< the search for the packs of the item */
        STATE_CHECKING_PACK     ,   /**< the pack records of the candidates */
        STATE_FINDING_ASSET     ,   /**< the assets of the pack */
        STATE_FOUND             ,   /**< the pack (and the asset when the pack has one) are known */
        STATE_NOT_IN_LIBRARY    ,   /**< the item has no pack in the library */
        STATE_FAILED                /**< the service could not tell: GetProblem() says why */
    };
    typedef enum EState TState;

    /** The most candidates of a search that are checked */
    static const UInt32 MaxCandidates = 25;

    /** The most assets of a pack that are looked at for the one to show */
    static const UInt32 MaxAssets = 500;

    CStoreItemOpenLookup( void );

    /** Starts the lookup of the pack of an item of a store; what ran before is forgotten. False without a service or an item */
    bool StartForItem( ASSETSTORE::CIAssetStoreService* service, const CORE::CString& storeId, const CORE::CString& itemId, const CORE::CString& itemTitle );

    /** Starts the lookup of the asset to show of a pack of the library */
    bool StartForPack( ASSETSTORE::CIAssetStoreService* service, const CORE::CString& packId, const CORE::CString& packTitle );

    /** Takes the answers that are there; true when the lookup has just ended (found, not in the library, failed) */
    bool Update( void );

    /** Forgets what runs: its answers are not taken */
    void Cancel( void );

    TState GetState( void ) const;
    bool IsBusy( void ) const;
    bool IsForItem( void ) const;

    const CORE::CString& GetStoreId( void ) const;
    const CORE::CString& GetItemId( void ) const;
    const CORE::CString& GetItemTitle( void ) const;
    const CORE::CString& GetPackId( void ) const;
    const ASSETSTORE::CAssetPackRecord& GetPack( void ) const;

    /** The asset to show, empty when the pack has none; the record of it */
    const CORE::CString& GetAssetId( void ) const;
    const ASSETSTORE::CAssetRecord& GetAsset( void ) const;

    /** What went wrong, for STATE_FAILED */
    const CORE::CString& GetProblem( void ) const;

    /** The asset to show of the assets of a pack: a converted model, a model, else the first asset; -1 for none */
    static Int32 ChooseAssetToShow( const ASSETSTORE::TAssetRecordVector& assets );

    private:

    void Fail( const CORE::CString& message );
    void CheckNextCandidate( void );
    void AskAssets( void );

    ASSETSTORE::CIAssetStoreService* m_service;
    TState m_state;
    bool m_forItem;
    CORE::CString m_storeId;
    CORE::CString m_itemId;
    CORE::CString m_itemTitle;
    CORE::CString::StringVector m_candidates;
    UInt32 m_nextCandidate;
    CORE::CString m_packId;
    ASSETSTORE::CAssetPackRecord m_pack;
    CORE::CString m_assetId;
    ASSETSTORE::CAssetRecord m_asset;
    CORE::CString m_message;
    CORE::CFutureResult m_future;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSTOREITEMOPENLOOKUP_H ? */
