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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CSECUREBUFFER_H
#include "gucefCORE_CSecureBuffer.h"
#define GUCEF_CORE_CSECUREBUFFER_H
#endif /* GUCEF_CORE_CSECUREBUFFER_H ? */

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_INPUT_H
#include "gucefINPUT.h"
#define GUCEF_INPUT_H
#endif /* GUCEF_INPUT_H ? */

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace GUCEF {
namespace GUI {

/** The way tests look into a CSecretInput: the storage of the value and the default action, both private */
class CSecretInputTestAccess
{
    public:

    static const CORE::CSecureBuffer& Storage( const CSecretInput& input ) { return input.m_value; }

    static bool DefaultAction( CSecretInput& input, const CORE::CEvent& eventId, CRoutedEventData& eventData ) { return input.OnDefaultAction( eventId, eventData ); }
};

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const Secret = "S3cr3t-Value-42";
static const char* const SecretPart = "S3cr3t";

bool
IsAllZero( const void* memory, UInt32 size )
{
    const UInt8* bytes = static_cast< const UInt8* >( memory );
    for ( UInt32 i=0; i<size; ++i )
        if ( 0 != bytes[ i ] )
            return false;
    return true;
}

/** What the clipboard hooks saw */
struct SClipboard
{
    CORE::CString text;
    UInt32 reads;
    UInt32 writes;
    bool readResult;
};

SClipboard g_clipboard;

bool
FakeClipboardRead( char* dest, UInt32 size, UInt32* written )
{
    ++g_clipboard.reads;
    if ( !g_clipboard.readResult )
        return false;
    UInt32 length = (UInt32) g_clipboard.text.ByteSize() - 1;
    if ( length > size - 1 )
        length = size - 1;
    memcpy( dest, g_clipboard.text.C_String(), length );
    dest[ length ] = '\0';
    *written = length;
    return true;
}

bool
FakeClipboardWrite( const char* text )
{
    ++g_clipboard.writes;
    g_clipboard.text = text;
    return true;
}

void
ResetClipboard( void )
{
    g_clipboard.text.Clear();
    g_clipboard.reads = 0;
    g_clipboard.writes = 0;
    g_clipboard.readResult = true;
}

/** What the release auditor saw of the last storage that was given back */
struct SRelease
{
    const void* storage;
    UInt32 size;
    bool wasZero;
    UInt32 count;
};

SRelease g_release = { GUCEF_NULL, 0, false, 0 };

void
Audit( const void* storage, UInt32 size )
{
    g_release.storage = storage;
    g_release.size = size;
    g_release.wasZero = IsAllZero( storage, size );
    ++g_release.count;
}

/** Counts events of one kind; remembers the old and new text of a ValueChangedEvent and the state of a CapsLockStateChangedEvent */
class CRecorder : public CORE::CObserver
{
    public:

    CORE::CEvent eventId;
    UInt32 count;
    CORE::CString lastOld;
    CORE::CString lastNew;
    bool lastBool;

    CRecorder( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), eventId( id ), count( 0 ), lastOld(), lastNew(), lastBool( false )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CRecorder() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id != eventId )
            return;
        GUI::CRoutedEventData* routed = GUI::CRoutedEventData::FromCloneable( eventData );
        if ( GUCEF_NULL != routed && routed->source != notifier )
            return;     // only the target phase
        ++count;
        GUI::CValueChangedEventData* changed = CORE::CICloneable::CloneableAs< GUI::CValueChangedEventData >( eventData );
        if ( GUCEF_NULL != changed )
        {
            lastOld = changed->oldValue;
            lastNew = changed->newValue;
        }
        if ( GUI::CSecretInput::CapsLockStateChangedEvent == id && GUCEF_NULL != eventData )
            lastBool = static_cast< CORE::TCloneableBool* >( eventData )->GetData();
    }
};

