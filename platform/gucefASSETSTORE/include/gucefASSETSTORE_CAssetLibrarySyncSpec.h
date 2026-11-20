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

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H
#include "gucefASSETSTORE_CAssetLibrarySyncProfile.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPROFILE_H ? */

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

class CAssetPipelineRequest;

/**
 *  The request of a library sync (library sync design, section 5.1): the store, the mode ("run" downloads, "plan" is the
 *  dry run of section 6.3 that writes nothing but the observations of the items), the profile, the retry of the items
 *  that failed before, a limit and an explicit list of items (the manual multi-download). It travels as the 'arguments' of
 *  the CAssetPipelineRequest of the job "sync-library" (StartPipelineJob()), so the start of a sync needs no new operation
 *  on the wire.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLibrarySyncSpec : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString ModeRun;             /**< "run" */
    static const CORE::CString ModePlan;            /**< "plan" */

    CORE::CString storeId;
    CORE::CString mode;                             /**< "run" (default) or "plan" */
    bool retryFailed;                               /**< the items that failed before are tried again, their counters reset */
    UInt32 maxItems;                                /**< at most this many items are queued, 0: no maximum */
    UInt32 pageSize;                                /**< the page size asked of the store, 0: the default of the store */
    CORE::CString::StringVector itemIds;            /**< only these items, empty for the whole library */
    bool scheduled;                                 /**< started by a schedule, not by a person */
    bool manual;                                    /**< a download of items a person selected: no space reserve, an item that does not fit is refused instead of waited for */
    bool useStoreProfile;                           /**< the run takes the profile the service keeps for the store (SetSyncProfile()); 'profile' is then not used nor sent */
    bool replan;                                    /**< a run lists the library afresh even when the stored plan of the store could be used */
    bool refreshDetails;                            /**< the details of every item that needs them are asked of the store again, the cache of details is not used (and is renewed) */
    bool allowLarge;                                /**< the items of itemIds go through the size guard of the store plugin (needs itemIds) */
    CAssetLibrarySyncProfile profile;

    CAssetLibrarySyncSpec( void );
    virtual ~CAssetLibrarySyncSpec() GUCEF_VIRTUAL_OVERRIDE;

    bool IsPlanOnly( void ) const;

    /** Whether the spec can be run; the reason when not */
    bool IsValid( CORE::CString& reason ) const;

    /** The spec of a pipeline request: its arguments, with the store id of the request when the arguments have none */
    static CAssetLibrarySyncSpec FromRequest( const CAssetPipelineRequest& request );

    /** A pipeline request of the job "sync-library" that carries the spec */
    void ToRequest( CAssetPipelineRequest& request ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSPEC_H ? */
