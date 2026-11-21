/*
 *  AssetConverterLib: the options of a pack index build
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

#ifndef ASSETCONVERTERLIB_CPACKGROUPRULES_H
#include "AssetConverterLib_CPackGroupRules.h"
#define ASSETCONVERTERLIB_CPACKGROUPRULES_H
#endif /* ASSETCONVERTERLIB_CPACKGROUPRULES_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "AssetConverterLib_CPackIndexBuildOptions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexBuildOptions::CPackIndexBuildOptions( void )
    : packName()
    , packTitle()
    , storePackId()
    , licence()
    , profileRules()
    , presentationDefault()
    , plugins()
    , tool( "AssetConverterLib" )
    , toolVersion()
    , outputRoot( "intermediate" )
    , useFileSystemSizes( true )
    , observer( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackIndexBuildOptions::GetSettingsHash( void ) const
{GUCEF_TRACE;

    CORE::CDataNode settings;
    settings.SetNodeType( GUCEF_DATATYPE_OBJECT );
    settings.SetAttribute( "subGroupDepth", CPackGroupRules::MaxSubGroupDepth );
    return CCanonicalJson::Hash( settings );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
