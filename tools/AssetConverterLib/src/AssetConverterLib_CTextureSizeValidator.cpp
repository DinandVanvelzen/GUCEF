/*
 *  AssetConverterLib: the built-in validator of texture sizes
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

#include "AssetConverterLib_CTextureSizeValidator.h"

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

const CORE::CString CTextureSizeValidator::TypeName = "TextureSize";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTextureSizeValidator::CTextureSizeValidator( void )
    : CIAssetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTextureSizeValidator::~CTextureSizeValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CTextureSizeValidator::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CTextureSizeValidator::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CTextureSizeValidator::Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{GUCEF_TRACE;

    const TReportItemKind severity = GetSeverity( settings, REPORTITEM_ERROR );
    const UInt32 maxSize = settings.GetAttributeValueOrChildValueByName( "maxSize" ).AsUInt32( 16384 );
    const bool powerOfTwo = settings.GetAttributeValueOrChildValueByName( "powerOfTwo" ).AsBool( false );
    for ( size_t t=0; t<scene.textures.size(); ++t )
    {
        const CAssetTexture& texture = scene.textures[ t ];
        if ( 0 == texture.width || 0 == texture.height )
            continue;
        const UInt32 larger = texture.width > texture.height ? texture.width : texture.height;
        const CORE::CString size = CORE::UInt32ToString( texture.width ) + "x" + CORE::UInt32ToString( texture.height );
        if ( maxSize > 0 && larger > maxSize )
            report.AddItem( severity, TypeName, texture.name, "is " + size + ", larger than " + CORE::UInt32ToString( maxSize ) );
        const bool widthPow2 = 0 == ( texture.width & ( texture.width - 1 ) );
        const bool heightPow2 = 0 == ( texture.height & ( texture.height - 1 ) );
        if ( powerOfTwo && ( !widthPow2 || !heightPow2 ) )
            report.AddItem( severity, TypeName, texture.name, "is " + size + ", not a power of two" );
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
