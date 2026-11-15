/*
 *  guidriverRmlUIOgre_TestApp: Test application for the GUI driver RmlUiOgre (guidriverRmlUIOgre)
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

#include <stdlib.h>
#include <string.h>

#include <Ogre.h>
#include <OgreCodec.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTexture.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#include "guidriverRmlUIOgre_CRmlUIOgreContext.h"
#include "guidriverRmlUIOgre_CRmlUIOgreGuiDriver.h"
#include "guidriverRmlUIOgre_CRmlUIOgreRenderContext.h"
#include "guidriverRmlUIOgre_CRmlUIOgreRenderInterface.h"

#include "TestOgreSupport.h"
#include "TestRmlUiOgreContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const GUI::CColour Red( 255, 0, 0, 255 );
const GUI::CColour Green( 0, 255, 0, 255 );
const GUI::CColour White( 255, 255, 255, 255 );
const GUI::CColour Yellow( 255, 255, 0, 255 );

/** The image codec of the tests: "OGW1" and the four bytes of a colour become a 4 x 4 RGBA picture of that colour */
class CTestTextureCodec : public IMAGE::CIImageCodec
{
    public:

    CTestTextureCodec( void ) : IMAGE::CIImageCodec() {}
    virtual ~CTestTextureCodec() GUCEF_VIRTUAL_OVERRIDE {}

    virtual bool Encode( const IMAGE::CImage& inputImage, CORE::CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_CODEC_CAPABILITY_DECODE; }
    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ogwtex"; }
    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "ImageCodec"; }
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestTextureCodec(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTestTextureCodec";
        return classTypeName;
    }

    virtual bool Decode( CORE::CIOAccess& encodedInput, IMAGE::CImage& outputImage ) GUCEF_VIRTUAL_OVERRIDE
    {
        char header[ 8 ];
        if ( 8 != encodedInput.Read( header, 1, 8 ) || 0 != memcmp( header, "OGW1", 4 ) )
            return false;
        UInt8 pixels[ 4 * 4 * 4 ];
        for ( UInt32 i=0; i<16; ++i )
            memcpy( pixels + i*4, header + 4, 4 );
        outputImage.Assign( IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( pixels, 4, 4, IMAGE::PSF_RGBA, DATATYPE_UINT8 ) ) );
        return true;
    }
};

typedef CORE::CTSharedPtr< CTestTextureCodec, MT::CMutex > TTestTextureCodecPtr;

/*-------------------------------------------------------------------------*/

GUI::CColour
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return GUI::CColour( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}

/** Same colour, give or take a little for the rounding of blending on a GPU (as the RmlUi render tests of gucefGUI) */
bool
Near( const GUI::CColour& a, const GUI::CColour& b, Int32 tolerance = 2 )
{
    return abs( (Int32) a.r - (Int32) b.r ) <= tolerance && abs( (Int32) a.g - (Int32) b.g ) <= tolerance &&
           abs( (Int32) a.b - (Int32) b.b ) <= tolerance;
}

IMAGE::TPixelMapPtr
MakeSolidImage( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 i=0; i<width*height; ++i )
    {
        data[ i*4 ] = r;
        data[ i*4+1 ] = g;
        data[ i*4+2 ] = b;
        data[ i*4+3 ] = a;
    }
    return pixels;
}

/** A white atlas with one cell per glyph: every glyph of ASCII 32..126 is a full block */
GUI::TBitmapFontPtr
MakeTestFont( Int32 width, Int32 height )
{
    UInt32 count = 126 - 32 + 1;
    GUI::TBitmapFontPtr font( GUCEF_NEW GUI::CBitmapFont() );
    font->SetAtlas( MakeSolidImage( (UInt32) width * count, (UInt32) height, 255, 255, 255, 255 ) );
    for ( UInt32 cp=32; cp<=126; ++cp )
    {
        GUI::CBitmapFont::TGlyph glyph;
        glyph.source = GUI::CRect( (Int32) ( cp - 32 ) * width, 0, width, height );
        glyph.advance = width;
        font->SetGlyph( cp, glyph );
    }
    font->SetLineHeight( height );
    font->SetBaseline( height );
    font->SetFallbackCodePoint( '?' );
    return font;
}

GUI::CWidget*
Screen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

