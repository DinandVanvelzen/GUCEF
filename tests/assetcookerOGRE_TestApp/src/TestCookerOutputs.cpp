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

#include <string.h>

#include "Ogre.h"
#include "OgreCodec.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#include "TestCookerSupport.h"
#include "TestCookerScenes.h"
#include "TestCookerOutputs.h"

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

const Ogre::String HouseGroup = "CookerHouse";
const CORE::CString HouseFolder = "pack/Models/House/";

/** Cooks the house of BuildHouseScene() as "Models/House.fbx" into "work/<folder>" with the profile */
bool
CookHouse( const CORE::CString& folder, const CCookProfile& profile, CConversionReport& report )
{
    MakeWorkFolder( folder );
    const CORE::CString root = "work/" + folder;
    if ( !WriteTestPng( root + "/src/tex/wood.png", 8, 8, 120, 80, 40, 255 ) || !WriteTestPng( root + "/inter/textures/paint.png", 4, 2, 10, 20, 200, 255 ) )
        return false;
    CAssetScene scene;
    BuildHouseScene( scene, root + "/src/tex/wood.png", root + "/inter/textures/paint.png" );
    CCookSettings settings;
    settings.profile = profile;
    CCookSettings::SAsset asset;
    if ( !MakeIntermediate( scene, folder, "Models/House.fbx", asset ) )
        return false;
    settings.assets.push_back( asset );
    return CookIntermediate( folder, settings, report );
}

/** The files below a folder of the VFS, relative to it, sorted */
void
ListFiles( const CORE::CString& folder, CORE::CString::StringVector& files )
{
    VFS::CVFS::TStringVector listed;
    VFS::CVfsGlobal::Instance()->GetVfs().GetFileList( listed, folder, true, true, CORE::CString::Empty, GUCEF_UINT32MAX );
    files.clear();
    for ( size_t i=0; i<listed.size(); ++i )
    {
        CORE::CString relative;
        if ( CVfsPaths::MakeRelative( folder, CVfsPaths::Normalize( listed[ i ] ), relative ) )
            files.push_back( relative );
    }
    std::sort( files.begin(), files.end() );
}

bool
SameBytes( const CORE::CString& a, const CORE::CString& b )
{
    CORE::CDynamicBuffer first;
    CORE::CDynamicBuffer second;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.LoadFile( first, a, "rb" ) || !vfs.LoadFile( second, b, "rb" ) )
        return false;
    return first.GetDataSize() == second.GetDataSize() && 0 == memcmp( first.GetConstBufferPtr(), second.GetConstBufferPtr(), first.GetDataSize() );
}

