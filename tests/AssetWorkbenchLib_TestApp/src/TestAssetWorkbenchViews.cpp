/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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
#include <string.h>
#include <Ogre.h>

#include "gucef_test_framework.h"
#include "gucefCORE_CTEventHandlerFunctor.h"
#include "gucefVFS.h"
#include "dvcppstringutils.h"

#include "AssetWorkbenchLib.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestAssetWorkbenchViews.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

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

/** The workbench in a window of the tests with a model of a cube mounted */
class CViewFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CAssetCapture* capture;
    CWorkbenchShell* shell;
    CORE::CString dir;
    bool ready;

    explicit CViewFixture( const CORE::CString& name, GUI::CLayoutManager* layout = GUCEF_NULL )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), capture( GUCEF_NULL ), shell( GUCEF_NULL ), dir(), ready( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        if ( !WriteTestFile( dir + "/model.gltf", BuildTestGltf() ) || !WriteTestPng( dir + "/checker.png", 8, 8 ) || !mounter.Mount( "avviews", dir ) )
            return;
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        capture = GUCEF_NEW CAssetCapture( window );
        if ( !capture->Initialize( 64 ) )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        ready = shell->Initialize( *context, capture, MakeCleanTestDir( name + "_cache" ) );
        if ( ready && GUCEF_NULL != layout )
            shell->SetLayoutManager( layout );
    }

    ~CViewFixture()
    {
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        contextPtr.Unlink();
        if ( GUCEF_NULL != capture )
        {
            capture->Shutdown();
            GUCEF_DELETE capture;
        }
        mounter.UnmountAll();
    }

    IMAGE::TPixelMapPtr Frames( UInt32 count )
    {
        IMAGE::TPixelMapPtr pixels;
        for ( UInt32 i=0; i<count; ++i )
        {
            shell->Update( 5 );
            window->RenderFrame( &pixels );
        }
        return pixels;
    }
};

/** What the list of the views shows */
CORE::CString
ViewListText( CWorkbenchShell& shell )
{
    GUI::CWidget* list = shell.GetViewport()->GetToggleWidget( "view" );
    return GUCEF_NULL != list ? list->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString( "no list" );
}

/** A hash of the pixels of a rectangle of a picture */
UInt64
HashRegion( const IMAGE::TPixelMapPtr& pixels, UInt32 left, UInt32 top, UInt32 width, UInt32 height )
{
    UInt64 hash = 1469598103934665603ULL;
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() );
    const UInt32 stride = pixels->GetWidthInPixels() * 4;
    for ( UInt32 y=top; y<top+height; ++y )
    {
        for ( UInt32 x=left*4; x<( left + width ) * 4; ++x )
        {
            hash ^= data[ (size_t) y * stride + x ];
            hash *= 1099511628211ULL;
        }
    }
    return hash;
}

/** How many pixels of the rectangle are not the background of the stage */
UInt32
CountContent( const IMAGE::TPixelMapPtr& pixels, UInt32 left, UInt32 top, UInt32 width, UInt32 height )
{
    UInt32 count = 0;
    for ( UInt32 y=top; y<top+height; y+=2 )
    {
        for ( UInt32 x=left; x<left+width; x+=2 )
        {
            UInt8 rgba[ 4 ];
            GetPixel( pixels, (Int32) x, (Int32) y, rgba );
            if ( abs( (Int32) rgba[ 0 ] - 43 ) > 6 || abs( (Int32) rgba[ 1 ] - 46 ) > 6 || abs( (Int32) rgba[ 2 ] - 54 ) > 6 )
                ++count;
        }
    }
    return count;
}

