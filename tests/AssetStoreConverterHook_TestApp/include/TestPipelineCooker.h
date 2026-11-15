/*
 *  AssetStoreConverterHook_TestApp: Test application for the converter hook of the primary asset store (AssetStoreConverterHook)
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

#ifndef TESTPIPELINECOOKER_H
#define TESTPIPELINECOOKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETCOOKER_H
#include "AssetConverterLib_CIAssetCooker.h"
#define ASSETCONVERTERLIB_CIASSETCOOKER_H
#endif /* ASSETCONVERTERLIB_CIASSETCOOKER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace HOOKTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A cooker of the tests of the pipeline operations (target "testcook"): per asset a "<stem>.mesh" that references a
 *  "<stem>.material", which references a picture shared by the pack, so that the reference and size reports of a
 *  profile have something to say
 */
class CPipelineTestCooker : public GUCEF::ASSETCONVERTER::CIAssetCooker
{
    public:

    CPipelineTestCooker( void );

    virtual GUCEF::CORE::CString GetTargetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Cook( const GUCEF::CORE::CString& intermediateDir, const GUCEF::CORE::CString& outDir,
                       const GUCEF::ASSETCONVERTER::CCookSettings& settings, GUCEF::ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HOOKTEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTPIPELINECOOKER_H ? */
