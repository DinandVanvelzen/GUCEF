/*
 *  AssetConverterLib: an engine-neutral particle system of the intermediate model
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

#ifndef ASSETCONVERTERLIB_CASSETPARTICLESYSTEM_H
#define ASSETCONVERTERLIB_CASSETPARTICLESYSTEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPARTICLECURVE_H
#include "AssetConverterLib_CAssetParticleCurve.h"
#define ASSETCONVERTERLIB_CASSETPARTICLECURVE_H
#endif /* ASSETCONVERTERLIB_CASSETPARTICLECURVE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPARTICLEGRADIENT_H
#include "AssetConverterLib_CAssetParticleGradient.h"
#define ASSETCONVERTERLIB_CASSETPARTICLEGRADIENT_H
#endif /* ASSETCONVERTERLIB_CASSETPARTICLEGRADIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A particle system in engine-neutral terms (design section 5.7.2), attached to a node of the scene
 *  (CAssetNode::particleSystem) and written to the "particles" list of the sidecar.
 *
 *  Conventions after the normalisation: metres, seconds, +Y up, right handed, angles in degrees; vectors in the space of
 *  the node of the system; gravity and forces in the simulation space; colours as authored. Curves over the lifetime
 *  use the normalised age of a particle (0..1), the emission curves the normalised time of the cycle. The materials and
 *  the mesh of the renderer are indices into the lists of the scene (-1 for none). A value type.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetParticleSystem
{
    public:

    struct ASSETCONVERTERLIB_PUBLIC_CPP SBurst
    {
        Float32 time;                   /**< seconds into the cycle */
        CAssetParticleCurve count;
        UInt32 cycles;                  /**< 0: repeats for ever */
        Float32 interval;               /**< seconds between the cycles of the burst */
        Float32 probability;

        SBurst( void );
    };
    typedef GUCEF::vector< SBurst > TBurstVector;

    struct ASSETCONVERTERLIB_PUBLIC_CPP SShape
    {
        TParticleShape type;
        Float32 position[ 3 ];
        Float32 rotation[ 4 ];          /**< quaternion x, y, z, w: turns the canonical shape (axis +Y) into place */
        Float32 scale[ 3 ];
        Float32 radius;
        Float32 radiusThickness;        /**< 0: the surface, 1: the whole volume */
        Float32 angle;                  /**< half angle of a cone */
        Float32 arc;                    /**< of a circle, cone or ring */
        Float32 length;                 /**< of a cone */
        Float32 boxSize[ 3 ];           /**< full sizes of a box (and the bounds of a mesh shape) */
        Float32 ringRadius;             /**< the tube radius of a ring */
        Float32 ringHeight;
        Float32 ringInnerRadius;
        Int32 mesh;                     /**< of a mesh shape */
        TParticleDirectionMode directionMode;
        Float32 direction[ 3 ];         /**< of fixed emission, normalised */
        Float32 spreadAngle;            /**< half angle around the direction */
        Float32 randomDirectionAmount;  /**< 0..1 towards a random direction */

        SShape( void );

        /** The axis of the shape: +Y turned by the rotation */
        void GetAxis( Float32 axis[ 3 ] ) const;
    };

    struct ASSETCONVERTERLIB_PUBLIC_CPP SVectorCurves
    {
        bool enabled;
        CAssetParticleCurve x;
        CAssetParticleCurve y;
        CAssetParticleCurve z;
        TParticleSpace space;

        SVectorCurves( void );

        /** Whether all three are constant (also when not enabled) */
        bool IsConstant( void ) const;

        void GetAverage( Float32 out[ 3 ] ) const;
    };

    struct ASSETCONVERTERLIB_PUBLIC_CPP SNoise
    {
        bool enabled;
        CAssetParticleCurve strength;
        Float32 frequency;
        Float32 scrollSpeed;
        UInt32 octaves;

        SNoise( void );
    };

    struct ASSETCONVERTERLIB_PUBLIC_CPP SPlane
    {
        Float32 point[ 3 ];
        Float32 normal[ 3 ];

        SPlane( void );
    };
    typedef GUCEF::vector< SPlane > TPlaneVector;

    struct ASSETCONVERTERLIB_PUBLIC_CPP SCollision
    {
        bool enabled;
        bool world;                     /**< collides with the scene, not with planes */
        TPlaneVector planes;
        Float32 bounce;
        Float32 dampen;
        Float32 lifetimeLoss;

        SCollision( void );
    };

    struct ASSETCONVERTERLIB_PUBLIC_CPP STextureSheet
    {
        bool enabled;
        UInt32 tilesX;
        UInt32 tilesY;
        bool singleRow;
        Int32 rowIndex;                 /**< -1: a random row */
        CAssetParticleCurve frameOverTime;  /**< 0..1 over the frames of a cycle */
        CAssetParticleCurve startFrame;     /**< 0..1 */
        Float32 cycles;                 /**< per lifetime */
        Float32 fps;                    /**< frames per second when animated by time, 0 when by lifetime */

        STextureSheet( void );
    };

    struct ASSETCONVERTERLIB_PUBLIC_CPP SRenderer
    {
        TParticleRenderMode mode;
        TParticleAlignment alignment;
        Int32 material;
        Int32 trailMaterial;
        Int32 mesh;
        Float32 quadSize[ 2 ];
        Float32 pivot[ 3 ];             /**< offset of the quad in its own sizes */
        Float32 lengthScale;
        Float32 velocityScale;
        TParticleBlendMode blendMode;
        Float32 alphaCutoff;            /**< 0 for none */
        Float32 tint[ 4 ];
        bool useParticleColour;
        bool lit;
        bool softParticles;
        TParticleSortMode sortMode;

        SRenderer( void );
    };

    struct ASSETCONVERTERLIB_PUBLIC_CPP SSubEmitter
    {
        Int32 system;                   /**< index into the particle systems of the scene */
        TParticleTrigger trigger;
        Float32 probability;
        UInt32 inherit;                 /**< flags of the source */

        SSubEmitter( void );
    };
    typedef GUCEF::vector< SSubEmitter > TSubEmitterVector;

    struct ASSETCONVERTERLIB_PUBLIC_CPP STrails
    {
        bool enabled;
        CAssetParticleCurve lifetime;
        CAssetParticleCurve widthOverTrail;
        CAssetParticleGradient colourOverTrail;
        Float32 minVertexDistance;
        bool worldSpace;
        bool inheritColour;

        STrails( void );
    };

    CORE::CString name;

    // main
    Float32 duration;
    bool looping;
    bool playOnAwake;               /**< emits from the start; false: a script starts it */
    Float32 prewarm;
    CAssetParticleCurve startDelay;
    Float32 simulationSpeed;
    TParticleSpace simulationSpace;
    UInt32 maxParticles;
    bool hasBounds;
    Float32 boundsMin[ 3 ];
    Float32 boundsMax[ 3 ];

    // emission
    CAssetParticleCurve rateOverTime;
    CAssetParticleCurve rateOverDistance;
    TBurstVector bursts;
    Float32 explosiveness;
    Float32 emissionRandomness;

    // start values
    CAssetParticleCurve startLifetime;
    CAssetParticleCurve startSpeed;
    CAssetParticleCurve startSize;
    bool startSize3D;
    CAssetParticleCurve startSizeY;
    CAssetParticleCurve startSizeZ;
    CAssetParticleCurve startRotation;
    bool startRotation3D;
    CAssetParticleGradient startColour;

    SShape shape;

    // over lifetime
    SVectorCurves velocity;
    CAssetParticleCurve speedModifier;
    SVectorCurves force;
    Float32 gravity[ 3 ];
    CAssetParticleCurve linearAcceleration;
    CAssetParticleCurve radialAcceleration;
    CAssetParticleCurve tangentialAcceleration;
    CAssetParticleCurve damping;
    bool limitSpeed;
    CAssetParticleCurve speedLimit;
    Float32 speedLimitDampen;
    CAssetParticleCurve drag;
    bool sizeOverLifetimeEnabled;
    CAssetParticleCurve sizeOverLifetime;
    bool sizeOverLifetimeSeparate;
    CAssetParticleCurve sizeOverLifetimeY;
    CAssetParticleCurve sizeOverLifetimeZ;
    CAssetParticleCurve angularVelocity;
    bool colourOverLifetimeEnabled;
    CAssetParticleGradient colourOverLifetime;
    bool alphaOverLifetimeEnabled;
    CAssetParticleCurve alphaOverLifetime;
    bool emissionOverLifetimeEnabled;
    CAssetParticleCurve emissionOverLifetime;

    SNoise noise;
    SCollision collision;
    STextureSheet textureSheet;
    SRenderer renderer;
    TSubEmitterVector subEmitters;
    STrails trails;

    /** the source properties as read: lossless provenance for a later mapping */
    CORE::CDataNode source;

    CAssetParticleSystem( void );

    /**
     *  Converts the vectors, the shape and the distances by the signed axis permutation 'p' (3 x 3, column major) and
     *  the unit scale of the normalisation; 'determinant' is -1 for a mirror. The canonical shape axis stays +Y.
     */
    void ConvertSpace( const Float32 p[ 9 ], Float32 scale, Float32 determinant );

    /** Whether one of the systems of the scene starts this one as a sub-emitter */
    static bool IsSubEmitter( const GUCEF::vector< CAssetParticleSystem >& systems, UInt32 index );

    bool Serialize( CORE::CDataNode& node ) const;

    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPARTICLESYSTEM_H ? */
