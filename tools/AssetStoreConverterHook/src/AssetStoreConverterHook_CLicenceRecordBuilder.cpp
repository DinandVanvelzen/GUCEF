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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "AssetStoreConverterHook_CLicenceRecordBuilder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CLicenceRecordBuilder::GetRecordId( const ASSETSTORE::CAssetPackRecord& pack )
{GUCEF_TRACE;

    return pack.licence.licenceId.IsNULLOrEmpty() ? pack.licence.MakeLicenceId() : pack.licence.licenceId;
}

/*-------------------------------------------------------------------------*/

bool
CLicenceRecordBuilder::Build( const ASSETSTORE::CAssetPackRecord& pack, const CConverterHookSettings& settings ,
                              ASSETCONVERTER::CLicenceRecord& record, CORE::CString& problem                     )
{GUCEF_TRACE;

    record = ASSETCONVERTER::CLicenceRecord();
    const ASSETSTORE::CAssetLicence& licence = pack.licence;
    const bool unknownWithoutDetails = "unknown" == licence.licenceClass && licence.name.IsNULLOrEmpty() && licence.url.IsNULLOrEmpty() && licence.text.IsNULLOrEmpty();
    if ( licence.licenceClass.IsNULLOrEmpty() || unknownWithoutDetails )
    {
        problem = "the pack has no licence record in the catalogue";
        return false;
    }

    record.id = GetRecordId( pack );
    record.store = pack.sourceId;
    record.product = pack.title;
    record.licenceName = licence.name.IsNULLOrEmpty() ? licence.licenceClass : licence.name;
    record.licenceUrl = licence.url;
    record.seats = 1;
    record.ueOnly = licence.HasRestriction( "ueOnly" );
    record.referenceOnly = licence.HasRestriction( "referenceOnly" );
    record.humbleOneSeat = licence.HasRestriction( "humbleOneSeat" );
    record.reviewed = licence.HasRestriction( "reviewed" );
    record.restrictedOrNonStandard = licence.HasRestriction( "restrictedOrNonStandard" ) ||
                                     settings.nonStandardLicenceClasses.end() != settings.nonStandardLicenceClasses.find( licence.licenceClass );
    record.notes = "made from the licence of the pack in the catalogue of the primary store (licence class " + licence.licenceClass + ")";

    // the other restriction flags (noMachineLearning, attributionRequired, ...) carry over by name
    CORE::CString::StringSet::const_iterator r = licence.restrictions.begin();
    while ( r != licence.restrictions.end() )
    {
        const CORE::CString& flag = (*r);
        if ( "ueOnly" != flag && "referenceOnly" != flag && "humbleOneSeat" != flag && "reviewed" != flag && "restrictedOrNonStandard" != flag )
            record.restrictions.insert( flag );
        ++r;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
