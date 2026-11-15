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

GUI::CTimelineForm::TClipVector
MakeClips( void )
{
    GUI::CTimelineForm::TClipVector clips;
    clips.push_back( GUI::CTimelineForm::TClip( "Walk", 4000 ) );
    clips.push_back( GUI::CTimelineForm::TClip( "Run", 2000 ) );
    clips.push_back( GUI::CTimelineForm::TClip( "Idle", 0 ) );
    return clips;
}

/** A form with the clips, in a context of its own */
class CTimelineScene : public CFormScene
{
    public:

    GUI::CTimelineForm form;

    explicit CTimelineScene( GUI::CGUIContext* context ) : CFormScene(), form( context )
    {
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        form.SetClips( MakeClips() );
        form.SelectClip( 0 );
        form.SetTime( 1500 );
        form.SetLooping( true );
        form.SetSpeed( 2.0f );
    }
};

CFormScene*
BuildTimelineScene( GUI::CGUIContext* context )
{
    return new CTimelineScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiTimelineFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.TimelineForm" );

    GUCEF_TESTFW_TESTCASE( "Timeline: The form builds as a window and in the place of a widget, with the widgets of a time line" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );

        GUI::CTimelineForm window( context );
        ASSERT_TRUE( window.Build( "Clips" ) );
        ASSERT_TRUE( window.IsVisible() );
        ASSERT_TRUE( "Window" == window.GetRootWidget()->GetType() && "Clips" == window.GetRootWidget()->GetProperty( "Title" ) );
        const char* names[] = { "Clips", "Play", "Pause", "StepBack", "StepForward", "Time", "Speed", "Loop", "Readout" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( window, names[ i ] ) );

        // nothing to play yet: the buttons that need a clip wait for one
        ASSERT_FALSE( window.GetWidget( "Play" )->IsEnabled() );
        ASSERT_FALSE( window.GetWidget( "Pause" )->IsEnabled() );
        ASSERT_FALSE( window.Play() );
        ASSERT_TRUE( -1 == window.GetSelectedClip() && 0 == window.GetTime() );

        // in the place of a widget of another form
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Position", "0,10 0,10" );
        placeholder->SetProperty( "Size", "0,400 0,300" );
        GUI::CTimelineForm inPlace;
        ASSERT_FALSE( inPlace.Build( static_cast< GUI::CWidget* >( GUCEF_NULL ) ) );
        ASSERT_TRUE( inPlace.Build( placeholder ) );
        ASSERT_TRUE( "Panel" == inPlace.GetRootWidget()->GetType() && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        context->RenderFrame( 16 );
        ASSERT_TRUE( inPlace.GetWidget( "Time" )->GetRect().width > 300 && inPlace.GetWidget( "Time" )->GetRect().left >= 10 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Timeline: Clips are selected by the user and by the functions, with ClipSelectedEvent, and the time goes back to the start" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTimelineForm form( context );
        ASSERT_TRUE( form.Build() );
        form.SetClips( MakeClips() );
        ASSERT_TRUE( 3 == form.GetClips().size() && "Walk" == form.GetClips()[ 0 ].name );
        GUI::CListbox* list = form.GetWidgetAs< GUI::CListbox >( "Clips" );
        ASSERT_TRUE( 3 == list->GetLineCount() );

        CEventRecorder selected( &form, GUI::CTimelineForm::ClipSelectedEvent );
        CEventRecorder timeChanged( &form, GUI::CTimelineForm::TimeChangedEvent );
        ASSERT_FALSE( form.SelectClip( 3 ) );
        ASSERT_TRUE( form.SelectClip( 1 ) );
        ASSERT_TRUE( 1 == selected.count && 1 == static_cast< CORE::TCloneableInt32* >( selected.data[ 0 ] )->GetData() );
        ASSERT_TRUE( 1 == form.GetSelectedClip() && 2000 == form.GetClipLength() && 1 == list->GetSelectedIndex() );
        ASSERT_TRUE( form.SelectClip( 1 ) && 1 == selected.count );        // already there

        // the user clicks the first clip in the list
        ASSERT_TRUE( form.SetTime( 700 ) && 700 == form.GetTime() );
        context->RenderFrame( 16 );
        const GUI::CRect& rect = list->GetRect();
        ClickAt( context, rect.left + 20, rect.top + list->GetRowHeight() / 2 + 1 );
        ASSERT_TRUE( 0 == form.GetSelectedClip() && 2 == selected.count );
        ASSERT_TRUE( 4000 == form.GetClipLength() && 0 == form.GetTime() );
        ASSERT_TRUE( timeChanged.count >= 2 && !static_cast< GUI::CTimelineForm::TTimeChangedEventData* >( timeChanged.data.back() )->GetData().scrubbed );
        ASSERT_TRUE( form.GetWidget( "Play" )->IsEnabled() );

        // a clip without length plays nothing
        ASSERT_TRUE( form.SelectClip( 2 ) );
        ASSERT_FALSE( form.Play() );
        ASSERT_FALSE( form.GetWidget( "Play" )->IsEnabled() );
        form.ClearClips();
        ASSERT_TRUE( -1 == form.GetSelectedClip() && 0 == list->GetLineCount() );
        ASSERT_TRUE( -1 == static_cast< CORE::TCloneableInt32* >( selected.data.back() )->GetData() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Timeline: Playing advances the time with the time that passes and the speed, loops or stops at the end, and notifies the changes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTimelineForm form( context );
        ASSERT_TRUE( form.Build() );
        form.SetClips( MakeClips() );
        ASSERT_TRUE( form.SelectClip( 1 ) );     // 2000 ms
        CEventRecorder timeChanged( &form, GUI::CTimelineForm::TimeChangedEvent );
        CEventRecorder playState( &form, GUI::CTimelineForm::PlayStateChangedEvent );

        form.Update( 500 );                      // not playing: nothing moves
        ASSERT_TRUE( 0 == form.GetTime() && 0 == timeChanged.count );
        ASSERT_TRUE( form.Play() && form.IsPlaying() );
        ASSERT_TRUE( 1 == playState.count && static_cast< CORE::TCloneableBool* >( playState.data[ 0 ] )->GetData() );
        ASSERT_FALSE( form.GetWidget( "Play" )->IsEnabled() );
        ASSERT_TRUE( form.GetWidget( "Pause" )->IsEnabled() );
        form.Update( 500 );
        ASSERT_TRUE( 500 == form.GetTime() && 1 == timeChanged.count );
        ASSERT_FALSE( static_cast< GUI::CTimelineForm::TTimeChangedEventData* >( timeChanged.data.back() )->GetData().scrubbed );
        ASSERT_TRUE( 500 == static_cast< GUI::CTimelineForm::TTimeChangedEventData* >( timeChanged.data.back() )->GetData().timeInMs );
        ASSERT_TRUE( "0:00.500 / 0:02.000" == form.GetWidget( "Readout" )->GetProperty( "Text" ) );
        ASSERT_TRUE( 500.0f == form.GetWidget( "Time" )->GetFloatProperty( GUI::EWidgetProperty_Value ) && 2000.0f == form.GetWidget( "Time" )->GetFloatProperty( GUI::EWidgetProperty_Max ) );

        // a speed
        ASSERT_TRUE( form.SetSpeed( 2.0f ) && 2.0f == form.GetSpeed() );
        ASSERT_FALSE( form.SetSpeed( 0.0f ) );
        form.Update( 250 );
        ASSERT_TRUE( 1000 == form.GetTime() );
        ASSERT_TRUE( 3 == form.GetWidget( "Speed" )->GetIntProperty( GUI::EWidgetProperty_SelectedIndex ) );

        // the end: the time stays there and playing stops
        form.Update( 600 );
        ASSERT_TRUE( 2000 == form.GetTime() && !form.IsPlaying() );
        ASSERT_TRUE( 2 == playState.count && !static_cast< CORE::TCloneableBool* >( playState.data[ 1 ] )->GetData() );

        // from the end it plays from the start
        ASSERT_TRUE( form.Play() && 0 == form.GetTime() && form.IsPlaying() );

        // looping goes round, also by more than one length
        form.SetLooping( true );
        ASSERT_TRUE( form.IsLooping() && form.GetWidget( "Loop" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
        form.SetSpeed( 1.0f );
        form.Update( 2500 );
        ASSERT_TRUE( 500 == form.GetTime() && form.IsPlaying() );
        form.Pause();
        ASSERT_FALSE( form.IsPlaying() );
        form.Update( 100 );
        ASSERT_TRUE( 500 == form.GetTime() );

        // the time of the form is kept within the clip
        ASSERT_TRUE( form.SetTime( 99999 ) && 2000 == form.GetTime() );
        form.SelectClip( -1 );
        ASSERT_FALSE( form.SetTime( 10 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Timeline: A form in a context that renders frames plays on its own, with the frame time" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTimelineForm form( context );
        ASSERT_TRUE( form.Build() );
        form.SetClips( MakeClips() );
        form.SelectClip( 0 );
        ASSERT_TRUE( form.Play() );
        context->RenderFrame( 100 );
        context->RenderFrame( 150 );
        ASSERT_TRUE( 250 == form.GetTime() );

        // the form loaded again plays on without a second subscription: the time goes once
        CORE::CDataNode layout;
        ASSERT_TRUE( form.SaveLayout( layout ) && form.LoadLayout( layout ) );
        context->RenderFrame( 50 );
        ASSERT_TRUE( 300 == form.GetTime() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Timeline: The buttons, the slider, the speed list and the loop box are the user's way to the same functions" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTimelineForm form( context );
        ASSERT_TRUE( form.Build() );
        form.SetClips( MakeClips() );
        form.SelectClip( 0 );
        context->RenderFrame( 16 );
        CEventRecorder timeChanged( &form, GUI::CTimelineForm::TimeChangedEvent );
        CEventRecorder playState( &form, GUI::CTimelineForm::PlayStateChangedEvent );

        // play and pause
        ClickWidget( context, form.GetWidget( "Play" ) );
        ASSERT_TRUE( form.IsPlaying() && 1 == playState.count );
        context->RenderFrame( 500 );
        context->RenderFrame( 16 );
        ClickWidget( context, form.GetWidget( "Pause" ) );
        ASSERT_FALSE( form.IsPlaying() );
        const UInt32 stopped = form.GetTime();
        ASSERT_TRUE( stopped >= 500 && stopped < 600 );

        // the steps are a step of the form long
        form.SetStepInMs( 100 );
        ASSERT_TRUE( 100 == form.GetStepInMs() );
        ClickWidget( context, form.GetWidget( "StepForward" ) );
        ASSERT_TRUE( stopped + 100 == form.GetTime() );
        ClickWidget( context, form.GetWidget( "StepBack" ) );
        ClickWidget( context, form.GetWidget( "StepBack" ) );
        ASSERT_TRUE( stopped - 100 == form.GetTime() );
        form.SetTime( 50 );
        ASSERT_TRUE( form.StepBack() && 0 == form.GetTime() );

        // a click in the track of the slider is a scrub: the time of the place, and the data says the user did it
        timeChanged.Reset();
        GUI::CWidget* slider = form.GetWidget( "Time" );
        const GUI::CRect& rect = slider->GetRect();
        ClickAt( context, rect.left + rect.width / 2, rect.top + rect.height / 2 );
        ASSERT_TRUE( timeChanged.count >= 1 );
        GUI::CTimelineForm::TTimeChangedEventData* last = timeChanged.Last< GUI::CTimelineForm::TTimeChangedEventData >();
        ASSERT_TRUE( last->GetData().scrubbed );
        ASSERT_TRUE( form.GetTime() > 1500 && form.GetTime() < 2500 && form.GetTime() == last->GetData().timeInMs );

        // the speed: a pick of the user in the list
        form.GetWidget( "Speed" )->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "0" );
        ASSERT_TRUE( 0.25f == form.GetSpeed() );
        form.GetWidget( "Speed" )->ChangeValueByUser( GUI::EWidgetProperty_SelectedIndex, "4" );
        ASSERT_TRUE( 4.0f == form.GetSpeed() && 4.0f == GUI::CTimelineForm::GetSpeedOfChoice( 4 ) );

        // loop
        ASSERT_FALSE( form.IsLooping() );
        ClickWidget( context, form.GetWidget( "Loop" ) );
        ASSERT_TRUE( form.IsLooping() );
        ClickWidget( context, form.GetWidget( "Loop" ) );
        ASSERT_FALSE( form.IsLooping() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Timeline: Space plays and pauses while the list or the slider has the focus, and the list keeps its arrow keys" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTimelineForm form( context );
        ASSERT_TRUE( form.Build() );
        form.SetClips( MakeClips() );
        context->RenderFrame( 16 );

        context->SetFocus( form.GetWidget( "Clips" ) );
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 0 == form.GetSelectedClip() );
        PressKey( context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( 1 == form.GetSelectedClip() );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( form.IsPlaying() );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_FALSE( form.IsPlaying() );
        context->SetFocus( form.GetWidget( "Time" ) );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( form.IsPlaying() );
        form.Pause();

        // the loop check box keeps Space for itself
        context->SetFocus( form.GetWidget( "Loop" ) );
        PressKey( context, INPUT::KEYCODE_SPACE );
        ASSERT_FALSE( form.IsPlaying() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Timeline: A layout of the form saves and loads again: a new form of the layout works as the first one" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTimelineForm first( context );
        ASSERT_TRUE( first.Build() );
        CORE::CDataNode layout;
        ASSERT_TRUE( first.SaveLayout( layout ) );

        GUI::CTimelineForm second( context );
        ASSERT_TRUE( second.LoadLayout( layout ) );
        ASSERT_TRUE( second.Show() );
        ASSERT_TRUE( HasWidget( second, "Play" ) && HasWidget( second, "Time" ) && HasWidget( second, "Speed" ) );
        second.SetClips( MakeClips() );
        second.SelectClip( 1 );
        context->RenderFrame( 16 );
        CEventRecorder playState( &second, GUI::CTimelineForm::PlayStateChangedEvent );
        ClickWidget( context, second.GetWidget( "Play" ) );
        ASSERT_TRUE( second.IsPlaying() && 1 == playState.count );
        second.Update( 250 );
        ASSERT_TRUE( 250 == second.GetTime() );
        ASSERT_TRUE( "0:00.250 / 0:02.000" == second.GetWidget( "Readout" )->GetProperty( "Text" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "time line", &BuildTimelineScene );
    RunFormLintBaseline( "Timeline", &BuildTimelineScene );
}

/*-------------------------------------------------------------------------*/
