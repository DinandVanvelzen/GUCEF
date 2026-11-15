/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"
#include "gucefGUI_TestInputSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::GUITEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const Int32 KeyboardTestPhysicalId = 9311;

/** What the recorder says for a key event: "D" or "U", the key, "/", the modifier bits */
CORE::CString
Tok( const char* kind, INPUT::KeyCode keyCode, UInt32 modifiers )
{
    return CORE::CString( kind ) + INPUT::KeyCodeToString( keyCode ) + "/" + CORE::UInt32ToString( modifiers );
}

/** The centre of a widget in the coordinates of its context */
void
CentreOf( GUI::CGUIContext& context, GUI::CWidget* widget, Int32& x, Int32& y )
{
    context.FlushChanges();
    const GUI::CRect rect = widget->GetRect();
    x = rect.left + rect.width / 2;
    y = rect.top + rect.height / 2;
}

/** The pointer goes down on the button of a key */
void
PointerDown( GUI::CGUIContext& context, GUI::COnScreenKeyboardForm& form, INPUT::KeyCode keyCode )
{
    Int32 x = 0, y = 0;
    CentreOf( context, form.GetKeyWidget( keyCode ), x, y );
    context.InjectMouseMove( x, y );
    context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
}

void
PointerUp( GUI::CGUIContext& context )
{
    context.InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
}

void
Tap( GUI::CGUIContext& context, GUI::COnScreenKeyboardForm& form, INPUT::KeyCode keyCode )
{
    PointerDown( context, form, keyCode );
    PointerUp( context );
}

