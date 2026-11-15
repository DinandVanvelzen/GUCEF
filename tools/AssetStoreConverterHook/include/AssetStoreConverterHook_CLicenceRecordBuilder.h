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

#ifndef ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H
#define ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */

#ifndef ASSETCONVERTERLIB_CLICENCERECORD_H
#include "AssetConverterLib_CLicenceRecord.h"
#define ASSETCONVERTERLIB_CLICENCERECORD_H
#endif /* ASSETCONVERTERLIB_CLICENCERECORD_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H
#include "AssetStoreConverterHook_CConverterHookSettings.h"
#define ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H
#endif /* ASSETSTORECONVERTERHOOK_CCONVERTERHOOKSETTINGS_H ? */

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
 *  Makes the licence record that the converter pipeline asks for (ASSETCONVERTER::CLicenceRecord) from the licence of a
 *  pack in the catalogue. The converter cannot decide licence questions and neither does this: the catalogue holds what
 *  a person or a store normaliser said about the licence, and this maps it:
 *
 *  - A pack without a licence in the catalogue (no licence class, or the class "unknown" with no name, address and
 *    text) has no record: Build() returns false and says so.
 *  - The restriction flags "ueOnly", "referenceOnly", "restrictedOrNonStandard", "humbleOneSeat" and "reviewed" of the
 *    licence are the flags of the record of the same name; every other restriction flag of the licence
 *    (noMachineLearning, attributionRequired, noCommercialUse, noRedistribution, editorialOnly, ...) goes into the
 *    restrictions of the record by its name.
 *  - A licence class in the "nonStandardLicenceClasses" of the settings (by default unknown, proprietary and
 *    editorial) sets restrictedOrNonStandard as well; such a pack is converted once its licence has "reviewed".
 *
 *  The converter then refuses the pack as its rules say (CLicenceRecord::AllowsConversion()).
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CLicenceRecordBuilder
{
    public:

    /** The identifier of the record of the pack: the id of its licence, or the one made from its licence */
    static CORE::CString GetRecordId( const ASSETSTORE::CAssetPackRecord& pack );

    /** Fills the record; false with 'problem' when the pack has no licence record */
    static bool Build( const ASSETSTORE::CAssetPackRecord& pack, const CConverterHookSettings& settings ,
                       ASSETCONVERTER::CLicenceRecord& record, CORE::CString& problem                     );

    private:

    CLicenceRecordBuilder( void );
    ~CLicenceRecordBuilder();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CLICENCERECORDBUILDER_H ? */
