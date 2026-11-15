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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#include "TestPipelineSlowValidator.h"

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace HOOKTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPipelineSlowValidator::CPipelineSlowValidator( void )
    : CIAssetValidator()
{
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPipelineSlowValidator::GetName( void ) const
{
    return "testslow";
}

/*-------------------------------------------------------------------------*/

UInt32
CPipelineSlowValidator::GetVersion( void ) const
{
    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineSlowValidator::Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{
    const UInt32 delay = settings.GetAttributeValueOrChildValueByName( "delayMs" ).AsUInt32( 40, true );
    MT::ThreadDelay( delay );
    report.AddWarning( "testslow", scene.provenance.assetPath, "the asset 'Models/x.gltf' has 3 triangles" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HOOKTEST */

/*-------------------------------------------------------------------------*/