/** A context of the null driver with an edit box that has the focus, the keyboard form built and shown, an input adapter attached */
struct SKeyboardScene
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUI::CEditbox* edit;
    GUI::COnScreenKeyboardForm* form;
    GUI::CGUIInputAdapter* adapter;

    SKeyboardScene( void ) : contextPtr( CreateNullContext() ), context( GUCEF_NULL ), edit( GUCEF_NULL ), form( GUCEF_NULL ), adapter( GUCEF_NULL )
    {
        context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "App", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        edit = static_cast< GUI::CEditbox* >( context->CreateWidget( "Editbox", "Name", root ) );
        edit->SetProperty( "Position", "0,10 0,10" );
        edit->SetProperty( "Size", "0,300 0,24" );
        context->ShowRoot( root, false );
        context->SetFocus( edit );

        adapter = GUCEF_NEW GUI::CGUIInputAdapter( *context );
        adapter->Attach();
        form = GUCEF_NEW GUI::COnScreenKeyboardForm( context );
    }

    bool Start( void )
    {
        if ( !form->Build() )
            return false;
        form->Show();
        context->FlushChanges();
        return true;
    }

    ~SKeyboardScene()
    {
        GUCEF_DELETE form;
        GUCEF_DELETE adapter;
    }

    private:

    SKeyboardScene( const SKeyboardScene& src );
    SKeyboardScene& operator=( const SKeyboardScene& src );
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiOnScreenKeyboardTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.OnScreenKeyboard" );

    GUCEF_TESTFW_TESTCASE( "Layout: The US layout is built in, has every row 15 key widths wide and survives a round trip through a data tree" )
    try
    {
        GUI::CKeyboardLayoutRegistry& registry = GUI::CGuiGlobal::Instance()->GetKeyboardLayoutRegistry();
        ASSERT_TRUE( registry.IsRegistered( "US" ) );
        GUI::CKeyboardLayout us;
        ASSERT_TRUE( registry.Get( "US", us ) && "US" == us.GetName() && us.IsValid() );
        ASSERT_TRUE( 6 == us.GetRowCount() );
        for ( UInt32 r=0; r<us.GetRowCount(); ++r )
            ASSERT_TRUE( 15.0f == us.GetRowWidth( r ) );
        ASSERT_TRUE( 15.0f == us.GetWidth() );

        UInt32 row = 0, index = 0;
        ASSERT_TRUE( us.FindKey( INPUT::KEYCODE_A, row, index ) && 3 == row );
        const GUI::CKeyboardLayout::TKey& a = us.GetRow( row )[ index ];
        ASSERT_TRUE( "a" == a.label && "A" == a.shiftLabel && "a" == a.text && "A" == a.shiftText && a.capsAffected );
        ASSERT_TRUE( us.FindKey( INPUT::KEYCODE_1, row, index ) && "!" == us.GetRow( row )[ index ].shiftText && !us.GetRow( row )[ index ].capsAffected );
        ASSERT_TRUE( us.FindKey( INPUT::KEYCODE_RETURN, row, index ) && us.GetRow( row )[ index ].text.IsNULLOrEmpty() );
        ASSERT_FALSE( us.FindKey( INPUT::KEYCODE_KANA, row, index ) );

        CORE::CDataNode tree;
        us.Save( tree );
        GUI::CKeyboardLayout copy;
        ASSERT_TRUE( copy.Load( tree ) && "US" == copy.GetName() && copy.GetKeyCount() == us.GetKeyCount() && copy.GetRowCount() == us.GetRowCount() );
        for ( UInt32 r=0; r<us.GetRowCount(); ++r )
        {
            ASSERT_TRUE( us.GetRow( r ).size() == copy.GetRow( r ).size() );
            for ( UInt32 k=0; k<(UInt32) us.GetRow( r ).size(); ++k )
            {
                const GUI::CKeyboardLayout::TKey& x = us.GetRow( r )[ k ];
                const GUI::CKeyboardLayout::TKey& y = copy.GetRow( r )[ k ];
                ASSERT_TRUE( x.keyCode == y.keyCode && x.label == y.label && x.shiftLabel == y.shiftLabel && x.text == y.text && x.shiftText == y.shiftText && x.width == y.width && x.capsAffected == y.capsAffected );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layout: A tree with an unknown key code, or without keys, is no layout and leaves the layout it is loaded into as it was" )
    try
    {
        GUI::CKeyboardLayout layout = GUI::CKeyboardLayout::MakeUS();
        layout.SetName( "Mine" );
        CORE::CDataNode tree;
        GUI::CKeyboardLayout::MakeUS().Save( tree );
        CORE::CDataNode* key = tree.FindChild( "Row" )->FindChild( "Key" );
        key->SetAttribute( "code", "NOSUCHKEY" );
        ASSERT_FALSE( layout.Load( tree ) );
        ASSERT_TRUE( "Mine" == layout.GetName() && 6 == layout.GetRowCount() );

        CORE::CDataNode empty;
        empty.SetName( "KeyboardLayout" );
        ASSERT_FALSE( layout.Load( empty ) );
        CORE::CDataNode other;
        other.SetName( "Something" );
        ASSERT_FALSE( layout.Load( other ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layout: Layouts can be registered by name, replaced and removed, and a layout that is not valid is refused" )
    try
    {
        GUI::CKeyboardLayoutRegistry& registry = GUI::CGuiGlobal::Instance()->GetKeyboardLayoutRegistry();

        GUI::CKeyboardLayout pad( "TestPad" );
        UInt32 row = pad.AddRow();
        pad.AddKey( row, INPUT::KEYCODE_1, "1", "1", "1", "1" );
        pad.AddKey( row, INPUT::KEYCODE_2, "2", "2", "2", "2" );
        pad.AddGap( row, 0.5f );
        pad.AddKey( row, INPUT::KEYCODE_BACK, "Bksp", 1.5f );
        ASSERT_TRUE( pad.IsValid() && 4.0f == pad.GetWidth() && 3 == pad.GetKeyCount() );
        ASSERT_TRUE( registry.Register( pad ) && registry.IsRegistered( "TestPad" ) );
        GUI::CKeyboardLayoutRegistry::TStringVector names = registry.GetNames();
        bool hasUs = false, hasPad = false;
        for ( UInt32 i=0; i<(UInt32) names.size(); ++i )
        {
            hasUs = hasUs || "US" == names[ i ];
            hasPad = hasPad || "TestPad" == names[ i ];
        }
        ASSERT_TRUE( hasUs && hasPad );

        GUI::CKeyboardLayout fetched;
        ASSERT_TRUE( registry.Get( "TestPad", fetched ) && 3 == fetched.GetKeyCount() );
        ASSERT_FALSE( registry.Get( "NoSuchLayout", fetched ) );

        ASSERT_FALSE( registry.Register( GUI::CKeyboardLayout( "Empty" ) ) );
        GUI::CKeyboardLayout unnamed = pad;
        unnamed.SetName( CORE::CString::Empty );
        ASSERT_FALSE( registry.Register( unnamed ) );
        GUI::CKeyboardLayout badWidth( "BadWidth" );
        badWidth.AddRow();
        badWidth.AddKey( 0, INPUT::KEYCODE_1, "1", 0.0f );
        ASSERT_FALSE( registry.Register( badWidth ) );

        ASSERT_TRUE( registry.Unregister( "TestPad" ) && !registry.IsRegistered( "TestPad" ) && !registry.Unregister( "TestPad" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: Typing on the keyboard sends the same events to the observers of the keyboards as a physical keyboard, and the focused edit box gets the text" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( GUCEF_NULL != scene.context && scene.Start() );

        CScriptedDriver physical;
        physical.Keyboard( KeyboardTestPhysicalId );
        CKeyboardRecorder fromScreen;
        CKeyboardRecorder fromHardware;
        fromScreen.Watch( scene.form->GetKeyboardDeviceId() );
        fromHardware.Watch( KeyboardTestPhysicalId );

        Tap( *scene.context, *scene.form, INPUT::KEYCODE_H );
        Tap( *scene.context, *scene.form, INPUT::KEYCODE_I );
        ASSERT_TRUE( "hi" == scene.edit->GetText() );
        ASSERT_TRUE( scene.context->GetFocus() == scene.edit );

        physical.Key( -1, KeyboardTestPhysicalId, INPUT::KEYCODE_H, true );
        physical.Text( -1, KeyboardTestPhysicalId, "h" );
        physical.Key( -1, KeyboardTestPhysicalId, INPUT::KEYCODE_H, false );
        physical.Key( -1, KeyboardTestPhysicalId, INPUT::KEYCODE_I, true );
        physical.Text( -1, KeyboardTestPhysicalId, "i" );
        physical.Key( -1, KeyboardTestPhysicalId, INPUT::KEYCODE_I, false );
        ASSERT_TRUE( fromHardware.Joined() == fromScreen.Joined() );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_H, 0 ) + " Th " + Tok( "U", INPUT::KEYCODE_H, 0 ) + " " + Tok( "D", INPUT::KEYCODE_I, 0 ) + " Ti " + Tok( "U", INPUT::KEYCODE_I, 0 ) == fromScreen.Joined() );

        // the keys of the physical keyboard typed "hi" again; keys that type no text: the key goes down and up only, and the focused edit box handles Backspace as for a physical key
        fromScreen.events.clear();
        Tap( *scene.context, *scene.form, INPUT::KEYCODE_BACK );
        ASSERT_TRUE( "hih" == scene.edit->GetText() );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_BACK, 0 ) + " " + Tok( "U", INPUT::KEYCODE_BACK, 0 ) == fromScreen.Joined() );

        physical.NoKeyboard( KeyboardTestPhysicalId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: The keyboard is a device of the virtual input driver in gucefINPUT, made by Build() and removed with the form" )
    try
    {
        INPUT::CInputController& controller = INPUT::CInputGlobal::Instance()->GetInputController();
        const UInt32 before = controller.GetKeyboardCount();
        {
            SKeyboardScene scene;
            ASSERT_TRUE( scene.form->GetKeyboardDeviceId() < 0 );
            ASSERT_TRUE( scene.Start() );
            const Int32 id = scene.form->GetKeyboardDeviceId();
            ASSERT_TRUE( id >= INPUT::CVirtualInputDriver::FirstVirtualDeviceId );
            ASSERT_TRUE( before + 1 == controller.GetKeyboardCount() );
            INPUT::CInputDriver* driver = controller.GetKeyboard( id ).GetInputDriver();
            ASSERT_TRUE( GUCEF_NULL != driver && "Virtual" == driver->GetName() );
        }
        ASSERT_TRUE( before == controller.GetKeyboardCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: A tap on Shift latches it for the next key: shift goes down, the key types the shifted text, then both go up; the second tap within the double tap time locks it" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( scene.Start() );
        CKeyboardRecorder rec;
        rec.Watch( scene.form->GetKeyboardDeviceId() );
        GUI::COnScreenKeyboardForm& form = *scene.form;

        Tap( *scene.context, form, INPUT::KEYCODE_LSHIFT );
        ASSERT_TRUE( GUI::COnScreenKeyboardForm::MODIFIER_LATCHED == form.GetModifierState( INPUT::KEYCODE_LSHIFT ) && form.IsShiftActive() );
        ASSERT_TRUE( "A" == form.GetKeyWidget( INPUT::KEYCODE_A )->GetProperty( "Text" ) && "!" == form.GetKeyWidget( INPUT::KEYCODE_1 )->GetProperty( "Text" ) );
        Tap( *scene.context, form, INPUT::KEYCODE_A );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_LSHIFT, INPUT::KEYMOD_SHIFT ) + " " + Tok( "D", INPUT::KEYCODE_A, INPUT::KEYMOD_SHIFT ) + " TA " +
                     Tok( "U", INPUT::KEYCODE_A, INPUT::KEYMOD_SHIFT ) + " " + Tok( "U", INPUT::KEYCODE_LSHIFT, 0 ) == rec.Joined() );
        ASSERT_TRUE( "A" == scene.edit->GetText() && !form.IsShiftActive() );
        ASSERT_TRUE( "a" == form.GetKeyWidget( INPUT::KEYCODE_A )->GetProperty( "Text" ) );

        // the next key is not shifted
        rec.events.clear();
        Tap( *scene.context, form, INPUT::KEYCODE_A );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_A, 0 ) + " Ta " + Tok( "U", INPUT::KEYCODE_A, 0 ) == rec.Joined() );

        // two taps lock it: it stays down for every key until it is tapped again
        rec.events.clear();
        Tap( *scene.context, form, INPUT::KEYCODE_RSHIFT );
        Tap( *scene.context, form, INPUT::KEYCODE_RSHIFT );
        ASSERT_TRUE( GUI::COnScreenKeyboardForm::MODIFIER_LOCKED == form.GetModifierState( INPUT::KEYCODE_RSHIFT ) );
        Tap( *scene.context, form, INPUT::KEYCODE_B );
        Tap( *scene.context, form, INPUT::KEYCODE_C );
        ASSERT_TRUE( 1 == rec.Count( "TB" ) && 1 == rec.Count( "TC" ) && 0 == rec.Count( Tok( "U", INPUT::KEYCODE_RSHIFT, 0 ) ) );
        Tap( *scene.context, form, INPUT::KEYCODE_RSHIFT );
        ASSERT_TRUE( GUI::COnScreenKeyboardForm::MODIFIER_OFF == form.GetModifierState( INPUT::KEYCODE_RSHIFT ) && 1 == rec.Count( Tok( "U", INPUT::KEYCODE_RSHIFT, 0 ) ) );
        Tap( *scene.context, form, INPUT::KEYCODE_D );
        ASSERT_TRUE( "AaBCd" == scene.edit->GetText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: A second tap on a latched modifier after the double tap time lets it go, by a manual clock" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( scene.Start() );
        CKeyboardRecorder rec;
        rec.Watch( scene.form->GetKeyboardDeviceId() );
        GUI::COnScreenKeyboardForm& form = *scene.form;
        form.SetDoubleTapTimeInMs( 300 );

        Tap( *scene.context, form, INPUT::KEYCODE_LSHIFT );
        form.Update( 301 );
        Tap( *scene.context, form, INPUT::KEYCODE_LSHIFT );
        ASSERT_TRUE( GUI::COnScreenKeyboardForm::MODIFIER_OFF == form.GetModifierState( INPUT::KEYCODE_LSHIFT ) );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_LSHIFT, INPUT::KEYMOD_SHIFT ) + " " + Tok( "U", INPUT::KEYCODE_LSHIFT, 0 ) == rec.Joined() );

        // inside the time it locks
        rec.events.clear();
        Tap( *scene.context, form, INPUT::KEYCODE_LSHIFT );
        form.Update( 299 );
        Tap( *scene.context, form, INPUT::KEYCODE_LSHIFT );
        ASSERT_TRUE( GUI::COnScreenKeyboardForm::MODIFIER_LOCKED == form.GetModifierState( INPUT::KEYCODE_LSHIFT ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: Caps lock is a key that is sent and that switches the case of the letters, not of the digits; with shift the letters are lower case again" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( scene.Start() );
        CKeyboardRecorder rec;
        rec.Watch( scene.form->GetKeyboardDeviceId() );
        GUI::COnScreenKeyboardForm& form = *scene.form;

        Tap( *scene.context, form, INPUT::KEYCODE_CAPITAL );
        ASSERT_TRUE( form.IsCapsLockActive() && !form.IsShiftActive() );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_CAPITAL, 0 ) + " " + Tok( "U", INPUT::KEYCODE_CAPITAL, 0 ) == rec.Joined() );
        ASSERT_TRUE( "A" == form.GetKeyWidget( INPUT::KEYCODE_A )->GetProperty( "Text" ) && "1" == form.GetKeyWidget( INPUT::KEYCODE_1 )->GetProperty( "Text" ) );
        Tap( *scene.context, form, INPUT::KEYCODE_A );
        Tap( *scene.context, form, INPUT::KEYCODE_1 );
        ASSERT_TRUE( "A1" == scene.edit->GetText() );

        Tap( *scene.context, form, INPUT::KEYCODE_LSHIFT );
        Tap( *scene.context, form, INPUT::KEYCODE_B );
        Tap( *scene.context, form, INPUT::KEYCODE_2 );
        ASSERT_TRUE( "A1b2" == scene.edit->GetText() );

        Tap( *scene.context, form, INPUT::KEYCODE_CAPITAL );
        ASSERT_FALSE( form.IsCapsLockActive() );
        Tap( *scene.context, form, INPUT::KEYCODE_C );
        ASSERT_TRUE( "A1b2c" == scene.edit->GetText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: With control down a key is a shortcut: the key events carry the modifier and no text is typed" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( scene.Start() );
        CKeyboardRecorder rec;
        rec.Watch( scene.form->GetKeyboardDeviceId() );
        GUI::COnScreenKeyboardForm& form = *scene.form;

        Tap( *scene.context, form, INPUT::KEYCODE_LCONTROL );
        Tap( *scene.context, form, INPUT::KEYCODE_C );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_LCONTROL, INPUT::KEYMOD_CTRL ) + " " + Tok( "D", INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL ) + " " +
                     Tok( "U", INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL ) + " " + Tok( "U", INPUT::KEYCODE_LCONTROL, 0 ) == rec.Joined() );
        ASSERT_TRUE( scene.edit->GetText().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: A key held repeats after the delay and then at the interval, as a physical key does, and goes up when released; the manual clock is Update()" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( scene.Start() );
        CKeyboardRecorder rec;
        rec.Watch( scene.form->GetKeyboardDeviceId() );
        GUI::COnScreenKeyboardForm& form = *scene.form;
        ASSERT_TRUE( 500 == form.GetRepeatDelayInMs() && 33 == form.GetRepeatIntervalInMs() );

        PointerDown( *scene.context, form, INPUT::KEYCODE_A );
        ASSERT_TRUE( INPUT::KEYCODE_A == form.GetHeldKey() );
        ASSERT_TRUE( 1 == rec.Count( Tok( "D", INPUT::KEYCODE_A, 0 ) ) && 1 == rec.Count( "Ta" ) );
        form.Update( 499 );
        ASSERT_TRUE( 1 == rec.Count( "Ta" ) );
        form.Update( 1 );
        ASSERT_TRUE( 2 == rec.Count( "Ta" ) );
        form.Update( 32 );
        ASSERT_TRUE( 2 == rec.Count( "Ta" ) );
        form.Update( 1 );
        ASSERT_TRUE( 3 == rec.Count( "Ta" ) );
        form.Update( 66 );
        ASSERT_TRUE( 5 == rec.Count( "Ta" ) && 5 == rec.Count( Tok( "D", INPUT::KEYCODE_A, 0 ) ) );
        ASSERT_TRUE( 0 == rec.Count( Tok( "U", INPUT::KEYCODE_A, 0 ) ) );

        // the frames of the context drive it as well
        scene.context->RenderFrame( 100 );
        ASSERT_TRUE( 8 == rec.Count( "Ta" ) );

        PointerUp( *scene.context );
        ASSERT_TRUE( INPUT::KEYCODE_UNASSIGNED == form.GetHeldKey() && 1 == rec.Count( Tok( "U", INPUT::KEYCODE_A, 0 ) ) );
        ASSERT_TRUE( 8 == scene.edit->GetText().Length() );
        form.Update( 2000 );
        ASSERT_TRUE( 8 == rec.Count( "Ta" ) );

        // modifiers and caps lock do not repeat; a shorter delay and interval are taken
        rec.events.clear();
        form.SetRepeatDelayInMs( 100 );
        form.SetRepeatIntervalInMs( 10 );
        PointerDown( *scene.context, form, INPUT::KEYCODE_CAPITAL );
        form.Update( 1000 );
        PointerUp( *scene.context );
        ASSERT_TRUE( 1 == rec.Count( Tok( "D", INPUT::KEYCODE_CAPITAL, 0 ) ) );
        PointerDown( *scene.context, form, INPUT::KEYCODE_B );
        form.Update( 100 );
        form.Update( 20 );
        PointerUp( *scene.context );
        ASSERT_TRUE( 4 == rec.Count( "TB" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: The pointer leaving the key, or the press being cancelled, still ends the key; hiding the keyboard lets go of latched modifiers" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( scene.Start() );
        CKeyboardRecorder rec;
        rec.Watch( scene.form->GetKeyboardDeviceId() );
        GUI::COnScreenKeyboardForm& form = *scene.form;

        // released away from the key it went down on
        PointerDown( *scene.context, form, INPUT::KEYCODE_A );
        scene.context->InjectMouseMove( 600, 5 );
        PointerUp( *scene.context );
        ASSERT_TRUE( 1 == rec.Count( Tok( "U", INPUT::KEYCODE_A, 0 ) ) && INPUT::KEYCODE_UNASSIGNED == form.GetHeldKey() );

        // the press is cancelled (the window lost the focus)
        rec.events.clear();
        PointerDown( *scene.context, form, INPUT::KEYCODE_S );
        scene.context->CancelPointerCapture( "test" );
        ASSERT_TRUE( 1 == rec.Count( Tok( "U", INPUT::KEYCODE_S, 0 ) ) && INPUT::KEYCODE_UNASSIGNED == form.GetHeldKey() );

        // hiding
        rec.events.clear();
        Tap( *scene.context, form, INPUT::KEYCODE_LSHIFT );
        Tap( *scene.context, form, INPUT::KEYCODE_LMENU );
        form.Hide();
        ASSERT_TRUE( GUI::COnScreenKeyboardForm::MODIFIER_OFF == form.GetModifierState( INPUT::KEYCODE_LSHIFT ) && GUI::COnScreenKeyboardForm::MODIFIER_OFF == form.GetModifierState( INPUT::KEYCODE_LMENU ) );
        ASSERT_TRUE( Tok( "D", INPUT::KEYCODE_LSHIFT, INPUT::KEYMOD_SHIFT ) + " " + Tok( "D", INPUT::KEYCODE_LMENU, INPUT::KEYMOD_SHIFT | INPUT::KEYMOD_ALT ) + " " +
                     Tok( "U", INPUT::KEYCODE_LSHIFT, INPUT::KEYMOD_ALT ) + " " + Tok( "U", INPUT::KEYCODE_LMENU, 0 ) == rec.Joined() );
        INPUT::CKeyboard& keyboard = INPUT::CInputGlobal::Instance()->GetInputController().GetKeyboard( form.GetKeyboardDeviceId() );
        ASSERT_TRUE( 0 == keyboard.GetKeyModStates() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: A form that goes while a modifier is latched lets it go first" )
    try
    {
        SKeyboardScene scene;
        ASSERT_TRUE( scene.Start() );
        CKeyboardRecorder rec;
        const Int32 id = scene.form->GetKeyboardDeviceId();
        rec.Watch( id );
        Tap( *scene.context, *scene.form, INPUT::KEYCODE_LCONTROL );
        INPUT::CKeyboard& keyboard = INPUT::CInputGlobal::Instance()->GetInputController().GetKeyboard( id );
        ASSERT_TRUE( keyboard.IsKeyModifierActive( INPUT::KEYMOD_CTRL ) );
        GUCEF_DELETE scene.form;
        scene.form = GUCEF_NULL;
        ASSERT_TRUE( 1 == rec.Count( Tok( "U", INPUT::KEYCODE_LCONTROL, 0 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layout: A registered layout is chosen by name, rebuilds the keys, and unknown names are refused" )
    try
    {
        GUI::CKeyboardLayoutRegistry& registry = GUI::CGuiGlobal::Instance()->GetKeyboardLayoutRegistry();
        GUI::CKeyboardLayout pad( "TestPad2" );
        UInt32 row = pad.AddRow();
        pad.AddKey( row, INPUT::KEYCODE_5, "5", "%", "5", "%" );
        pad.AddKey( row, INPUT::KEYCODE_6, "six", "SIX", "6", "^" );
        ASSERT_TRUE( registry.Register( pad ) );

        SKeyboardScene scene;
        GUI::COnScreenKeyboardForm& form = *scene.form;
        ASSERT_FALSE( form.SetLayout( "NoSuchLayout" ) );
        ASSERT_TRUE( "US" == form.GetLayout().GetName() );
        ASSERT_TRUE( scene.Start() );
        ASSERT_TRUE( GUCEF_NULL != form.GetKeyWidget( INPUT::KEYCODE_Q ) );

        ASSERT_TRUE( form.SetLayout( "TestPad2" ) && "TestPad2" == form.GetLayout().GetName() );
        ASSERT_TRUE( form.IsVisible() );
        ASSERT_TRUE( GUCEF_NULL == form.GetKeyWidget( INPUT::KEYCODE_Q ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetKeyWidget( INPUT::KEYCODE_6 ) && "six" == form.GetKeyWidget( INPUT::KEYCODE_6 )->GetProperty( "Text" ) );
        scene.context->FlushChanges();
        Tap( *scene.context, form, INPUT::KEYCODE_6 );
        ASSERT_TRUE( "6" == scene.edit->GetText() );

        // a layout that is not registered can be given as it is
        GUI::CKeyboardLayout other( "Unregistered" );
        row = other.AddRow();
        other.AddKey( row, INPUT::KEYCODE_Z, "z", "Z", "z", "Z", 1.0f, true );
        ASSERT_TRUE( form.SetLayout( other ) && !registry.IsRegistered( "Unregistered" ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetKeyWidget( INPUT::KEYCODE_Z ) && GUCEF_NULL == form.GetKeyWidget( INPUT::KEYCODE_6 ) );
        ASSERT_FALSE( form.SetLayout( GUI::CKeyboardLayout( "Invalid" ) ) );

        registry.Unregister( "TestPad2" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Keys: Only the input context the keyboard was told to send for gets its keys" )
    try
    {
        SKeyboardScene scene;
        scene.adapter->SetInputContextId( 7 );
        ASSERT_TRUE( scene.Start() );

        scene.form->SetInputContextId( 8 );
        ASSERT_TRUE( 8 == scene.form->GetInputContextId() );
        Tap( *scene.context, *scene.form, INPUT::KEYCODE_A );
        ASSERT_TRUE( scene.edit->GetText().IsNULLOrEmpty() );

        scene.form->SetInputContextId( 7 );
        Tap( *scene.context, *scene.form, INPUT::KEYCODE_A );
        ASSERT_TRUE( "a" == scene.edit->GetText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layers: A keyboard in a layer above the application gets its keys to the application when the layer passes the keyboard on, and not otherwise" )
    try
    {
        CORE::CTSharedPtr< CRecordingWindow, MT::CMutex > windowImpl( GUCEF_NEW CRecordingWindow() );
        GUI::TWindowContextPtr window( windowImpl );
        CLayerTestContext app( window );
        CLayerTestContext overlay( window );
        overlay.SetLayer( 1 );

        GUI::CWidget* appRoot = app.CreateWidget( "Panel", "App", GUCEF_NULL );
        appRoot->SetProperty( "Size", "1,0 1,0" );
        GUI::CEditbox* edit = static_cast< GUI::CEditbox* >( app.CreateWidget( "Editbox", "Name", appRoot ) );
        edit->SetProperty( "Size", "0,300 0,24" );
        app.ShowRoot( appRoot, false );
        app.SetFocus( edit );

        GUI::CGUIInputAdapter appAdapter( app );
        GUI::CGUIInputAdapter overlayAdapter( overlay );
        appAdapter.Attach();
        overlayAdapter.Attach();

        GUI::COnScreenKeyboardForm form( &overlay );
        ASSERT_TRUE( form.Build() );
        form.Show();
        overlay.FlushChanges();

        Tap( overlay, form, INPUT::KEYCODE_A );
        ASSERT_TRUE( edit->GetText().IsNULLOrEmpty() );

        overlay.SetKeyboardInputPassThrough( true );
        Tap( overlay, form, INPUT::KEYCODE_B );
        ASSERT_TRUE( "b" == edit->GetText() );
        ASSERT_TRUE( app.GetFocus() == edit );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
