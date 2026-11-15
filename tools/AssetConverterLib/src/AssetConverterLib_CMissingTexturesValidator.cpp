/*
 *  AssetConverterLib: the built-in validator of missing textures
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

#include "AssetConverterLib_CMissingTexturesValidator.h"

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

const CORE::CString CMissingTexturesValidator::TypeName = "MissingTextures";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMissingTexturesValidator::CMissingTexturesValidator( void )
    : CIAssetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMissingTexturesValidator::~CMissingTexturesValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CMissingTexturesValidator::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CMissingTexturesValidator::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CMissingTexturesValidator::Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{GUCEF_TRACE;

    const TReportItemKind severity = GetSeverity( settings, REPORTITEM_ERROR );
    for ( size_t t=0; t<scene.textures.size(); ++t )
    {
        if ( scene.textures[ t ].missing )
            report.AddItem( severity, TypeName, scene.textures[ t ].name, "is missing: " + ( scene.textures[ t ].sources.empty() ? CORE::CString( "no source" ) : scene.textures[ t ].sources[ 0 ] ) );
    }
    static const char* const slots[] = { "baseColorTexture", "metallicRoughnessTexture", "normalTexture", "occlusionTexture", "emissiveTexture" };
    for ( size_t m=0; m<scene.materials.size(); ++m )
    {
        GUCEF::vector< const CAssetTextureReference* > references;
        scene.materials[ m ].GetTextureReferences( references );
        for ( size_t r=0; r<references.size(); ++r )
        {
            if ( references[ r ]->IsUsed() && (size_t) references[ r ]->textureIndex >= scene.textures.size() )
                report.AddItem( severity, TypeName, scene.materials[ m ].name, CORE::CString( slots[ r ] ) + " refers to texture " + CORE::Int32ToString( references[ r ]->textureIndex ) + ", which is not there" );
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