/** Loads a DotScene file of the group into a new scene manager */
Ogre::SceneManager*
LoadScene( const Ogre::String& file )
{
    Ogre::ResourceGroupManager::getSingleton().setWorldResourceGroupName( HouseGroup );
    Ogre::SceneManager* sceneManager = Ogre::Root::getSingleton().createSceneManager();
    Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource( file, HouseGroup );
    Ogre::Codec::getCodec( "scene" )->decode( stream, Ogre::Any( sceneManager->getRootSceneNode() ) );
    return sceneManager;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCookerOutputsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetcookerOGRE.Outputs" );

    CConversionReport report;
    bool cooked = false;

    GUCEF_TESTFW_TESTCASE( "DotScene: the scene of the asset loads with OgrePlugin_DotScene: nodes, entities with the cooked meshes, the light, user data and a node animation" )
    try
    {
        cooked = CookHouse( "house", CCookProfile(), report );
        ASSERT_TRUE( cooked );
        ASSERT_FALSE( report.HasFailures() );
        ASSERT_TRUE( Ogre::Codec::isCodecRegistered( "scene" ) );
        ASSERT_TRUE( UseOgreGroup( HouseGroup, GetProfileDir( "house" ), false ) );
        Ogre::SceneManager* sceneManager = LoadScene( ( HouseFolder + "House.scene" ).STL_String() );
        ASSERT_TRUE( GUCEF_NULL != sceneManager );
        if ( GUCEF_NULL != sceneManager )
        {
            const char* const nodes[ 5 ] = { "House", "Door", "Lamp", "Sign", "Fan" };
            for ( UInt32 n=0; n<5; ++n )
                ASSERT_TRUE( sceneManager->hasSceneNode( nodes[ n ] ) );
            if ( sceneManager->hasSceneNode( "House" ) && sceneManager->hasSceneNode( "Door" ) )
            {
                Ogre::SceneNode* house = sceneManager->getSceneNode( "House" );
                ASSERT_TRUE( sceneManager->getRootSceneNode() == house->getParent() );
                ASSERT_TRUE( Near( (float) house->getPosition().x, 1.0f ) && Near( (float) house->getPosition().z, 2.0f ) );
                ASSERT_TRUE( house == sceneManager->getSceneNode( "Door" )->getParent() );
            }
            ASSERT_TRUE( sceneManager->hasEntity( "House" ) && sceneManager->hasEntity( "Door" ) && sceneManager->hasEntity( "Sign" ) && sceneManager->hasEntity( "Fan" ) );
            if ( sceneManager->hasEntity( "House" ) && sceneManager->hasEntity( "Sign" ) )
            {
                ASSERT_TRUE( ( HouseFolder + "HouseMesh.mesh" ).STL_String() == sceneManager->getEntity( "House" )->getMesh()->getName() );
                ASSERT_TRUE( sceneManager->getEntity( "House" )->getMesh() == sceneManager->getEntity( "Sign" )->getMesh() );
                const Ogre::Any tag = sceneManager->getSceneNode( "Sign" )->getUserObjectBindings().getUserAny( "tag" );
                ASSERT_TRUE( tag.has_value() && "sign" == Ogre::any_cast< Ogre::String >( tag ) );
            }
            // the materials come with the meshes; the scene of the asset overrides none
            ASSERT_TRUE( ReadVfsText( GetProfileDir( "house" ) + "/" + HouseFolder + "House.scene" ).HasSubstr( "material=" ) < 0 );
            ASSERT_TRUE( sceneManager->hasLight( "Lamp" ) );
            if ( sceneManager->hasLight( "Lamp" ) )
            {
                Ogre::Light* light = sceneManager->getLight( "Lamp" );
                ASSERT_TRUE( Ogre::Light::LT_POINT == light->getType() );
                ASSERT_TRUE( Near( light->getDiffuseColour().g, 0.9f ) && Near( (float) light->getPowerScale(), 5.0f ) && Near( (float) light->getAttenuationRange(), 10.0f ) );
            }
            ASSERT_TRUE( sceneManager->hasAnimation( "Spin/Fan" ) );
            if ( sceneManager->hasAnimation( "Spin/Fan" ) )
                ASSERT_TRUE( Near( (float) sceneManager->getAnimation( "Spin/Fan" )->getLength(), 2.0f ) && 3 == sceneManager->getAnimation( "Spin/Fan" )->getNodeTrack( 0 )->getNumKeyFrames() );
            Ogre::Root::getSingleton().destroySceneManager( sceneManager );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "DotScene: a prefab becomes a scene of its own, with the material override of its instance" )
    try
    {
        ASSERT_TRUE( cooked );
        ASSERT_TRUE( VfsFileExists( GetProfileDir( "house" ) + "/" + HouseFolder + "Doorway.scene" ) );
        Ogre::SceneManager* sceneManager = LoadScene( ( HouseFolder + "Doorway.scene" ).STL_String() );
        if ( GUCEF_NULL != sceneManager )
        {
            ASSERT_TRUE( sceneManager->hasSceneNode( "Door" ) && !sceneManager->hasSceneNode( "House" ) && !sceneManager->hasSceneNode( "Lamp" ) );
            ASSERT_TRUE( sceneManager->hasEntity( "Door" ) );
            if ( sceneManager->hasEntity( "Door" ) )
            {
                ASSERT_TRUE( ( HouseFolder + "DoorMesh.mesh" ).STL_String() == sceneManager->getEntity( "Door" )->getMesh()->getName() );
            }
            // the override of the instance: the entity of the door takes Wood in place of Paint (the materials are not
            // made here: without a render system an entity cannot compile them)
            const CORE::CString text = ReadVfsText( GetProfileDir( "house" ) + "/" + HouseFolder + "Doorway.scene" );
            ASSERT_TRUE( text.HasSubstr( "<entity name=\"Door\" meshFile=\"pack/Models/House/DoorMesh.mesh\" material=\"pack/Models/House/Wood\" />" ) >= 0 );
            const Ogre::Any instance = sceneManager->getSceneNode( "Door" )->getUserObjectBindings().getUserAny( "prefabInstance" );
            ASSERT_TRUE( instance.has_value() && "Prefabs/Door.prefab" == Ogre::any_cast< Ogre::String >( instance ) );
            Ogre::Root::getSingleton().destroySceneManager( sceneManager );
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

    GUCEF_TESTFW_TESTCASE( "DotScene: a skinned mesh is drawn in the space of the scene, the clip of a node that is no joint is a node animation" )
    try
    {
        MakeWorkFolder( "skinscene" );
        ASSERT_TRUE( WriteTestPng( "work/skinscene/src/tex/skin.png", 4, 4, 200, 150, 120, 255 ) && WriteTestPng( "work/skinscene/src/tex/normal.png", 4, 4, 128, 128, 255, 255 ) );
        CAssetScene scene;
        BuildCharacterScene( scene, "work/skinscene/src/tex" );
        CCookSettings settings;
        CCookSettings::SAsset asset;
        ASSERT_TRUE( MakeIntermediate( scene, "skinscene", "Hero.fbx", asset ) );
        settings.assets.push_back( asset );
        CConversionReport skinReport;
        ASSERT_TRUE( CookIntermediate( "skinscene", settings, skinReport ) );
        ASSERT_TRUE( UseOgreGroup( HouseGroup, GetProfileDir( "skinscene" ), false ) );
        Ogre::SceneManager* sceneManager = LoadScene( "pack/Hero/Hero.scene" );
        if ( GUCEF_NULL != sceneManager )
        {
            ASSERT_TRUE( sceneManager->hasSceneNode( "Body.skin" ) && sceneManager->hasEntity( "Body" ) );
            if ( sceneManager->hasSceneNode( "Body.skin" ) && sceneManager->hasEntity( "Body" ) )
            {
                ASSERT_TRUE( sceneManager->getRootSceneNode() == sceneManager->getSceneNode( "Body.skin" )->getParent() );
                Ogre::Entity* body = sceneManager->getEntity( "Body" );
                ASSERT_TRUE( body->hasSkeleton() && body->getParentSceneNode() == sceneManager->getSceneNode( "Body.skin" ) );
                ASSERT_TRUE( body->getAllAnimationStates()->hasAnimationState( "Walk" ) );
            }
            ASSERT_TRUE( sceneManager->hasEntity( "Prop" ) && sceneManager->hasAnimation( "Spin/Prop" ) );
            if ( sceneManager->hasEntity( "Prop" ) )
                ASSERT_TRUE( sceneManager->getEntity( "Prop" )->getAllAnimationStates()->hasAnimationState( "Blink" ) );
            Ogre::Root::getSingleton().destroySceneManager( sceneManager );
        }
        else
        {
            ERRORHERE;
        }
        DropOgreGroup( HouseGroup );
        UseOgreGroup( HouseGroup, GetProfileDir( "house" ), false );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Colliders: <asset>.collision.json holds the shapes in node space, the mesh of a convex shape included" )
    try
    {
        ASSERT_TRUE( cooked );
        const CORE::CString path = GetProfileDir( "house" ) + "/" + HouseFolder + "House.collision.json";
        CORE::CDataNode document;
        ASSERT_TRUE( CCanonicalJson::Load( path, document ) );
        ASSERT_TRUE( "Models/House.fbx" == document.GetAttributeValueOrChildValueByName( "asset" ).AsString() );
        ASSERT_TRUE( ( HouseFolder + "House.scene" ) == document.GetAttributeValueOrChildValueByName( "scene" ).AsString() );
        const CORE::CDataNode* colliders = document.FindChild( "colliders" );
        ASSERT_TRUE( GUCEF_NULL != colliders && 2 == colliders->GetNrOfDirectChildNodes() );
        if ( GUCEF_NULL != colliders && 2 == colliders->GetNrOfDirectChildNodes() )
        {
            CORE::CDataNode::const_iterator i = colliders->ConstBegin();
            const CORE::CDataNode& box = *(*i);
            ++i;
            const CORE::CDataNode& hull = *(*i);
            ASSERT_TRUE( "House" == box.GetAttributeValueOrChildValueByName( "node" ).AsString() && "box" == box.GetAttributeValueOrChildValueByName( "shape" ).AsString() );
            CORE::CString::StringVector values;
            box.GetValuesOfChildByName( "halfExtents", values );
            ASSERT_TRUE( 3 == values.size() && Near( CORE::StringToFloat( values[ 0 ] ), 1.0f ) && Near( CORE::StringToFloat( values[ 2 ] ), 3.0f ) );
            box.GetValuesOfChildByName( "translation", values );
            ASSERT_TRUE( 3 == values.size() && Near( CORE::StringToFloat( values[ 1 ] ), 2.0f ) );
            ASSERT_FALSE( box.GetAttributeValueOrChildValueByName( "trigger" ).AsBool() );
            ASSERT_TRUE( "Door" == hull.GetAttributeValueOrChildValueByName( "node" ).AsString() && "House/Door" == hull.GetAttributeValueOrChildValueByName( "nodePath" ).AsString() );
            ASSERT_TRUE( "convex" == hull.GetAttributeValueOrChildValueByName( "shape" ).AsString() && hull.GetAttributeValueOrChildValueByName( "trigger" ).AsBool() );
            const CORE::CDataNode* mesh = hull.FindChild( "mesh" );
            ASSERT_TRUE( GUCEF_NULL != mesh );
            if ( GUCEF_NULL != mesh )
            {
                mesh->GetValuesOfChildByName( "vertices", values );
                ASSERT_TRUE( 12 == values.size() );
                mesh->GetValuesOfChildByName( "indices", values );
                ASSERT_TRUE( 6 == values.size() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Names: the files of an asset have stable names below <pack id>/<asset path>, are reported with their references and are the same bytes for the same input" )
    try
    {
        ASSERT_TRUE( cooked );
        CORE::CString::StringVector files;
        ListFiles( GetProfileDir( "house" ), files );
        CORE::CString::StringVector expected;
        expected.push_back( "pack/Models/House/DoorMesh.mesh" );
        expected.push_back( "pack/Models/House/Doorway.scene" );
        expected.push_back( "pack/Models/House/House.collision.json" );
        expected.push_back( "pack/Models/House/House.material" );
        expected.push_back( "pack/Models/House/House.scene" );
        expected.push_back( "pack/Models/House/HouseMesh.mesh" );
        expected.push_back( "pack/tex/wood.png" );
        expected.push_back( "pack/textures/paint.png" );
        ASSERT_TRUE( expected == files );

        // every file is reported, with what it references
        const CORE::CString profile = GetProfileDir( "house" ) + "/";
        for ( size_t f=0; f<expected.size(); ++f )
            ASSERT_TRUE( ReportHasFile( report, profile + expected[ f ] ) );
        const CConversionReportEntry* entry = report.FindEntry( "Models/House.fbx" );
        ASSERT_TRUE( GUCEF_NULL != entry );
        if ( GUCEF_NULL != entry )
        {
            for ( size_t f=0; f<entry->files.size(); ++f )
            {
                const CConversionReportFile& file = entry->files[ f ];
                ASSERT_TRUE( "desktop" == file.profile );
                if ( file.path == profile + "pack/Models/House/HouseMesh.mesh" )
                    ASSERT_TRUE( 1 == file.references.size() && profile + "pack/Models/House/House.material" == file.references[ 0 ] );
                if ( file.path == profile + "pack/Models/House/House.material" )
                    ASSERT_TRUE( 2 == file.references.size() );
                if ( file.path == profile + "pack/Models/House/House.scene" )
                    ASSERT_TRUE( 2 == file.references.size() );
            }
        }

        // the same input again: the same bytes
        CConversionReport again;
        ASSERT_TRUE( CookHouse( "house2", CCookProfile(), again ) );
        for ( size_t f=0; f<expected.size(); ++f )
            ASSERT_TRUE( SameBytes( profile + expected[ f ], GetProfileDir( "house2" ) + "/" + expected[ f ] ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Textures: copied as they are, or written as PNG within the largest size of the profile; mips and formats of the profile" )
    try
    {
        ASSERT_TRUE( cooked );
        // as they are
        ASSERT_TRUE( SameBytes( "work/house/src/tex/wood.png", GetProfileDir( "house" ) + "/pack/tex/wood.png" ) );
        ASSERT_TRUE( SameBytes( "work/house/inter/textures/paint.png", GetProfileDir( "house" ) + "/pack/textures/paint.png" ) );

        // a profile with a largest size of 2, no mips and BC7 for colour
        CCookProfile small( "small" );
        small.maxTextureSize = 2;
        small.generateMips = false;
        small.textureFormats[ TEXTUREUSAGE_COLOUR ] = "BC7";
        CConversionReport smallReport;
        ASSERT_TRUE( CookHouse( "housesmall", small, smallReport ) );
        UInt32 width = 0, height = 0;
        UInt8 texel[ 4 ] = { 0, 0, 0, 0 };
        ASSERT_TRUE( ReadTestPicture( GetProfileDir( "housesmall", "small" ) + "/pack/tex/wood.png", width, height, texel ) );
        ASSERT_TRUE( 2 == width && 2 == height );
        ASSERT_TRUE( abs( (int) texel[ 0 ] - 120 ) <= 1 && abs( (int) texel[ 1 ] - 80 ) <= 1 );
        ASSERT_TRUE( ReadTestPicture( GetProfileDir( "housesmall", "small" ) + "/pack/textures/paint.png", width, height, texel ) );
        ASSERT_TRUE( 2 == width && 1 == height );
        const CORE::CString script = ReadVfsText( GetProfileDir( "housesmall", "small" ) + "/" + HouseFolder + "House.material" );
        ASSERT_TRUE( script.HasSubstr( "texture \"pack/tex/wood.png\" 0 gamma" ) >= 0 );
        ASSERT_TRUE( ReportHas( smallReport, REPORTITEM_APPROXIMATION, "asks for BC7, which is not made yet" ) );
        DropOgreGroup( HouseGroup );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
