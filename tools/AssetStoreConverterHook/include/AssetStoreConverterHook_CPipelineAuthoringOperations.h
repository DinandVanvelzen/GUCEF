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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEAUTHORINGOPERATIONS_H
#define ASSETSTORECONVERTERHOOK_CPIPELINEAUTHORINGOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#include "AssetStoreConverterHook_CPipelineOperationCall.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H ? */

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
 *  The writes of the pipeline operations (section 7.4 of the asset tools design): manifest_create, manifest_edit,
 *  override_write and override_remove, each in two steps. A request without apply previews: it checks the change,
 *  builds the new content and answers the unified diff and a change token, writing nothing. A request with apply and
 *  the token of its preview writes: the token is single use and bound to the principal, the operation, the arguments
 *  and the target; the write is refused with change-conflict when the target changed since the preview. Writes are
 *  atomic (a temporary file in the same folder, then a rename), keep the line endings of the file they replace, and keep
 *  the earlier content in the history folder of the settings. Licence records are never written, and only roots that
 *  allow writes are written.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineAuthoringOperations
{
    public:

    static ASSETSTORE::TAssetStoreStatus ManifestCreate( CPipelineOperationCall& call, CPendingChangeStore& changes );
    static ASSETSTORE::TAssetStoreStatus ManifestEdit( CPipelineOperationCall& call, CPendingChangeStore& changes );
    static ASSETSTORE::TAssetStoreStatus OverrideWrite( CPipelineOperationCall& call, CPendingChangeStore& changes );
    static ASSETSTORE::TAssetStoreStatus OverrideRemove( CPipelineOperationCall& call, CPendingChangeStore& changes );

    /**
     *  Applies JSON pointer operations ({ op: set|remove|append, path, value }) to a tree; false with the reason of the
     *  first that cannot be applied
     */
    static bool ApplyOperations( CORE::CDataNode& tree, const CORE::CDataNode& operations, CORE::CString& problem );

    private:

    CPipelineAuthoringOperations( void );   /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEAUTHORINGOPERATIONS_H ? */
