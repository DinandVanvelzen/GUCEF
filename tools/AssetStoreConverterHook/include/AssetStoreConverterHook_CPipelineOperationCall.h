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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERPACK_H
#include "AssetStoreConverterHook_CConverterPack.h"
#define ASSETSTORECONVERTERHOOK_CCONVERTERPACK_H
#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERPACK_H ? */

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
 *  One call of a pipeline operation (CPipelineConverterHook::RunPipelineOperation()): the request, the observer of a
 *  job, the result being written and a copy of the settings, with what every operation does the same way: the error
 *  answer ("errorCode" and "message" in the document, the status that goes with the code), untrusted text, rooted
 *  paths, paging, the lookup of a pack and of a rooted path. The error codes are those of section 5.6 of the asset
 *  tools design, which the tool set maps onto its tool errors.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelineOperationCall
{
    public:

    static const CORE::CString ErrorPathRefused;         /**< "path-refused" */
    static const CORE::CString ErrorPackUnknown;         /**< "pack-unknown" */
    static const CORE::CString ErrorPackAmbiguous;       /**< "pack-ambiguous" */
    static const CORE::CString ErrorLicenceRefused;      /**< "licence-refused" */
    static const CORE::CString ErrorContentWithheld;     /**< "content-withheld" */
    static const CORE::CString ErrorChangeTokenInvalid;  /**< "change-token-invalid" */
    static const CORE::CString ErrorChangeConflict;      /**< "change-conflict" */
    static const CORE::CString ErrorBusy;                /**< "busy" */
    static const CORE::CString ErrorInvalidArguments;    /**< "invalid-arguments" */
    static const CORE::CString ErrorNotFound;            /**< "not-found" */
    static const CORE::CString ErrorUnsupported;         /**< "unsupported" */
    static const CORE::CString ErrorTooLarge;            /**< "too-large" */
    static const CORE::CString ErrorBackendUnavailable;  /**< "backend-unavailable" */

    const ASSETSTORE::CAssetPipelineRequest& request;
    ASSETSTORE::CIAssetConversionObserver* observer;
    ASSETSTORE::CAssetServiceDocument& result;
    CPipelineOperationSettings settings;
    CORE::CString principal;
    bool exposePurchaseDetails;
    UInt32 maxUntrustedTextChars;

    CPipelineOperationCall( const ASSETSTORE::CAssetPipelineRequest& callRequest  ,
                            ASSETSTORE::CIAssetConversionObserver* callObserver   ,
                            ASSETSTORE::CAssetServiceDocument& callResult         ,
                            const CPipelineOperationSettings& callSettings        );

    /** The document of the result, an object node */
    CORE::CDataNode& Document( void );

    /** Ends the call with an error: the status of the code, "errorCode" and "message" in the document */
    ASSETSTORE::TAssetStoreStatus Fail( const CORE::CString& errorCode, const CORE::CString& message );

    /** Ends the call successfully */
    ASSETSTORE::TAssetStoreStatus Succeed( const CORE::CString& message = CORE::CString::Empty );

    /** The status that goes with an error code */
    static ASSETSTORE::TAssetStoreStatus StatusOfError( const CORE::CString& errorCode );

    /** An untrusted text object of section 5.4 with the name, capped at maxUntrustedTextChars */
    CORE::CDataNode Untrusted( const CORE::CString& name, const CORE::CString& source, const CORE::CString& text ) const;

    /** Sets the member to the rooted path of the VFS path, or to null when no root holds it */
    void SetRooted( CORE::CDataNode& node, const CORE::CString& member, const CORE::CString& vfsPath ) const;

    /** Sets the member to the text, or to null when it is empty */
    static void SetTextOrNull( CORE::CDataNode& node, const CORE::CString& member, const CORE::CString& text );

    /** An argument of the request as text; empty when absent */
    CORE::CString GetArgument( const CORE::CString& name ) const;

    /** An argument of the request as a boolean */
    bool GetBoolArgument( const CORE::CString& name, bool defaultValue ) const;

    /** An argument that is a list of texts */
    void GetListArgument( const CORE::CString& name, CORE::CString::StringVector& values ) const;

    /** An argument that is an object or a list; GUCEF_NULL when absent */
    const CORE::CDataNode* GetNodeArgument( const CORE::CString& name ) const;

    /** Every pack of the roots */
    void GetPacks( TConverterPackVector& packs ) const;

    /** The pack of the id; fails the call with pack-unknown or pack-ambiguous otherwise */
    bool FindPack( const CORE::CString& packId, CConverterPack& pack );

    /** The pack of request.packId, with the check that it is runnable (path-refused otherwise) when 'runnable' */
    bool FindRequestPack( CConverterPack& pack, bool runnable );

    /**
     *  A rooted path of the request resolved in a root of the kinds; fails the call with path-refused otherwise.
     *  'mustWrite': the root must allow writes
     */
    bool ResolveRootedPath( const CORE::CString& text, const CORE::CString& kinds, bool mustWrite, CORE::CString& vfsPath,
                            const ASSETSTORE::CAssetToolRoot*& root, CORE::CString& rooted );

    /** Checks a pack relative asset path; fails the call with path-refused when it is not safe */
    bool CheckAssetPath( const CORE::CString& assetPath );

    /** The first entry of the page of the request; false (the call failed) for a page token that cannot be read */
    bool GetPage( UInt32 defaultEntries, UInt32& first, UInt32& count );

    /** Sets the next page token and "total" after a page of 'count' from 'first' of 'total' entries */
    void FinishPage( UInt32 first, UInt32 count, UInt32 total );

    /** Whether the observer asks to stop */
    bool IsCancelRequested( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H ? */
