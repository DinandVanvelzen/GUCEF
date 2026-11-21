/*
 *  AssetWorkbenchLib: a post-processor for the preview of a presentation: the rig of a character is dropped so that its mesh loads as one static model
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

#ifndef ASSETWORKBENCHLIB_CPACKPREVIEWPOSTPROCESSOR_H
#define ASSETWORKBENCHLIB_CPACKPREVIEWPOSTPROCESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#include "AssetConverterLib_CIAssetPostProcessor.h"
#define ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  For the picture of a character that a presentation shows: of every group of levels of detail only the first level keeps its
 *  mesh, so that the body is one mesh and not four on top of each other. The rig (the skins, the joint weights of the meshes and
 *  the animations) is kept: the character is shown skinned, with its bones, and plays its animations.
 *
 *  The merging form (MergedProcessorName) is for the scene of a pack that has thousands of nodes with a mesh: the static geometry, the
 *  meshes of nodes that have no skin and no morph targets, is baked into the world and merged by material into a few meshes of one
 *  node, so that the preview has some tens of submeshes and not tens of thousands. It is for the picture only, the intermediate
 *  of the conversion (the result of the importer, which the exporters and cookers read) is not changed: the processor runs on the copy
 *  of the scene that the preview exports. A node that moves (an animation targets it) or is a skin is left alone.
 *  It runs only for the conversions of a presentation (CPackAssetConverter), after the default post-processors.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackPreviewPostProcessor : public ASSETCONVERTER::CIAssetPostProcessor
{
    public:

    static const CORE::CString ProcessorName;           /**< "workbench-preview" */
    static const CORE::CString MergedProcessorName;     /**< "workbench-preview-merged" */

    explicit CPackPreviewPostProcessor( bool merge = false );
    virtual ~CPackPreviewPostProcessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Process( ASSETCONVERTER::CAssetScene& scene, const CORE::CDataNode& settings, ASSETCONVERTER::CConversionReport& report ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Bakes the static meshes of the scene into the world and merges them by material (see the class). The number of submeshes before and
     *  after is given. A scene with fewer meshes than 'minimumMeshNodes' is left as it is
     */
    static bool MergeStaticMeshes( ASSETCONVERTER::CAssetScene& scene, UInt32 minimumMeshNodes, UInt32& submeshesBefore, UInt32& submeshesAfter );

    private:

    bool m_merge;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKPREVIEWPOSTPROCESSOR_H ? */
