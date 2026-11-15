/*
 *  AssetConverterLib: the built-in post-processor that removes nodes that hold nothing
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

#include "AssetConverterLib_CPruneEmptyNodesPostProcessor.h"

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

const CORE::CString CPruneEmptyNodesPostProcessor::TypeName = "PruneEmptyNodes";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPruneEmptyNodesPostProcessor::CPruneEmptyNodesPostProcessor( void )
    : CIAssetPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPruneEmptyNodesPostProcessor::~CPruneEmptyNodesPostProcessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CPruneEmptyNodesPostProcessor::GetName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CPruneEmptyNodesPostProcessor::GetVersion( void ) const
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CPruneEmptyNodesPostProcessor::Process( CAssetScene& scene, const CORE::CDataNode& /* settings */, CConversionReport& report )
{GUCEF_TRACE;

    UInt32 pruned = 0;
    while ( true )
    {
        CAssetScene::TUInt32Set empty;
        for ( size_t n=0; n<scene.nodes.size(); ++n )
        {
            const CAssetNode& node = scene.nodes[ n ];
            if ( !node.children.empty() || node.mesh >= 0 || node.skin >= 0 || node.light >= 0 || node.particleSystem >= 0 || node.lodGroup >= 0 ||
                 !node.colliders.empty() || !node.prefabInstance.IsNULLOrEmpty() || 0 != node.extras.GetAttCount() || node.extras.HasChildren() )
                continue;
            const UInt32 index = (UInt32) n;
            if ( scene.IsJoint( index ) || scene.IsAnimationTarget( index ) || scene.IsLodLevel( index ) || scene.IsPrefabNode( index ) )
                continue;
            empty.insert( index );
        }
        if ( empty.empty() )
            break;
        pruned += scene.RemoveNodes( empty );
    }
    if ( pruned > 0 )
        report.AddInfo( TypeName, "nodes", CORE::UInt32ToString( pruned ) + " empty node(s) pruned" );
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
