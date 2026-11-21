/*
 *  AssetConverterLib: the recognised pack profile as the pack index stores it
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXPROFILE_H
#define ASSETCONVERTERLIB_CPACKINDEXPROFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#include "AssetConverterLib_CAssetPackProfile.h"
#define ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPROFILE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKGROUPRULE_H
#include "AssetConverterLib_CPackGroupRule.h"
#define ASSETCONVERTERLIB_CPACKGROUPRULE_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPRULE_H ? */

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
 *  The data of CAssetPackProfile plus what the index adds: explicit group rules, the default presentation of the pack (F2) and the
 *  recognition key (the hash of the listing paths and of the files the recogniser read). Written as
 *  { packType, packTypeName, engine, engineVersion?, recognisedBy, confidence, showcaseCandidates, groupingHints, noisePatterns,
 *    groupRules, presentation: { default }, recognitionKey }.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexProfile
{
    public:

    CORE::CString packType;
    CORE::CString packTypeName;
    CORE::CString engine;
    CORE::CString engineVersion;
    CORE::CString recognisedBy;
    UInt32 confidence;
    CORE::CString::StringVector showcaseCandidates;
    CORE::CString::StringVector groupingHints;
    CORE::CString::StringVector noisePatterns;
    CPackGroupRule::TRuleVector groupRules;
    CORE::CString presentationDefault;
    CORE::CString recognitionKey;

    CPackIndexProfile( void );

    /** Takes the data of the profile a recogniser filled; the rules and the presentation stay as they are */
    void SetFromProfile( const CAssetPackProfile& profile );

    /** The data of the profile again, for the code that groups by it (the root is the folder of the listing) */
    void GetProfile( CAssetPackProfile& profile ) const;

    bool IsRecognised( void ) const;
    bool operator==( const CPackIndexProfile& other ) const;
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

#endif /* ASSETCONVERTERLIB_CPACKINDEXPROFILE_H ? */

