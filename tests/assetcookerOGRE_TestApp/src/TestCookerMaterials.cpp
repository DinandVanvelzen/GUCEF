/*
 *  assetcookerOGRE_TestApp: Test application for the Ogre 1.x cooker of the asset converter (assetcookerOGRE)
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

#include <math.h>

#include "Ogre.h"
#include "OgreRTShaderSystem.h"
#include "OgreShaderCookTorranceLighting.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestCookerSupport.h"
#include "TestCookerScenes.h"
#include "TestCookerMaterials.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const Ogre::String MaterialGroup = "CookerMaterials";
const CORE::CString Prefix = "pack/Materials/Board/";

Ogre::Pass*
GetPass( const CORE::CString& material )
{
    Ogre::MaterialPtr found = Ogre::MaterialManager::getSingleton().getByName( ( Prefix + material ).STL_String(), MaterialGroup );
    if ( !found || 0 == found->getNumTechniques() || 0 == found->getTechnique( 0 )->getNumPasses() )
        return GUCEF_NULL;
    return found->getTechnique( 0 )->getPass( 0 );
}

/** The sub render state of the type that the RTSS script gave the first pass of the material, NULL when there is none */
Ogre::RTShader::SubRenderState*
GetSubRenderState( const CORE::CString& material, const Ogre::String& type )
{
    Ogre::MaterialPtr found = Ogre::MaterialManager::getSingleton().getByName( ( Prefix + material ).STL_String(), MaterialGroup );
    if ( !found )
        return GUCEF_NULL;
    Ogre::RTShader::RenderState* state = Ogre::RTShader::ShaderGenerator::getSingleton().getRenderState( Ogre::MSN_SHADERGEN, *found, 0 );
    if ( GUCEF_NULL == state )
        return GUCEF_NULL;
    const Ogre::RTShader::SubRenderStateList& list = state->getSubRenderStates();
    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ]->getType() == type )
            return list[ i ];
    return GUCEF_NULL;
}

/** The metal-roughness texture of the CookTorrance stage of the material; 'present' whether the stage is there */
Ogre::String
GetMetalRoughnessTexture( const CORE::CString& material, bool& present, bool& occlusion )
{
    Ogre::RTShader::SubRenderState* state = GetSubRenderState( material, Ogre::RTShader::SRS_COOK_TORRANCE_LIGHTING );
    present = GUCEF_NULL != state;
    occlusion = false;
    if ( !present )
        return Ogre::BLANKSTRING;
    const Ogre::RTShader::CookTorranceLighting* lighting = static_cast< const Ogre::RTShader::CookTorranceLighting* >( state );
    occlusion = lighting->getTextureOcclusion();
    return lighting->getMetalRoughnessMapName();
}

