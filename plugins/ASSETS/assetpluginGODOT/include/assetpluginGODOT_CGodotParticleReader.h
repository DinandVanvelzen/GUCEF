/*
 *  assetpluginGODOT: the particle systems of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPARTICLEREADER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPARTICLEREADER_H

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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CIGODOTRESOURCERESOLVER_H
#include "assetpluginGODOT_CIGodotResourceResolver.h"
#define GUCEF_ASSETPLUGIN_GODOT_CIGODOTRESOURCERESOLVER_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CIGODOTRESOURCERESOLVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a GPUParticles3D node with its ParticleProcessMaterial, or a CPUParticles3D node (the same properties on the
 *  node, curves and gradients without their textures), into a CAssetParticleSystem of the scene (the table of design
 *  section 5.7.3). Values the file does not write are the class defaults of Godot 4. Curves are the points of a Curve
 *  (directly or through a CurveTexture), gradients the offsets and colours of a Gradient (directly or through a
 *  GradientTexture1D). The draw pass gives the renderer: a QuadMesh is a billboard of its size, its centre offset the
 *  pivot; a RibbonTrailMesh or TubeTrailMesh a streak along the velocity; another mesh a mesh particle. The material of
 *  the draw pass (or the material override of the node) gives the billboard mode, the blend mode, the tint, the use of
 *  the particle colour and the frames of a sprite sheet. Every property without a counterpart in the model is reported.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotParticleReader
{
    public:

    CGodotParticleReader( CIGodotResourceResolver& resolver, ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    ~CGodotParticleReader();

    static bool IsParticlesType( const CORE::CString& type );

    /**
     *  Adds the particle system of the section (of the node type 'type') to the scene; its index, -1 when the node is
     *  not a particle node. 'subEmitterPath' receives the node path of the sub-emitter ("" without one); 'subEmitterTrigger'
     *  the event that starts it, for the caller to resolve once all nodes are read.
     */
    Int32 Read( const CGodotDocument& document                      ,
                const CORE::CDataNode& section                      ,
                const CORE::CString& type                           ,
                const CORE::CString& nodeName                       ,
                CORE::CString& subEmitterPath                       ,
                ASSETCONVERTER::TParticleTrigger& subEmitterTrigger );

    private:

    CGodotParticleReader( const CGodotParticleReader& src );            /**< not implemented */
    CGodotParticleReader& operator=( const CGodotParticleReader& src ); /**< not implemented */

    /** A property of the process material (GPU) or of the node (CPU), with the CPU names of the properties that differ */
    const CORE::CDataNode* Property( const CORE::CString& key ) const;
    Float32 Number( const CORE::CString& key, Float32 fallback ) const;
    void Vector3( const CORE::CString& key, const Float32 fallback[ 3 ], Float32 out[ 3 ] ) const;

    /** The keys of a Curve, or of the Curve of a CurveTexture, that a reference names; false when there is none */
    bool ReadCurve( const CORE::CDataNode* value, ASSETCONVERTER::CAssetParticleCurve::TKeyVector& keys );

    /** The keys of a Gradient, or of the Gradient of a GradientTexture1D, that a reference names */
    bool ReadGradient( const CORE::CDataNode* value, ASSETCONVERTER::CAssetParticleGradient::SGradient& gradient );

    /** "<name>_min", "<name>_max" with the curve "<name>_curve" as one value */
    ASSETCONVERTER::CAssetParticleCurve ReadRange( const CORE::CString& name, Float32 defaultMin, Float32 defaultMax );

    void ReadEmission( ASSETCONVERTER::CAssetParticleSystem& system );
    void ReadShape( ASSETCONVERTER::CAssetParticleSystem& system );
    void ReadMotion( ASSETCONVERTER::CAssetParticleSystem& system );
    void ReadAppearance( ASSETCONVERTER::CAssetParticleSystem& system );
    void ReadDrawPass( ASSETCONVERTER::CAssetParticleSystem& system );
    void ReadMaterial( ASSETCONVERTER::CAssetParticleSystem& system, Int32 material, Float32 animSpeed );

    void Skipped( const CORE::CString& message );
    void Approximated( const CORE::CString& message );

    private:

    CIGodotResourceResolver& m_resolver;
    ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    const CGodotDocument* m_document;           /**< of the node */
    const CORE::CDataNode* m_node;              /**< the section of the node */
    const CGodotDocument* m_processDocument;    /**< of the process material (the node's for CPU particles) */
    const CORE::CDataNode* m_process;           /**< the process material, or the node for CPU particles */
    bool m_cpu;
    CORE::CString m_name;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPARTICLEREADER_H ? */