bool
Capture( GUI::CGUIContext* context, IMAGE::TPixelMapPtr& pixels )
{
    context->RenderFrame( 16 );
    return context->CaptureFrame( pixels ) && !pixels.IsNULL() && 640 == pixels->GetWidthInPixels() && 480 == pixels->GetHeightInPixels();
}

/** The scene of every kind of drawing of the render tests of gucefGUI (TestGuiRender.cpp, BuildMixedScene) */
void
BuildMixedScene( GUI::CGUIContext* context )
{
    context->RegisterFont( "Block", MakeTestFont( 6, 9 ) );
    IMAGE::TPixelMapPtr checker = MakeSolidImage( 4, 4, 255, 255, 255, 255 );
    UInt8* data = static_cast< UInt8* >( checker->GetDataPtr() );
    for ( UInt32 i=0; i<16; ++i )
    {
        if ( 0 == ( ( i % 4 ) + ( i / 4 ) ) % 2 )
        {
            data[ i*4 ] = 200; data[ i*4+1 ] = 30; data[ i*4+2 ] = 60; data[ i*4+3 ] = ( i % 3 ) ? 255 : 128;
        }
    }
    context->RegisterImage( "Checker", checker );

    GUI::CWidget* screen = Screen( context );
    screen->SetColourProperty( "BackgroundColour", GUI::CColour( 20, 40, 60, 255 ) );

    GUI::CWidget* panel = context->CreateWidget( "Panel", "Panel", screen );
    panel->SetProperty( "Position", "0,13 0,17" );
    panel->SetProperty( "Size", "0,201 0,143" );
    panel->SetColourProperty( "BackgroundColour", GUI::CColour( 90, 120, 30, 200 ) );
    panel->SetColourProperty( "BorderColour", White );
    panel->SetBoolProperty( "Clip", true );
    GUI::CWidget* inner = context->CreateWidget( "Panel", "Inner", panel );
    inner->SetProperty( "Position", "0,150 0,100" );
    inner->SetProperty( "Size", "0,90 0,70" );
    inner->SetColourProperty( "BackgroundColour", GUI::CColour( 250, 200, 10, 255 ) );
    inner->SetFloatProperty( "Opacity", 0.6f );

    GUI::CWidget* frame = context->CreateWidget( "ImageFrame", "Checker", screen );
    frame->SetProperty( "Position", "0,240 0,20" );
    frame->SetProperty( "Size", "0,48 0,36" );
    frame->SetProperty( "Image", "Checker" );
    frame->SetProperty( "ScaleMode", "Stretch" );

    GUI::CWidget* label = context->CreateWidget( "Label", "Label", screen );
    label->SetProperty( "Position", "0,300 0,40" );
    label->SetProperty( "Size", "0,120 0,40" );
    label->SetProperty( "Font", "Block" );
    label->SetProperty( "Text", "Hello there wide world" );
    label->SetBoolProperty( "WordWrap", true );
    label->SetColourProperty( "TextColour", GUI::CColour( 120, 220, 255, 230 ) );

    GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( context->CreateWidget( "DrawSurface", "Surface", screen ) );
    surface->SetProperty( "Position", "0,20 0,200" );
    surface->SetProperty( "Size", "0,300 0,200" );
    surface->FillRect( GUI::CRect( 5, 5, 40, 30 ), GUI::CColour( 255, 0, 255, 160 ) );
    surface->StrokeRect( GUI::CRect( 50, 5, 40, 30 ), Green, 3 );
    surface->DrawLine( 0, 199, 299, 50, Yellow );
    surface->DrawLine( 10, 60, 120, 190, Red, 3 );
    surface->DrawLine( 150, 120, 150, 180, White );
    GUI::CDrawSurface::TPointVector points;
    for ( Int32 i=0; i<30; ++i )
    {
        GUI::CDrawSurface::SPoint point;
        point.x = 200 + i * 3;
        point.y = 20 + ( i * 7 ) % 40;
        points.push_back( point );
    }
    surface->DrawPoints( points, White );
    surface->PushClip( GUI::CRect( 100, 60, 50, 50 ) );
    surface->DrawImage( "Checker", GUI::CRect( 90, 50, 70, 70 ), GUI::CRect( 1, 1, 3, 3 ), GUI::CColour( 255, 255, 128, 200 ) );
    surface->PopClip();
    surface->DrawString( "Block", "Text!", 170, 150, GUI::CColour( 255, 160, 0, 255 ) );
    surface->Commit();
}

