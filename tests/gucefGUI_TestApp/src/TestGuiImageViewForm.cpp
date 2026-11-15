/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestAssetFormSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

using namespace GUITEST;

/** A picture of 8 x 6 pixels: red from the column, green from the row, blue the shade; column 1 is clear, column 2 half clear, the others solid */
IMAGE::TPixelMapPtr
MakePicture( UInt8 shade, UInt32 width = 8, UInt32 height = 6 )
{
    IMAGE::TPixelMapPtr pixels = MakeSolidImage( width, height, 0, 0, shade, 255 );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 y=0; y<height; ++y )
    {
        for ( UInt32 x=0; x<width; ++x )
        {
            UInt8* p = data + ( (size_t) y * width + x ) * 4;
            p[ 0 ] = (UInt8) ( 20 + x * 30 );
            p[ 1 ] = (UInt8) ( 20 + y * 40 );
            p[ 2 ] = shade;
            p[ 3 ] = 1 == x ? 0 : ( 2 == x ? 128 : 255 );
        }
    }
    return pixels;
}

const UInt8*
PixelOf( const IMAGE::TPixelMapPtr& pixels, UInt32 x, UInt32 y )
{
    return static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
}

bool
IsPixel( const IMAGE::TPixelMapPtr& pixels, UInt32 x, UInt32 y, Int32 r, Int32 g, Int32 b, Int32 a = 255, Int32 tolerance = 1 )
{
    if ( pixels.IsNULL() || x >= pixels->GetWidthInPixels() || y >= pixels->GetHeightInPixels() )
        return false;
    const UInt8* p = PixelOf( pixels, x, y );
    return abs( (Int32) p[ 0 ] - r ) <= tolerance && abs( (Int32) p[ 1 ] - g ) <= tolerance && abs( (Int32) p[ 2 ] - b ) <= tolerance && abs( (Int32) p[ 3 ] - a ) <= tolerance;
}

/** An image of the frames given, each a picture of a shade */
void
AddFrames( IMAGE::CImage& image, UInt32 frames )
{
    for ( UInt32 i=0; i<frames; ++i )
    {
        IMAGE::TPixelMapPtr picture = MakePicture( (UInt8) ( 50 + i * 80 ) );
        image.AddFrame( picture );
    }
}

/** Where a point of the picture is in the panel, as the form has it now */
void
ViewPointOf( GUI::CImageViewForm& form, Float32 pictureX, Float32 pictureY, Int32& viewX, Int32& viewY )
{
    viewX = form.GetPanX() + (Int32) ( pictureX * form.GetZoom() );
    viewY = form.GetPanY() + (Int32) ( pictureY * form.GetZoom() );
}

class CImageViewScene : public CFormScene
{
    public:

    GUI::CImageViewForm form;
    IMAGE::CImage first;
    IMAGE::CImage second;

    explicit CImageViewScene( GUI::CGUIContext* context ) : CFormScene(), form( context ), first( MakePicture( 90 ) ), second( MakePicture( 220 ) )
    {
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        context->RenderFrame( 16 );
        form.SetImages( &first, &second );
        form.SetZoom( 32.0f );
        form.SetPan( 40, 10 );
        form.SetSplit( 0.4f );
        form.ProbeAt( 40 + 32 * 5 + 3, 10 + 32 * 2 + 3 );
        context->RenderFrame( 16 );
    }
};

