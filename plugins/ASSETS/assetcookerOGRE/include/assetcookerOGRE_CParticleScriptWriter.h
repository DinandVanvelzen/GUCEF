/*
 *  assetcookerOGRE: writes Ogre particle scripts of the particle systems of the intermediate
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

#ifndef GUCEF_ASSETCOOKER_OGRE_CPARTICLESCRIPTWRITER_H
#define GUCEF_ASSETCOOKER_OGRE_CPARTICLESCRIPTWRITER_H

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

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes the particle systems of an asset as the particle_system templates of one Ogre particle script, for the
 *  ParticleFX plugin of Ogre 14.6.3 (design section 5.7.4): the system with its billboard renderer options, an emitter
 *  for the rate and one per burst, and the affectors (LinearForce, ColourInterpolator, Scaler, Rotator,
 *  DirectionRandomiser, DeflectorPlane, TextureAnimator). The simulation speed is baked into times, rates and speeds.
 *  Every property that Ogre cannot express is reported per system, as an approximation or as skipped.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CParticleScriptWriter
{
    public:

    /** A colour over the normalised lifetime as at most six stages of Ogre's ColourInterpolator */
    struct SColourStage
    {
        Float32 time;
        Float32 colour[ 4 ];
    };
    typedef GUCEF::vector< SColourStage > TColourStageVector;

    CParticleScriptWriter( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    /** Adds the template of a system with the material it draws with; false when the system draws nothing */
    bool AddSystem( const CORE::CString& templateName, const CORE::CString& materialName, const ASSETCONVERTER::CAssetParticleSystem& system );

    UInt32 GetSystemCount( void ) const;

    const CORE::CString& GetText( void ) const;

    /**
     *  The colour of the particles over their lifetime (start colour, colour and alpha over the lifetime, the
     *  emission brightening and the tint of the renderer) reduced to at most 'maxStages' stages: the largest error
     *  first. 'maxError' receives the largest remaining difference of a channel. False when the colour does not
     *  change over the lifetime (no stages are needed then).
     */
    static bool BuildColourStages( const ASSETCONVERTER::CAssetParticleSystem& system, UInt32 maxStages, TColourStageVector& stages, Float32& maxError );

    /** The colour of the particles at the normalised age, before clamping: the product of the colour terms */
    static void SampleColour( const ASSETCONVERTER::CAssetParticleSystem& system, Float32 time, Float32 rgba[ 4 ] );

    static CORE::CString FormatNumber( Float32 value );

    private:

    void Line( UInt32 depth, const CORE::CString& text );
    void Approximated( const CORE::CString& message );
    void Skipped( const CORE::CString& message );

    void WriteRenderer( const ASSETCONVERTER::CAssetParticleSystem& system, Float32 speed, Float32& width, Float32& height );
    void WriteEmitters( const ASSETCONVERTER::CAssetParticleSystem& system, Float32 speed );
    void WriteEmitterBody( const ASSETCONVERTER::CAssetParticleSystem& system, Float32 speed, bool withColour );
    void WriteAffectors( const ASSETCONVERTER::CAssetParticleSystem& system, Float32 speed, Float32 width );
    void ReportUnsupported( const ASSETCONVERTER::CAssetParticleSystem& system );

    private:

    const ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    CORE::CString m_text;
    CORE::CString m_subject;
    UInt32 m_count;
    bool m_colourAffector;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_CPARTICLESCRIPTWRITER_H ? */