/** The number of textures of the render interface of RmlUiOgre that exist in the texture manager */
UInt32
CountRmlUiOgreTextures( void )
{
    UInt32 count = 0;
    Ogre::ResourceManager::ResourceMapIterator i = Ogre::TextureManager::getSingleton().getResourceIterator();
    while ( i.hasMoreElements() )
    {
        Ogre::ResourcePtr resource = i.getNext();
        if ( 0 == resource->getName().find( "RmlUiOgre/Texture/" ) )
            ++count;
    }
    return count;
}

/** A scene manager with a camera and a viewport of one colour over the whole window */
Ogre::Viewport*
AddColourViewport( Ogre::RenderTarget* target, const char* name, const Ogre::ColourValue& colour )
{
    Ogre::Root& root = Ogre::Root::getSingleton();
    Ogre::SceneManager* sceneManager = root.hasSceneManager( name ) ? root.getSceneManager( name ) : root.createSceneManager( "DefaultSceneManager", name );
    Ogre::Camera* camera = sceneManager->hasCamera( name ) ? sceneManager->getCamera( name ) : sceneManager->createCamera( name );
    if ( GUCEF_NULL == camera->getParentSceneNode() )
        sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject( camera );
    Ogre::Viewport* viewport = target->addViewport( camera );
    viewport->setBackgroundColour( colour );
    viewport->setOverlaysEnabled( false );
    return viewport;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
PerformRmlUiOgreSetUpTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "guidriverRmlUIOgre" );

    bool ready = false;
    GUCEF_TESTFW_TESTCASE( "Set up: the Ogre plugins, the window backend Ogre and the GUI driver RmlUiOgre load, and the main window is made" )
    if ( TESTWINDOWMODE_NONE == GetTestWindowMode() )
        GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
    try
    {
        // the image codec of the texture test is in gucefIMAGE before the codec plugin of Ogre starts
        TTestTextureCodecPtr codec( GUCEF_NEW CTestTextureCodec() );
        IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().TryRegister( "ogwtex", codec );

        ASSERT_TRUE( SetUpOgreForTests() );
        GUIDRIVEROGRE::COgreWindowContext* window = AsOgreWindow( GetMainTestWindow() );
        ASSERT_TRUE( GUCEF_NULL != window );
        ready = GUCEF_NULL != window;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    return ready;
}

/*-------------------------------------------------------------------------*/

