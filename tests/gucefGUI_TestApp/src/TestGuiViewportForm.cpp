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

/** A controller that writes down what it is given, in the coordinates it is given them in */
class CRecordingController : public GUI::CIViewportController
{
    public:

    struct SRecord
    {
        CORE::CString kind;
        Int32 x;
        Int32 y;
        Int32 value;
        UInt32 modifiers;
    };

    GUCEF::vector< SRecord > records;
    UInt32 width;
    UInt32 height;
    UInt32 resizes;
    UInt32 captureLost;

    CRecordingController( void ) : GUI::CIViewportController(), records(), width( 0 ), height( 0 ), resizes( 0 ), captureLost( 0 ) {}

    void Add( const char* kind, Int32 x, Int32 y, Int32 value, UInt32 modifiers )
    {
        SRecord record;
        record.kind = kind;
        record.x = x;
        record.y = y;
        record.value = value;
        record.modifiers = modifiers;
        records.push_back( record );
    }

    UInt32 Count( const char* kind ) const
    {
        UInt32 count = 0;
        for ( size_t i=0; i<records.size(); ++i )
            if ( records[ i ].kind == kind )
                ++count;
        return count;
    }

    const SRecord* Last( const char* kind ) const
    {
        for ( size_t i=records.size(); i>0; --i )
            if ( records[ i - 1 ].kind == kind )
                return &records[ i - 1 ];
        return GUCEF_NULL;
    }

