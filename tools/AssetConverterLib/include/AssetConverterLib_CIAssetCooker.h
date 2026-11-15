/*
 *  AssetConverterLib: the interface of the cookers of the intermediate into runtime files
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

#ifndef ASSETCONVERTERLIB_CIASSETCOOKER_H
#define ASSETCONVERTERLIB_CIASSETCOOKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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

class CCookSettings;
class CConversionReport;

/*-------------------------------------------------------------------------*/

/**
 *  A cooker: turns the intermediate of a pack into the runtime files of a target ("ogre", "ogrenext"). The pipeline
 *  calls Cook() once per cook profile, on the calling thread, with the assets to cook (those whose cache entry is not
 *  current and whose validation allows it) and the output folder of the profile. For each asset the cooker selects it
 *  in the report (CConversionReport::SelectAsset() with CCookSettings::SAsset::assetPath) and reports the files it
 *  wrote with the profile name and the files each references: the reference graph and the size report are made of
 *  them. GetVersion() is part of the cache key.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CIAssetCooker
{
    public:

    CIAssetCooker( void );

    virtual ~CIAssetCooker();

    virtual CORE::CString GetTargetName( void ) const = 0;

    virtual UInt32 GetVersion( void ) const = 0;

    /** The most bones a submesh may use with the hardware skinning of the target, for the BoneLimit validator; 0 for no limit (the default) */
    virtual UInt32 GetMaxBonesPerSubMesh( void ) const;

    virtual bool Cook( const CORE::CString& intermediateDir, const CORE::CString& outDir, const CCookSettings& settings, CConversionReport& report ) = 0;

    private:

    CIAssetCooker( const CIAssetCooker& src );            /**< not implemented */
    CIAssetCooker& operator=( const CIAssetCooker& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIAssetCooker, MT::CMutex > TAssetCookerPtr;
typedef CORE::CTGlobalONRegistry< CIAssetCooker, MT::CMutex > TAssetCookerRegistry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CIASSETCOOKER_H ? */
