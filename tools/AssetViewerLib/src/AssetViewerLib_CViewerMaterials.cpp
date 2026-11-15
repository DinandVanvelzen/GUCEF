/*
 *  AssetViewerLib: the materials and textures the views draw with themselves
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

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "AssetViewerLib_CViewerMaterials.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CViewerMaterials::LinesMaterialName = "AssetViewer/Lines";
const CORE::CString CViewerMaterials::UvCheckerMaterialName = "AssetViewer/UvChecker";
const CORE::CString CViewerMaterials::VertexColoursMaterialName = "AssetViewer/VertexColours";
const CORE::CString CViewerMaterials::UvCheckerTextureName = "AssetViewer/UvChecker";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A square checkerboard of 8 x 8 cells that runs from dark to light in u and from red to green in v */
void
MakeCheckerImage( Ogre::Image& image )
{GUCEF_TRACE;

    const UInt32 size = 256;
    const UInt32 cells = 8;
    const Ogre::PixelFormat format = Ogre::PF_BYTE_RGBA;
    image.create( format, size, size );
    UInt8* data = image.getData();
    for ( UInt32 y=0; y<size; ++y )
    {
        for ( UInt32 x=0; x<size; ++x )
        {
            const UInt32 cellX = x * cells / size;
            const UInt32 cellY = y * cells / size;
            const bool dark = 0 != ( ( cellX + cellY ) & 1 );
            const Float32 u = (Float32) x / (Float32) size;
            const Float32 v = (Float32) y / (Float32) size;
            const Float32 shade = dark ? 0.55f : 1.0f;
            UInt8* pixel = data + ( y * size + x ) * 4;
            pixel[ 0 ] = (UInt8) ( 255.0f * shade * ( 0.35f + 0.65f * u ) );
            pixel[ 1 ] = (UInt8) ( 255.0f * shade * ( 0.35f + 0.65f * v ) );
            pixel[ 2 ] = (UInt8) ( 255.0f * shade * 0.45f );
            pixel[ 3 ] = 255;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CViewerMaterials::Ensure( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() || GUCEF_NULL == Ogre::MaterialManager::getSingletonPtr() ||
         GUCEF_NULL == Ogre::TextureManager::getSingletonPtr() )
        return false;

    try
    {
        Ogre::MaterialManager& materials = Ogre::MaterialManager::getSingleton();

        if ( !materials.resourceExists( LinesMaterialName.STL_String() ) )
        {
            Ogre::MaterialPtr lines = materials.create( LinesMaterialName.STL_String(), Ogre::RGN_INTERNAL );
            Ogre::Pass* pass = lines->getTechnique( 0 )->getPass( 0 );
            pass->setLightingEnabled( false );
            pass->setVertexColourTracking( Ogre::TVC_DIFFUSE );
        }

        if ( !materials.resourceExists( VertexColoursMaterialName.STL_String() ) )
        {
            Ogre::MaterialPtr colours = materials.create( VertexColoursMaterialName.STL_String(), Ogre::RGN_INTERNAL );
            Ogre::Pass* pass = colours->getTechnique( 0 )->getPass( 0 );
            pass->setLightingEnabled( false );
            pass->setVertexColourTracking( Ogre::TVC_DIFFUSE );
        }

        if ( !Ogre::TextureManager::getSingleton().resourceExists( UvCheckerTextureName.STL_String() ) )
        {
            Ogre::Image image;
            MakeCheckerImage( image );
            Ogre::TextureManager::getSingleton().loadImage( UvCheckerTextureName.STL_String(), Ogre::RGN_INTERNAL, image );
        }

        if ( !materials.resourceExists( UvCheckerMaterialName.STL_String() ) )
        {
            Ogre::MaterialPtr checker = materials.create( UvCheckerMaterialName.STL_String(), Ogre::RGN_INTERNAL );
            Ogre::Pass* pass = checker->getTechnique( 0 )->getPass( 0 );
            pass->setLightingEnabled( true );
            pass->setAmbient( 1.0f, 1.0f, 1.0f );
            pass->setDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
            pass->setSpecular( 0.0f, 0.0f, 0.0f, 1.0f );
            pass->createTextureUnitState( UvCheckerTextureName.STL_String() );
        }
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerMaterials: The materials of the views cannot be made: " << e.getFullDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CViewerMaterials::Release( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == Ogre::Root::getSingletonPtr() )
        return;
    try
    {
        const CORE::CString names[ 3 ] = { LinesMaterialName, UvCheckerMaterialName, VertexColoursMaterialName };
        for ( UInt32 i=0; i<3; ++i )
            if ( GUCEF_NULL != Ogre::MaterialManager::getSingletonPtr() && Ogre::MaterialManager::getSingleton().resourceExists( names[ i ].STL_String() ) )
                Ogre::MaterialManager::getSingleton().remove( names[ i ].STL_String() );
        if ( GUCEF_NULL != Ogre::TextureManager::getSingletonPtr() && Ogre::TextureManager::getSingleton().resourceExists( UvCheckerTextureName.STL_String() ) )
            Ogre::TextureManager::getSingleton().remove( UvCheckerTextureName.STL_String() );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CViewerMaterials: The materials of the views cannot be released: " << e.getFullDescription() );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
