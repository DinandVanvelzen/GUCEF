/*
 *  AssetViewerLib: renders assets from fixed views into contact sheets and thumbnails, with or without a window on the screen
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

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_IMAGE_CPNGIMAGECODEC_H
#include "gucefIMAGE_CPngImageCodec.h"
#define GUCEF_IMAGE_CPNGIMAGECODEC_H
#endif /* GUCEF_IMAGE_CPNGIMAGECODEC_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#include "gucefIMAGE_CPixelMapResampler.h"
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */

#ifndef ASSETVIEWERLIB_CSCENEVIEW_H
#include "AssetViewerLib_CSceneView.h"
#include "AssetViewerLib_CParticlePreview.h"
#define ASSETVIEWERLIB_CSCENEVIEW_H
#endif /* ASSETVIEWERLIB_CSCENEVIEW_H ? */

#ifndef ASSETVIEWERLIB_CMATERIALVIEW_H
#include "AssetViewerLib_CMaterialView.h"
#define ASSETVIEWERLIB_CMATERIALVIEW_H
#endif /* ASSETVIEWERLIB_CMATERIALVIEW_H ? */

#ifndef ASSETVIEWERLIB_CTEXTUREVIEW_H
#include "AssetViewerLib_CTextureView.h"
#define ASSETVIEWERLIB_CTEXTUREVIEW_H
#endif /* ASSETVIEWERLIB_CTEXTUREVIEW_H ? */

#include "AssetViewerLib_CAssetCapture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** How long particle systems run before a capture: long enough for most effects to fill */
const Float32 CaptureParticleTime = 3.0f;

const UInt8 SheetRed = 20;
const UInt8 SheetGreen = 21;
const UInt8 SheetBlue = 25;

/** Draws the stage when the frame starts, which is where the render systems and the shader generator expect render targets to be drawn */
class CStageFrameListener : public Ogre::FrameListener
{
    public:

    explicit CStageFrameListener( CViewerStage& stage ) : m_stage( stage ), m_rendered( false ) {}

    virtual bool frameStarted( const Ogre::FrameEvent& evt ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_rendered = m_stage.Render();
        return true;
    }

    bool WasRendered( void ) const { return m_rendered; }

    private:

    CViewerStage& m_stage;
    bool m_rendered;
};

