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

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H
#define GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */

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
 *  What the last conversion of a pack did (section 7.4 of the design), the row of the table pack_conversions: the cook
 *  profile it was made for, the versions of the converter parts that made it ("name/version", for example
 *  "importer:gltf/1" or "converter:AssetConverterLib/3"), the outcome with its reason, the report that came out of it
 *  (relative to the pack root), when it ran and the counts of the report. The versions let a later run see that the
 *  conversion is stale: CIAssetConverterHook::IsConversionStale().
 *
 *  The outcome is "converted" (the report has no errors), "refused" (the converter would not convert the pack, for
 *  example its licence record does not allow it) or "failed" (the conversion ran and the report has errors; the pack
 *  stays as original). An empty outcome says the pack was never converted.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetConversionRecord : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString OutcomeConverted;        /**< "converted" */
    static const CORE::CString OutcomeRefused;          /**< "refused" */
    static const CORE::CString OutcomeFailed;           /**< "failed" */

    CORE::CString profile;                              /**< the cook profile, the folder name below "converted/" */
    CORE::CString::StringSet converters;                /**< "name/version" of the parts that made the conversion */
    CORE::CString outcome;                              /**< "converted", "refused" or "failed"; empty: never converted */
    CORE::CString reason;                               /**< why it was refused or failed */
    CORE::CString reportPath;                           /**< relative to the root of the pack, empty without a report */
    Int64 convertedAtMs;                                /**< UTC milliseconds since the Unix epoch */
    UInt32 convertedCount;                              /**< assets converted */
    UInt32 approximatedCount;                           /**< assets converted with approximations */
    UInt32 cachedCount;                                 /**< assets that came from the cache of the converter */
    UInt32 skippedCount;                                /**< assets the converter skipped */
    UInt32 failedCount;                                 /**< assets with errors */

    CAssetConversionRecord( void );
    virtual ~CAssetConversionRecord() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether a conversion was recorded */
    bool IsSet( void ) const;

    /** The text of a conversion version: "name/version" */
    static CORE::CString MakeConverterEntry( const CORE::CString& name, UInt32 version );

    /** The parts of a converter entry; false when the text has no version */
    static bool ParseConverterEntry( const CORE::CString& entry, CORE::CString& name, UInt32& version );

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

#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H ? */
