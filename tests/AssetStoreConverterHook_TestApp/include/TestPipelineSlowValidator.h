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

#ifndef TESTPIPELINESLOWVALIDATOR_H
#define TESTPIPELINESLOWVALIDATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#include "AssetConverterLib_CIAssetValidator.h"
#define ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CIASSETVALIDATOR_H ? */

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
 *  A validator of the tests of the pipeline operations ("testslow"): takes its time per asset (the settings member
 *  "delayMs", default 40) and reports one warning that quotes the asset, so that a run can be cancelled midway and
 *  validate_pack has findings to group
 */
class CPipelineSlowValidator : public GUCEF::ASSETCONVERTER::CIAssetValidator
{
    public:

    CPipelineSlowValidator( void );

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Validate( const GUCEF::ASSETCONVERTER::CAssetScene& scene, const GUCEF::CORE::CDataNode& settings,
                           GUCEF::ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HOOKTEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTPIPELINESLOWVALIDATOR_H ? */