bool
NearColour( const Ogre::ColourValue& colour, float r, float g, float b, float a )
{
    return Near( colour.r, r ) && Near( colour.g, g ) && Near( colour.b, b ) && Near( colour.a, a );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCookerMaterialsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetcookerOGRE.Materials" );

    CConversionReport report;
    CORE::CString script;
    bool parsed = false;

    GUCEF_TESTFW_TESTCASE( "Materials: the script of every material parses with Ogre's script compiler, the RTSS sections included" )
    try
    {
        MakeWorkFolder( "materials" );
        ASSERT_TRUE( WriteTestPng( "work/materials/src/tex/albedo.png", 4, 4, 250, 200, 150, 255 ) );
        ASSERT_TRUE( WriteTestPng( "work/materials/src/tex/orm.png", 4, 4, 10, 200, 100, 255 ) );
        ASSERT_TRUE( WriteTestPng( "work/materials/src/tex/normal.png", 4, 4, 128, 128, 255, 255 ) );
        ASSERT_TRUE( WriteTestPng( "work/materials/src/tex/emissive.png", 4, 4, 100, 200, 50, 255 ) );
        ASSERT_TRUE( WriteTestPng( "work/materials/src/tex/occlusion.png", 4, 4, 50, 50, 50, 255 ) );
        CAssetScene scene;
        BuildMaterialScene( scene, "work/materials/src/tex" );
        CCookSettings settings;
        CCookSettings::SAsset asset;
        ASSERT_TRUE( MakeIntermediate( scene, "materials", "Materials/Board.fbx", asset ) );
        settings.assets.push_back( asset );
        ASSERT_TRUE( CookIntermediate( "materials", settings, report ) );
        ASSERT_FALSE( report.HasFailures() );

        const CORE::CString scriptPath = GetProfileDir( "materials" ) + "/pack/Materials/Board/Board.material";
        ASSERT_TRUE( VfsFileExists( scriptPath ) );
        script = ReadVfsText( scriptPath );
        ASSERT_TRUE( UseOgreGroup( MaterialGroup, GetProfileDir( "materials" ) ) );
        const char* const names[ 10 ] = { "Plain", "Metal", "Factored", "Bumpy", "Glow", "Hot", "Leaf", "Glass", "Coat", "Dim" };
        parsed = true;
        for ( UInt32 m=0; m<10; ++m )
        {
            const bool found = GUCEF_NULL != GetPass( names[ m ] );
            ASSERT_TRUE( found );
            parsed = parsed && found;
        }
        // the submeshes name the materials of the script
        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load( "pack/Materials/Board/BoardMesh.mesh", MaterialGroup );
        ASSERT_TRUE( mesh && 10 == mesh->getNumSubMeshes() );
        if ( mesh && 10 == mesh->getNumSubMeshes() )
            for ( unsigned short s=0; s<10; ++s )
                ASSERT_TRUE( ( Prefix + names[ s ] ).STL_String() == mesh->getSubMesh( s )->getMaterialName() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: base colour factor and texture (sRGB, UV set, transform), metal-roughness factors as parameters" )
    try
    {
        ASSERT_TRUE( parsed );
        Ogre::Pass* pass = GetPass( "Plain" );
        if ( GUCEF_NULL != pass )
        {
            ASSERT_TRUE( NearColour( pass->getDiffuse(), 0.8f, 0.7f, 0.6f, 1.0f ) );
            // roughness in red, metalness in green, as the metal_roughness stage reads them without texture
            ASSERT_TRUE( Near( pass->getSpecular().r, 0.75f ) && Near( pass->getSpecular().g, 0.25f ) );
            ASSERT_TRUE( 1 == pass->getNumTextureUnitStates() );
            if ( 1 == pass->getNumTextureUnitStates() )
            {
                Ogre::TextureUnitState* unit = pass->getTextureUnitState( 0 );
                ASSERT_TRUE( "BaseColour" == unit->getName() );
                ASSERT_TRUE( "pack/tex/albedo.png" == unit->getTextureName() );
                ASSERT_TRUE( unit->isHardwareGammaEnabled() );
                ASSERT_TRUE( 1 == unit->getTextureCoordSet() );
                // tiling 2 x 4 with offset 0.25, 0.5
                ASSERT_TRUE( Near( unit->getTextureUScale(), 0.5f ) && Near( unit->getTextureVScale(), 0.25f ) );
                ASSERT_TRUE( Near( unit->getTextureUScroll(), 0.75f ) && Near( unit->getTextureVScroll(), 2.0f ) );
            }
            bool present = false, occlusion = false;
            ASSERT_TRUE( GetMetalRoughnessTexture( "Plain", present, occlusion ).empty() );
            ASSERT_TRUE( present );
        }
        ASSERT_TRUE( VfsFileExists( GetProfileDir( "materials" ) + "/pack/tex/albedo.png" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: the metal-roughness texture for the metal_roughness lighting stage, as it is or derived with occlusion and factors" )
    try
    {
        ASSERT_TRUE( parsed );
        bool present = false, occlusion = false;
        ASSERT_TRUE( "pack/tex/orm.png" == GetMetalRoughnessTexture( "Metal", present, occlusion ) );
        ASSERT_TRUE( present && occlusion );
        ASSERT_TRUE( script.HasSubstr( "lighting_stage metal_roughness texture \"pack/tex/orm.png\" occlusion" ) >= 0 );

        const CORE::CString derived = GetMetalRoughnessTexture( "Factored", present, occlusion );
        ASSERT_TRUE( present && occlusion );
        ASSERT_TRUE( derived.StartsWith( "pack/textures/derived/" ) );
        UInt32 width = 0, height = 0;
        UInt8 texel[ 4 ] = { 0, 0, 0, 0 };
        ASSERT_TRUE( ReadTestPicture( GetProfileDir( "materials" ) + "/" + derived, width, height, texel ) );
        // occlusion from its own texture in R, roughness 200 * 0.25 in G, metalness 100 * 0.5 in B
        ASSERT_TRUE( 4 == width && 4 == height );
        ASSERT_TRUE( 50 == texel[ 0 ] && 50 == texel[ 1 ] && 50 == texel[ 2 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: the normal map is a texture unit with the RTSS normal map sub render state" )
    try
    {
        ASSERT_TRUE( parsed );
        Ogre::Pass* pass = GetPass( "Bumpy" );
        if ( GUCEF_NULL != pass )
        {
            ASSERT_TRUE( 1 == pass->getNumTextureUnitStates() );
            if ( 1 == pass->getNumTextureUnitStates() )
            {
                ASSERT_TRUE( "NormalMap" == pass->getTextureUnitState( 0 )->getName() );
                ASSERT_TRUE( "pack/tex/normal.png" == pass->getTextureUnitState( 0 )->getTextureName() );
                ASSERT_FALSE( pass->getTextureUnitState( 0 )->isHardwareGammaEnabled() );
            }
        }
        ASSERT_TRUE( GUCEF_NULL != GetSubRenderState( "Bumpy", Ogre::RTShader::SRS_NORMALMAP ) );
        ASSERT_TRUE( script.HasSubstr( "normal_map tangent_space" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: emission as a colour, as a texture added after the lighting, and as a derived texture with the factor folded in" )
    try
    {
        ASSERT_TRUE( parsed );
        Ogre::Pass* hot = GetPass( "Hot" );
        if ( GUCEF_NULL != hot )
            ASSERT_TRUE( Near( hot->getSelfIllumination().r, 0.4f ) && Near( hot->getSelfIllumination().g, 0.8f ) && Near( hot->getSelfIllumination().b, 1.2f ) );
        Ogre::Pass* glow = GetPass( "Glow" );
        if ( GUCEF_NULL != glow )
        {
            ASSERT_TRUE( 1 == glow->getNumTextureUnitStates() );
            if ( 1 == glow->getNumTextureUnitStates() )
            {
                Ogre::TextureUnitState* unit = glow->getTextureUnitState( 0 );
                ASSERT_TRUE( "Emissive" == unit->getName() && "pack/tex/emissive.png" == unit->getTextureName() && unit->isHardwareGammaEnabled() );
                ASSERT_TRUE( Ogre::LBX_ADD == unit->getColourBlendMode().operation );
            }
        }
        ASSERT_TRUE( script.HasSubstr( "texturing_stage late_add_blend" ) >= 0 );
        Ogre::Pass* dim = GetPass( "Dim" );
        if ( GUCEF_NULL != dim && 1 == dim->getNumTextureUnitStates() )
        {
            const CORE::CString name = dim->getTextureUnitState( 0 )->getTextureName();
            ASSERT_TRUE( name.StartsWith( "pack/textures/derived/" ) );
            UInt32 width = 0, height = 0;
            UInt8 texel[ 4 ] = { 0, 0, 0, 0 };
            ASSERT_TRUE( ReadTestPicture( GetProfileDir( "materials" ) + "/" + name, width, height, texel ) );
            // 0.5 on the sRGB values with the exponent 1 / 2.2: a factor of 0.7297
            ASSERT_TRUE( abs( (int) texel[ 0 ] - 73 ) <= 1 && abs( (int) texel[ 1 ] - 146 ) <= 1 && abs( (int) texel[ 2 ] - 36 ) <= 1 );
        }
        else
        {
            ERRORHERE;
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: alpha MASK, alpha BLEND and double sided" )
    try
    {
        ASSERT_TRUE( parsed );
        Ogre::Pass* leaf = GetPass( "Leaf" );
        if ( GUCEF_NULL != leaf )
        {
            ASSERT_TRUE( Ogre::CMPF_GREATER_EQUAL == leaf->getAlphaRejectFunction() && 89 == leaf->getAlphaRejectValue() );
            ASSERT_TRUE( Ogre::CULL_NONE == leaf->getCullingMode() );
            ASSERT_TRUE( leaf->getDepthWriteEnabled() );
        }
        Ogre::Pass* glass = GetPass( "Glass" );
        if ( GUCEF_NULL != glass )
        {
            ASSERT_TRUE( Ogre::SBF_SOURCE_ALPHA == glass->getSourceBlendFactor() && Ogre::SBF_ONE_MINUS_SOURCE_ALPHA == glass->getDestBlendFactor() );
            ASSERT_FALSE( glass->getDepthWriteEnabled() );
            ASSERT_TRUE( Near( glass->getDiffuse().a, 0.5f ) );
            ASSERT_TRUE( Ogre::CULL_CLOCKWISE == glass->getCullingMode() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Materials: an extension without counterpart is reported, the material is still written" )
    try
    {
        ASSERT_TRUE( parsed );
        ASSERT_TRUE( ReportHas( report, REPORTITEM_APPROXIMATION, "the extension KHR_materials_clearcoat has no counterpart" ) );
        ASSERT_TRUE( GUCEF_NULL != GetPass( "Coat" ) );
        DropOgreGroup( MaterialGroup );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
