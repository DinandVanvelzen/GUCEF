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

#ifndef GUCEF_ASSETSTORE_CASSETCONTENTEXPOSUREGATE_H
#define GUCEF_ASSETSTORE_CASSETCONTENTEXPOSUREGATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

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
 *  The licence content gate of the asset tools (section 10.3 of the asset tools design): whether the content of a
 *  pack (pictures, geometry, raw files, audio) may be given to an MCP client. Metadata stays available either way.
 *  Content is withheld when any of these holds, the reason names the first that applies:
 *
 *  1. a converter pack without a licence record, or a store pack whose licence class is unknown;
 *  2. the record or the store licence carries noMachineLearning;
 *  3. the record is refused for conversion and the stage asked for is "source";
 *  4. a reference that cannot be checked (the service does not answer): fail closed.
 *
 *  Every reference given must allow it: a target that names a converter pack and a store pack is withheld when either
 *  withholds. Asked through the service only (CIAssetStoreService: the licence_check operation of the converter
 *  hook for converter packs, GetLicence() for store packs), so standalone and remote behave the same. Thread safe; no
 *  cache, so a changed record counts at once.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetContentExposureGate
{
    public:

    explicit CAssetContentExposureGate( CIAssetStoreService* service );

    void SetService( CIAssetStoreService* service );

    /** A converter pack; 'stage' is "source", "intermediate" or "cooked" (empty: not "source") */
    bool IsConverterPackAllowed( const CORE::CString& packId, const CORE::CString& stage, CORE::CString& reason ) const;

    /** A pack of the primary store */
    bool IsStorePackAllowed( const CORE::CString& storePackId, CORE::CString& reason ) const;

    /** An asset of the primary store, by its pack */
    bool IsStoreAssetAllowed( const CORE::CString& storeAssetId, CORE::CString& reason ) const;

    /** Every reference that is not empty must allow it; no reference at all is withheld */
    bool IsAllowed( const CORE::CString& converterPackId ,
                    const CORE::CString& storePackId     ,
                    const CORE::CString& storeAssetId    ,
                    const CORE::CString& stage           ,
                    CORE::CString& reason                ) const;

    private:

    CAssetContentExposureGate( const CAssetContentExposureGate& src );              /**< not implemented */
    CAssetContentExposureGate& operator=( const CAssetContentExposureGate& src );   /**< not implemented */

    CIAssetStoreService* m_service;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETCONTENTEXPOSUREGATE_H ? */