/** A context with a screen and one secret input on it, focused */
struct SFixture
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUI::CWidget* screen;
    GUI::CSecretInput* input;

    explicit SFixture( const GUI::CSecretInputSettings* settings = GUCEF_NULL )
        : contextPtr( CreateNullContext() ), context( GUCEF_NULL ), screen( GUCEF_NULL ), input( GUCEF_NULL )
    {
        context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        screen->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( screen, false );
        input = static_cast< GUI::CSecretInput* >( context->CreateWidget( "SecretInput", "Secret", screen ) );
        input->SetProperty( "Size", "0,300 0,24" );
        input->SetProperty( "Font", "Test" );
        if ( GUCEF_NULL != settings )
            input->SetSettings( *settings );
        context->FlushChanges();
        context->SetFocus( input );
    }

    ~SFixture()
    {
        if ( GUCEF_NULL != context && GUCEF_NULL != screen )
            context->DestroyWidget( screen );
    }

    void Type( const char* text ) { context->InjectText( text ); }

    void Key( INPUT::KeyCode key, UInt32 modifiers = 0 )
    {
        context->InjectKey( key, true, modifiers );
        context->InjectKey( key, false, modifiers );
    }

    bool ValueIs( const char* expected )
    {
        CORE::CSecureBuffer value = input->GetValue();
        return value.Equals( expected, (UInt32) strlen( expected ) );
    }

    /** The text of every text command the painter makes for the input */
    CORE::CString Painted( void )
    {
        GUI::CDrawSurface::TCommandVector commands;
        GUI::CWidgetPainter::Paint( *input, *context, commands );
        CORE::CString all;
        for ( size_t i=0; i<commands.size(); ++i )
            if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type )
            {
                all += commands[ i ].text;
                all += "|";
            }
        return all;
    }

    /** Every property of the input as text, every way a property can be read or written out */
    CORE::CString Dump( void )
    {
        CORE::CString all;
        CORE::CValueList list;
        input->GetProperties( list );
        all += CORE::ToString( list );
        CORE::CDynamicBuffer buffer;
        CORE::CDynamicBufferAccess access( buffer );
        input->SaveProperties( access );
        if ( buffer.GetDataSize() > 0 )
        {
            CORE::CString saved;
            saved.Set( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
            all += saved;
        }
        GUI::CWidget::TPropertyMap explicitProperties = input->GetExplicitProperties();
        for ( GUI::CWidget::TPropertyMap::iterator i=explicitProperties.begin(); i!=explicitProperties.end(); ++i )
        {
            all += (*i).first;
            all += "=";
            all += (*i).second;
            all += "\n";
        }
        all += input->GetText();
        all += input->GetProperty( "Text" );
        all += input->GetProperty( "Tooltip" );
        CORE::CString propertyValue;
        input->GetPropertyValue( "Text", propertyValue );
        all += propertyValue;
        return all;
    }
};

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiSecretInputTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.SecretInput" );

    GUCEF_TESTFW_TESTCASE( "SecretInput: it is an Editbox type of its own, and the default settings are the most secure ones" )
    try
    {
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        ASSERT_TRUE( registry.IsRegistered( "SecretInput" ) && registry.IsA( "SecretInput", "Editbox" ) && registry.IsA( "SecretInput", "Widget" ) );

        SFixture fixture;
        ASSERT_TRUE( "SecretInput" == fixture.input->GetType() && fixture.input->IsRegisteredAs( "Editbox" ) );
        ASSERT_TRUE( GUCEF_NULL != GUI::CWidget::As< GUI::CSecretInput >( fixture.input ) && GUCEF_NULL != GUI::CWidget::As< GUI::CEditbox >( fixture.input ) );

        const GUI::CSecretInputSettings& s = fixture.input->GetSettings();
        ASSERT_TRUE( 256 == s.maxLength && 0x2022 == s.maskCodePoint && s.hideLength && 12 == s.fixedMaskLength );
        ASSERT_FALSE( s.allowCopy );
        ASSERT_TRUE( s.allowPaste );
        ASSERT_FALSE( s.allowReveal );
        ASSERT_TRUE( 5000 == s.revealTimeoutInMs );
        ASSERT_FALSE( s.clearOnFocusLoss );
        ASSERT_TRUE( s.clearOnHide );
        ASSERT_FALSE( s.notifyCapsLock );
        ASSERT_TRUE( 0 == s.lastCharRevealInMs );

        // the values of a settings object are brought inside what the input can work with
        GUI::CSecretInputSettings wild;
        wild.maxLength = 1000000;
        wild.maskCodePoint = 7;
        wild.fixedMaskLength = 0;
        wild.revealTimeoutInMs = 0;
        wild.Sanitize();
        ASSERT_TRUE( 4096 == wild.maxLength && 0x2022 == wild.maskCodePoint && 12 == wild.fixedMaskLength && 5000 == wild.revealTimeoutInMs );
        wild.maxLength = 0;
        wild.Sanitize();
        ASSERT_TRUE( 256 == wild.maxLength );

        // the settings are in the config tree and come back from it; what the tree lacks stays as strict as it was
        GUI::CSecretInputSettings custom;
        custom.maxLength = 64;
        custom.maskCodePoint = '*';
        custom.hideLength = false;
        custom.allowCopy = true;
        custom.allowReveal = true;
        custom.clearOnHide = false;
        custom.lastCharRevealInMs = 700;
        CORE::CDataNode tree;
        ASSERT_TRUE( custom.SaveConfig( tree ) );
        GUI::CSecretInputSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) && loaded == custom );
        CORE::CDataNode partial;
        partial.Structure( "SecretInput", '/' )->SetAttribute( "allowReveal", true );
        GUI::CSecretInputSettings fromPartial;
        ASSERT_TRUE( fromPartial.LoadConfig( partial ) && fromPartial.allowReveal && !fromPartial.allowCopy && fromPartial.hideLength && fromPartial.clearOnHide );
        ASSERT_FALSE( fromPartial.LoadConfig( CORE::CDataNode() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: typed text is drawn as mask characters only, as a fixed number of them by default, and the painter never gets the value" )
    try
    {
        SFixture fixture;
        CRecorder secretChanged( fixture.input, GUI::CSecretInput::SecretChangedEvent );
        CRecorder valueChanged( fixture.input, GUI::CWidget::ValueChangedEvent );
        ASSERT_TRUE( fixture.input->GetText().IsNULLOrEmpty() && fixture.Painted().IsNULLOrEmpty() );

        fixture.Type( "S3" );
        ASSERT_TRUE( fixture.ValueIs( "S3" ) && 2 == fixture.input->GetValueLength() && fixture.input->HasValue() );
        // the length is hidden: 12 bullets for 2 characters, and for 14
        CORE::CString twoChars = fixture.input->GetText();
        fixture.Type( "cr3t-Value-42" );
        ASSERT_TRUE( fixture.ValueIs( Secret ) && 15 == fixture.input->GetValueLength() );
        ASSERT_TRUE( twoChars == fixture.input->GetText() && 12 == fixture.input->GetText().Length() );
        ASSERT_TRUE( fixture.input->GetText().HasSubstr( "S3" ) < 0 && fixture.input->GetText().HasSubstr( SecretPart ) < 0 );
        ASSERT_TRUE( fixture.input->GetText() == fixture.input->GetProperty( "Text" ) );
        ASSERT_TRUE( fixture.Painted().HasSubstr( SecretPart ) < 0 && fixture.Painted().HasSubstr( "S3" ) < 0 && fixture.Painted().HasSubstr( fixture.input->GetText() ) >= 0 );
        ASSERT_TRUE( secretChanged.count == 2 && valueChanged.count == 2 );
        // what a change notification carries is the mask, never the value
        ASSERT_TRUE( valueChanged.lastNew.HasSubstr( SecretPart ) < 0 && valueChanged.lastOld.HasSubstr( SecretPart ) < 0 && valueChanged.lastNew == fixture.input->GetText() );

        // an empty value draws nothing at all
        fixture.input->ClearValue();
        ASSERT_TRUE( fixture.input->GetText().IsNULLOrEmpty() && fixture.Painted().IsNULLOrEmpty() );

        // with the length shown there is one mask character per code point (the accented character counts once)
        GUI::CSecretInputSettings settings;
        settings.hideLength = false;
        settings.maskCodePoint = '*';
        fixture.input->SetSettings( settings );
        fixture.Type( "ab\xC3\xA9" );
        ASSERT_TRUE( "***" == fixture.input->GetText() && fixture.ValueIs( "ab\xC3\xA9" ) && "***|" == fixture.Painted() );
        fixture.input->ClearValue();

        // the default mask character is a bullet
        settings = GUI::CSecretInputSettings();
        settings.hideLength = false;
        fixture.input->SetSettings( settings );
        fixture.Type( "xy" );
        ASSERT_TRUE( "\xE2\x80\xA2\xE2\x80\xA2" == fixture.input->GetText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the length limit counts code points and is never exceeded, however the text arrives" )
    try
    {
        GUI::CSecretInputSettings settings;
        settings.maxLength = 5;
        settings.hideLength = false;
        SFixture fixture( &settings );

        fixture.Type( "abcdefgh" );
        ASSERT_TRUE( fixture.ValueIs( "abcde" ) && 5 == fixture.input->GetValueLength() );
        fixture.Type( "z" );
        ASSERT_TRUE( fixture.ValueIs( "abcde" ) );

        // four byte characters take four bytes of storage each, and five of them fit
        fixture.input->ClearValue();
        fixture.Type( "\xF0\x9F\x98\x80\xF0\x9F\x98\x80\xF0\x9F\x98\x80\xF0\x9F\x98\x80\xF0\x9F\x98\x80\xF0\x9F\x98\x80" );
        ASSERT_TRUE( 5 == fixture.input->GetValueLength() && 20 == fixture.input->GetValue().GetSize() );

        // a value that is set is refused whole when it is too long
        ASSERT_FALSE( fixture.input->SetValue( "123456" ) );
        ASSERT_FALSE( fixture.input->HasValue() );
        ASSERT_TRUE( fixture.input->SetValue( "12345" ) && fixture.ValueIs( "12345" ) );
        ASSERT_FALSE( fixture.input->SetValue( CORE::CString( "\xE9" ) ) );          // not valid UTF-8
        ASSERT_FALSE( fixture.input->HasValue() );

        // a pasted text is cut at the limit
        ResetClipboard();
        GUI::CSecretInput::SetClipboardReader( &FakeClipboardRead );
        g_clipboard.text = "0123456789";
        fixture.input->ClearValue();
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        ASSERT_TRUE( fixture.ValueIs( "01234" ) );

        // a smaller limit cuts the value and wipes the part that went
        settings.maxLength = 3;
        fixture.input->SetSettings( settings );
        ASSERT_TRUE( fixture.ValueIs( "012" ) && IsAllZero( GUI::CSecretInputTestAccess::Storage( *fixture.input ).GetRawStorage() + 3, 4 * 3 + 1 - 3 ) );

        // the limit of a settings object has a ceiling
        settings.maxLength = 1000000;
        fixture.input->SetSettings( settings );
        ASSERT_TRUE( 4096 == fixture.input->GetSettings().maxLength );
    }
    catch ( ... )
    {
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: control characters and broken UTF-8 are ignored" )
    try
    {
        GUI::CSecretInputSettings settings;
        settings.hideLength = false;
        SFixture fixture( &settings );
        fixture.Type( "a\x01" "b\x7f\tc\r\nd\x1b" "e" );
        ASSERT_TRUE( fixture.ValueIs( "abcde" ) );
        fixture.Type( "\x01\t\r\n" );
        ASSERT_TRUE( fixture.ValueIs( "abcde" ) );
        // C1 controls (U+0085 as C2 85) are control characters too
        fixture.Type( "\xC2\x85" "f" );
        ASSERT_TRUE( fixture.ValueIs( "abcdef" ) );

        // text that is not valid UTF-8 is not taken at all, not even the good part of it
        fixture.Type( "gh\xC3(" );
        fixture.Type( "\xFF\xFE" );
        fixture.Type( "\xED\xA0\x80" );          // a surrogate
        fixture.Type( "\xC0\xAF" );              // an overlong slash
        fixture.Type( "\xE2\x82" );              // cut off
        ASSERT_TRUE( fixture.ValueIs( "abcdef" ) );

        // other keys do not become characters
        fixture.Key( INPUT::KEYCODE_TAB );
        fixture.Key( INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( fixture.ValueIs( "abcdef" ) );

        // something typed after a refused text still goes in
        fixture.context->SetFocus( fixture.input );
        fixture.Type( "g" );
        ASSERT_TRUE( fixture.ValueIs( "abcdefg" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: Backspace, Delete, Home, End, Ctrl+A and Ctrl+Backspace edit the value; Enter reports; read only takes nothing" )
    try
    {
        GUI::CSecretInputSettings settings;
        settings.hideLength = false;
        SFixture fixture( &settings );
        CRecorder entered( fixture.input, GUI::CEditbox::TextEnteredEvent );
        CRecorder selection( fixture.input, GUI::CEditbox::SelectionChangedEvent );

        fixture.Type( "ab\xC3\xA9" );
        fixture.Key( INPUT::KEYCODE_BACK );
        ASSERT_TRUE( fixture.ValueIs( "ab" ) && 2 == fixture.input->GetValueLength() );
        fixture.Key( INPUT::KEYCODE_BACK );
        fixture.Key( INPUT::KEYCODE_BACK );
        fixture.Key( INPUT::KEYCODE_BACK );
        ASSERT_TRUE( !fixture.input->HasValue() && fixture.input->GetText().IsNULLOrEmpty() );

        // Delete has nothing to delete after the caret, unless everything is selected
        fixture.Type( "abc" );
        fixture.Key( INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( fixture.ValueIs( "abc" ) );
        fixture.Key( INPUT::KEYCODE_HOME );
        fixture.Key( INPUT::KEYCODE_END );
        fixture.Key( INPUT::KEYCODE_LEFT );
        fixture.Key( INPUT::KEYCODE_RIGHT );
        ASSERT_TRUE( fixture.ValueIs( "abc" ) && !fixture.input->GetBoolProperty( "Selected" ) );

        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( fixture.input->GetBoolProperty( "Selected" ) && fixture.ValueIs( "abc" ) && selection.count > 0 );
        fixture.Key( INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( !fixture.input->HasValue() && !fixture.input->GetBoolProperty( "Selected" ) );

        // Ctrl+A on an empty input selects nothing; typing over a selection replaces it; Backspace on a selection removes it
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        ASSERT_FALSE( fixture.input->GetBoolProperty( "Selected" ) );
        fixture.Type( "abc" );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        fixture.Type( "xy" );
        ASSERT_TRUE( fixture.ValueIs( "xy" ) && !fixture.input->GetBoolProperty( "Selected" ) );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_BACK );
        ASSERT_FALSE( fixture.input->HasValue() );

        // Home and End drop a selection and keep the value
        fixture.Type( "keep" );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_END );
        ASSERT_TRUE( fixture.ValueIs( "keep" ) && !fixture.input->GetBoolProperty( "Selected" ) );

        // Ctrl+Backspace takes the whole value, a secret has no words
        fixture.Key( INPUT::KEYCODE_BACK, INPUT::KEYMOD_CTRL );
        ASSERT_FALSE( fixture.input->HasValue() );

        // Enter reports and does not touch the value
        fixture.Type( "pw" );
        fixture.Key( INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 1 == entered.count && fixture.ValueIs( "pw" ) );

        // read only
        fixture.input->SetBoolProperty( "ReadOnly", true );
        fixture.Type( "zz" );
        fixture.Key( INPUT::KEYCODE_BACK );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( fixture.ValueIs( "pw" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: copy and cut do nothing by default, not even to the clipboard; when allowed they put the value on it" )
    try
    {
        ResetClipboard();
        GUI::CSecretInput::SetClipboardWriter( &FakeClipboardWrite );
        SFixture fixture;
        fixture.Type( Secret );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );

        fixture.Key( INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_INSERT, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_DELETE, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 0 == g_clipboard.writes && g_clipboard.text.IsNULLOrEmpty() );
        ASSERT_TRUE( fixture.ValueIs( Secret ) );

        // the selection cannot be exported either
        CORE::CString exported = "unchanged";
        ASSERT_FALSE( fixture.input->GetSelectedText( exported ) );
        ASSERT_TRUE( exported.IsNULLOrEmpty() );

        // allowed: copy leaves the value, cut takes it
        GUI::CSecretInputSettings settings;
        settings.allowCopy = true;
        fixture.input->SetSettings( settings );
        fixture.Key( INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 1 == g_clipboard.writes && Secret == g_clipboard.text && fixture.ValueIs( Secret ) );
        g_clipboard.text.Clear();
        fixture.Key( INPUT::KEYCODE_INSERT, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 2 == g_clipboard.writes && Secret == g_clipboard.text );
        g_clipboard.text.Clear();
        fixture.Key( INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 3 == g_clipboard.writes && Secret == g_clipboard.text && !fixture.input->HasValue() );

        // nothing to copy: the clipboard is left alone
        fixture.Key( INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 3 == g_clipboard.writes );

        // a read only input can be copied from and cannot be cut from
        fixture.Type( "pw" );
        fixture.input->SetBoolProperty( "ReadOnly", true );
        fixture.Key( INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 4 == g_clipboard.writes && fixture.ValueIs( "pw" ) );
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
    }
    catch ( ... )
    {
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: paste works by default (Ctrl+V and Shift+Insert, control characters left out, a selection replaced) and can be switched off" )
    try
    {
        ResetClipboard();
        GUI::CSecretInput::SetClipboardReader( &FakeClipboardRead );
        GUI::CSecretInputSettings settings;
        settings.hideLength = false;
        SFixture fixture( &settings );
        CRecorder secretChanged( fixture.input, GUI::CSecretInput::SecretChangedEvent );

        g_clipboard.text = "p4ss\r\n";
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 1 == g_clipboard.reads && fixture.ValueIs( "p4ss" ) && "\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2\xE2\x80\xA2" == fixture.input->GetText() );
        g_clipboard.text = "w0rd";
        fixture.Key( INPUT::KEYCODE_INSERT, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 2 == g_clipboard.reads && fixture.ValueIs( "p4ssw0rd" ) && 2 == secretChanged.count );

        // over a selection
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        g_clipboard.text = "fresh";
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( fixture.ValueIs( "fresh" ) && !fixture.input->GetBoolProperty( "Selected" ) );

        // a clipboard with nothing in it, or with nothing but control characters, or that is not text, changes nothing
        g_clipboard.readResult = false;
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        g_clipboard.readResult = true;
        g_clipboard.text = "\r\n\t";
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        g_clipboard.text = "\xFF\xFE";
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( fixture.ValueIs( "fresh" ) );

        // text that arrives in one piece (a drop, an input method) is a paste too
        fixture.input->ClearValue();
        fixture.Type( "in-one-piece" );
        ASSERT_TRUE( fixture.ValueIs( "in-one-piece" ) );

        // paste off: the clipboard is not even read, and text that is more than one character is not taken
        settings.allowPaste = false;
        fixture.input->SetSettings( settings );
        fixture.input->ClearValue();
        const UInt32 reads = g_clipboard.reads;
        g_clipboard.text = "nope";
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_INSERT, INPUT::KEYMOD_SHIFT );
        fixture.Type( "in-one-piece" );
        ASSERT_TRUE( reads == g_clipboard.reads && !fixture.input->HasValue() );
        fixture.Type( "k" );
        fixture.Type( "\xC3\xA9" );
        ASSERT_TRUE( fixture.ValueIs( "k\xC3\xA9" ) );

        // read only takes no paste
        settings.allowPaste = true;
        fixture.input->SetSettings( settings );
        fixture.input->SetBoolProperty( "ReadOnly", true );
        fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( fixture.ValueIs( "k\xC3\xA9" ) );
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
    }
    catch ( ... )
    {
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the value cannot be revealed by default; when allowed it shows, ends after the timeout, on focus loss and on hide, and it never enters a property" )
    try
    {
        SFixture fixture;
        CRecorder revealChanged( fixture.input, GUI::CSecretInput::RevealStateChangedEvent );
        fixture.Type( Secret );

        ASSERT_FALSE( fixture.input->SetRevealed( true ) );
        ASSERT_TRUE( !fixture.input->IsRevealed() && !fixture.input->IsDisplayingClearText() && 0 == revealChanged.count );
        ASSERT_TRUE( fixture.Painted().HasSubstr( SecretPart ) < 0 );
        ASSERT_TRUE( fixture.input->SetRevealed( false ) );

        GUI::CSecretInputSettings settings;
        settings.allowReveal = true;
        settings.revealTimeoutInMs = 1000;
        fixture.input->SetSettings( settings );
        ASSERT_TRUE( fixture.input->SetRevealed( true ) && fixture.input->IsRevealed() && fixture.input->IsDisplayingClearText() && 1 == revealChanged.count );
        CORE::CString shown;
        fixture.input->GetDisplayText( shown );
        ASSERT_TRUE( Secret == shown );
        ASSERT_TRUE( fixture.Painted().HasSubstr( Secret ) >= 0 );
        // even then the text property is the mask and nothing that dumps properties sees the value
        ASSERT_TRUE( fixture.Dump().HasSubstr( SecretPart ) < 0 && 12 == fixture.input->GetText().Length() );

        // the time is counted from the moment of the reveal
        fixture.context->RenderFrame( 600 );
        ASSERT_TRUE( fixture.input->IsRevealed() );
        fixture.context->RenderFrame( 600 );
        ASSERT_TRUE( !fixture.input->IsRevealed() && 2 == revealChanged.count && fixture.Painted().HasSubstr( SecretPart ) < 0 );
        fixture.context->RenderFrame( 5000 );
        ASSERT_TRUE( 2 == revealChanged.count );

        // focus loss
        ASSERT_TRUE( fixture.input->SetRevealed( true ) );
        fixture.context->SetFocus( GUCEF_NULL );
        ASSERT_TRUE( !fixture.input->IsRevealed() && fixture.input->HasValue() && 4 == revealChanged.count );

        // hide
        fixture.context->SetFocus( fixture.input );
        ASSERT_TRUE( fixture.input->SetRevealed( true ) );
        fixture.screen->SetVisibility( false );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( !fixture.input->IsRevealed() );
        fixture.screen->SetVisibility( true );

        // switching the permission off ends a reveal
        fixture.context->FlushChanges();
        fixture.input->SetValue( "again" );
        ASSERT_TRUE( fixture.input->SetRevealed( true ) );
        settings.allowReveal = false;
        fixture.input->SetSettings( settings );
        ASSERT_TRUE( !fixture.input->IsRevealed() && !fixture.input->SetRevealed( true ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the code point typed last can be shown for a moment, off by default" )
    try
    {
        GUI::CSecretInputSettings settings;
        settings.hideLength = false;
        settings.maskCodePoint = '*';
        SFixture fixture( &settings );
        fixture.Type( "ab" );
        ASSERT_TRUE( "**" == fixture.input->GetText() && !fixture.input->IsDisplayingClearText() && "**|" == fixture.Painted() );

        settings.lastCharRevealInMs = 400;
        fixture.input->SetSettings( settings );
        fixture.Type( "c" );
        ASSERT_TRUE( fixture.input->IsDisplayingClearText() && "**c|" == fixture.Painted() );
        // the text property keeps the mask
        ASSERT_TRUE( "***" == fixture.input->GetText() );
        fixture.context->RenderFrame( 300 );
        ASSERT_TRUE( "**c|" == fixture.Painted() );
        fixture.context->RenderFrame( 300 );
        ASSERT_TRUE( !fixture.input->IsDisplayingClearText() && "***|" == fixture.Painted() );

        // a multi byte character is shown whole, and Backspace ends the moment of the character that went
        fixture.Type( "\xC3\xA9" );
        ASSERT_TRUE( "***\xC3\xA9|" == fixture.Painted() );
        fixture.Key( INPUT::KEYCODE_BACK );
        ASSERT_TRUE( !fixture.input->IsDisplayingClearText() && "***|" == fixture.Painted() );

        // a fixed number of masks shows the code point in the last of them
        settings.hideLength = true;
        settings.fixedMaskLength = 4;
        fixture.input->SetSettings( settings );
        fixture.Type( "q" );
        ASSERT_TRUE( "***q|" == fixture.Painted() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the memory is wiped when the value is cleared, shortened, replaced, taken away by Backspace, and when the widget is destroyed" )
    try
    {
        GUI::CSecretInputSettings settings;
        settings.maxLength = 32;
        SFixture fixture( &settings );
        const CORE::CSecureBuffer& storage = GUI::CSecretInputTestAccess::Storage( *fixture.input );
        ASSERT_TRUE( 32 * 4 == storage.GetCapacity() );

        fixture.Type( Secret );
        ASSERT_TRUE( 0 == memcmp( storage.GetRawStorage(), Secret, strlen( Secret ) ) );

        // Backspace: the byte that went is zero
        const UInt32 size = (UInt32) strlen( Secret );
        fixture.Key( INPUT::KEYCODE_BACK );
        ASSERT_TRUE( 0 == storage.GetRawStorage()[ size - 1 ] && IsAllZero( storage.GetRawStorage() + size - 1, storage.GetRawStorageSize() - size + 1 ) );

        // a shorter value in place of a longer one leaves nothing of the longer one behind
        ASSERT_TRUE( fixture.input->SetValue( "xy" ) );
        ASSERT_TRUE( 0 == memcmp( storage.GetRawStorage(), "xy", 2 ) && IsAllZero( storage.GetRawStorage() + 2, storage.GetRawStorageSize() - 2 ) );

        // typing over a selection
        fixture.Type( "-longer-than-xy" );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        fixture.Type( "z" );
        ASSERT_TRUE( 'z' == storage.GetRawStorage()[ 0 ] && IsAllZero( storage.GetRawStorage() + 1, storage.GetRawStorageSize() - 1 ) );

        // Delete of a selection and ClearValue
        fixture.Type( Secret );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        fixture.Key( INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) );
        fixture.Type( Secret );
        fixture.input->ClearValue();
        ASSERT_TRUE( IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) && !fixture.input->HasValue() );
        fixture.Type( Secret );
        fixture.input->Clear();
        ASSERT_TRUE( IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) );

        // destroyed: the storage is zero at the moment it is given back
        CORE::CSecureBuffer::SetReleaseAuditor( &Audit );
        fixture.Type( Secret );
        const void* storagePointer = storage.GetRawStorage();
        const UInt32 storageSize = storage.GetRawStorageSize();
        g_release.count = 0;
        fixture.context->DestroyWidget( fixture.input );
        fixture.input = GUCEF_NULL;
        fixture.context->FlushChanges();
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        ASSERT_TRUE( g_release.count >= 1 && storagePointer == g_release.storage && storageSize == g_release.size && g_release.wasZero );
    }
    catch ( ... )
    {
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: GetValue hands out a scoped copy that is wiped when it goes out of scope and does not follow the input" )
    try
    {
        SFixture fixture;
        fixture.Type( Secret );
        CORE::CSecureBuffer::SetReleaseAuditor( &Audit );
        g_release.count = 0;
        const void* copyStorage = GUCEF_NULL;
        {
            CORE::CSecureBuffer value = fixture.input->GetValue();
            ASSERT_TRUE( value.Equals( Secret, (UInt32) strlen( Secret ) ) && value.GetRawStorage() != GUI::CSecretInputTestAccess::Storage( *fixture.input ).GetRawStorage() );
            copyStorage = value.GetRawStorage();
            // the copy does not change when the input does
            fixture.input->ClearValue();
            ASSERT_TRUE( value.Equals( Secret, (UInt32) strlen( Secret ) ) && !fixture.input->HasValue() );
        }
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        ASSERT_TRUE( g_release.count >= 1 && copyStorage == g_release.storage && g_release.wasZero );

        // no value, an empty copy
        CORE::CSecureBuffer empty = fixture.input->GetValue();
        ASSERT_TRUE( empty.IsEmpty() );

        // SetValue takes a copy of its argument and leaves the argument alone
        CORE::CString text = Secret;
        ASSERT_TRUE( fixture.input->SetValue( text ) && Secret == text && fixture.ValueIs( Secret ) );
        CORE::CSecureBuffer buffer( 64 );
        buffer.Assign( "from-a-buffer", 13 );
        ASSERT_TRUE( fixture.input->SetValue( buffer ) && fixture.ValueIs( "from-a-buffer" ) && buffer.Equals( "from-a-buffer", 13 ) );
        CORE::CSecureBuffer tooBig( 2000 );
        for ( UInt32 i=0; i<300; ++i )
            tooBig.Append( "abcdefg", 7 );
        ASSERT_FALSE( fixture.input->SetValue( tooBig ) );
        ASSERT_FALSE( fixture.input->HasValue() );
    }
    catch ( ... )
    {
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the value is in no property, no dump, no saved property file, no selection export and no log line, and the Text property cannot be used to set or read it" )
    try
    {
        CORE::CString logText;
        {
            TESTFW::CLogTextCapture capture;
            GUI::CSecretInputSettings settings;
            settings.allowReveal = true;
            settings.allowCopy = true;
            settings.lastCharRevealInMs = 100;
            SFixture fixture( &settings );
            ResetClipboard();
            GUI::CSecretInput::SetClipboardReader( &FakeClipboardRead );
            GUI::CSecretInput::SetClipboardWriter( &FakeClipboardWrite );

            // typed, with key repeat (a key down again and again), backspace and retyping, paste, reveal, clear
            for ( const char* p=Secret; *p; ++p )
            {
                char one[ 2 ] = { *p, 0 };
                fixture.context->InjectKey( INPUT::KEYCODE_A, true, 0 );
                fixture.context->InjectKey( INPUT::KEYCODE_A, true, 0 );
                fixture.Type( one );
                fixture.context->InjectKey( INPUT::KEYCODE_A, false, 0 );
            }
            fixture.Key( INPUT::KEYCODE_BACK );
            fixture.Type( "2" );
            g_clipboard.text = Secret;
            fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
            fixture.Key( INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
            fixture.input->SetRevealed( true );
            fixture.context->RenderFrame( 16 );
            ASSERT_TRUE( fixture.ValueIs( Secret ) );
            ASSERT_TRUE( fixture.Dump().HasSubstr( SecretPart ) < 0 && fixture.Dump().HasSubstr( "S3" ) < 0 );
            fixture.input->SetRevealed( false );

            // Text can be neither written (refused, the mask stays) nor read as the value
            fixture.input->SetProperty( "Text", "intruder-text" );
            ASSERT_TRUE( fixture.input->GetProperty( "Text" ).HasSubstr( "intruder" ) < 0 );
            fixture.input->SetProperty( GUI::EWidgetProperty_Text, Secret );
            ASSERT_TRUE( fixture.input->GetText().HasSubstr( SecretPart ) < 0 && fixture.input->GetProperty( "Text" ).HasSubstr( "intruder" ) < 0 );
            ASSERT_TRUE( fixture.ValueIs( Secret ) );
            fixture.input->ChangeValueByUser( GUI::EWidgetProperty_Text, "intruder-text" );
            ASSERT_TRUE( fixture.input->GetText().HasSubstr( "intruder" ) < 0 && fixture.ValueIs( Secret ) );
            ASSERT_TRUE( fixture.Dump().HasSubstr( "intruder" ) < 0 );

            // the drivers get the changed properties of the widget: the Text they get is the mask
            CORE::CValueList all;
            fixture.input->GetProperties( all );
            ASSERT_TRUE( CORE::ToString( all ).HasSubstr( SecretPart ) < 0 );

            // set and cleared by the program
            ASSERT_TRUE( fixture.input->SetValue( Secret ) );
            fixture.input->ClearValue();
            fixture.input->SetValue( Secret );
            fixture.input->SetText( Secret );
            fixture.input->SetSelectedText( Secret );
            fixture.input->Clear();
            GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
            GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
            logText = capture.GetText();
        }
        ASSERT_TRUE( logText.HasSubstr( SecretPart ) < 0 && logText.HasSubstr( "S3cr" ) < 0 && logText.HasSubstr( "intruder" ) < 0 );
    }
    catch ( ... )
    {
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the text of the input event is overwritten once the input has taken it" )
    try
    {
        SFixture fixture;
        GUI::CTextEventData textData;
        textData.text = Secret;
        ASSERT_TRUE( GUI::CSecretInputTestAccess::DefaultAction( *fixture.input, GUI::CWidget::TextInputEvent, textData ) );
        ASSERT_TRUE( fixture.ValueIs( Secret ) && textData.text.IsNULLOrEmpty() );

        // also when nothing of it was good
        textData.text = "\x01\x02";
        GUI::CSecretInputTestAccess::DefaultAction( *fixture.input, GUI::CWidget::TextInputEvent, textData );
        ASSERT_TRUE( textData.text.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the value is wiped when the input is hidden (default), not when it only has not been shown yet, and on focus loss when asked" )
    try
    {
        SFixture fixture;
        fixture.Type( Secret );
        const CORE::CSecureBuffer& storage = GUI::CSecretInputTestAccess::Storage( *fixture.input );
        CRecorder secretChanged( fixture.input, GUI::CSecretInput::SecretChangedEvent );

        // losing the focus keeps the value by default
        fixture.context->SetFocus( GUCEF_NULL );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( fixture.ValueIs( Secret ) );

        // hiding the screen it is on wipes it
        fixture.context->RenderFrame( 16 );
        fixture.screen->SetVisibility( false );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( !fixture.input->HasValue() && IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) && secretChanged.count >= 1 );
        fixture.screen->SetVisibility( true );
        fixture.context->RenderFrame( 16 );

        // hiding the input itself too
        fixture.input->SetValue( Secret );
        fixture.input->SetVisibility( false );
        fixture.context->RenderFrame( 16 );
        ASSERT_FALSE( fixture.input->HasValue() );
        fixture.input->SetVisibility( true );
        fixture.context->RenderFrame( 16 );

        // a value that is set while the input is not shown yet stays until it has been shown and hidden again
        fixture.screen->SetVisibility( false );
        fixture.context->RenderFrame( 16 );
        fixture.input->SetValue( "prefilled" );
        fixture.context->RenderFrame( 16 );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( fixture.ValueIs( "prefilled" ) );
        fixture.screen->SetVisibility( true );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( fixture.ValueIs( "prefilled" ) );
        fixture.screen->SetVisibility( false );
        fixture.context->RenderFrame( 16 );
        ASSERT_FALSE( fixture.input->HasValue() );
        fixture.screen->SetVisibility( true );
        fixture.context->RenderFrame( 16 );

        // switched off: the value survives a hide
        GUI::CSecretInputSettings settings;
        settings.clearOnHide = false;
        fixture.input->SetSettings( settings );
        fixture.input->SetValue( "stays" );
        fixture.screen->SetVisibility( false );
        fixture.context->RenderFrame( 16 );
        ASSERT_TRUE( fixture.ValueIs( "stays" ) );
        fixture.screen->SetVisibility( true );
        fixture.context->RenderFrame( 16 );

        // clear on focus loss, when asked for
        settings.clearOnFocusLoss = true;
        fixture.input->SetSettings( settings );
        fixture.context->SetFocus( fixture.input );
        ASSERT_TRUE( fixture.ValueIs( "stays" ) );
        fixture.context->SetFocus( GUCEF_NULL );
        ASSERT_TRUE( !fixture.input->HasValue() && IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: the state of Caps Lock is reported only when asked for, and only when it changes" )
    try
    {
        SFixture fixture;
        CRecorder caps( fixture.input, GUI::CSecretInput::CapsLockStateChangedEvent );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CAPSLOCK );
        ASSERT_TRUE( 0 == caps.count && !fixture.input->IsCapsLockOn() );

        GUI::CSecretInputSettings settings;
        settings.notifyCapsLock = true;
        fixture.input->SetSettings( settings );
        fixture.Key( INPUT::KEYCODE_B, 0 );
        ASSERT_TRUE( 0 == caps.count && !fixture.input->IsCapsLockOn() );
        fixture.Key( INPUT::KEYCODE_A, INPUT::KEYMOD_CAPSLOCK );
        ASSERT_TRUE( 1 == caps.count && caps.lastBool && fixture.input->IsCapsLockOn() );
        fixture.Key( INPUT::KEYCODE_B, INPUT::KEYMOD_CAPSLOCK | INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 1 == caps.count );
        fixture.Key( INPUT::KEYCODE_B, 0 );
        ASSERT_TRUE( 2 == caps.count && !caps.lastBool && !fixture.input->IsCapsLockOn() );
        // typing does not need it, and the key presses are not text
        ASSERT_FALSE( fixture.input->HasValue() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecretInput: a layout can make one with its settings and a text it carries is not taken as the value" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
        GUI::CWidget* made = context->CreateWidget( "SecretInput", "Pw", screen );
        ASSERT_TRUE( GUCEF_NULL != made );
        made->SetProperty( "Text", Secret );
        made->SetIntProperty( "MaxLength", 4 );
        GUI::CSecretInput* input = GUI::CWidget::As< GUI::CSecretInput >( made );
        ASSERT_TRUE( GUCEF_NULL != input && !input->HasValue() && input->GetText().IsNULLOrEmpty() );
        ASSERT_TRUE( input->SetValue( "abcd" ) && !input->SetValue( "abcde" ) );

        // as an Editbox it is read like one and what it gives is the mask
        GUI::CEditbox* edit = GUI::CWidget::As< GUI::CEditbox >( made );
        ASSERT_TRUE( GUCEF_NULL != edit );
        input->SetValue( "abc" );
        ASSERT_TRUE( edit->GetText() == input->GetText() && edit->GetText().HasSubstr( "abc" ) < 0 );
        CORE::CString selected = "x";
        ASSERT_FALSE( edit->GetSelectedText( selected ) );
        ASSERT_TRUE( edit->SetText( "xyz" ) && input->GetValueLength() == 3 );
        edit->Clear();
        ASSERT_FALSE( input->HasValue() );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