void
PerformRmlUiOgreTearDownTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "guidriverRmlUIOgre" );

    GUCEF_TESTFW_TESTCASE( "Tear down: the window goes, then the plugins and the Root" )
    if ( TESTWINDOWMODE_NONE == GetTestWindowMode() )
        GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
    try
    {
        TearDownOgreForTests();
        ASSERT_TRUE( GUCEF_NULL == Ogre::Root::getSingletonPtr() );
        ASSERT_TRUE( GUCEF_NULL == GUI::CGuiGlobal::Instance()->GetGuiManager().GetGuiDriver( GUIDRIVERRMLUIOGRE::CRmlUIOgreGuiDriver::RegistrationName ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRmlUiOgreContextTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "guidriverRmlUIOgre.Context" );

    GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
    GUIDRIVEROGRE::COgreWindowContext* window = AsOgreWindow( GetMainTestWindow() );

    GUCEF_TESTFW_TESTCASE( "Driver: registered as RmlUiOgre, named RmlUi for the backend hints of layouts, contexts only in Ogre windows" )
    try
    {
        GUI::CGUIDriver* driver = manager.GetGuiDriver( "RmlUiOgre" );
        ASSERT_TRUE( GUCEF_NULL != driver );
        if ( GUCEF_NULL != driver )
        {
            ASSERT_TRUE( "RmlUi" == driver->GetDriverName() );
            ASSERT_TRUE( driver->HasCapability( "CaptureFrame" ) );
            ASSERT_TRUE( driver->HasCapability( "BitmapFonts" ) );
            ASSERT_TRUE( driver->HasCapability( "DrawSurface" ) );
            ASSERT_FALSE( driver->HasCapability( "PresentFilters" ) );
            ASSERT_TRUE( GUI::CGUIDriver::InterfaceVersion == driver->GetInterfaceVersion() );
        }
        ASSERT_TRUE( manager.CreateGUIContext( "RmlUiOgre", GUI::TWindowContextPtr() ).IsNULL() );

        // the OpenGL driver of RmlUi refuses the Ogre window: it has no GL context to draw in
        if ( GUCEF_NULL == manager.GetGuiDriver( "RmlUi" ) )
            LoadTestPlugin( "guidriverRmlUIOpenGL", "guidriver" );
        if ( GUCEF_NULL != manager.GetGuiDriver( "RmlUi" ) )
            ASSERT_TRUE( manager.CreateGUIContext( "RmlUi", GetMainTestWindow() ).IsNULL() );
        else
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "guidriverRmlUIOgre_TestApp: guidriverRmlUIOpenGL is not there, its refusal of Ogre windows is not checked" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Context: an overlay of its window in the order of its layer, with the renderer of the window in the driver properties" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateRmlUiOgreContext( 640, 480 );
        ASSERT_TRUE( !contextPtr.IsNULL() );
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL != context )
        {
            ASSERT_TRUE( GUIDRIVERRMLUIOGRE::CRmlUIOgreContext::ClassTypeName == context->GetClassTypeName() );
            GUIDRIVERRMLUIOGRE::CRmlUIOgreContext* ogreContext = static_cast< GUIDRIVERRMLUIOGRE::CRmlUIOgreContext* >( context );
            ASSERT_TRUE( 1 == window->GetOverlayCount() );
            ASSERT_TRUE( window == ogreContext->GetOgreWindowContext() );
            context->SetLayer( 4 );
            ASSERT_TRUE( 4 == ogreContext->GetOverlayOrder() );
            ASSERT_TRUE( "Ogre OpenGL 3+ Rendering Subsystem" == manager.GetGuiDriver( "RmlUiOgre" )->GetDriverProperty( "Renderer" ) );
        }
        contextPtr.Unlink();
        ASSERT_TRUE( 0 == window->GetOverlayCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUiOgre): A scene of every kind of drawing has the same pixels as the software reference" )
    try
    {
        GUI::TGuiContextPtr referencePtr = CreateNullContext( 640, 480 );
        GUI::TGuiContextPtr ogrePtr = CreateRmlUiOgreContext( 640, 480 );
        GUI::CGUIContext* reference = referencePtr.IsNULL() ? GUCEF_NULL : referencePtr->AsGUIContext();
        GUI::CGUIContext* ogre = ogrePtr.IsNULL() ? GUCEF_NULL : ogrePtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != reference && GUCEF_NULL != ogre );
        if ( GUCEF_NULL != reference && GUCEF_NULL != ogre )
        {
            BuildMixedScene( reference );
            BuildMixedScene( ogre );

            IMAGE::TPixelMapPtr expected, actual;
            ASSERT_TRUE( Capture( reference, expected ) );
            ASSERT_TRUE( Capture( ogre, actual ) );
            UInt32 different = 0;
            Int32 firstX = -1, firstY = -1;
            for ( Int32 y=0; y<480 && !actual.IsNULL(); ++y )
            {
                for ( Int32 x=0; x<640; ++x )
                {
                    if ( !Near( PixelAt( expected, x, y ), PixelAt( actual, x, y ) ) )
                    {
                        if ( 0 == different )
                        {
                            firstX = x;
                            firstY = y;
                        }
                        ++different;
                    }
                }
            }
            if ( 0 != different )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUiOgre differs from the reference in " << different << " pixels, the first at " << firstX << "," << firstY );
            }
            ASSERT_TRUE( 0 == different );

            // nothing changed: the second frame draws from the geometry and textures of the first
            guidriverRmlUI::CRmlUIContext* rml = static_cast< guidriverRmlUI::CRmlUIContext* >( ogre );
            const UInt32 textures = rml->GetTextureCreationCount();
            ASSERT_TRUE( Capture( ogre, actual ) );
            ASSERT_TRUE( textures == rml->GetTextureCreationCount() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUiOgre): the window shows the context over the 3D viewport, layers in their order, blended with premultiplied alpha" )
    try
    {
        Ogre::Viewport* viewport = AddColourViewport( window->GetOgreRenderWindow(), "Layers", Ogre::ColourValue( 0.0f, 0.0f, 1.0f, 1.0f ) );
        GUI::TGuiContextPtr lowerPtr = CreateRmlUiOgreContext( 640, 480 );
        GUI::TGuiContextPtr upperPtr = CreateRmlUiOgreContext( 640, 480 );
        GUI::CGUIContext* lower = lowerPtr.IsNULL() ? GUCEF_NULL : lowerPtr->AsGUIContext();
        GUI::CGUIContext* upper = upperPtr.IsNULL() ? GUCEF_NULL : upperPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != lower && GUCEF_NULL != upper );
        if ( GUCEF_NULL != lower && GUCEF_NULL != upper )
        {
            upper->SetLayer( 1 );
            GUI::CWidget* red = lower->CreateWidget( "Panel", "Red", GUCEF_NULL );
            red->SetProperty( "Position", "0,10 0,10" );
            red->SetProperty( "Size", "0,100 0,100" );
            red->SetColourProperty( "BackgroundColour", GUI::CColour( 255, 0, 0, 128 ) );
            lower->ShowRoot( red, false );
            GUI::CWidget* green = upper->CreateWidget( "Panel", "Green", GUCEF_NULL );
            green->SetProperty( "Position", "0,60 0,60" );
            green->SetProperty( "Size", "0,100 0,100" );
            green->SetColourProperty( "BackgroundColour", GUI::CColour( 0, 255, 0, 255 ) );
            upper->ShowRoot( green, false );

            // a frame of the window: the redraw lets the layers prepare, the overlays draw after the viewport
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( window->RenderFrame( &pixels ) );
            ASSERT_TRUE( !pixels.IsNULL() && 640 == pixels->GetWidthInPixels() );
            ASSERT_TRUE( PixelIs( pixels, 300, 300, 0, 0, 255 ) );
            ASSERT_TRUE( PixelIs( pixels, 30, 30, 128, 0, 127 ) );
            ASSERT_TRUE( PixelIs( pixels, 80, 80, 0, 255, 0 ) );
            ASSERT_TRUE( PixelIs( pixels, 150, 150, 0, 255, 0 ) );

            // the upper layer goes below
            upper->SetLayer( -1 );
            ASSERT_TRUE( window->RenderFrame( &pixels ) );
            ASSERT_TRUE( PixelIs( pixels, 80, 80, 128, 127, 0 ) );

            // the presented frame of a context is the window's picture
            ASSERT_TRUE( lower->CapturePresentedFrame( pixels ) );
            ASSERT_TRUE( PixelIs( pixels, 300, 300, 0, 0, 255 ) );
        }
        window->GetOgreRenderWindow()->removeViewport( viewport->getZOrder() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render (RmlUiOgre): a context with a smaller logical resolution is scaled onto its present rectangle" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateRmlUiOgreContext( 320, 240 );
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            context->SetPresentMode( GUI::PRESENTMODE_STRETCH );
            GUI::CWidget* panel = context->CreateWidget( "Panel", "Panel", GUCEF_NULL );
            panel->SetProperty( "Position", "0,10 0,10" );
            panel->SetProperty( "Size", "0,20 0,20" );
            panel->SetColourProperty( "BackgroundColour", GUI::CColour( 255, 255, 0, 255 ) );
            context->ShowRoot( panel, false );
            const GUI::CRect present = context->GetPresentRect();
            ASSERT_TRUE( 0 == present.left && 0 == present.top && 640 == present.width && 480 == present.height );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( window->RenderFrame( &pixels ) );
            // logical 10..30 is window 20..60
            ASSERT_TRUE( PixelIs( pixels, 20, 20, 255, 255, 0 ) );
            ASSERT_TRUE( PixelIs( pixels, 59, 59, 255, 255, 0 ) );
            ASSERT_TRUE( PixelIs( pixels, 61, 61, 0, 0, 0 ) );
            // the capture has the render size
            ASSERT_TRUE( context->CaptureFrame( pixels ) && 320 == pixels->GetWidthInPixels() && 240 == pixels->GetHeightInPixels() );
            ASSERT_TRUE( PixelIs( pixels, 10, 10, 255, 255, 0 ) );
            ASSERT_TRUE( PixelIs( pixels, 30, 30, 0, 0, 0 ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Render texture: a RenderContext widget shows an Ogre render texture of the application without a copy, and its later content" )
    try
    {
        // the application's render texture, drawn by its own viewport
        Ogre::TexturePtr target = Ogre::TextureManager::getSingleton().createManual( "RmlUiOgreTest/Target", Ogre::RGN_INTERNAL, Ogre::TEX_TYPE_2D,
                                                                                   64, 64, 0, Ogre::PF_BYTE_RGBA, Ogre::TU_RENDERTARGET );
        Ogre::RenderTarget* renderTarget = target->getBuffer()->getRenderTarget();
        renderTarget->setAutoUpdated( false );
        Ogre::Viewport* viewport = AddColourViewport( renderTarget, "Target", Ogre::ColourValue( 0.0f, 1.0f, 1.0f, 1.0f ) );
        renderTarget->update();

        GUI::TGuiContextPtr contextPtr = CreateRmlUiOgreContext( 640, 480 );
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUIDRIVERRMLUIOGRE::CRmlUIOgreContext* ogreContext = static_cast< GUIDRIVERRMLUIOGRE::CRmlUIOgreContext* >( context );
            GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( context->CreateWidget( "RenderContext", "View", GUCEF_NULL ) );
            surface->SetProperty( "Position", "0,100 0,100" );
            surface->SetProperty( "Size", "0,128 0,96" );
            context->ShowRoot( surface, false );
            ASSERT_TRUE( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderContext::SetOgreTexture( *surface, target, GUI::CRect( 0, 0, 128, 96 ) ) );
            ASSERT_TRUE( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderContext::GetOgreTexture( *surface ) == target );

            const UInt32 texturesBefore = CountRmlUiOgreTextures();
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( Capture( context, pixels ) );
            ASSERT_TRUE( PixelIs( pixels, 150, 150, 0, 255, 255 ) );
            ASSERT_TRUE( PixelIs( pixels, 227, 195, 0, 255, 255 ) );
            ASSERT_TRUE( PixelIs( pixels, 228, 150, 0, 0, 0 ) );
            // no texture of the render interface was made for it
            ASSERT_TRUE( texturesBefore == CountRmlUiOgreTextures() );
            ASSERT_TRUE( 1 == ogreContext->GetRenderInterface()->GetTextureCount() );

            // the application draws something else into its texture: the widget shows it at once
            viewport->setBackgroundColour( Ogre::ColourValue( 1.0f, 0.0f, 1.0f, 1.0f ) );
            renderTarget->update();
            ASSERT_TRUE( Capture( context, pixels ) );
            ASSERT_TRUE( PixelIs( pixels, 150, 150, 255, 0, 255 ) );

            // a widget of another driver's context is refused, and the texture goes with the widget
            ASSERT_TRUE( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderContext::ClearOgreTexture( *surface ) );
            ASSERT_FALSE( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderContext::GetOgreTexture( *surface ) );
            ASSERT_TRUE( Capture( context, pixels ) );
            ASSERT_TRUE( PixelIs( pixels, 150, 150, 0, 0, 0 ) );
        }
        GUI::TGuiContextPtr nullPtr = CreateNullContext( 640, 480 );
        GUI::CGUIContext* nullContext = nullPtr.IsNULL() ? GUCEF_NULL : nullPtr->AsGUIContext();
        if ( GUCEF_NULL != nullContext )
        {
            GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( nullContext->CreateWidget( "RenderContext", "View", GUCEF_NULL ) );
            ASSERT_FALSE( GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderContext::SetOgreTexture( *surface, target, GUI::CRect( 0, 0, 10, 10 ) ) );
        }
        contextPtr.Unlink();
        renderTarget->removeAllViewports();
        Ogre::TextureManager::getSingleton().remove( target );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Release: a context gives its geometry and textures back to Ogre before its render interface goes" )
    try
    {
        const UInt32 texturesBefore = CountRmlUiOgreTextures();
        GUI::TGuiContextPtr contextPtr = CreateRmlUiOgreContext( 640, 480 );
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            BuildMixedScene( context );
            IMAGE::TPixelMapPtr pixels;
            ASSERT_TRUE( Capture( context, pixels ) );
            GUIDRIVERRMLUIOGRE::CRmlUIOgreRenderInterface* renderer = static_cast< GUIDRIVERRMLUIOGRE::CRmlUIOgreContext* >( context )->GetRenderInterface();
            ASSERT_TRUE( renderer->GetGeometryCount() > 0 );
            ASSERT_TRUE( renderer->GetTextureCount() > 0 );
            ASSERT_TRUE( CountRmlUiOgreTextures() > texturesBefore );
        }
        contextPtr.Unlink();
        ASSERT_TRUE( texturesBefore == CountRmlUiOgreTextures() );
        ASSERT_TRUE( 0 == window->GetOverlayCount() );
        // the window draws on without it
        IMAGE::TPixelMapPtr pixels;
        ASSERT_TRUE( window->RenderFrame( &pixels ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
