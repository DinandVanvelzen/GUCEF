/*
 *  AssetConverterLib: the licence reference and restriction flags of a pack in the pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXLICENCE_H
#define ASSETCONVERTERLIB_CPACKINDEXLICENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  { recordId?, licenceId?, class?, restrictions: [] }. The restrictions are carried as data with the flag names of the store
 *  (noMachineLearning, attributionRequired, noCommercialUse, noRedistribution, editorialOnly) and of the converter record
 *  (ueOnly, referenceOnly, restrictedOrNonStandard, humbleOneSeat, reviewed). The index only carries them: enforcement is
 *  elsewhere, and a flag that this build does not know is kept.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexLicence
{
    public:

    CORE::CString recordId;
    CORE::CString licenceId;
    CORE::CString licenceClass;
    CORE::CString::StringVector restrictions;

    CPackIndexLicence( void );

    bool IsEmpty( void ) const;
    bool HasRestriction( const CORE::CString& flag ) const;
    bool operator==( const CPackIndexLicence& other ) const;
    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXLICENCE_H ? */

