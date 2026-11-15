/*
 *  assetpluginUNITY: the particle systems of Unity
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPARTICLEREADER_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPARTICLEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H
#include "assetpluginUNITY_CUnityMaterialReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYMATERIALREADER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a ParticleSystem (class 198) with the ParticleSystemRenderer (class 199) of its GameObject into a
 *  CAssetParticleSystem of the scene of the project (the table of design section 5.7.3), in Unity's coordinates (the
 *  normalisation converts them). MinMaxCurve: minMaxState 0 constant (scalar), 1 curve (maxCurve times scalar), 2
 *  random between curves, 3 random between constants (minScalar, scalar). MinMaxGradient: minMaxState 0 colour, 1
 *  gradient, 2 random between colours, 3 random between gradients, 4 random colour from the gradient. Angles of the
 *  rotation modules are radians in the file and degrees in the model. The modules and properties without a
 *  counterpart in the model are reported. The mesh of mesh particles and the sub-emitters (file IDs of other
 *  ParticleSystem documents) are left to the caller, which knows the objects of the file.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityParticleReader
{
    public:

    struct SSubEmitterReference
    {
        Int64 emitter;                          /**< file ID of the ParticleSystem document */
        ASSETCONVERTER::TParticleTrigger trigger;
        Float32 probability;
        UInt32 inherit;
    };
    typedef GUCEF::vector< SSubEmitterReference > TSubEmitterReferenceVector;

    CUnityParticleReader( CUnityProject& project, CUnityMaterialReader& materials );

    ~CUnityParticleReader();

    /**
     *  Adds the particle system to the scene; its index. 'renderer' may be NULL (nothing is drawn then). 'meshReference'
     *  receives the mesh of mesh particles, 'subEmitters' the sub-emitters by file ID.
     */
    Int32 Read( const CORE::CDataNode& system                       ,
                const CORE::CDataNode* renderer                     ,
                const CORE::CString& name                           ,
                CUnityReference& meshReference                      ,
                TSubEmitterReferenceVector& subEmitters             );

    /** A MinMaxCurve scaled by 'scale'; 'weighted' is set when a key has weighted tangents (not carried) */
    static ASSETCONVERTER::CAssetParticleCurve ReadCurve( const CORE::CDataNode* minMaxCurve, Float32 scale, bool& weighted );

    /** A MinMaxGradient */
    static ASSETCONVERTER::CAssetParticleGradient ReadGradient( const CORE::CDataNode* minMaxGradient );

    /** A Gradient: key0..7 (RGBA), ctime0..7 and atime0..7 (0..65535), m_NumColorKeys, m_NumAlphaKeys, m_Mode */
    static ASSETCONVERTER::CAssetParticleGradient::SGradient ReadGradientKeys( const CORE::CDataNode* gradient );

    /** The quaternion (x, y, z, w) of Euler angles in degrees that Unity applies in the order Z, X, Y */
    static void EulerToQuaternion( const Float32 degrees[ 3 ], Float32 q[ 4 ] );

    private:

    CUnityParticleReader( const CUnityParticleReader& src );            /**< not implemented */
    CUnityParticleReader& operator=( const CUnityParticleReader& src ); /**< not implemented */

    ASSETCONVERTER::CAssetParticleCurve Curve( const CORE::CDataNode& module, const CORE::CString& key, Float32 scale = 1.0f );
    ASSETCONVERTER::CAssetParticleGradient Gradient( const CORE::CDataNode& module, const CORE::CString& key );

    void ReadMain( const CORE::CDataNode& system, ASSETCONVERTER::CAssetParticleSystem& out );
    void ReadShape( const CORE::CDataNode& shape, ASSETCONVERTER::CAssetParticleSystem& out );
    void ReadEmission( const CORE::CDataNode& emission, ASSETCONVERTER::CAssetParticleSystem& out );
    void ReadModules( const CORE::CDataNode& system, ASSETCONVERTER::CAssetParticleSystem& out, TSubEmitterReferenceVector& subEmitters );
    void ReadRenderer( const CORE::CDataNode& renderer, ASSETCONVERTER::CAssetParticleSystem& out, CUnityReference& meshReference );
    void ReadMaterial( ASSETCONVERTER::CAssetParticleSystem& out );

    private:

    CUnityProject& m_project;
    CUnityMaterialReader& m_materials;
    CORE::CString m_name;
    bool m_weighted;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPARTICLEREADER_H ? */
