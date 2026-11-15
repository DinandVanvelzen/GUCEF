/*
 *  AssetConverterLib: the licence record of a pack, checked before conversion
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

#ifndef ASSETCONVERTERLIB_CLICENCERECORD_H
#define ASSETCONVERTERLIB_CLICENCERECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The licence record of a pack, <id>.licence.json, private: the store, the product, the order or bundle reference,
 *  the purchaser, the licence name and URL, the date the licence was checked, the seat count, the flags ueOnly,
 *  referenceOnly, restrictedOrNonStandard and humbleOneSeat, whether a restricted or non-standard licence was
 *  reviewed, and notes.
 *
 *  The converter cannot decide licence questions: the record is filled by a person, per pack. AllowsConversion()
 *  only applies the rules of the converter: a record with ueOnly or referenceOnly refuses, and so does one with
 *  restrictedOrNonStandard unless reviewed is true. A pack without a record is refused by the pipeline.
 *
 *  Restriction flags ("restrictions", a list of names) state what the licence does not allow besides conversion, with
 *  the names the primary store uses for the restriction flags of its licences (gucefASSETSTORE CAssetLicence):
 *  noMachineLearning (no training on the assets and no use as reference by learning systems: no content of the pack
 *  goes to an LLM), attributionRequired, noCommercialUse, noRedistribution and editorialOnly. Other names are kept as
 *  they are. Absent means "none stated", which is not the same as "allowed": readers that need an allowance decide
 *  for themselves how to treat a record without the flag. The flags do not change AllowsConversion().
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CLicenceRecord
{
    public:

    CORE::CString id;
    CORE::CString store;
    CORE::CString product;
    CORE::CString orderReference;
    CORE::CString purchaser;
    CORE::CString licenceName;
    CORE::CString licenceUrl;
    CORE::CString checked;
    UInt32 seats;
    bool ueOnly;
    bool referenceOnly;
    bool restrictedOrNonStandard;
    bool humbleOneSeat;
    bool reviewed;
    CORE::CString notes;
    CORE::CString::StringSet restrictions;

    static const CORE::CString NoMachineLearning;       /**< "noMachineLearning" */
    static const CORE::CString AttributionRequired;     /**< "attributionRequired" */
    static const CORE::CString NoCommercialUse;         /**< "noCommercialUse" */
    static const CORE::CString NoRedistribution;        /**< "noRedistribution" */
    static const CORE::CString EditorialOnly;           /**< "editorialOnly" */

    CLicenceRecord( void );

    bool Load( const CORE::CString& vfsPath );

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    /** Whether the record carries the restriction flag */
    bool HasRestriction( const CORE::CString& flag ) const;

    /** Whether the rules allow the conversion; when not, why */
    bool AllowsConversion( CORE::CString& reason ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CLICENCERECORD_H ? */