IMAGE::TPixelMapPtr
MakeSheet( UInt32 width, UInt32 height )
{
    IMAGE::TPixelMapPtr sheet( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( sheet->GetDataPtr() );
    for ( size_t i=0; i<(size_t) width*height; ++i )
    {
        data[ i*4 ] = SheetRed;
        data[ i*4+1 ] = SheetGreen;
        data[ i*4+2 ] = SheetBlue;
        data[ i*4+3 ] = 255;
    }
    return sheet;
}

/** Copies a picture into a sheet at a place, row by row */
void
Blit( const IMAGE::TPixelMapPtr& source, IMAGE::TPixelMapPtr& target, UInt32 x, UInt32 y )
{
    const UInt32 width = source->GetWidthInPixels();
    const UInt32 height = source->GetHeightInPixels();
    const UInt32 targetWidth = target->GetWidthInPixels();
    const UInt32 targetHeight = target->GetHeightInPixels();
    for ( UInt32 row=0; row<height && y+row<targetHeight; ++row )
    {
        const UInt32 columns = x+width > targetWidth ? targetWidth - x : width;
        memcpy( static_cast< UInt8* >( target->GetDataPtr() ) + ( (size_t) ( y+row ) * targetWidth + x ) * 4 ,
                static_cast< const UInt8* >( source->GetDataPtr() ) + (size_t) row * width * 4                ,
                (size_t) columns * 4                                                                          );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetCapture::CAssetCapture( GUIDRIVEROGRE::COgreWindowContext* window )
    : m_window( window )
    , m_stage( "Capture" )
    , m_tileSize( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetCapture::~CAssetCapture()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::Initialize( UInt32 tileSize )
{GUCEF_TRACE;

    if ( tileSize < 16 )
        tileSize = 16;
    if ( m_stage.IsInitialized() )
        return m_stage.Resize( tileSize, tileSize );
    if ( !m_stage.Initialize( tileSize, tileSize ) )
        return false;
    m_tileSize = tileSize;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetCapture::Shutdown( void )
{GUCEF_TRACE;

    m_stage.Shutdown();
    m_tileSize = 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_stage.IsInitialized();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetCapture::GetTileSize( void ) const
{GUCEF_TRACE;

    return m_tileSize;
}

/*-------------------------------------------------------------------------*/

CViewerStage&
CAssetCapture::GetStage( void )
{GUCEF_TRACE;

    return m_stage;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetCapture::GetSheetSize( UInt32 tileSize )
{GUCEF_TRACE;

    return 2 * tileSize + 3 * (UInt32) SHEET_MARGIN;
}

/*-------------------------------------------------------------------------*/

void
CAssetCapture::GetTileOrigin( UInt32 tileSize, UInt32 tileIndex, UInt32& x, UInt32& y )
{GUCEF_TRACE;

    x = (UInt32) SHEET_MARGIN + ( tileIndex % 2 ) * ( tileSize + (UInt32) SHEET_MARGIN );
    y = (UInt32) SHEET_MARGIN + ( ( tileIndex / 2 ) % 2 ) * ( tileSize + (UInt32) SHEET_MARGIN );
}

/*-------------------------------------------------------------------------*/

void
CAssetCapture::GetSheetColour( UInt8& red, UInt8& green, UInt8& blue )
{GUCEF_TRACE;

    red = SheetRed;
    green = SheetGreen;
    blue = SheetBlue;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::RenderStage( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !m_stage.IsInitialized() )
        return false;
    CStageFrameListener listener( m_stage );
    Ogre::Root& root = Ogre::Root::getSingleton();
    root.addFrameListener( &listener );
    bool framed = false;
    try
    {
        framed = GUCEF_NULL != m_window ? m_window->RenderFrame() : root.renderOneFrame();
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CAssetCapture: The frame cannot be drawn: " << e.getFullDescription() );
    }
    root.removeFrameListener( &listener );
    if ( !framed || !listener.WasRendered() )
        return false;
    return m_stage.ReadPixels( pixels );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::CaptureViewsOf( CModelView* model, COrbitController& controller, IMAGE::TPixelMapPtr& sheet )
{GUCEF_TRACE;

    if ( !m_stage.IsInitialized() )
        return false;
    const UInt32 tile = m_stage.GetWidth();
    m_stage.SetGridVisible( true );
    m_stage.SetAxesVisible( true );
    sheet = MakeSheet( GetSheetSize( tile ), GetSheetSize( tile ) );
    controller.SetAspectRatio( 1.0f );
    for ( UInt32 view=0; view<(UInt32) VIEWS_PER_SHEET; ++view )
    {
        controller.ApplyPreset( static_cast< COrbitController::EViewPreset >( view ) );
        if ( GUCEF_NULL != model )
            model->Update( controller );
        m_stage.ApplyController( controller );
        IMAGE::TPixelMapPtr picture;
        if ( !RenderStage( picture ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetCapture: View " << view << " cannot be drawn" );
            sheet.Unlink();
            return false;
        }
        UInt32 x = 0, y = 0;
        GetTileOrigin( tile, view, x, y );
        Blit( picture, sheet, x, y );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::CaptureMeshSheet( const CORE::CString& groupName, const CORE::CString& meshName, IMAGE::TPixelMapPtr& sheet )
{GUCEF_TRACE;

    if ( !m_stage.IsInitialized() )
        return false;
    m_stage.Resize( m_tileSize, m_tileSize );
    CModelView model( m_stage );
    if ( !model.LoadMesh( groupName, meshName ) )
        return false;
    COrbitController controller;
    model.FrameInController( controller );
    return CaptureViewsOf( &model, controller, sheet );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::CaptureTextureSheet( const CORE::CString& vfsPath, UInt32 size, bool square, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    CTextureView texture;
    if ( !texture.Load( vfsPath ) )
        return false;
    IMAGE::TPixelMapPtr source = texture.GetFirstPixelMap();
    if ( source.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetCapture: " << vfsPath << " has no picture to show: " << texture.GetInfo().note );
        return false;
    }

    // fitted in the square, keeping the shape of the picture
    const UInt32 width = source->GetWidthInPixels();
    const UInt32 height = source->GetHeightInPixels();
    const Float32 scale = (Float32) size / (Float32) ( width > height ? width : height );
    UInt32 targetWidth = (UInt32) ( (Float32) width * scale + 0.5f );
    UInt32 targetHeight = (UInt32) ( (Float32) height * scale + 0.5f );
    if ( targetWidth < 1 )
        targetWidth = 1;
    if ( targetHeight < 1 )
        targetHeight = 1;
    IMAGE::TPixelMapPtr scaled = source;
    if ( targetWidth != width || targetHeight != height )
        if ( !IMAGE::CPixelMapResampler::Resample( *source, targetWidth, targetHeight, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::COLOURSPACE_SRGB, IMAGE::WRAPMODE_CLAMP, scaled ) )
            return false;
    if ( !square )
    {
        pixels = scaled;
        return true;
    }
    pixels = MakeSheet( size, size );
    Blit( scaled, pixels, ( size - targetWidth ) / 2, ( size - targetHeight ) / 2 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::CaptureNonTexture( CAssetMounter& mounter, const CORE::CString& vfsPath, EAssetKind kind, bool sheetNotThumbnail, UInt32 thumbnailSize, IMAGE::TPixelMapPtr& result )
{GUCEF_TRACE;

    if ( !m_stage.IsInitialized() )
        return false;
    CORE::CString relative;
    const CORE::CString group = mounter.GetResourceGroup( vfsPath, relative );
    if ( group.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetCapture: " << vfsPath << " is not in a mount" );
        return false;
    }
    const UInt32 size = sheetNotThumbnail ? m_tileSize : thumbnailSize;
    m_stage.Resize( size, size );
    COrbitController controller;
    controller.SetAspectRatio( 1.0f );

    CModelView model( m_stage );
    CMaterialView material( m_stage );
    CSceneView scene( m_stage );
    CParticlePreview particles;
    CModelView* shown = GUCEF_NULL;
    switch ( kind )
    {
        case ASSETKIND_MODEL:
        {
            if ( !model.LoadMesh( group, relative ) )
                return false;
            model.FrameInController( controller );
            shown = &model;
            break;
        }
        case ASSETKIND_MATERIAL:
        {
            if ( !material.LoadScript( group, relative ) )
                return false;
            material.GetPreview().FrameInController( controller );
            shown = &material.GetPreview();
            break;
        }
        case ASSETKIND_SCENE:
        {
            if ( !scene.Load( group, relative ) )
                return false;
            // particle systems are shown after they have run a while
            if ( particles.Collect( scene.GetRoot() ) > 0 )
                particles.SetTime( CaptureParticleTime );
            COrbitController::SVector3 center;
            Float32 radius = 1.0f;
            scene.GetBoundingSphere( center, radius );
            controller.SetBounds( center, radius );
            break;
        }
        case ASSETKIND_PARTICLE:
        {
            CORE::CString::StringVector templates;
            if ( !CParticlePreview::ParseParticleScript( group, relative, templates ) ||
                 0 == particles.CreateFromTemplates( m_stage.GetSceneManager(), m_stage.GetContentNode(), templates ) )
                return false;
            particles.SetTime( CaptureParticleTime );
            COrbitController::SVector3 center;
            Float32 radius = 1.0f;
            particles.GetBoundingSphere( center, radius );
            controller.SetBounds( center, radius );
            break;
        }
        default:
        {
            return false;
        }
    }

    if ( sheetNotThumbnail )
        return CaptureViewsOf( shown, controller, result );

    // one picture, the three quarter view; without the grid, which is clutter at this size
    m_stage.SetGridVisible( false );
    m_stage.SetAxesVisible( false );
    controller.ApplyPreset( COrbitController::VIEWPRESET_THREE_QUARTER );
    if ( GUCEF_NULL != shown )
        shown->Update( controller );
    m_stage.ApplyController( controller );
    return RenderStage( result );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::CaptureContactSheet( CAssetMounter& mounter, const CORE::CString& vfsPath, IMAGE::TPixelMapPtr& sheet )
{GUCEF_TRACE;

    const EAssetKind kind = CAssetTypeResolver::GetKind( vfsPath );
    if ( ASSETKIND_TEXTURE == kind )
        return CaptureTextureSheet( vfsPath, GetSheetSize( m_tileSize ), true, sheet );
    return CaptureNonTexture( mounter, vfsPath, kind, true, 0, sheet );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::CaptureThumbnail( CAssetMounter& mounter, const CORE::CString& vfsPath, UInt32 size, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    const EAssetKind kind = CAssetTypeResolver::GetKind( vfsPath );
    if ( ASSETKIND_TEXTURE == kind )
        return CaptureTextureSheet( vfsPath, size, false, pixels );
    return CaptureNonTexture( mounter, vfsPath, kind, false, size, pixels );
}

/*-------------------------------------------------------------------------*/

bool
CAssetCapture::WritePng( const IMAGE::TPixelMapPtr& pixels, const CORE::CString& path )
{GUCEF_TRACE;

    if ( pixels.IsNULL() )
        return false;
    CORE::CString problem;
    if ( !IMAGE::CPngImageCodec::WritePixelMapToFile( *pixels, path, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CAssetCapture: The picture cannot be written to " << path << ": " << problem );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetCapture::MakeSheetFileName( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CString name = CAssetMounter::NormalizePath( vfsPath );
    name = name.ReplaceChar( '/', '_' ).ReplaceChar( ' ', '_' ).ReplaceChar( ':', '_' );
    return name + ".png";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
