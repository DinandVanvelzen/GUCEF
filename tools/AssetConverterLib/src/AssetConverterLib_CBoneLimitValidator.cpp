/*
 *  AssetConverterLib: the built-in validator of the bones per submesh
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#include "AssetConverterLib_CBoneLimitValidator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CBoneLimitValidator::TypeName = "BoneLimit";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBoneLimitValidator::CBoneLimitValidator( void )
    : CIAssetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBoneLimitValidator::~CBoneLimitValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CBoneLimitValidator::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CBoneLimitValidator::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CBoneLimitValidator::Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{GUCEF_TRACE;

    const TReportItemKind severity = GetSeverity( settings, REPORTITEM_ERROR );
    const UInt32 maxBones = settings.GetAttributeValueOrChildValueByName( "maxBones" ).AsUInt32( 0 );
    if ( 0 == maxBones )
        return true;
    for ( size_t m=0; m<scene.meshes.size(); ++m )
    {
        for ( size_t s=0; s<scene.meshes[ m ].subMeshes.size(); ++s )
        {
            const UInt32 used = scene.meshes[ m ].subMeshes[ s ].GetUsedJointCount();
            if ( used > maxBones )
                report.AddItem( severity, TypeName, scene.meshes[ m ].name + "/" + CORE::UInt32ToString( (UInt32) s ), "uses " + CORE::UInt32ToString( used ) + " bones, more than " + CORE::UInt32ToString( maxBones ) );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