CFormScene*
BuildImageViewScene( GUI::CGUIContext* context )
{
    return new CImageViewScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiImageViewFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.ImageViewForm" );

    GUCEF_TESTFW_TESTCASE( "Image view: The form builds as a window and in the place of a widget, and an image is shown fitted to the panel" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build( "Texture" ) && form.IsVisible() && "Texture" == form.GetRootWidget()->GetProperty( "Title" ) );
        const char* names[] = { "ChannelRGB", "ChannelR", "ChannelG", "ChannelB", "ChannelA", "Checker", "Tile", "Fit", "Actual", "Level", "Frame", "ColourSpace", "View", "ImageA", "BClip", "ImageB", "SplitLine", "Status" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( form, names[ i ] ) );
        ASSERT_TRUE( GUCEF_NULL == form.GetImage() && form.GetShownPixels().IsNULL() && 0 == form.GetLevelCount() && 0 == form.GetFrameCount() );
        ASSERT_FALSE( form.ProbeAt( 10, 10 ) || form.IsComparing() );

        IMAGE::CImage image( MakePicture( 100 ) );
        context->RenderFrame( 16 );
        form.SetImage( &image );
        ASSERT_TRUE( &image == form.GetImage() && !form.GetShownPixels().IsNULL() && 8 == form.GetShownPixels()->GetWidthInPixels() && 6 == form.GetShownPixels()->GetHeightInPixels() );
        context->RenderFrame( 16 );
        Int32 width = 0, height = 0;
        form.GetViewSize( width, height );
        ASSERT_TRUE( width > 300 && height > 150 && form.IsFitted() );
        const Float32 fitted = (Float32) width / 8.0f < (Float32) height / 6.0f ? (Float32) width / 8.0f : (Float32) height / 6.0f;
        ASSERT_TRUE( form.GetZoom() > fitted - 0.01f && form.GetZoom() < fitted + 0.01f );

        // the picture is in the middle of the panel, in a frame that has its size
        GUI::CWidget* frame = form.GetWidget( "ImageA" );
        ASSERT_TRUE( frame->IsVisible() && frame->GetRect().width == (Int32) ( 8.0f * form.GetZoom() + 0.5f ) && frame->GetRect().height == (Int32) ( 6.0f * form.GetZoom() + 0.5f ) );
        const GUI::CRect view = form.GetWidget( "View" )->GetRect();
        ASSERT_TRUE( frame->GetRect().left - view.left == form.GetPanX() && frame->GetRect().top - view.top == form.GetPanY() );
        ASSERT_TRUE( frame->GetRect().left + frame->GetRect().width / 2 - view.left - view.width / 2 <= 1 );
        ASSERT_FALSE( form.GetWidget( "BClip" )->IsVisible() || form.GetWidget( "SplitLine" )->IsVisible() );

        // in the place of a widget of another form
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,500 0,400" );
        GUI::CImageViewForm inPlace;
        ASSERT_FALSE( inPlace.Build( static_cast< GUI::CWidget* >( GUCEF_NULL ) ) );
        ASSERT_TRUE( inPlace.Build( placeholder ) && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        inPlace.SetImage( &image );
        ASSERT_TRUE( !inPlace.GetShownPixels().IsNULL() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: The channels, alpha over a checkerboard and the sRGB or linear display are operations on a copy of the level, made when the choice changes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );
        IMAGE::CImage image( MakePicture( 100 ) );
        CEventRecorder viewChanged( &form, GUI::CImageViewForm::ViewChangedEvent );
        form.SetImage( &image );
        context->RenderFrame( 16 );

        // colour with alpha over the checkerboard: solid pixels as they are, clear ones the checkerboard, half clear a mix
        IMAGE::TPixelMapPtr shown = form.GetShownPixels();
        ASSERT_TRUE( IsPixel( shown, 0, 0, 20, 20, 100 ) && IsPixel( shown, 3, 2, 110, 100, 100 ) );
        ASSERT_TRUE( IsPixel( shown, 1, 0, 102, 102, 102 ) && IsPixel( shown, 1, 1, 102, 102, 102 ) );
        const UInt8* mixed = PixelOf( shown, 2, 0 );
        ASSERT_TRUE( mixed[ 0 ] > 85 && mixed[ 0 ] < 100 && 255 == mixed[ 3 ] );
        ASSERT_TRUE( GUI::CImageViewForm::CHANNEL_RGB == form.GetChannel() && form.GetAlphaCheckerboard() );
        const UInt32 built = form.GetRebuildCount();

        // moving and zooming make nothing again, nor do frames
        form.SetZoom( 3.0f );
        form.SetPan( 5, 6 );
        form.ZoomAt( 2.0f, 40, 40 );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( built == form.GetRebuildCount() );
        ASSERT_TRUE( shown.GetPointerAlways() == form.GetShownPixels().GetPointerAlways() );

        // without the checkerboard the alpha is left out
        form.SetAlphaCheckerboard( false );
        ASSERT_TRUE( built + 1 == form.GetRebuildCount() && IsPixel( form.GetShownPixels(), 1, 0, 50, 20, 100 ) );
        form.SetAlphaCheckerboard( false );
        ASSERT_TRUE( built + 1 == form.GetRebuildCount() );
        form.SetAlphaCheckerboard( true );

        // one channel at a time, as grey
        form.SetChannel( GUI::CImageViewForm::CHANNEL_RED );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 3, 2, 110, 110, 110 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_GREEN );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 3, 2, 100, 100, 100 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_BLUE );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 3, 2, 100, 100, 100 ) && IsPixel( form.GetShownPixels(), 0, 0, 100, 100, 100 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_ALPHA );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 255, 255, 255 ) && IsPixel( form.GetShownPixels(), 1, 0, 0, 0, 0 ) && IsPixel( form.GetShownPixels(), 2, 0, 128, 128, 128 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_RGB );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 20, 20, 100 ) );
        ASSERT_TRUE( viewChanged.count >= 8 );
        GUI::CImageViewForm::TViewState state = viewChanged.Last< GUI::CImageViewForm::TViewChangedEventData >()->GetData();
        ASSERT_TRUE( GUI::CImageViewForm::CHANNEL_RGB == state.channel );

        // the buttons of the tool bar choose the channel
        ClickWidget( context, form.GetWidget( "ChannelG" ) );
        ASSERT_TRUE( GUI::CImageViewForm::CHANNEL_GREEN == form.GetChannel() && "Primary" == form.GetWidget( "ChannelG" )->GetProperty( "StyleClass" ) );
        ASSERT_TRUE( form.GetWidget( "ChannelRGB" )->GetProperty( "StyleClass" ).IsNULLOrEmpty() );
        ClickWidget( context, form.GetWidget( "ChannelA" ) );
        ASSERT_TRUE( GUI::CImageViewForm::CHANNEL_ALPHA == form.GetChannel() );
        ClickWidget( context, form.GetWidget( "ChannelRGB" ) );
        ASSERT_TRUE( GUI::CImageViewForm::CHANNEL_RGB == form.GetChannel() );

        // a picture without alpha shows no checkerboard, and its alpha is all there
        IMAGE::TPixelMapPtr rgb( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 4, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
        UInt8* data = static_cast< UInt8* >( rgb->GetDataPtr() );
        for ( UInt32 i=0; i<16; ++i )
        {
            data[ i * 3 ] = 10;
            data[ i * 3 + 1 ] = 200;
            data[ i * 3 + 2 ] = 30;
        }
        IMAGE::CImage rgbImage( rgb );
        form.SetImage( &rgbImage );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 2, 2, 10, 200, 30 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_ALPHA );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 2, 2, 255, 255, 255 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_GREEN );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 2, 2, 200, 200, 200 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_RGB );

        // a picture of one channel is grey in every channel
        IMAGE::TPixelMapPtr grey( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 4, IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 ) );
        memset( grey->GetDataPtr(), 77, 16 );
        IMAGE::CImage greyImage( grey );
        form.SetImage( &greyImage );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 1, 1, 77, 77, 77 ) );
        form.SetImage( GUCEF_NULL );
        ASSERT_TRUE( form.GetShownPixels().IsNULL() && !form.GetWidget( "ImageA" )->IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: The colour space converts the values that are shown, from the space the data is in to the one chosen" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );
        IMAGE::CImage image( MakeSolidImage( 4, 4, 128, 128, 128, 255 ) );
        form.SetImage( &image );
        ASSERT_TRUE( IMAGE::COLOURSPACE_SRGB == form.GetSourceColourSpace() && IMAGE::COLOURSPACE_SRGB == form.GetDisplayColourSpace() );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 128, 128, 128 ) );

        // sRGB data in linear light: darker
        form.SetDisplayColourSpace( IMAGE::COLOURSPACE_LINEAR );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 55, 55, 55, 255, 2 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_ALPHA );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 255, 255, 255 ) );      // the alpha is no colour
        form.SetChannel( GUI::CImageViewForm::CHANNEL_RGB );

        // linear data shown as sRGB: lighter; shown as linear: as it is
        form.SetSourceColourSpace( IMAGE::COLOURSPACE_LINEAR );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 128, 128, 128 ) );
        form.SetDisplayColourSpace( IMAGE::COLOURSPACE_SRGB );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 188, 188, 188, 255, 2 ) );

        // the list of the tool bar
        form.GetWidget( "ColourSpace" )->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "1" );
        ASSERT_TRUE( IMAGE::COLOURSPACE_LINEAR == form.GetDisplayColourSpace() );
        form.GetWidget( "ColourSpace" )->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "0" );
        ASSERT_TRUE( IMAGE::COLOURSPACE_SRGB == form.GetDisplayColourSpace() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: The levels of an image are shown, and the ones it does not have are made from the first with the resampler; frames are cube faces or layers" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );

        // no levels of its own: the ones down to 1x1 are made
        IMAGE::CImage plain( MakePicture( 100, 16, 8 ) );
        form.SetImage( &plain );
        ASSERT_TRUE( 5 == form.GetLevelCount() && 0 == form.GetLevel() && 16 == form.GetShownPixels()->GetWidthInPixels() );
        const UInt32 built = form.GetRebuildCount();
        form.SetLevel( 1 );
        ASSERT_TRUE( 1 == form.GetLevel() && 8 == form.GetShownPixels()->GetWidthInPixels() && 4 == form.GetShownPixels()->GetHeightInPixels() && built + 1 == form.GetRebuildCount() );
        form.SetLevel( 4 );
        ASSERT_TRUE( 1 == form.GetShownPixels()->GetWidthInPixels() && 1 == form.GetShownPixels()->GetHeightInPixels() );
        form.SetLevel( 99 );
        ASSERT_TRUE( 4 == form.GetLevel() );
        form.SetLevel( 0 );
        ASSERT_TRUE( 16 == form.GetShownPixels()->GetWidthInPixels() );

        // the list of the tool bar has the levels, and a pick of the user chooses one
        GUI::CCombobox* level = form.GetWidgetAs< GUI::CCombobox >( "Level" );
        GUI::CCombobox::TStringVector items;
        ASSERT_TRUE( level->GetListItems( items ) && 5 == items.size() && "4" == items[ 4 ] );
        level->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "2" );
        ASSERT_TRUE( 2 == form.GetLevel() && 4 == form.GetShownPixels()->GetWidthInPixels() );
        form.SetLevel( 0 );

        // levels the image has: they are the ones that are shown
        IMAGE::CImage::TMipMapList mips;
        mips.push_back( MakePicture( 100, 8, 8 ) );
        mips.push_back( MakeSolidImage( 4, 4, 9, 8, 7, 255 ) );
        IMAGE::CImage chain( mips );
        form.SetImage( &chain );
        ASSERT_TRUE( 2 == form.GetLevelCount() );
        form.SetLevel( 1 );
        ASSERT_TRUE( 4 == form.GetShownPixels()->GetWidthInPixels() && IsPixel( form.GetShownPixels(), 0, 0, 9, 8, 7 ) );
        form.SetLevel( 5 );
        ASSERT_TRUE( 1 == form.GetLevel() );

        // frames
        IMAGE::CImage frames;
        AddFrames( frames, 6 );
        form.SetImage( &frames );
        ASSERT_TRUE( 6 == form.GetFrameCount() && 0 == form.GetFrame() && IsPixel( form.GetShownPixels(), 0, 0, 20, 20, 50 ) );
        form.SetFrame( 2 );
        ASSERT_TRUE( 2 == form.GetFrame() && IsPixel( form.GetShownPixels(), 0, 0, 20, 20, 210 ) );
        form.SetFrame( 50 );
        ASSERT_TRUE( 5 == form.GetFrame() );
        form.SetFrame( 0 );
        GUI::CCombobox* frame = form.GetWidgetAs< GUI::CCombobox >( "Frame" );
        ASSERT_TRUE( frame->GetListItems( items ) && 6 == items.size() && "5" == items[ 5 ] && "Frame" == form.GetWidget( "FrameLabel" )->GetProperty( "Text" ) );
        form.SetFrameMeaning( GUI::CImageViewForm::FRAMEMEANING_CUBE_FACES );
        ASSERT_TRUE( GUI::CImageViewForm::FRAMEMEANING_CUBE_FACES == form.GetFrameMeaning() && frame->GetListItems( items ) && "+X" == items[ 0 ] && "-Z" == items[ 5 ] );
        ASSERT_TRUE( "Face" == form.GetWidget( "FrameLabel" )->GetProperty( "Text" ) );
        form.SetFrameMeaning( GUI::CImageViewForm::FRAMEMEANING_ARRAY_LAYERS );
        ASSERT_TRUE( "Layer" == form.GetWidget( "FrameLabel" )->GetProperty( "Text" ) );
        frame->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "3" );
        ASSERT_TRUE( 3 == form.GetFrame() && IsPixel( form.GetShownPixels(), 0, 0, 20, 20, 34 ) );        // the shade of the frame is 290 as a byte
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: Tiling shows the picture 3 times in 3 rows, and a probe of a pixel in a tile reads the pixel of the picture" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );
        IMAGE::CImage image( MakePicture( 100 ) );
        form.SetImage( &image );
        context->RenderFrame( 16 );
        const Float32 single = form.GetZoom();
        ASSERT_FALSE( form.SetTiling( 2 ) );
        ASSERT_TRUE( form.SetTiling( 3 ) && 3 == form.GetTiling() );
        IMAGE::TPixelMapPtr shown = form.GetShownPixels();
        ASSERT_TRUE( 24 == shown->GetWidthInPixels() && 18 == shown->GetHeightInPixels() );
        ASSERT_TRUE( IsPixel( shown, 3, 2, 110, 100, 100 ) && IsPixel( shown, 11, 8, 110, 100, 100 ) && IsPixel( shown, 19, 14, 110, 100, 100 ) );
        ASSERT_TRUE( form.IsFitted() && form.GetZoom() < single );          // the fit follows the new size

        // a probe in the middle tile reads the pixel of the picture
        CEventRecorder probed( &form, GUI::CImageViewForm::PixelProbedEvent );
        Int32 vx = 0, vy = 0;
        ViewPointOf( form, 8.0f + 3.5f, 6.0f + 2.5f, vx, vy );
        ASSERT_TRUE( form.ProbeAt( vx, vy ) && 1 == probed.count );
        GUI::CImageViewForm::TPixelProbe probe = probed.Last< GUI::CImageViewForm::TPixelProbedEventData >()->GetData();
        ASSERT_TRUE( 3 == probe.x && 2 == probe.y && !probe.inSecond );
        ASSERT_TRUE( "R 110  G 100  B 100  A 255" == probe.storedValue && "R 110  G 100  B 100  A 255" == probe.shownValue );

        // the unit of the stored values is the one the data has
        form.SetTiling( 1 );
        form.SetAlphaCheckerboard( true );
        ASSERT_TRUE( 1 == form.GetTiling() && 8 == form.GetShownPixels()->GetWidthInPixels() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: The wheel zooms about the pointer, a drag pans, Fit and 1:1 and the keys set the view, and every change is notified" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );
        IMAGE::CImage image( MakePicture( 100 ) );
        form.SetImage( &image );
        context->RenderFrame( 16 );
        const GUI::CRect view = form.GetWidget( "View" )->GetRect();
        CEventRecorder viewChanged( &form, GUI::CImageViewForm::ViewChangedEvent );
        ASSERT_TRUE( form.IsFitted() );

        // the wheel about a point of the picture: the pixel below the pointer stays below it
        Int32 vx = 0, vy = 0;
        ViewPointOf( form, 4.5f, 3.5f, vx, vy );
        const Float32 before = form.GetZoom();
        context->InjectMouseMove( view.left + vx, view.top + vy );
        context->InjectMouseWheel( 1 );
        ASSERT_TRUE( form.GetZoom() > before * 1.24f && form.GetZoom() < before * 1.26f && !form.IsFitted() && viewChanged.count >= 1 );
        CEventRecorder probed( &form, GUI::CImageViewForm::PixelProbedEvent );
        ASSERT_TRUE( form.ProbeAt( vx, vy ) );
        GUI::CImageViewForm::TPixelProbe probe = probed.Last< GUI::CImageViewForm::TPixelProbedEventData >()->GetData();
        ASSERT_TRUE( 4 == probe.x && 3 == probe.y );
        context->InjectMouseWheel( -1 );
        ASSERT_TRUE( form.GetZoom() > before * 0.99f && form.GetZoom() < before * 1.01f );

        // a drag moves the picture by what the pointer moved
        const Int32 panX = form.GetPanX(), panY = form.GetPanY();
        context->InjectMouseMove( view.left + 100, view.top + 100 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( view.left + 130, view.top + 112 );
        context->InjectMouseMove( view.left + 140, view.top + 105 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( panX + 40 == form.GetPanX() && panY + 5 == form.GetPanY() );
        context->InjectMouseMove( view.left + 200, view.top + 100 );                  // no button: no pan
        ASSERT_TRUE( panX + 40 == form.GetPanX() );

        // the size of the picture's frame follows the zoom
        form.SetZoom( 10.0f );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 80 == form.GetWidget( "ImageA" )->GetRect().width && 60 == form.GetWidget( "ImageA" )->GetRect().height );
        ASSERT_FALSE( form.GetWidget( "ImageA" )->GetBoolProperty( GUI::EWidgetProperty_Smooth ) );
        form.SetZoom( 0.5f );
        ASSERT_TRUE( form.GetWidget( "ImageA" )->GetBoolProperty( GUI::EWidgetProperty_Smooth ) );

        // limits
        form.SetZoom( 100000.0f );
        ASSERT_TRUE( 64.0f == form.GetZoom() );
        form.SetZoom( 0.0f );
        ASSERT_TRUE( 1.0f / 64.0f == form.GetZoom() );

        // 1:1 and fit, by the buttons
        ClickWidget( context, form.GetWidget( "Actual" ) );
        ASSERT_TRUE( 1.0f == form.GetZoom() && !form.IsFitted() );
        ASSERT_TRUE( form.GetPanX() == ( view.width - 8 ) / 2 && form.GetPanY() == ( view.height - 6 ) / 2 );
        ClickWidget( context, form.GetWidget( "Fit" ) );
        ASSERT_TRUE( form.IsFitted() && form.GetZoom() > before * 0.99f && form.GetZoom() < before * 1.01f );

        // the keys, while the panel has the focus (a click gave it)
        context->InjectMouseMove( view.left + 10, view.top + 10 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        PressKey( context, INPUT::KEYCODE_1 );
        ASSERT_TRUE( 1.0f == form.GetZoom() );
        PressKey( context, INPUT::KEYCODE_EQUALS );
        ASSERT_TRUE( form.GetZoom() > 1.24f && form.GetZoom() < 1.26f );
        PressKey( context, INPUT::KEYCODE_MINUS );
        ASSERT_TRUE( form.GetZoom() > 0.99f && form.GetZoom() < 1.01f );
        const Int32 x = form.GetPanX();
        PressKey( context, INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( x - 24 == form.GetPanX() );
        PressKey( context, INPUT::KEYCODE_F );
        ASSERT_TRUE( form.IsFitted() );

        // a fitted picture follows the size of the panel; a picture that was moved stays
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,300 0,200" );
        GUI::CImageViewForm inPlace;
        ASSERT_TRUE( inPlace.Build( placeholder ) );
        inPlace.SetImage( &image );
        context->RenderFrame( 16 );
        const Float32 small = inPlace.GetZoom();
        placeholder->SetProperty( "Size", "0,500 0,400" );
        context->RenderFrame( 16 );
        ASSERT_TRUE( inPlace.GetZoom() > small && inPlace.IsFitted() );
        inPlace.SetPan( 3, 4 );
        placeholder->SetProperty( "Size", "0,300 0,200" );
        context->RenderFrame( 16 );
        ASSERT_TRUE( !inPlace.IsFitted() && 3 == inPlace.GetPanX() && 4 == inPlace.GetPanY() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: The pointer over a pixel gives its coordinates, the value as stored and the value as shown in the status line and in PixelProbedEvent" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );
        IMAGE::CImage image( MakePicture( 100 ) );
        form.SetImage( &image );
        context->RenderFrame( 16 );
        const GUI::CRect view = form.GetWidget( "View" )->GetRect();
        CEventRecorder probed( &form, GUI::CImageViewForm::PixelProbedEvent );

        Int32 vx = 0, vy = 0;
        ViewPointOf( form, 2.5f, 1.5f, vx, vy );
        context->InjectMouseMove( view.left + vx, view.top + vy );
        ASSERT_TRUE( 1 == probed.count );
        GUI::CImageViewForm::TPixelProbe probe = probed.Last< GUI::CImageViewForm::TPixelProbedEventData >()->GetData();
        ASSERT_TRUE( 2 == probe.x && 1 == probe.y );
        ASSERT_TRUE( "R 80  G 60  B 100  A 128" == probe.storedValue );
        ASSERT_TRUE( probe.shownValue.HasSubstr( "A 255" ) >= 0 );
        const CORE::CString status = form.GetWidget( "Status" )->GetProperty( "Text" );
        ASSERT_TRUE( status.HasSubstr( "2, 1" ) >= 0 && status.HasSubstr( "Stored: R 80  G 60  B 100  A 128" ) >= 0 && form.GetStatusText() == status );

        // the shown value follows the channel
        form.SetChannel( GUI::CImageViewForm::CHANNEL_ALPHA );
        ASSERT_TRUE( form.ProbeAt( vx, vy ) );
        probe = probed.Last< GUI::CImageViewForm::TPixelProbedEventData >()->GetData();
        ASSERT_TRUE( "R 80  G 60  B 100  A 128" == probe.storedValue && "R 128  G 128  B 128  A 255" == probe.shownValue );

        // outside the picture: nothing
        const UInt32 count = probed.count;
        ASSERT_FALSE( form.ProbeAt( -5, -5 ) );
        ViewPointOf( form, 9.5f, 1.5f, vx, vy );
        ASSERT_FALSE( form.ProbeAt( vx, vy ) );
        ASSERT_TRUE( count == probed.count );

        // a picture with 16 bit and floating point values shows them as they are stored
        IMAGE::TPixelMapPtr floats( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 2, 2, IMAGE::PSF_RGB, GUCEF::DATATYPE_LE_FLOAT32 ) );
        Float32* values = static_cast< Float32* >( floats->GetDataPtr() );
        for ( UInt32 i=0; i<12; ++i )
            values[ i ] = 0.25f * (Float32) ( i % 3 + 1 );
        values[ 0 ] = 2.0f;           // brighter than white
        IMAGE::CImage floatImage( floats );
        form.SetImage( &floatImage );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_RGB );
        context->RenderFrame( 16 );
        ViewPointOf( form, 0.5f, 0.5f, vx, vy );
        ASSERT_TRUE( form.ProbeAt( vx, vy ) );
        probe = probed.Last< GUI::CImageViewForm::TPixelProbedEventData >()->GetData();
        ASSERT_TRUE( "R 2  G 0.5  B 0.75" == probe.storedValue );
        ASSERT_TRUE( "R 255  G 128  B 191  A 255" == probe.shownValue );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: Two images are compared at a split line that the user drags, with the same choices, pan and zoom for both" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );
        IMAGE::CImage first( MakePicture( 100 ) );
        IMAGE::CImage second( MakePicture( 200 ) );
        form.SetImages( &first, &second );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( form.IsComparing() && &second == form.GetSecondImage() && !form.GetSecondShownPixels().IsNULL() );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 20, 20, 100 ) && IsPixel( form.GetSecondShownPixels(), 0, 0, 20, 20, 200 ) );
        ASSERT_TRUE( form.GetWidget( "BClip" )->IsVisible() && form.GetWidget( "SplitLine" )->IsVisible() && 0.5f == form.GetSplit() );
        const GUI::CRect view = form.GetWidget( "View" )->GetRect();
        const GUI::CRect clip = form.GetWidget( "BClip" )->GetRect();
        ASSERT_TRUE( clip.left == view.left && clip.width == (Int32) ( 0.5f * (Float32) view.width + 0.5f ) );
        ASSERT_TRUE( form.GetWidget( "ImageB" )->GetRect().left == form.GetWidget( "ImageA" )->GetRect().left );

        // a probe left of the line is of the second, right of it of the first
        CEventRecorder probed( &form, GUI::CImageViewForm::PixelProbedEvent );
        const Int32 splitX = (Int32) ( form.GetSplit() * (Float32) view.width + 0.5f );
        Int32 vx = 0, vy = 0;
        ViewPointOf( form, 0.5f, 0.5f, vx, vy );
        ASSERT_TRUE( vx < splitX && form.ProbeAt( vx, vy ) );
        ASSERT_TRUE( probed.Last< GUI::CImageViewForm::TPixelProbedEventData >()->GetData().inSecond );
        ViewPointOf( form, 7.5f, 0.5f, vx, vy );
        ASSERT_TRUE( vx > splitX && form.ProbeAt( vx, vy ) );
        ASSERT_FALSE( probed.Last< GUI::CImageViewForm::TPixelProbedEventData >()->GetData().inSecond );

        // the line is dragged: a press at it, not a pan
        const Int32 panX = form.GetPanX();
        context->InjectMouseMove( view.left + splitX + 2, view.top + 20 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( view.left + splitX - 100, view.top + 30 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( form.GetSplit() > ( (Float32) ( splitX - 100 ) / (Float32) view.width ) - 0.01f && form.GetSplit() < ( (Float32) ( splitX - 100 ) / (Float32) view.width ) + 0.01f );
        ASSERT_TRUE( panX == form.GetPanX() && form.IsFitted() );
        form.SetSplit( 2.0f );
        ASSERT_TRUE( 1.0f == form.GetSplit() );
        form.SetSplit( -1.0f );
        ASSERT_TRUE( 0.0f == form.GetSplit() );
        form.SetSplit( 0.5f );

        // the choices are for both
        form.SetChannel( GUI::CImageViewForm::CHANNEL_BLUE );
        ASSERT_TRUE( IsPixel( form.GetShownPixels(), 0, 0, 100, 100, 100 ) && IsPixel( form.GetSecondShownPixels(), 0, 0, 200, 200, 200 ) );
        form.SetChannel( GUI::CImageViewForm::CHANNEL_RGB );
        form.SetZoom( 20.0f );
        ASSERT_TRUE( form.GetWidget( "ImageB" )->GetRect().width == form.GetWidget( "ImageA" )->GetRect().width );
        ASSERT_TRUE( form.GetWidget( "ImageB" )->GetRect().top == form.GetWidget( "ImageA" )->GetRect().top );

        // one image: no comparison any more
        form.SetImage( &first );
        ASSERT_TRUE( !form.IsComparing() && !form.GetWidget( "BClip" )->IsVisible() && !form.GetWidget( "SplitLine" )->IsVisible() && form.GetSecondShownPixels().IsNULL() );
        form.SetImages( GUCEF_NULL, &second );
        ASSERT_TRUE( GUCEF_NULL == form.GetImage() && GUCEF_NULL == form.GetSecondImage() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Image view: The check boxes and lists of the tool bar are the user's way to the same choices, and a layout of the form saves and loads again" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CImageViewForm form( context );
        ASSERT_TRUE( form.Build() );
        IMAGE::CImage image( MakePicture( 100 ) );
        form.SetImage( &image );
        context->RenderFrame( 16 );

        ClickWidget( context, form.GetWidget( "Checker" ) );
        ASSERT_FALSE( form.GetAlphaCheckerboard() );
        ClickWidget( context, form.GetWidget( "Checker" ) );
        ASSERT_TRUE( form.GetAlphaCheckerboard() );
        ClickWidget( context, form.GetWidget( "Tile" ) );
        ASSERT_TRUE( 3 == form.GetTiling() );
        ClickWidget( context, form.GetWidget( "Tile" ) );
        ASSERT_TRUE( 1 == form.GetTiling() );

        CORE::CDataNode layout;
        ASSERT_TRUE( form.SaveLayout( layout ) );
        GUI::CImageViewForm second( context );
        ASSERT_TRUE( second.LoadLayout( layout ) && second.Show() );
        ASSERT_TRUE( HasWidget( second, "View" ) && HasWidget( second, "ColourSpace" ) );
        second.SetImage( &image );
        context->RenderFrame( 16 );
        ASSERT_TRUE( !second.GetShownPixels().IsNULL() && second.IsFitted() );
        ClickWidget( context, second.GetWidget( "ChannelR" ) );
        ASSERT_TRUE( GUI::CImageViewForm::CHANNEL_RED == second.GetChannel() && IsPixel( second.GetShownPixels(), 3, 2, 110, 110, 110 ) );
        ClickWidget( context, second.GetWidget( "Tile" ) );
        ASSERT_TRUE( 3 == second.GetTiling() );

        // the same form loaded again goes on showing what it shows
        ASSERT_TRUE( second.LoadLayout( layout ) );
        ASSERT_TRUE( !second.GetShownPixels().IsNULL() && GUI::CImageViewForm::CHANNEL_RED == second.GetChannel() );
        ASSERT_TRUE( "Primary" == second.GetWidget( "ChannelR" )->GetProperty( "StyleClass" ) && second.GetWidget( "Tile" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
        ASSERT_TRUE( second.GetWidget( "ImageA" )->IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "image view", &BuildImageViewScene );
    RunFormLintBaseline( "ImageView", &BuildImageViewScene );
}

/*-------------------------------------------------------------------------*/
