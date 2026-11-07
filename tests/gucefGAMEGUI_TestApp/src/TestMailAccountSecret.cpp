/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

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

#include "gucefGAMEGUI_CMailAccountForm.h"
#include "gucefGAMEGUI_TestCommon.h"

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

/** The way tests look into a CSecretInput: the storage of the value, which is private */
class CSecretInputTestAccess
{
    public:

    static const CORE::CSecureBuffer& Storage( const CSecretInput& input ) { return input.m_value; }
};

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const Password = "Zq9-form-secret-77";
static const char* const PasswordPart = "form-secret";

bool
IsAllZero( const void* memory, UInt32 size )
{
    const UInt8* bytes = static_cast< const UInt8* >( memory );
    for ( UInt32 i=0; i<size; ++i )
        if ( 0 != bytes[ i ] )
            return false;
    return true;
}

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

/** Records the data of the events of the mail form */
class CAccountRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CAccountRecorder > TEventCallback;

    UInt32 saves;
    UInt32 tests;
    GAMEGUI::CMailAccountFormData account;

    explicit CAccountRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), saves( 0 ), tests( 0 ), account()
    {
        TEventCallback saved( this, &CAccountRecorder::OnSave );
        SubscribeTo( form, GAMEGUI::CMailAccountForm::SaveRequestedEvent, saved );
        TEventCallback tested( this, &CAccountRecorder::OnTest );
        SubscribeTo( form, GAMEGUI::CMailAccountForm::TestRequestedEvent, tested );
    }

    virtual ~CAccountRecorder() { UnsubscribeAllFromObserver(); }

    void OnSave( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++saves;
        GAMEGUI::CMailAccountFormData* data = CORE::CICloneable::CloneableAs< GAMEGUI::CMailAccountFormData >( eventData );
        if ( GUCEF_NULL != data )
            account = *data;
    }

    void OnTest( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++tests;
        GAMEGUI::CMailAccountFormData* data = CORE::CICloneable::CloneableAs< GAMEGUI::CMailAccountFormData >( eventData );
        if ( GUCEF_NULL != data )
            account = *data;
    }
};

