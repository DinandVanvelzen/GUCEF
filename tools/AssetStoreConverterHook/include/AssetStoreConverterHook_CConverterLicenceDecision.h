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

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERLICENCEDECISION_H
#define ASSETSTORECONVERTERHOOK_CCONVERTERLICENCEDECISION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CLICENCERECORD_H
#include "AssetConverterLib_CLicenceRecord.h"
#define ASSETCONVERTERLIB_CLICENCERECORD_H
#endif /* ASSETCONVERTERLIB_CLICENCERECORD_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#include "AssetStoreConverterHook_macros.h"
#define ASSETSTORECONVERTERHOOK_MACROS_H
#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */

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
 *  What the licence record of a converter pack allows (sections 10.1 and 10.3 of the asset tools design): whether the
 *  pipeline converts the pack (the rules of CLicenceRecord::AllowsConversion(), as CAssetPipeline applies them) and
 *  whether content of the pack (pictures, geometry, files) may be shown to a client of the tools. Content is withheld,
 *  fail closed, when the pack has no licence record or the record cannot be read, when the record carries the
 *  restriction noMachineLearning, and, for the source stage only, when the conversion is refused. The reasons name no
 *  paths of the VFS or the disk.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CConverterLicenceDecision
{
    public:

    static const CORE::CString ExposureAllowed;     /**< "allowed" */
    static const CORE::CString ExposureWithheld;    /**< "withheld" */

    CORE::CString recordId;
    bool recordFound;
    ASSETCONVERTER::CLicenceRecord record;          /**< as read; meaningful when recordFound */
    bool conversionAllowed;
    CORE::CString reason;                           /**< why the conversion is refused; empty when it is allowed */
    CORE::CString contentExposure;                  /**< ExposureAllowed or ExposureWithheld */
    CORE::CString exposureReason;                   /**< why content is withheld; empty when it is allowed */
    CORE::CString recordVfsPath;                    /**< where the record was looked for; never shown to a client */

    CConverterLicenceDecision( void );

    /** The decision of the licence record the manifest names, read as the pipeline reads it */
    void Evaluate( const ASSETCONVERTER::CAssetPackManifest& manifest );

    /** The decision of a record of the id at the VFS path (a record that a request names) */
    void EvaluateRecord( const CORE::CString& id, const CORE::CString& vfsPath );

    /** Whether content of the stage ("source", "intermediate", "cooked") is withheld; the reason when it is */
    bool IsStageWithheld( const CORE::CString& stage, CORE::CString& why ) const;

    /** The members of the record a review of the licence needs that are empty */
    void GetMissingFields( CORE::CString::StringVector& fields ) const;

    /**
     *  { recordId, recordFound, conversionAllowed, reason, restrictions[], contentExposure, exposureReason }; with
     *  'details' also licenceName, licenceUrl, checked and the notes as untrusted text, and purchaser and
     *  orderReference only when 'exposePurchaseDetails'
     */
    void ToDataNode( CORE::CDataNode& node, bool details, bool exposePurchaseDetails, UInt32 maxUntrustedTextChars ) const;

    private:

    void Decide( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERLICENCEDECISION_H ? */
