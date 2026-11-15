/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONRUNNER_H
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONRUNNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#include "gucefASSETSTORE_CAssetPipelineRequest.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEREQUEST_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#include "gucefASSETSTORE_CAssetServiceDocument.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEDOCUMENT_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#include "gucefASSETSTORE_CIAssetConversionObserver.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONSETTINGS_H
#include "AssetStoreConverterHook_CPipelineOperationSettings.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONSETTINGS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONSETTINGS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H
#include "AssetStoreConverterHook_CPipelinePlanStore.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPENDINGCHANGESTORE_H
#include "AssetStoreConverterHook_CPendingChangeStore.h"
#define ASSETSTORECONVERTERHOOK_CPENDINGCHANGESTORE_H
#endif /* ASSETSTORECONVERTERHOOK_CPENDINGCHANGESTORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Carries out the operations of the converter pipeline for CPipelineConverterHook::RunPipelineOperation(): it knows
 *  which operations there are (every name of ASSETSTORE::CAssetPipelineOperations but render_sheet), hands each to the
 *  class of its group and keeps what lives between calls: the last plan of each pack and the previews of writes that
 *  wait for their apply. Every call works on a copy of the settings, so the settings can change while calls run.
 *  Thread safe; the jobs block the calling thread (the thread of the job) until the run is done.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineOperationRunner
{
    public:

    CPipelineOperationRunner( void );

    ~CPipelineOperationRunner();

    void SetSettings( const CPipelineOperationSettings& settings );

    CPipelineOperationSettings GetSettings( void ) const;

    bool IsSupported( const CORE::CString& operation ) const;

    ASSETSTORE::TAssetStoreStatus Run( const ASSETSTORE::CAssetPipelineRequest& request   ,
                                       ASSETSTORE::CIAssetConversionObserver* observer    ,
                                       ASSETSTORE::CAssetServiceDocument& result          ,
                                       const CORE::CString& threadPoolName                ,
                                       UInt32 threads                                     );

    /** The previews that wait for their apply (tests) */
    UInt32 GetPendingChangeCount( void ) const;

    private:

    CPipelineOperationRunner( const CPipelineOperationRunner& src );              /**< not implemented */
    CPipelineOperationRunner& operator=( const CPipelineOperationRunner& src );   /**< not implemented */

    mutable MT::CMutex m_lock;
    CPipelineOperationSettings m_settings;
    CPipelinePlanStore m_plans;
    CPendingChangeStore m_changes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONRUNNER_H ? */
