/*
 *  AssetConverterLib: the built-in validator of names
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

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CNamingValidator.h"

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

const CORE::CString CNamingValidator::TypeName = "Naming";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNamingValidator::CNamingValidator( void )
    : CIAssetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNamingValidator::~CNamingValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CNamingValidator::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CNamingValidator::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CNamingValidator::Validate( const CAssetScene& scene, const CORE::CDataNode& settings, CConversionReport& report )
{GUCEF_TRACE;

    const TReportItemKind severity = GetSeverity( settings, REPORTITEM_WARNING );
    const CORE::CDataNode* patterns = settings.FindChild( "patterns" );
    if ( GUCEF_NULL == patterns )
        return true;
    static const char* const kinds[] = { "mesh", "material", "texture", "node", "animation", "asset" };
    for ( UInt32 k=0; k<sizeof( kinds ) / sizeof( kinds[ 0 ] ); ++k )
    {
        CORE::CString::StringVector globs;
        if ( !patterns->GetStringListByName( kinds[ k ], globs, '\x01' ) || globs.empty() )
            continue;
        CORE::CString::StringVector names;
        switch ( k )
        {
            case 0: for ( size_t i=0; i<scene.meshes.size(); ++i ) names.push_back( scene.meshes[ i ].name ); break;
            case 1: for ( size_t i=0; i<scene.materials.size(); ++i ) names.push_back( scene.materials[ i ].name ); break;
            case 2: for ( size_t i=0; i<scene.textures.size(); ++i ) names.push_back( scene.textures[ i ].name ); break;
            case 3: for ( size_t i=0; i<scene.nodes.size(); ++i ) names.push_back( scene.nodes[ i ].name ); break;
            case 4: for ( size_t i=0; i<scene.animations.size(); ++i ) names.push_back( scene.animations[ i ].name ); break;
            default: names.push_back( CVfsPaths::NameOf( scene.provenance.assetPath ) ); break;
        }
        for ( size_t n=0; n<names.size(); ++n )
        {
            bool matched = false;
            for ( size_t g=0; g<globs.size() && !matched; ++g )
                matched = CORE::CGlobPattern::IsMatch( globs[ g ], names[ n ], false );
            if ( !matched )
                report.AddItem( severity, TypeName, names[ n ], CORE::CString( "the " ) + kinds[ k ] + " name does not match " + globs[ 0 ] + ( globs.size() > 1 ? CORE::CString( " or the other patterns" ) : CORE::CString::Empty ) );
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
