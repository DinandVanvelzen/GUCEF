/*
 *  AssetConverterLib: the built-in post-processor that merges materials that render the same
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

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#include "AssetConverterLib_CMergeIdenticalMaterialsPostProcessor.h"

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

const CORE::CString CMergeIdenticalMaterialsPostProcessor::TypeName = "MergeIdenticalMaterials";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMergeIdenticalMaterialsPostProcessor::CMergeIdenticalMaterialsPostProcessor( void )
    : CIAssetPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMergeIdenticalMaterialsPostProcessor::~CMergeIdenticalMaterialsPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CMergeIdenticalMaterialsPostProcessor::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CMergeIdenticalMaterialsPostProcessor::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CMergeIdenticalMaterialsPostProcessor::Process( CAssetScene& scene, const CORE::CDataNode& /* settings */, CConversionReport& report )
{GUCEF_TRACE;

    CAssetScene::TUInt32Set merged;
    for ( size_t i=1; i<scene.materials.size(); ++i )
    {
        for ( size_t j=0; j<i; ++j )
        {
            if ( merged.find( (UInt32) j ) != merged.end() || !scene.materials[ i ].HasSameAppearance( scene.materials[ j ] ) )
                continue;
            scene.ReplaceMaterial( (UInt32) i, (UInt32) j );
            merged.insert( (UInt32) i );
            report.AddInfo( TypeName, scene.materials[ i ].name, "merged into " + scene.materials[ j ].name + ", which looks the same" );
            break;
        }
    }
    scene.RemoveMaterials( merged );
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
