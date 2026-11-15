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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H
#define ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#include "AssetStoreConverterHook_CPipelineOperationCall.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H ? */

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
 *  The licence operations of the pipeline (section 7.1 of the asset tools design): licence_check, the decision of the
 *  record of a pack or of a source, and licence_draft_record, a draft of a record for the owner to complete and save.
 *  Neither writes: no tool writes licence records (decision D7), and a draft leaves every flag that decides what the
 *  pipeline may do unset, listed as a decision of the owner.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineLicenceOperations
{
    public:

    static ASSETSTORE::TAssetStoreStatus LicenceCheck( CPipelineOperationCall& call );
    static ASSETSTORE::TAssetStoreStatus LicenceDraftRecord( CPipelineOperationCall& call );

    /**
     *  The pack of the request: by request.packId, or the pack whose manifest has the source of request.sourcePath
     *  (a rooted path in a source or staging root). 'found' is false for a source that no manifest names; false when
     *  the call failed
     */
    static bool FindPackOfRequest( CPipelineOperationCall& call, CConverterPack& pack, bool& found, CORE::CString& sourceVfsPath );

    /** An id made of a name for a record or a pack: lower case letters, digits and '-' */
    static CORE::CString MakeId( const CORE::CString& name );

    private:

    CPipelineLicenceOperations( void );     /**< not implemented: static functions only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H ? */