    virtual void OnPointerMove( Int32 x, Int32 y, UInt32 buttonStates, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE { Add( "move", x, y, (Int32) buttonStates, modifiers ); }
    virtual void OnPointerButton( Int32 x, Int32 y, GUI::TMouseButton button, bool pressed, UInt32 clickCount, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE { Add( pressed ? "down" : "up", x, y, (Int32) button, modifiers ); }
    virtual void OnWheel( Int32 x, Int32 y, Int32 delta, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE { Add( "wheel", x, y, delta, modifiers ); }
    virtual void OnKey( INPUT::KeyCode keyCode, bool pressed, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE { Add( pressed ? "keydown" : "keyup", 0, 0, (Int32) keyCode, modifiers ); }
    virtual void OnPointerCaptureLost( void ) GUCEF_VIRTUAL_OVERRIDE { ++captureLost; }
    virtual void OnSurfaceResized( UInt32 w, UInt32 h ) GUCEF_VIRTUAL_OVERRIDE { width = w; height = h; ++resizes; }
};

GUI::CViewportForm::TToggleVector
MakeToggles( void )
{
    GUI::CViewportForm::TToggleVector toggles;
    toggles.push_back( GUI::CViewportForm::TToggle( "wire", "Wireframe", "Shows the edges", false ) );
    toggles.push_back( GUI::CViewportForm::TToggle( "shadows", "Shadows", "", true ) );
    GUI::CViewportForm::TToggle camera( "camera", "Camera", "The camera in use" );
    camera.choices.push_back( "Orbit" );
    camera.choices.push_back( "Fly" );
    camera.choices.push_back( "Top" );
    toggles.push_back( camera );
    return toggles;
}

class CViewportScene : public CFormScene
{
    public:

    GUI::CViewportForm form;

    explicit CViewportScene( GUI::CGUIContext* context ) : CFormScene(), form( context )
    {
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        form.SetToggles( MakeToggles() );
        form.SetToggleChoice( "camera", 2 );
        GUI::CDrawSurface* surface = form.GetSurface();
        surface->FillRect( GUI::CRect( 10, 10, 200, 120 ), GUI::CColour( 200, 90, 40, 255 ) );
        surface->StrokeRect( GUI::CRect( 5, 5, 300, 200 ), GUI::CColour( 255, 255, 255, 255 ), 2 );
        surface->DrawLine( 0, 0, 280, 190, GUI::CColour( 40, 200, 90, 255 ) );
        surface->DrawString( "Test", "Viewport", 20, 150, GUI::CColour( 255, 255, 0, 255 ) );
        surface->Commit();
    }
};

CFormScene*
BuildViewportScene( GUI::CGUIContext* context )
{
    return new CViewportScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiViewportFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.ViewportForm" );

    GUCEF_TESTFW_TESTCASE( "Viewport: The form builds as a window and in the place of a widget, with a surface that fills it and exposes its rectangle" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CViewportForm form( context );
        ASSERT_TRUE( GUCEF_NULL == form.GetSurface() );
        ASSERT_TRUE( form.Build( "View" ) && form.IsVisible() );
        GUI::CDrawSurface* surface = form.GetSurface();
        ASSERT_TRUE( GUCEF_NULL != surface && "DrawSurface" == surface->GetType() && HasWidget( form, "Toolbar" ) );
        ASSERT_TRUE( GUCEF_NULL == form.GetController() );

        // no toggles: no tool bar, the surface has all the room
        context->RenderFrame( 16 );
        ASSERT_FALSE( form.GetWidget( "Toolbar" )->IsVisible() );
        const GUI::CRect content = form.GetWidget( "Content" )->GetRect();
        ASSERT_TRUE( form.GetSurfaceRect() == surface->GetRect() && content == surface->GetRect() );
        UInt32 width = 0, height = 0;
        form.GetSurfaceSize( width, height );
        ASSERT_TRUE( (Int32) width == content.width && (Int32) height == content.height && width > 100 );

        // the toggles make the tool bar, which the surface leaves room for
        form.SetToggles( MakeToggles() );
        context->RenderFrame( 16 );
        ASSERT_TRUE( form.GetWidget( "Toolbar" )->IsVisible() );
        ASSERT_TRUE( form.GetSurfaceRect().top >= content.top + 30 && form.GetSurfaceRect().height < content.height );

        // in the place of a widget of another form
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Position", "0,20 0,30" );
        placeholder->SetProperty( "Size", "0,300 0,200" );
        GUI::CViewportForm inPlace;
        ASSERT_FALSE( inPlace.Build( static_cast< GUI::CWidget* >( GUCEF_NULL ) ) );
        ASSERT_TRUE( inPlace.Build( placeholder ) && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        context->RenderFrame( 16 );
        ASSERT_TRUE( GUI::CRect( 20, 30, 300, 200 ) == inPlace.GetSurfaceRect() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Viewport: The toggles of the application are check boxes and lists, and the user's changes are notified with the id and the state" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CViewportForm form( context );
        ASSERT_TRUE( form.Build() );
        form.SetToggles( MakeToggles() );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 3 == form.GetToggles().size() );
        GUI::CWidget* wire = form.GetToggleWidget( "wire" );
        GUI::CWidget* camera = form.GetToggleWidget( "camera" );
        ASSERT_TRUE( GUCEF_NULL != wire && "Checkbox" == wire->GetType() && "Wireframe" == wire->GetProperty( "Text" ) && "Shows the edges" == wire->GetProperty( "Tooltip" ) );
        ASSERT_TRUE( GUCEF_NULL != camera && "Combobox" == camera->GetType() && GUCEF_NULL == form.GetToggleWidget( "none" ) );
        ASSERT_FALSE( form.GetToggleChecked( "wire" ) );
        ASSERT_TRUE( form.GetToggleChecked( "shadows" ) && form.GetToggleWidget( "shadows" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
        ASSERT_TRUE( 0 == form.GetToggleChoice( "camera" ) && -1 == form.GetToggleChoice( "wire" ) );

        CEventRecorder changed( &form, GUI::CViewportForm::ToggleChangedEvent );

        // a click on the check box
        ClickWidget( context, wire );
        ASSERT_TRUE( 1 == changed.count && form.GetToggleChecked( "wire" ) );
        GUI::CViewportForm::TToggleChange change = changed.Last< GUI::CViewportForm::TToggleChangedEventData >()->GetData();
        ASSERT_TRUE( "wire" == change.id && change.checked && -1 == change.choice );
        ClickWidget( context, wire );
        ASSERT_TRUE( 2 == changed.count && !form.GetToggleChecked( "wire" ) );

        // a pick in the list
        camera->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "1" );
        ASSERT_TRUE( 3 == changed.count && 1 == form.GetToggleChoice( "camera" ) );
        change = changed.Last< GUI::CViewportForm::TToggleChangedEventData >()->GetData();
        ASSERT_TRUE( "camera" == change.id && 1 == change.choice );

        // the functions change the same state, with the same event, and the widgets follow
        ASSERT_TRUE( form.SetToggleChecked( "shadows", false ) );
        ASSERT_TRUE( 4 == changed.count && !form.GetToggleWidget( "shadows" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
        ASSERT_TRUE( form.SetToggleChecked( "shadows", false ) && 4 == changed.count );        // no change, no event
        ASSERT_TRUE( form.SetToggleChoice( "camera", 2 ) && 5 == changed.count && 2 == camera->GetIntProperty( GUI::EWidgetProperty_SelectedIndex ) );
        ASSERT_FALSE( form.SetToggleChoice( "camera", 3 ) );
        ASSERT_FALSE( form.SetToggleChecked( "camera", true ) );
        ASSERT_FALSE( form.SetToggleChoice( "wire", 0 ) );
        ASSERT_FALSE( form.SetToggleChecked( "unknown", true ) );

        // no toggles: the tool bar goes
        form.SetToggles( GUI::CViewportForm::TToggleVector() );
        context->RenderFrame( 16 );
        ASSERT_FALSE( form.GetWidget( "Toolbar" )->IsVisible() );
        ASSERT_TRUE( GUCEF_NULL == form.GetToggleWidget( "wire" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Viewport: The controller gets the pointer, the wheel and the keys of the surface in the coordinates of the surface, also outside it while a press is held" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CViewportForm form( context );
        ASSERT_TRUE( form.Build() );
        form.SetToggles( MakeToggles() );
        CRecordingController controller;
        form.SetController( &controller );
        ASSERT_TRUE( &controller == form.GetController() );
        context->RenderFrame( 16 );
        const GUI::CRect rect = form.GetSurfaceRect();

        // a move and a click inside
        context->InjectMouseMove( rect.left + 20, rect.top + 30 );
        ASSERT_TRUE( controller.Count( "move" ) >= 1 );
        ASSERT_TRUE( 20 == controller.Last( "move" )->x && 30 == controller.Last( "move" )->y );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( 1 == controller.Count( "down" ) && 20 == controller.Last( "down" )->x && 30 == controller.Last( "down" )->y );
        ASSERT_TRUE( (Int32) GUI::MOUSEBUTTON_LEFT == controller.Last( "down" )->value );

        // a drag that leaves the surface keeps going to the controller, with the coordinates of the surface
        context->InjectMouseMove( rect.left + rect.width + 40, rect.top + 50 );
        ASSERT_TRUE( rect.width + 40 == controller.Last( "move" )->x && 50 == controller.Last( "move" )->y );
        ASSERT_TRUE( 0 != ( controller.Last( "move" )->value & ( 1 << (Int32) GUI::MOUSEBUTTON_LEFT ) ) );
        context->InjectMouseMove( rect.left - 15, rect.top + 60 );
        ASSERT_TRUE( -15 == controller.Last( "move" )->x );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == controller.Count( "up" ) && -15 == controller.Last( "up" )->x );

        // the right button, with a modifier
        context->InjectMouseMove( rect.left + 100, rect.top + 100 );
        context->InjectKey( INPUT::KEYCODE_LCONTROL, true, INPUT::KEYMOD_CTRL );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, false );
        context->InjectKey( INPUT::KEYCODE_LCONTROL, false, 0 );
        ASSERT_TRUE( 2 == controller.Count( "down" ) && (Int32) GUI::MOUSEBUTTON_RIGHT == controller.Last( "down" )->value );
        ASSERT_TRUE( 0 != ( controller.Last( "down" )->modifiers & INPUT::KEYMOD_CTRL ) );

        // the wheel
        context->InjectMouseWheel( 2 );
        ASSERT_TRUE( 1 == controller.Count( "wheel" ) && 2 == controller.Last( "wheel" )->value && 100 == controller.Last( "wheel" )->x && 100 == controller.Last( "wheel" )->y );

        // the keys, while the surface has the focus (a click gave it)
        const UInt32 keysBefore = controller.Count( "keydown" );
        PressKey( context, INPUT::KEYCODE_W );
        ASSERT_TRUE( keysBefore + 1 <= controller.Count( "keydown" ) && controller.Count( "keyup" ) >= 1 );
        ASSERT_TRUE( (Int32) INPUT::KEYCODE_W == controller.Last( "keyup" )->value );

        // a press that ended without a release
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->CancelPointerCapture( "test" );
        ASSERT_TRUE( 1 == controller.captureLost );

        // without a controller nothing goes anywhere, and nothing breaks
        form.SetController( GUCEF_NULL );
        const size_t records = controller.records.size();
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->InjectMouseMove( rect.left + 5, rect.top + 5 );
        context->InjectMouseWheel( 1 );
        ASSERT_TRUE( records == controller.records.size() );

        // the pointer over the tool bar is not the surface's
        form.SetController( &controller );
        const size_t before = controller.records.size();
        context->InjectMouseMove( form.GetWidget( "Toolbar" )->GetRect().left + 3, form.GetWidget( "Toolbar" )->GetRect().top + 3 );
        ASSERT_TRUE( before == controller.records.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Viewport: The size of the surface is told when it is known and when it changes, to the controller and in SurfaceResizedEvent" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,300 0,200" );
        GUI::CViewportForm form( context );
        CEventRecorder resized( &form, GUI::CViewportForm::SurfaceResizedEvent );
        CRecordingController controller;
        form.SetController( &controller );
        ASSERT_TRUE( form.Build( placeholder ) );
        ASSERT_TRUE( 0 == resized.count && 0 == controller.resizes );        // not laid out yet
        context->RenderFrame( 16 );
        ASSERT_TRUE( 1 == resized.count && 1 == controller.resizes );
        GUI::CViewportForm::TSurfaceSize size = resized.Last< GUI::CViewportForm::TSurfaceSizeEventData >()->GetData();
        ASSERT_TRUE( 300 == size.width && 200 == size.height && 300 == controller.width && 200 == controller.height );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 1 == resized.count );                                    // the same size is not told again

        placeholder->SetProperty( "Size", "0,420 0,260" );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 2 == resized.count && 2 == controller.resizes );
        size = resized.Last< GUI::CViewportForm::TSurfaceSizeEventData >()->GetData();
        ASSERT_TRUE( 420 == size.width && 260 == size.height && 420 == controller.width );

        // the toggles take room: the surface is smaller, and that is told
        form.SetToggles( MakeToggles() );
        context->RenderFrame( 16 );
        ASSERT_TRUE( 3 == resized.count );
        size = resized.Last< GUI::CViewportForm::TSurfaceSizeEventData >()->GetData();
        ASSERT_TRUE( 420 == size.width && size.height < 260 && size.height > 200 );

        // a controller that is set later hears the size at once
        CRecordingController late;
        form.SetController( &late );
        ASSERT_TRUE( 1 == late.resizes && 420 == late.width );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Viewport: The application draws on the surface, and a layout of the form saves and loads again to a form that works the same" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CViewportForm first( context );
        ASSERT_TRUE( first.Build() );
        first.SetToggles( MakeToggles() );
        first.GetSurface()->FillRect( GUI::CRect( 0, 0, 10, 10 ), GUI::CColour( 255, 0, 0, 255 ) );
        first.GetSurface()->Commit();
        ASSERT_TRUE( 1 == first.GetSurface()->GetCommands().size() );
        CORE::CDataNode layout;
        ASSERT_TRUE( first.SaveLayout( layout ) );

        GUI::CViewportForm second( context );
        CRecordingController controller;
        second.SetController( &controller );
        ASSERT_TRUE( second.LoadLayout( layout ) && second.Show() );
        ASSERT_TRUE( GUCEF_NULL != second.GetSurface() && 0 == second.GetSurface()->GetCommands().size() );
        ASSERT_TRUE( 0 == second.GetToggles().size() );          // the toggles are the application's: the loaded form has none until it is given them
        second.SetToggles( MakeToggles() );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        const GUI::CRect rect = second.GetSurfaceRect();
        ASSERT_TRUE( controller.resizes >= 1 && rect.width > 100 );
        context->InjectMouseMove( rect.left + 7, rect.top + 9 );
        ASSERT_TRUE( 7 == controller.Last( "move" )->x && 9 == controller.Last( "move" )->y );
        CEventRecorder changed( &second, GUI::CViewportForm::ToggleChangedEvent );
        ClickWidget( context, second.GetToggleWidget( "wire" ) );
        ASSERT_TRUE( 1 == changed.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "viewport", &BuildViewportScene );
}

/*-------------------------------------------------------------------------*/