/** Everything a property dump of a widget and all below it says, as one text */
void
DumpWidget( GUI::CWidget* widget, CORE::CString& all )
{
    CORE::CValueList list;
    widget->GetProperties( list );
    all += CORE::ToString( list );
    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( buffer );
    widget->SaveProperties( access );
    if ( buffer.GetDataSize() > 0 )
    {
        CORE::CString saved;
        saved.Set( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
        all += saved;
    }
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( size_t i=0; i<children.size(); ++i )
        DumpWidget( children[ i ], all );
}

/** The text of every text command the painter makes for a widget */
CORE::CString
PaintedText( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    GUI::CDrawSurface::TCommandVector commands;
    GUI::CWidgetPainter::Paint( *widget, *context, commands );
    CORE::CString all;
    for ( size_t i=0; i<commands.size(); ++i )
        if ( GUI::CDrawSurface::COMMAND_TEXT == commands[ i ].type )
        {
            all += commands[ i ].text;
            all += "|";
        }
    return all;
}

/** Whether the pixels of a rectangle are the same in two frames */
bool
SameRegion( const IMAGE::TPixelMapPtr& a, const IMAGE::TPixelMapPtr& b, const GUI::CRect& rect )
{
    if ( a.IsNULL() || b.IsNULL() || a->GetWidthInPixels() != b->GetWidthInPixels() || a->GetHeightInPixels() != b->GetHeightInPixels() )
        return false;
    const UInt32 width = a->GetWidthInPixels();
    const UInt8* pa = static_cast< const UInt8* >( a->GetDataPtr() );
    const UInt8* pb = static_cast< const UInt8* >( b->GetDataPtr() );
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        if ( 0 != memcmp( pa + ( (size_t) y * width + rect.left ) * 4, pb + ( (size_t) y * width + rect.left ) * 4, (size_t) rect.width * 4 ) )
            return false;
    return true;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailAccountSecretTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.MailAccountSecret" );

    GUCEF_TESTFW_TESTCASE( "MailAccount: the password field is a secret input; what is typed in it reaches Save and Test and is not shown, kept in a property or logged" )
    try
    {
        CORE::CString logText;
        {
            TESTFW::CLogTextCapture capture;
            GUI::TGuiContextPtr contextPtr = CreateFormContext();
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            GAMEGUI::CMailAccountForm form( context );
            CAccountRecorder recorder( &form );
            ASSERT_TRUE( form.Open() );

            GUI::CSecretInput* field = form.GetWidgetAs< GUI::CSecretInput >( "Password" );
            ASSERT_TRUE( GUCEF_NULL != field && "SecretInput" == field->GetType() );
            if ( GUCEF_NULL == field )
                return;
            // the other fields are still plain edit boxes
            ASSERT_TRUE( "Editbox" == form.GetWidget( "UserName" )->GetType() && "Editbox" == form.GetWidget( "Address" )->GetType() );

            // typed by the user
            context->FlushChanges();
            ClickWidget( context, field );
            ASSERT_TRUE( field->HasFocus() );
            context->InjectText( Password );
            ASSERT_TRUE( field->GetText().HasSubstr( PasswordPart ) < 0 && field->GetText().HasSubstr( "Zq9" ) < 0 && !field->GetText().IsNULLOrEmpty() );
            ASSERT_TRUE( PaintedText( context, field ).HasSubstr( PasswordPart ) < 0 );
            ASSERT_TRUE( Password == form.GetData().password );

            ClickWidget( context, form.GetWidget( "Save" ) );
            ASSERT_TRUE( 1 == recorder.saves && Password == recorder.account.password );
            ClickWidget( context, form.GetWidget( "Test" ) );
            ASSERT_TRUE( 1 == recorder.tests && Password == recorder.account.password );

            // set by the program, read back
            GAMEGUI::CMailAccountFormData data;
            data.address = "alice@mail.example";
            data.password = "stored-" "form-secret";
            form.SetData( data );
            ASSERT_TRUE( field->GetText().HasSubstr( PasswordPart ) < 0 && "stored-form-secret" == form.GetData().password );
            form.RequestSave();
            ASSERT_TRUE( 2 == recorder.saves && "stored-form-secret" == recorder.account.password );

            // nothing the form holds says it: not the properties of any widget, not a saved property file
            form.SetData( data );
            context->RenderFrame( 16 );
            CORE::CString dump;
            DumpWidget( form.GetRootWidget(), dump );
            ASSERT_TRUE( dump.HasSubstr( "form-secret" ) < 0 && dump.HasSubstr( PasswordPart ) < 0 && dump.HasSubstr( "Zq9" ) < 0 );
            logText = capture.GetText();
        }
        ASSERT_TRUE( logText.HasSubstr( "form-secret" ) < 0 && logText.HasSubstr( "Zq9" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccount: the password is wiped when the form is closed, cancelled or destroyed" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        const void* storagePointer = GUCEF_NULL;
        UInt32 storageSize = 0;
        CORE::CSecureBuffer::SetReleaseAuditor( &Audit );
        g_release.count = 0;
        {
            GAMEGUI::CMailAccountForm form( context );
            ASSERT_TRUE( form.Open() );
            GUI::CSecretInput* field = form.GetWidgetAs< GUI::CSecretInput >( "Password" );
            ASSERT_TRUE( GUCEF_NULL != field );
            if ( GUCEF_NULL == field )
            {
                CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
                return;
            }
            const CORE::CSecureBuffer& storage = GUI::CSecretInputTestAccess::Storage( *field );
            storagePointer = storage.GetRawStorage();
            storageSize = storage.GetRawStorageSize();

            GAMEGUI::CMailAccountFormData data;
            data.password = Password;
            form.SetData( data );
            ASSERT_TRUE( field->HasValue() );

            // Close, as the application does after a save
            form.Close();
            ASSERT_TRUE( !field->HasValue() && IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) && form.GetData().password.IsNULLOrEmpty() );

            // Cancel
            ASSERT_TRUE( form.Open() );
            form.SetData( data );
            ASSERT_TRUE( Password == form.GetData().password );
            ClickWidget( context, form.GetWidget( "Cancel" ) );
            ASSERT_TRUE( !form.IsOpen() && !field->HasValue() && IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) );

            // a password that is filled in before the form is opened is there when it is opened
            form.SetData( data );
            ASSERT_TRUE( form.Open() );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( Password == form.GetData().password );

            // the explicit call
            form.ClearSecrets();
            ASSERT_TRUE( !field->HasValue() && IsAllZero( storage.GetRawStorage(), storage.GetRawStorageSize() ) );

            form.SetData( data );
        }
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        // destroyed with the form: the storage was zero when it was given back
        ASSERT_TRUE( g_release.count >= 1 && storagePointer == g_release.storage && storageSize == g_release.size && g_release.wasZero );
    }
    catch ( ... )
    {
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccount: the data of the form event is still copied and assigned like plain data (the password follows the copy, the wiping of one object leaves the other alone)" )
    try
    {
        GAMEGUI::CMailAccountFormData* data = GUCEF_NEW GAMEGUI::CMailAccountFormData();
        data->password = Password;
        const char* text = data->password.C_String();
        ASSERT_TRUE( 0 == strcmp( text, Password ) );
        GAMEGUI::CMailAccountFormData copy( *data );
        ASSERT_TRUE( Password == copy.password );
        GAMEGUI::CMailAccountFormData assigned;
        assigned = *data;
        ASSERT_TRUE( Password == assigned.password );
        assigned = GAMEGUI::CMailAccountFormData();
        ASSERT_TRUE( assigned.password.IsNULLOrEmpty() );
        GUCEF_DELETE data;
        // the copies are the receiver's own
        ASSERT_TRUE( Password == copy.password );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccount: showing the password is not offered by default; when the form allows it a toggle appears, shows the password for a short time and closes with the form" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CMailAccountForm form( context );
        ASSERT_TRUE( form.Open() );
        GUI::CSecretInput* field = form.GetWidgetAs< GUI::CSecretInput >( "Password" );
        ASSERT_TRUE( GUCEF_NULL != field );
        if ( GUCEF_NULL == field )
            return;
        GUI::CWidget* toggle = form.GetWidget( "PasswordReveal" );
        ASSERT_TRUE( GUCEF_NULL != toggle && !toggle->IsVisible() );
        ASSERT_TRUE( !form.GetPasswordSettings().allowReveal && !form.GetPasswordSettings().allowCopy && form.GetPasswordSettings().hideLength && form.GetPasswordSettings().clearOnHide );
        ASSERT_FALSE( field->SetRevealed( true ) );

        GAMEGUI::CMailAccountFormData data;
        data.password = Password;
        form.SetData( data );

        GUI::CSecretInputSettings settings;
        settings.allowReveal = true;
        settings.revealTimeoutInMs = 2000;
        form.SetPasswordSettings( settings );
        context->FlushChanges();
        ASSERT_TRUE( toggle->IsVisible() && field->GetSettings().allowReveal && field->HasValue() );

        ClickWidget( context, toggle );
        ASSERT_TRUE( field->IsRevealed() && PaintedText( context, field ).HasSubstr( Password ) >= 0 );
        context->RenderFrame( 2100 );
        ASSERT_TRUE( !field->IsRevealed() && !toggle->GetBoolProperty( "Checked" ) && PaintedText( context, field ).HasSubstr( PasswordPart ) < 0 );

        ClickWidget( context, toggle );
        ASSERT_TRUE( field->IsRevealed() );
        ClickWidget( context, toggle );
        ASSERT_TRUE( !field->IsRevealed() );

        ClickWidget( context, toggle );
        ASSERT_TRUE( field->IsRevealed() );
        form.Close();
        ASSERT_TRUE( !field->IsRevealed() && !toggle->GetBoolProperty( "Checked" ) );

        // taking the permission away hides the toggle again
        form.SetPasswordSettings( GUI::CSecretInputSettings() );
        context->FlushChanges();
        ASSERT_TRUE( !toggle->IsVisible() && !field->GetSettings().allowReveal );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccount: rendered offscreen at 1024 x 768 and 3440 x 1440 the password field shows mask characters that do not depend on the password, and the form still fits" )
    try
    {
        struct SSize { UInt32 width; UInt32 height; };
        const SSize sizes[] = { { 1024, 768 }, { 3440, 1440 } };
        for ( int s=0; s<2; ++s )
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ s ].width, sizes[ s ].height, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            GAMEGUI::CMailAccountForm form( context );
            ASSERT_TRUE( form.Open() );
            GUI::CSecretInput* field = form.GetWidgetAs< GUI::CSecretInput >( "Password" );
            ASSERT_TRUE( GUCEF_NULL != field );
            if ( GUCEF_NULL == field )
                return;
            // the font of the game draws the default mask character (without it the mask would be drawn as question marks)
            if ( HasRealFont( context ) )
                ASSERT_TRUE( GUCEF_NULL != context->GetBitmapFont( "UI" )->GetGlyph( GUI::CSecretInputSettings::DefaultMaskCodePoint ) );
            const CORE::CString sizeName = CORE::UInt32ToString( sizes[ s ].width ) + "x" + CORE::UInt32ToString( sizes[ s ].height );

            GAMEGUI::CMailAccountFormData data;
            data.address = "alice@mail.example";
            context->SetFocus( form.GetWidget( "Address" ) );
            form.SetData( data );
            context->RenderFrame( 16 );
            context->FlushChanges();
            IMAGE::TPixelMapPtr empty = CaptureFrame( context );
            ASSERT_TRUE( !empty.IsNULL() );
            const GUI::CRect area = field->GetRect();
            ASSERT_TRUE( area.width > 100 && area.height > 10 && area.Right() <= (Int32) sizes[ s ].width && area.Bottom() <= (Int32) sizes[ s ].height );

            data.password = "first-secret-value";
            form.SetData( data );
            context->FlushChanges();
            IMAGE::TPixelMapPtr first = CaptureFrame( context );
            data.password = "other-secret-here!";
            form.SetData( data );
            context->FlushChanges();
            IMAGE::TPixelMapPtr second = CaptureFrame( context );
            WriteShot( second, "mail_account_secret_" + sizeName );

            // the picture of the field does not say what the password is, not even how long it is, and it does say that there is one
            ASSERT_TRUE( !first.IsNULL() && !second.IsNULL() );
            ASSERT_TRUE( SameRegion( first, second, area ) );
            ASSERT_TRUE( !SameRegion( empty, first, area ) );
            ASSERT_TRUE( PaintedText( context, field ).HasSubstr( "secret" ) < 0 );

            // the length shown: the picture then differs with the length and still not with the characters
            GUI::CSecretInputSettings settings;
            settings.hideLength = false;
            form.SetPasswordSettings( settings );
            data.password = "abcd";
            form.SetData( data );
            IMAGE::TPixelMapPtr four = CaptureFrame( context );
            data.password = "wxyz";
            form.SetData( data );
            IMAGE::TPixelMapPtr fourOther = CaptureFrame( context );
            data.password = "abcdefgh";
            form.SetData( data );
            IMAGE::TPixelMapPtr eight = CaptureFrame( context );
            ASSERT_TRUE( SameRegion( four, fourOther, area ) && !SameRegion( four, eight, area ) );
            WriteShot( eight, "mail_account_secret_length_" + sizeName );

            // the form still fits: the password row is inside the window and does not overlap its neighbours
            GUI::CWidget* window = form.GetRootWidget();
            const GUI::CRect& windowRect = window->GetRect();
            ASSERT_TRUE( area.left >= windowRect.left && area.Right() <= windowRect.Right() && area.top >= windowRect.top && area.Bottom() <= windowRect.Bottom() );
            const char* const neighbours[] = { "UserName", "PlayerCode" };
            for ( int n=0; n<2; ++n )
            {
                const GUI::CRect& other = form.GetWidget( neighbours[ n ] )->GetRect();
                ASSERT_TRUE( !( area.left < other.Right() && other.left < area.Right() && area.top < other.Bottom() && other.top < area.Bottom() ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