/** Counts the events of a notifier */
class CEventCounter : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CEventCounter > TEventCallback;

    UInt32 count;

    CEventCounter( CORE::CNotifier* notifier, const CORE::CEvent& id ) : CORE::CObserver(), count( 0 )
    {
        TEventCallback callback( this, &CEventCounter::OnEvent );
        SubscribeTo( notifier, id, callback );
    }

    virtual ~CEventCounter() { UnsubscribeAllFromObserver(); }

    void OnEvent( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) { ++count; }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestViewPresetTracker( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "View presets: the label is the preset only while the camera is at it, any orbit, pan, zoom or numeric change makes it Custom and going back makes it the preset again" )
    try
    {
        COrbitController controller;
        controller.SetBounds( COrbitController::SVector3( 1.0f, 2.0f, 3.0f ), 2.0f );
        CViewPresetTracker tracker;
        ASSERT_TRUE( "Frame" == tracker.GetLabel() && !tracker.IsCustom() && !tracker.IsFourViews() );

        const CViewPresetTracker::EPreset fixedViews[] = { CViewPresetTracker::PRESET_FRONT, CViewPresetTracker::PRESET_SIDE, CViewPresetTracker::PRESET_TOP, CViewPresetTracker::PRESET_THREE_QUARTER };
        for ( UInt32 i=0; i<4; ++i )
        {
            tracker.Apply( fixedViews[ i ], controller );
            ASSERT_TRUE( CViewPresetTracker::GetPresetName( fixedViews[ i ] ) == tracker.GetLabel() );
            ASSERT_FALSE( tracker.Update( controller ) );

            // orbit, pan, zoom and a number typed in each leave the preset
            COrbitController::SCameraState state = controller.GetState();
            controller.OnPointerButton( 10, 10, GUI::MOUSEBUTTON_LEFT, true, 1, 0 );
            controller.OnPointerMove( 14, 10, 1u << (UInt32) GUI::MOUSEBUTTON_LEFT, 0 );
            controller.OnPointerButton( 14, 10, GUI::MOUSEBUTTON_LEFT, false, 1, 0 );
            ASSERT_TRUE( tracker.Update( controller ) && tracker.IsCustom() && "Custom" == tracker.GetLabel() );
            ASSERT_FALSE( tracker.Update( controller ) );

            tracker.Apply( fixedViews[ i ], controller );
            controller.OnWheel( 0, 0, 1, 0 );
            ASSERT_TRUE( tracker.Update( controller ) && "Custom" == tracker.GetLabel() );
            tracker.Apply( fixedViews[ i ], controller );
            controller.SetTarget( COrbitController::SVector3( state.target.x + 0.5f, state.target.y, state.target.z ) );
            ASSERT_TRUE( tracker.Update( controller ) && "Custom" == tracker.GetLabel() );
            tracker.Apply( fixedViews[ i ], controller );
            controller.SetDistance( controller.GetDistance() * 1.02f );
            ASSERT_TRUE( tracker.Update( controller ) && "Custom" == tracker.GetLabel() );
            tracker.Apply( fixedViews[ i ], controller );
            controller.SetAngles( controller.GetYaw() + 0.05f, controller.GetPitch() );
            ASSERT_TRUE( tracker.Update( controller ) && "Custom" == tracker.GetLabel() );

            // back at the preset by the preset itself, or by the keys of the view (1 to 4)
            controller.ApplyPreset( static_cast< COrbitController::EViewPreset >( (Int32) fixedViews[ i ] - 1 ) );
            ASSERT_TRUE( tracker.Update( controller ) && CViewPresetTracker::GetPresetName( fixedViews[ i ] ) == tracker.GetLabel() );
        }

        // another preset by the keys is that preset, not Custom
        tracker.Apply( CViewPresetTracker::PRESET_FRONT, controller );
        controller.OnKey( INPUT::KEYCODE_3, true, 0 );
        ASSERT_TRUE( tracker.Update( controller ) && "Top" == tracker.GetLabel() );

        // Frame is the bounds framed from the angles it was chosen at; orbiting from there is Custom, and back is Frame again
        controller.SetAngles( 0.7f, 0.2f );
        tracker.Apply( CViewPresetTracker::PRESET_FRAME, controller );
        ASSERT_TRUE( "Frame" == tracker.GetLabel() );
        controller.SetAngles( 0.9f, 0.2f );
        ASSERT_TRUE( tracker.Update( controller ) && "Custom" == tracker.GetLabel() );
        controller.SetAngles( 0.7f, 0.2f );
        ASSERT_TRUE( tracker.Update( controller ) && "Frame" == tracker.GetLabel() );
        controller.SetDistance( controller.GetDistance() * 2.0f );
        ASSERT_TRUE( tracker.Update( controller ) && "Custom" == tracker.GetLabel() );
        controller.Frame();
        ASSERT_TRUE( tracker.Update( controller ) && "Frame" == tracker.GetLabel() );

        // names of presets, the text of the command line
        CViewPresetTracker::EPreset parsed = CViewPresetTracker::PRESET_FRAME;
        ASSERT_TRUE( CViewPresetTracker::TryParsePreset( "four views", parsed ) && CViewPresetTracker::PRESET_FOUR_VIEWS == parsed );
        ASSERT_TRUE( CViewPresetTracker::TryParsePreset( "QUAD", parsed ) && CViewPresetTracker::PRESET_FOUR_VIEWS == parsed );
        ASSERT_TRUE( CViewPresetTracker::TryParsePreset( "Three quarter", parsed ) && CViewPresetTracker::PRESET_THREE_QUARTER == parsed );
        ASSERT_FALSE( CViewPresetTracker::TryParsePreset( "sideways", parsed ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "View presets: in the four views the label is Four views, Custom 1/4 while the perspective camera is away from its preset, and Four views again when it is back" )
    try
    {
        COrbitController controller;
        controller.SetBounds( COrbitController::SVector3(), 1.0f );
        CViewPresetTracker tracker;
        tracker.Apply( CViewPresetTracker::PRESET_FOUR_VIEWS, controller );
        ASSERT_TRUE( tracker.IsFourViews() && "Four views" == tracker.GetLabel() && CViewPresetTracker::PRESET_FOUR_VIEWS == tracker.GetPreset() );
        ASSERT_FALSE( tracker.Update( controller ) );

        controller.SetAngles( controller.GetYaw() + 0.3f, controller.GetPitch() );
        ASSERT_TRUE( tracker.Update( controller ) && tracker.IsCustom() && "Custom 1/4" == tracker.GetLabel() && tracker.IsFourViews() );
        controller.OnWheel( 0, 0, -1, 0 );
        ASSERT_FALSE( tracker.Update( controller ) );
        ASSERT_TRUE( "Custom 1/4" == tracker.GetLabel() );
        controller.ApplyPreset( COrbitController::VIEWPRESET_THREE_QUARTER );
        ASSERT_TRUE( tracker.Update( controller ) && "Four views" == tracker.GetLabel() );

        // another preset leaves the four views
        tracker.Apply( CViewPresetTracker::PRESET_TOP, controller );
        ASSERT_FALSE( tracker.IsFourViews() );
        controller.SetAngles( 0.2f, 0.3f );
        ASSERT_TRUE( tracker.Update( controller ) && "Custom" == tracker.GetLabel() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestViewList( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell: the view list follows the camera: a preset of the list is the label until the camera moves, then Custom without the preset being applied again" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CViewFixture fixture( "views_list" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.OpenAsset( "/avviews/model.gltf" ) );
        fixture.Frames( 3 );
        GUI::CViewportForm& form = *shell.GetViewport();
        ASSERT_TRUE( "Frame" == ViewListText( shell ) && "Frame" == shell.GetViewPresetLabel() );

        // the list has the choices of the presets
        const GUI::CViewportForm::TToggleVector& toggles = form.GetToggles();
        UInt32 viewChoices = 0;
        for ( size_t i=0; i<toggles.size(); ++i )
            if ( "view" == toggles[ i ].id )
                viewChoices = (UInt32) toggles[ i ].choices.size();
        ASSERT_TRUE( 6 == viewChoices );

        // Front chosen as the user does
        ASSERT_TRUE( form.SetToggleChoice( "view", 1 ) );
        fixture.Frames( 2 );
        ASSERT_TRUE( "Front" == ViewListText( shell ) && 0.0f == shell.GetController().GetYaw() && 0.0f == shell.GetController().GetPitch() );

        // the camera moves: the list says Custom and the camera stays where the user put it
        shell.GetController().SetAngles( 0.6f, 0.2f );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Custom" == ViewListText( shell ) && -1 == form.GetToggleChoice( "view" ) );
        ASSERT_TRUE( 0.6f == shell.GetController().GetYaw() && 0.2f == shell.GetController().GetPitch() );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Custom" == ViewListText( shell ) && 0.6f == shell.GetController().GetYaw() );

        // by the mouse in the view: a drag of the left button orbits
        shell.GetController().ApplyPreset( COrbitController::VIEWPRESET_SIDE );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Side" == ViewListText( shell ) );
        shell.GetController().OnPointerButton( 10, 10, GUI::MOUSEBUTTON_LEFT, true, 1, 0 );
        shell.GetController().OnPointerMove( 30, 10, 1u << (UInt32) GUI::MOUSEBUTTON_LEFT, 0 );
        shell.GetController().OnPointerButton( 30, 10, GUI::MOUSEBUTTON_LEFT, false, 1, 0 );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Custom" == ViewListText( shell ) );

        // the preset of the list again: the list shows it and the camera is at it
        ASSERT_TRUE( form.SetToggleChoice( "view", 3 ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Top" == ViewListText( shell ) && 3 == form.GetToggleChoice( "view" ) );
        shell.GetController().OnKey( INPUT::KEYCODE_1, true, 0 );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Front" == ViewListText( shell ) && 1 == form.GetToggleChoice( "view" ) );

        // the picture of the stage is drawn from the camera that the list names
        ASSERT_TRUE( shell.GetStage().GetViewCount() == 1 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestFourViews( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Four views: the layout has a perspective view and the front, side and top views in a grid of two by two that each show the model, with the same selection of the scene, and draw once when nothing changes" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CViewFixture fixture( "views_four" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.OpenAsset( "/avviews/model.gltf" ) );
        fixture.Frames( 3 );
        CWorkbenchStage& stage = shell.GetStage();
        ASSERT_TRUE( 1 == stage.GetViewCount() && !shell.IsFourViewsShown() );

        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "view", 5 ) );
        fixture.Frames( 4 );
        ASSERT_TRUE( shell.IsFourViewsShown() && 4 == stage.GetViewCount() && "Four views" == ViewListText( shell ) );
        ASSERT_TRUE( stage.GetViewLayout().GetCell( 0 ).projection == CWorkbenchViewLayout::PROJECTION_PERSPECTIVE );
        for ( UInt32 i=1; i<4; ++i )
            ASSERT_TRUE( stage.GetViewLayout().GetCell( i ).projection == CWorkbenchViewLayout::PROJECTION_ORTHOGRAPHIC && Ogre::PT_ORTHOGRAPHIC == stage.GetViewCamera( i )->getProjectionType() );
        ASSERT_TRUE( Ogre::PT_PERSPECTIVE == stage.GetViewCamera( 0 )->getProjectionType() );

        // the four views share the target (the orthographic cameras look at what the controller looks at) and the extents of the bounds
        const COrbitController::SVector3& target = shell.GetController().GetTarget();
        for ( UInt32 i=1; i<4; ++i )
        {
            const Ogre::Camera* camera = stage.GetViewCamera( i );
            const Ogre::Vector3 forward = camera->getDerivedDirection();
            const Ogre::Vector3 toTarget = ( Ogre::Vector3( target.x, target.y, target.z ) - camera->getDerivedPosition() ).normalisedCopy();
            ASSERT_TRUE( forward.dotProduct( toTarget ) > 0.999f );
        }
        // the extents follow the bounds (and the box where there is one): each orthographic view shows its two sides of the box, filling the part of the view the controller asks for
        {
            const COrbitController& controller = shell.GetController();
            ASSERT_TRUE( controller.HasBoundsBox() );
            const Float32 size[ 3 ] = { controller.GetBoundsMaximum().x - controller.GetBoundsMinimum().x, controller.GetBoundsMaximum().y - controller.GetBoundsMinimum().y, controller.GetBoundsMaximum().z - controller.GetBoundsMinimum().z };
            // front: x by y, side: z by y, top: x by z
            const Float32 widths[ 3 ] = { size[ 0 ], size[ 2 ], size[ 0 ] };
            const Float32 heights[ 3 ] = { size[ 1 ], size[ 1 ], size[ 2 ] };
            for ( UInt32 i=1; i<4; ++i )
            {
                const Ogre::Camera* camera = stage.GetViewCamera( i );
                const Float32 filled = COrbitController::GetFilledFraction( widths[ i-1 ], heights[ i-1 ], camera->getOrthoWindowHeight(), camera->getAspectRatio() );
                ASSERT_TRUE( fabsf( filled - controller.GetFillFraction() ) < 1e-3f );
            }
        }

        // the views are four rectangles of the texture, with a gap between them
        UInt32 rects[ 4 ][ 4 ];
        for ( UInt32 i=0; i<4; ++i )
        {
            ASSERT_TRUE( stage.GetViewRect( i, rects[ i ][ 0 ], rects[ i ][ 1 ], rects[ i ][ 2 ], rects[ i ][ 3 ] ) );
            ASSERT_TRUE( rects[ i ][ 2 ] > 40 && rects[ i ][ 3 ] > 40 );
        }
        ASSERT_TRUE( rects[ 1 ][ 0 ] >= rects[ 0 ][ 0 ] + rects[ 0 ][ 2 ] + 1 && rects[ 2 ][ 1 ] >= rects[ 0 ][ 1 ] + rects[ 0 ][ 3 ] + 1 );
        ASSERT_TRUE( rects[ 3 ][ 0 ] == rects[ 1 ][ 0 ] && rects[ 3 ][ 1 ] == rects[ 2 ][ 1 ] && rects[ 2 ][ 0 ] == rects[ 0 ][ 0 ] );

        IMAGE::TPixelMapPtr picture;
        ASSERT_TRUE( stage.ReadPixels( picture ) );
        DumpPicture( picture, "four_views_stage_" + CORE::CString( GetTestRenderer() ) );
        UInt64 hashes[ 4 ];
        for ( UInt32 i=0; i<4; ++i )
        {
            ASSERT_TRUE( CountContent( picture, rects[ i ][ 0 ], rects[ i ][ 1 ], rects[ i ][ 2 ], rects[ i ][ 3 ] ) > 30 );
            hashes[ i ] = HashRegion( picture, rects[ i ][ 0 ], rects[ i ][ 1 ], rects[ i ][ 2 ], rects[ i ][ 3 ] );
        }
        for ( UInt32 a=0; a<4; ++a )
            for ( UInt32 b=a+1; b<4; ++b )
                ASSERT_TRUE( hashes[ a ] != hashes[ b ] );

        // the gap between the views is the colour of the gap, not a view
        UInt8 gap[ 4 ];
        GetPixel( picture, (Int32) rects[ 1 ][ 0 ] - 1, (Int32) rects[ 0 ][ 1 ] + 20, gap );
        ASSERT_TRUE( abs( (Int32) gap[ 0 ] - 18 ) < 8 && abs( (Int32) gap[ 1 ] - 19 ) < 8 && abs( (Int32) gap[ 2 ] - 23 ) < 8 );

        // nothing changed: nothing is drawn
        fixture.Frames( 2 );
        ASSERT_FALSE( stage.IsDirty() );

        // the display of one view: the front view without grid, axes and bounds changes only the front view
        ASSERT_TRUE( stage.SetViewDisplay( 3, false, false, false ) );
        fixture.Frames( 2 );
        IMAGE::TPixelMapPtr changed;
        ASSERT_TRUE( stage.ReadPixels( changed ) );
        ASSERT_TRUE( hashes[ 0 ] == HashRegion( changed, rects[ 0 ][ 0 ], rects[ 0 ][ 1 ], rects[ 0 ][ 2 ], rects[ 0 ][ 3 ] ) );
        ASSERT_TRUE( hashes[ 1 ] == HashRegion( changed, rects[ 1 ][ 0 ], rects[ 1 ][ 1 ], rects[ 1 ][ 2 ], rects[ 1 ][ 3 ] ) );
        ASSERT_TRUE( hashes[ 2 ] == HashRegion( changed, rects[ 2 ][ 0 ], rects[ 2 ][ 1 ], rects[ 2 ][ 2 ], rects[ 2 ][ 3 ] ) );
        ASSERT_TRUE( hashes[ 3 ] != HashRegion( changed, rects[ 3 ][ 0 ], rects[ 3 ][ 1 ], rects[ 3 ][ 2 ], rects[ 3 ][ 3 ] ) );
        ASSERT_TRUE( stage.SetViewDisplay( 3, true, true, true ) );

        // the perspective camera moves: Custom 1/4, the layout stays and the orthographic views do not move with it
        const UInt64 frontBefore = hashes[ 1 ];
        shell.GetController().SetAngles( shell.GetController().GetYaw() + 0.4f, shell.GetController().GetPitch() );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Custom 1/4" == ViewListText( shell ) && 4 == stage.GetViewCount() && shell.IsFourViewsShown() );
        ASSERT_TRUE( stage.ReadPixels( changed ) );
        ASSERT_TRUE( hashes[ 0 ] != HashRegion( changed, rects[ 0 ][ 0 ], rects[ 0 ][ 1 ], rects[ 0 ][ 2 ], rects[ 0 ][ 3 ] ) );
        ASSERT_TRUE( frontBefore == HashRegion( changed, rects[ 1 ][ 0 ], rects[ 1 ][ 1 ], rects[ 1 ][ 2 ], rects[ 1 ][ 3 ] ) );

        // the perspective camera back at its preset: Four views again
        shell.GetController().ApplyPreset( COrbitController::VIEWPRESET_THREE_QUARTER );
        fixture.Frames( 3 );
        ASSERT_TRUE( "Four views" == ViewListText( shell ) );

        // another asset keeps the layout, another preset leaves it
        ASSERT_TRUE( shell.OpenAsset( "/avviews/model.gltf" ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( 4 == stage.GetViewCount() && "Four views" == ViewListText( shell ) );
        ASSERT_TRUE( shell.GetViewport()->SetToggleChoice( "view", 1 ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( 1 == stage.GetViewCount() && !shell.IsFourViewsShown() && "Front" == ViewListText( shell ) );
        IMAGE::TPixelMapPtr single;
        ASSERT_TRUE( stage.ReadPixels( single ) && CountContent( single, 0, 0, stage.GetWidth(), stage.GetHeight() ) > 100 );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Four views: the capture writes the sheet of the four views with different content in every quadrant, and the contact sheet is another picture" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CViewFixture fixture( "views_capture" );
        ASSERT_TRUE( fixture.ready );
        IMAGE::TPixelMapPtr sheet, contact;
        ASSERT_TRUE( fixture.capture->CaptureContactSheet( fixture.mounter, "/avviews/model.gltf", sheet, true ) );
        ASSERT_TRUE( !sheet.IsNULL() && CAssetCapture::GetSheetSize( 64 ) == sheet->GetWidthInPixels() && CAssetCapture::GetSheetSize( 64 ) == sheet->GetHeightInPixels() );
        DumpPicture( sheet, "capture_four_views_" + CORE::CString( GetTestRenderer() ) );
        ASSERT_TRUE( fixture.capture->CaptureContactSheet( fixture.mounter, "/avviews/model.gltf", contact, false ) );
        ASSERT_TRUE( !contact.IsNULL() && sheet->GetWidthInPixels() == contact->GetWidthInPixels() );

        // the stage is the single view again, at the size of a tile
        ASSERT_TRUE( 1 == fixture.capture->GetStage().GetViewCount() && 64 == fixture.capture->GetStage().GetWidth() );

        const UInt32 half = sheet->GetWidthInPixels() / 2;
        UInt64 hashes[ 4 ];
        for ( UInt32 q=0; q<4; ++q )
        {
            const UInt32 left = ( q % 2 ) * half + 3, top = ( q / 2 ) * half + 3;
            ASSERT_TRUE( CountContent( sheet, left, top, half - 6, half - 6 ) > 20 );
            hashes[ q ] = HashRegion( sheet, left, top, half - 6, half - 6 );
        }
        for ( UInt32 a=0; a<4; ++a )
            for ( UInt32 b=a+1; b<4; ++b )
                ASSERT_TRUE( hashes[ a ] != hashes[ b ] );
        ASSERT_TRUE( HashRegion( sheet, 0, 0, sheet->GetWidthInPixels(), sheet->GetHeightInPixels() ) != HashRegion( contact, 0, 0, contact->GetWidthInPixels(), contact->GetHeightInPixels() ) );

        // a texture has no four views: its sheet is the picture as before
        IMAGE::TPixelMapPtr textureSheet;
        ASSERT_TRUE( fixture.capture->CaptureContactSheet( fixture.mounter, "/avviews/checker.png", textureSheet, true ) && !textureSheet.IsNULL() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Four views: four views of the stage stay within the frame budget of a software renderer, drawn only when something changed" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CViewFixture fixture( "views_budget" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.OpenAsset( "/avviews/model.gltf" ) );
        fixture.Frames( 4 );
        const UInt64 frequency = MT::PrecisionTimerResolution();
        ASSERT_TRUE( frequency > 0 );

        // the same frames with the single view first: what a frame of the window costs on this renderer without the four views
        UInt64 singleWorst = 0;
        for ( UInt32 frame=0; frame<20; ++frame )
        {
            shell.GetController().SetAngles( shell.GetController().GetYaw() + 0.02f, shell.GetController().GetPitch() );
            const UInt64 start = MT::PrecisionTickCount();
            fixture.Frames( 1 );
            const UInt64 took = ( MT::PrecisionTickCount() - start ) * 1000 / frequency;
            if ( took > singleWorst )
                singleWorst = took;
        }
        ASSERT_TRUE( shell.ApplyViewPreset( "Four views" ) );
        fixture.Frames( 4 );
        ASSERT_TRUE( 4 == shell.GetStage().GetViewCount() );
        // a software renderer of Vulkan needs more than 100 ms for a frame of the window with one view: there the four views may cost twice that
        const UInt64 budget = singleWorst < 50 ? 100 : 2 * singleWorst + 50;
        for ( UInt32 run=0; run<3; ++run )
        {
            UInt64 worst = 0, total = 0;
            for ( UInt32 frame=0; frame<20; ++frame )
            {
                // every frame has a changed camera, so every frame draws the four views
                shell.GetController().SetAngles( shell.GetController().GetYaw() + 0.02f, shell.GetController().GetPitch() );
                const UInt64 start = MT::PrecisionTickCount();
                fixture.Frames( 1 );
                const UInt64 took = ( MT::PrecisionTickCount() - start ) * 1000 / frequency;
                if ( took > worst )
                    worst = took;
                total += took;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Four views: run " << run << ", the slowest frame " << worst << " ms, " << total / 20 << " ms on average (one view: slowest " << singleWorst << " ms, budget " << budget << " ms)" );
            ASSERT_TRUE( worst < budget );
        }
        // an idle frame draws nothing
        fixture.Frames( 3 );
        ASSERT_FALSE( shell.GetStage().IsDirty() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Four views: an asset kind can name the view preset its assets open with" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CViewFixture fixture( "views_default" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( "Four views" == shell.GetDefaultViewPreset( ASSETKIND_PARTICLE ) && shell.GetDefaultViewPreset( ASSETKIND_MODEL ).IsNULLOrEmpty() );
        shell.SetDefaultViewPreset( ASSETKIND_MODEL, "Four views" );
        ASSERT_TRUE( "Four views" == shell.GetDefaultViewPreset( ASSETKIND_MODEL ) );
        ASSERT_TRUE( shell.OpenAsset( "/avviews/model.gltf" ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( shell.IsFourViewsShown() && 4 == shell.GetStage().GetViewCount() && "Four views" == ViewListText( shell ) );
        shell.SetDefaultViewPreset( ASSETKIND_MODEL, "Top" );
        ASSERT_TRUE( shell.OpenAsset( "/avviews/model.gltf" ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( !shell.IsFourViewsShown() && 1 == shell.GetStage().GetViewCount() && "Top" == ViewListText( shell ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestWindowArrangement( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Shell layout: the resources, the views and the inspector are separated by splitters at the sizes of the start layout, and the report row is below its own splitter" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CViewFixture fixture( "views_arrange" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frames( 3 );
        ASSERT_TRUE( GUCEF_NULL != shell.GetMainSplitter() && GUCEF_NULL != shell.GetSideSplitter() && GUCEF_NULL != shell.GetViewSplitter() );
        ASSERT_TRUE( GUCEF_NULL != shell.GetLeftPanel() && GUCEF_NULL != shell.GetRightPanel() );
        const GUI::CRect root = shell.GetMainSplitter()->GetRect();
        const GUI::CRect left = shell.GetLeftPanel()->GetRect();
        const GUI::CRect right = shell.GetRightPanel()->GetRect();
        GUI::CWidget* center = shell.GetViewSplitter()->GetFirstPane();
        ASSERT_TRUE( GUCEF_NULL != center );

        // as the fixed fractions were: the resources a bit over a quarter, the inspector a quarter, the rest the views
        const Float32 leftShare = (Float32) left.width / (Float32) root.width;
        const Float32 rightShare = (Float32) right.width / (Float32) root.width;
        ASSERT_TRUE( leftShare > 0.25f && leftShare < 0.30f );
        ASSERT_TRUE( rightShare > 0.20f && rightShare < 0.26f );
        ASSERT_TRUE( left.Right() + shell.GetSideSplitter()->GetHandleExtent() == shell.GetViewSplitter()->GetRect().left );
        ASSERT_TRUE( center->GetRect().Right() + shell.GetViewSplitter()->GetHandleExtent() == right.left && right.Right() == root.Right() );
        ASSERT_TRUE( shell.GetMainSplitter()->GetRect().Bottom() == shell.GetSideSplitter()->GetRect().Bottom() + shell.GetMainSplitter()->GetHandleExtent() + shell.GetMainSplitter()->GetSecondPaneExtent() );
        ASSERT_TRUE( shell.GetMainSplitter()->GetSecondPane()->GetRect().top >= shell.GetSideSplitter()->GetRect().Bottom() + shell.GetMainSplitter()->GetHandleExtent() );

        // the splitter between the rows: dragging the handle gives the report row more room
        GUI::CSplitter* main = shell.GetMainSplitter();
        const GUI::CRect handle = main->GetHandleRect();
        const Int32 reportBefore = main->GetSecondPaneExtent();
        fixture.context->InjectMouseMove( main->GetRect().left + 400, main->GetRect().top + handle.top + handle.height / 2 );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        fixture.context->InjectMouseMove( main->GetRect().left + 400, main->GetRect().top + handle.top + handle.height / 2 - 60 );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        fixture.Frames( 3 );
        ASSERT_TRUE( main->GetSecondPaneExtent() >= reportBefore + 50 );
        ASSERT_TRUE( shell.GetLayoutManager().GetActivePreset().IsNULLOrEmpty() );

        // a double click on the handle puts it back
        const GUI::CRect moved = main->GetHandleRect();
        const Int32 x = main->GetRect().left + 400, y = main->GetRect().top + moved.top + moved.height / 2;
        for ( UInt32 click=0; click<2; ++click )
        {
            fixture.context->InjectMouseMove( x, y );
            fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        }
        fixture.Frames( 3 );
        ASSERT_TRUE( reportBefore == main->GetSecondPaneExtent() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shell layout: F9 and F10 fold the resources and the inspector into strips, F11 is the Preview layout and back, and the view gets the width" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CViewFixture fixture( "views_fold" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frames( 3 );
        GUI::CWidget* center = shell.GetViewSplitter()->GetFirstPane();
        const Int32 total = shell.GetMainSplitter()->GetRect().width;
        const Int32 centerBefore = center->GetRect().width;
        const Int32 leftBefore = shell.GetLeftPanel()->GetRect().width;
        const Int32 rightBefore = shell.GetRightPanel()->GetRect().width;
        ASSERT_TRUE( total == leftBefore + shell.GetSideSplitter()->GetHandleExtent() + centerBefore + shell.GetViewSplitter()->GetHandleExtent() + rightBefore );
        ASSERT_TRUE( "F9" == shell.GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldLeft ) && "F10" == shell.GetShortcuts().GetShortcutText( CWorkbenchShell::ActionFoldRight ) );

        GUI::CFoldablePanel* leftPanel = shell.GetLeftPanel();
        GUI::CFoldablePanel* rightPanel = shell.GetRightPanel();
        const Int32 strip = leftPanel->GetFoldedExtent();
        CEventCounter changed( &shell.GetLayoutManager(), GUI::CLayoutManager::LayoutChangedEvent );
        fixture.context->InjectKey( INPUT::KEYCODE_F9, true, 0 );
        fixture.context->InjectKey( INPUT::KEYCODE_F9, false, 0 );
        fixture.Frames( 3 );
        ASSERT_TRUE( leftPanel->IsFolded() && !rightPanel->IsFolded() && strip == leftPanel->GetRect().width && 1 == changed.count );
        ASSERT_TRUE( center->GetRect().width > centerBefore );
        fixture.context->InjectKey( INPUT::KEYCODE_F10, true, 0 );
        fixture.context->InjectKey( INPUT::KEYCODE_F10, false, 0 );
        fixture.Frames( 3 );
        ASSERT_TRUE( leftPanel->IsFolded() && rightPanel->IsFolded() && strip == rightPanel->GetRect().width );

        // both folded: the view has the whole width but the two strips, and its surface follows
        ASSERT_TRUE( center->GetRect().width == total - 2 * strip );
        UInt32 surfaceWidth = 0, surfaceHeight = 0;
        shell.GetViewport()->GetSurfaceSize( surfaceWidth, surfaceHeight );
        ASSERT_TRUE( (Int32) surfaceWidth > centerBefore && shell.GetStage().GetWidth() == surfaceWidth );
        IMAGE::TPixelMapPtr folded = fixture.Frames( 1 );
        DumpPicture( folded, "layout_folded_" + CORE::CString( GetTestRenderer() ) );

        // unfolded again by a click on the strip: the sizes come back
        fixture.context->InjectMouseMove( leftPanel->GetRect().left + 10, leftPanel->GetRect().top + 200 );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        fixture.Frames( 3 );
        ASSERT_TRUE( !leftPanel->IsFolded() && leftBefore == leftPanel->GetRect().width );
        shell.ToggleRightSide();
        fixture.Frames( 3 );
        ASSERT_TRUE( !rightPanel->IsFolded() && centerBefore == center->GetRect().width && rightBefore == rightPanel->GetRect().width );

        // F11: the Preview layout, and again the layout from before
        shell.GetSideSplitter()->SetSplitRatio( 0.35f );
        fixture.Frames( 2 );
        const Int32 sizedLeft = leftPanel->GetRect().width;
        fixture.context->InjectKey( INPUT::KEYCODE_F11, true, 0 );
        fixture.context->InjectKey( INPUT::KEYCODE_F11, false, 0 );
        fixture.Frames( 3 );
        ASSERT_TRUE( leftPanel->IsFolded() && rightPanel->IsFolded() && CWorkbenchShell::LayoutPreview == shell.GetLayoutManager().GetActivePreset() );
        ASSERT_TRUE( center->GetRect().width == total - 2 * strip );
        fixture.context->InjectKey( INPUT::KEYCODE_F11, true, 0 );
        fixture.context->InjectKey( INPUT::KEYCODE_F11, false, 0 );
        fixture.Frames( 3 );
        ASSERT_TRUE( !leftPanel->IsFolded() && !rightPanel->IsFolded() && sizedLeft == leftPanel->GetRect().width );

        // the named layouts
        ASSERT_TRUE( shell.ApplyLayoutPreset( CWorkbenchShell::LayoutReview ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( leftPanel->IsFolded() && !rightPanel->IsFolded() && CWorkbenchShell::LayoutReview == shell.GetLayoutManager().GetActivePreset() );
        ASSERT_TRUE( shell.ApplyLayoutPreset( CWorkbenchShell::LayoutBrowse ) );
        fixture.Frames( 3 );
        ASSERT_TRUE( !leftPanel->IsFolded() && !rightPanel->IsFolded() && centerBefore + 40 > center->GetRect().width && center->GetRect().width + 40 > centerBefore );
        ASSERT_FALSE( shell.ApplyLayoutPreset( "Nothing" ) );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shell layout: the layout of the user is kept in the settings and comes back, with the sides that were folded" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CORE::CDataNode settings( "Config" );
        CORE::CDataNode* section = settings.Structure( "AssetWorkbench", '/' );
        Float32 shapedRatio = 0.0f;
        {
            GUI::CLayoutManager layout;
            CViewFixture fixture( "views_keep_a", &layout );
            ASSERT_TRUE( fixture.ready );
            fixture.Frames( 3 );
            fixture.shell->GetSideSplitter()->SetSplitRatio( 0.4f );
            fixture.shell->GetMainSplitter()->SetSplitRatio( 0.6f );
            fixture.shell->GetRightPanel()->SetFolded( true );
            shapedRatio = fixture.shell->GetSideSplitter()->GetSplitRatio();
            ASSERT_TRUE( layout.Save( *section ) );
        }
        {
            // a settings file of another version has more and less than this one needs
            CORE::CDataNode* layoutNode = section->FindChild( "Layout" );
            ASSERT_TRUE( GUCEF_NULL != layoutNode );
            CORE::CDataNode* current = layoutNode->FindChild( "Current" );
            ASSERT_TRUE( GUCEF_NULL != current );
            CORE::CDataNode* entry = current->AddChild( "Splitter" );
            entry->SetAttribute( "name", "TheSplitterOfALaterVersion" );
            entry->SetAttribute( "ratio", "0.3" );
            current->AddChild( "WhatEverElse" )->SetAttribute( "name", "x" );
        }
        GUI::CLayoutManager restored;
        ASSERT_TRUE( restored.Load( *section ) );
        {
            CViewFixture fixture( "views_keep_b", &restored );
            ASSERT_TRUE( fixture.ready );
            fixture.Frames( 3 );
            ASSERT_TRUE( 0.4f - fixture.shell->GetSideSplitter()->GetSplitRatio() < 0.001f && fixture.shell->GetSideSplitter()->GetSplitRatio() - 0.4f < 0.001f );
            ASSERT_TRUE( shapedRatio == fixture.shell->GetSideSplitter()->GetSplitRatio() );
            ASSERT_TRUE( fixture.shell->GetRightPanel()->IsFolded() && !fixture.shell->GetLeftPanel()->IsFolded() );
            ASSERT_TRUE( fixture.shell->GetRightPanel()->GetFoldedExtent() == fixture.shell->GetRightPanel()->GetRect().width );
            // the built in layouts are still there, the user ones are the user's
            ASSERT_TRUE( fixture.shell->GetLayoutManager().HasPreset( CWorkbenchShell::LayoutPreview ) );
        }

        // nothing stored: the window starts as it did
        GUI::CLayoutManager none;
        CViewFixture fixture( "views_keep_c", &none );
        ASSERT_TRUE( fixture.ready );
        fixture.Frames( 3 );
        ASSERT_TRUE( !fixture.shell->GetLeftPanel()->IsFolded() && !fixture.shell->GetRightPanel()->IsFolded() );
    }
    catch ( const ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchViewTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.Views" );

    TestViewPresetTracker();
    TestViewList();
    TestFourViews();
    TestWindowArrangement();
}

/*-------------------------------------------------------------------------*/
