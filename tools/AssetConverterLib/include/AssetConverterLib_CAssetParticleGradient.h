/*
 *  AssetConverterLib: a colour of a particle system that can vary randomly and over time
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

#ifndef ASSETCONVERTERLIB_CASSETPARTICLEGRADIENT_H
#define ASSETCONVERTERLIB_CASSETPARTICLEGRADIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

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
 *  A colour (RGBA, as authored) of a particle system: one colour, random between two colours, a gradient over a
 *  normalised time (0..1), random between two gradients, or a colour picked at random from a gradient (the time is not
 *  used then). A gradient keeps colour keys and alpha keys apart, as Unity does; Godot's keys give both lists. Keys
 *  are interpolated linearly, or held (stepped). A gradient without keys is white. A value type.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetParticleGradient
{
    public:

    struct ASSETCONVERTERLIB_PUBLIC_CPP SColourKey
    {
        Float32 time;
        Float32 colour[ 3 ];

        SColourKey( void );
        SColourKey( Float32 keyTime, Float32 red, Float32 green, Float32 blue );
    };
    typedef GUCEF::vector< SColourKey > TColourKeyVector;

    struct ASSETCONVERTERLIB_PUBLIC_CPP SAlphaKey
    {
        Float32 time;
        Float32 alpha;

        SAlphaKey( void );
        SAlphaKey( Float32 keyTime, Float32 keyAlpha );
    };
    typedef GUCEF::vector< SAlphaKey > TAlphaKeyVector;

    struct ASSETCONVERTERLIB_PUBLIC_CPP SGradient
    {
        TColourKeyVector colourKeys;
        TAlphaKeyVector alphaKeys;
        bool stepped;

        SGradient( void );

        /** Adds a key to both lists */
        void AddKey( Float32 time, const Float32 rgba[ 4 ] );

        void Evaluate( Float32 time, Float32 rgba[ 4 ] ) const;

        /** The times of all keys, sorted, each once */
        void GetKeyTimes( GUCEF::vector< Float32 >& times ) const;
    };

    TParticleColourMode mode;
    Float32 colourMin[ 4 ];     /**< the colour, or the lower colour */
    Float32 colourMax[ 4 ];     /**< the upper colour */
    SGradient gradientMin;      /**< the lower gradient (random between gradients) */
    SGradient gradientMax;      /**< the gradient, or the upper gradient */

    /** Opaque white */
    CAssetParticleGradient( void );

    static CAssetParticleGradient Colour( const Float32 rgba[ 4 ] );

    static CAssetParticleGradient RandomBetweenColours( const Float32 low[ 4 ], const Float32 high[ 4 ] );

    static CAssetParticleGradient Gradient( const SGradient& gradient );

    /** Whether the colour is the same at every time (it may still be random per particle) */
    bool IsConstantOverTime( void ) const;

    /** Whether every particle has the same colour at every time */
    bool IsConstant( void ) const;

    /** Whether the colour is opaque white at every time */
    bool IsWhite( void ) const;

    /** The colour at the normalised time; 'random' (0..1) chooses between the lower and the upper colour or gradient */
    void Evaluate( Float32 time, Float32 random, Float32 rgba[ 4 ] ) const;

    /** The mean colour over the particles at the time */
    void GetAverageAt( Float32 time, Float32 rgba[ 4 ] ) const;

    /** The times of the keys of the gradients (0 and 1 for colours), sorted, each once */
    void GetKeyTimes( GUCEF::vector< Float32 >& times ) const;

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

#endif /* ASSETCONVERTERLIB_CASSETPARTICLEGRADIENT_H ? */
