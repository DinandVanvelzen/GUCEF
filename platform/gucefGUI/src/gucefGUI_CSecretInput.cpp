/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#include "gucefGUI_CSecretInput.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CSecretInput::SecretChangedEvent = "GUCEF::GUI::CSecretInput::SecretChangedEvent";
const CORE::CEvent CSecretInput::RevealStateChangedEvent = "GUCEF::GUI::CSecretInput::RevealStateChangedEvent";
const CORE::CEvent CSecretInput::CapsLockStateChangedEvent = "GUCEF::GUI::CSecretInput::CapsLockStateChangedEvent";

static CSecretInput::TClipboardReader g_clipboardReader = GUCEF_NULL;
static CSecretInput::TClipboardWriter g_clipboardWriter = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The room for text that comes from the clipboard besides the code points that fit: leading and trailing white space and line ends */
const UInt32 PasteSlackInBytes = 64;

bool
DefaultClipboardRead( char* dest, UInt32 size, UInt32* written )
{
    *written = 0;
    return OSWRAP_TRUE == CORE::StringFromClipboard( dest, size, written ) && *written > 0;
}

bool
DefaultClipboardWrite( const char* text )
{
    return OSWRAP_TRUE == CORE::StringToClipboard( text );
}

/** Control characters: C0, DEL and C1 */
inline bool
IsControlCodePoint( UInt32 codePoint )
{
    return codePoint < 32 || 127 == codePoint || ( codePoint >= 0x80 && codePoint < 0xA0 );
}

/**
 *  Reads one code point of UTF-8 text, strictly: no overlong forms, no surrogates, nothing above U+10FFFF.
 *  Returns the number of bytes it takes, 0 when the bytes are not a valid code point.
 */
UInt32
ReadCodePoint( const UInt8* bytes, UInt32 available, UInt32& codePoint )
{
    if ( 0 == available )
        return 0;
    const UInt8 lead = bytes[ 0 ];
    if ( lead < 0x80 )
    {
        codePoint = lead;
        return 1;
    }
    UInt32 extra = 0;
    UInt32 minimum = 0;
    if ( lead >= 0xC2 && lead <= 0xDF )         { extra = 1; codePoint = lead & 0x1F; minimum = 0x80; }
    else if ( 0xE0 == ( lead & 0xF0 ) )         { extra = 2; codePoint = lead & 0x0F; minimum = 0x800; }
    else if ( lead >= 0xF0 && lead <= 0xF4 )    { extra = 3; codePoint = lead & 0x07; minimum = 0x10000; }
    else
        return 0;
    if ( available < 1 + extra )
        return 0;
    for ( UInt32 i=1; i<=extra; ++i )
    {
        if ( 0x80 != ( bytes[ i ] & 0xC0 ) )
            return 0;
        codePoint = ( codePoint << 6 ) | ( bytes[ i ] & 0x3F );
    }
    if ( codePoint < minimum || codePoint > 0x10FFFF || ( codePoint >= 0xD800 && codePoint <= 0xDFFF ) )
        return 0;
    return 1 + extra;
}

/** The size without an incomplete code point at the end, for text that was cut off at a fixed size */
UInt32
WithoutIncompleteTail( const UInt8* bytes, UInt32 size )
{
    UInt32 start = size;
    UInt32 back = 0;
    while ( start > 0 && back < 4 )
    {
        --start;
        ++back;
        if ( 0x80 != ( bytes[ start ] & 0xC0 ) )
        {
            UInt32 codePoint = 0;
            return 0 == ReadCodePoint( bytes + start, size - start, codePoint ) ? start : size;
        }
    }
    return size;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSecretInput::RegisterEvents( void )
{GUCEF_TRACE;

    SecretChangedEvent.Initialize();
    RevealStateChangedEvent.Initialize();
    CapsLockStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::SetClipboardReader( TClipboardReader reader )
{GUCEF_TRACE;

    g_clipboardReader = reader;
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::SetClipboardWriter( TClipboardWriter writer )
{GUCEF_TRACE;

    g_clipboardWriter = writer;
}

/*-------------------------------------------------------------------------*/

CSecretInput::CSecretInput( void )
    : CEditbox( "SecretInput" )
    , m_settings()
    , m_value( CSecretInputSettings::DefaultMaxLength * 4 )
    , m_length( 0 )
    , m_revealed( false )
    , m_revealRemainingInMs( 0 )
    , m_lastCharRemainingInMs( 0 )
    , m_lastCharBytes( 0 )
    , m_capsLockOn( false )
    , m_wasVisible( false )
    , m_updatingMask( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSecretInput::~CSecretInput()
{GUCEF_TRACE;

    // the storage of the value is wiped by its own destructor
}

/*-------------------------------------------------------------------------*/

CWidget*
CSecretInput::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CSecretInput();
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::SetSettings( const CSecretInputSettings& settings )
{GUCEF_TRACE;

    CSecretInputSettings sanitized( settings );
    sanitized.Sanitize();

    const bool lengthChanged = sanitized.maxLength != m_settings.maxLength;
    m_settings = sanitized;

    bool valueChanged = false;
    if ( lengthChanged )
    {
        // the value is cut to whole code points before the storage becomes smaller
        if ( m_length > m_settings.maxLength )
        {
            TruncateToCodePoints( m_settings.maxLength );
            valueChanged = true;
        }
        m_value.SetCapacity( m_settings.maxLength * 4 );
    }
    if ( !m_settings.allowReveal )
        EndReveal();
    if ( 0 == m_settings.lastCharRevealInMs )
        m_lastCharRemainingInMs = 0;
    if ( !m_settings.notifyCapsLock )
        m_capsLockOn = false;

    UpdateMask();
    Touch( EWidgetProperty_Text );
    if ( valueChanged )
        NotifyObservers( SecretChangedEvent );
}

/*-------------------------------------------------------------------------*/

const CSecretInputSettings&
CSecretInput::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

UInt32
CSecretInput::GetEffectiveMaxLength( void ) const
{GUCEF_TRACE;

    UInt32 maxLength = m_settings.maxLength;
    const Int32 fromProperty = GetIntProperty( EWidgetProperty_MaxLength );
    if ( fromProperty > 0 && (UInt32) fromProperty < maxLength )
        maxLength = (UInt32) fromProperty;
    return maxLength;
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::TruncateToCodePoints( UInt32 count )
{GUCEF_TRACE;

    const UInt8* bytes = m_value.GetData();
    const UInt32 size = m_value.GetSize();
    UInt32 position = 0;
    UInt32 kept = 0;
    while ( kept < count && position < size )
    {
        UInt32 codePoint = 0;
        UInt32 taken = ReadCodePoint( bytes + position, size - position, codePoint );
        if ( 0 == taken )
            break;
        position += taken;
        ++kept;
    }
    m_value.Truncate( position );
    m_length = kept;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::ReplaceValue( const char* text, UInt32 size )
{GUCEF_TRACE;

    const UInt8* bytes = reinterpret_cast< const UInt8* >( text );
    UInt32 count = 0;
    UInt32 position = 0;
    bool valid = true;
    while ( position < size )
    {
        UInt32 codePoint = 0;
        const UInt32 taken = ReadCodePoint( bytes + position, size - position, codePoint );
        if ( 0 == taken )
        {
            valid = false;
            break;
        }
        position += taken;
        ++count;
    }

    const bool hadValue = m_length > 0;
    if ( !valid || count > GetEffectiveMaxLength() || !m_value.Assign( text, size ) )
    {
        WipeValue();
        UpdateMask();
        if ( hadValue )
            NotifyObservers( SecretChangedEvent );
        return false;
    }
    m_length = count;
    m_lastCharRemainingInMs = 0;
    SetBoolProperty( EWidgetProperty_Selected, false );
    UpdateMask();
    NotifyObservers( SecretChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::SetValue( const CString& value )
{GUCEF_TRACE;

    return ReplaceValue( value.C_String(), (UInt32) ::strlen( value.C_String() ) );
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::SetValue( const CORE::CSecureBuffer& value )
{GUCEF_TRACE;

    return ReplaceValue( reinterpret_cast< const char* >( value.GetData() ), value.GetSize() );
}

/*-------------------------------------------------------------------------*/

CORE::CSecureBuffer
CSecretInput::GetValue( void ) const
{GUCEF_TRACE;

    return CORE::CSecureBuffer( m_value );
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::HasValue( void ) const
{GUCEF_TRACE;

    return m_length > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CSecretInput::GetValueLength( void ) const
{GUCEF_TRACE;

    return m_length;
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::WipeValue( void )
{GUCEF_TRACE;

    m_value.Clear();
    m_length = 0;
    m_lastCharRemainingInMs = 0;
    m_lastCharBytes = 0;
    EndReveal();
    SetBoolProperty( EWidgetProperty_Selected, false );
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::ClearValue( void )
{GUCEF_TRACE;

    const bool hadValue = m_length > 0;
    WipeValue();
    UpdateMask();
    if ( hadValue )
        NotifyObservers( SecretChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::ClearByUser( void )
{GUCEF_TRACE;

    const bool hadValue = m_length > 0;
    WipeValue();
    if ( hadValue )
        ValueChanged( true );
    else
        UpdateMask();
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::RemoveLastCodePoint( void )
{GUCEF_TRACE;

    UInt32 size = m_value.GetSize();
    if ( 0 == size )
        return;
    const UInt8* bytes = m_value.GetData();
    UInt32 start = size - 1;
    while ( start > 0 && 0x80 == ( bytes[ start ] & 0xC0 ) )
        --start;
    m_value.Truncate( start );
    if ( m_length > 0 )
        --m_length;
    m_lastCharRemainingInMs = 0;
    m_lastCharBytes = 0;
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::ValueChanged( bool byUser )
{GUCEF_TRACE;

    SetBoolProperty( EWidgetProperty_Selected, false );
    const CString previous = GetPropertyRef( EWidgetProperty_Text );
    UpdateMask();
    NotifyObservers( SecretChangedEvent );
    if ( byUser )
    {
        // the notification says what changed on the screen: the mask before and after
        CValueChangedEventData eventData;
        eventData.property = EWidgetProperty_Text;
        eventData.oldValue = previous;
        eventData.newValue = GetPropertyRef( EWidgetProperty_Text );
        DispatchEvent( ValueChangedEvent, eventData );
    }
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::BuildMask( CString& mask, bool showLastCodePoint ) const
{GUCEF_TRACE;

    mask.Clear();
    if ( 0 == m_length )
        return;
    const UInt32 count = m_settings.hideLength ? m_settings.fixedMaskLength : m_length;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( showLastCodePoint && i + 1 == count && m_lastCharBytes > 0 && m_lastCharBytes <= m_value.GetSize() )
            mask.Append( reinterpret_cast< const char* >( m_value.GetData() ) + m_value.GetSize() - m_lastCharBytes, m_lastCharBytes );
        else
            mask.Append( m_settings.maskCodePoint );
    }
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::UpdateMask( void )
{GUCEF_TRACE;

    CString mask;
    BuildMask( mask, false );
    if ( mask != GetPropertyRef( EWidgetProperty_Text ) )
    {
        m_updatingMask = true;
        SetProperty( EWidgetProperty_Text, mask );
        m_updatingMask = false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::IsDisplayingClearText( void ) const
{GUCEF_TRACE;

    return m_revealed || ( m_lastCharRemainingInMs > 0 && m_length > 0 );
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::GetDisplayText( CString& text ) const
{GUCEF_TRACE;

    if ( m_revealed )
        m_value.CopyToString( text );
    else
        BuildMask( text, m_lastCharRemainingInMs > 0 );
}

/*-------------------------------------------------------------------------*/

CString
CSecretInput::GetText( void ) const
{GUCEF_TRACE;

    // the text property is what is drawn when nothing is shown: the mask
    return GetProperty( EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::SetText( const CString& text )
{GUCEF_TRACE;

    return SetValue( text );
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::SetSelectedText( const CString& text )
{GUCEF_TRACE;

    const UInt32 size = (UInt32) ::strlen( text.C_String() );
    return AddCodePoints( text.C_String(), size );
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::GetSelectedText( CString& text ) const
{GUCEF_TRACE;

    text.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::Clear( void )
{GUCEF_TRACE;

    ClearValue();
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::SetRevealed( bool revealed )
{GUCEF_TRACE;

    if ( !revealed )
    {
        EndReveal();
        return true;
    }
    if ( !m_settings.allowReveal )
        return false;
    if ( !m_revealed )
    {
        m_revealed = true;
        m_revealRemainingInMs = m_settings.revealTimeoutInMs;
        Touch( EWidgetProperty_Text );
        NotifyObservers( RevealStateChangedEvent );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::EndReveal( void )
{GUCEF_TRACE;

    if ( m_revealed )
    {
        m_revealed = false;
        m_revealRemainingInMs = 0;
        Touch( EWidgetProperty_Text );
        NotifyObservers( RevealStateChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::IsRevealed( void ) const
{GUCEF_TRACE;

    return m_revealed;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::IsCapsLockOn( void ) const
{GUCEF_TRACE;

    return m_settings.notifyCapsLock && m_capsLockOn;
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::ReportCapsLock( UInt32 modifiers )
{GUCEF_TRACE;

    if ( !m_settings.notifyCapsLock )
        return;
    const bool on = 0 != ( modifiers & INPUT::KEYMOD_CAPSLOCK );
    if ( on != m_capsLockOn )
    {
        m_capsLockOn = on;
        CORE::TCloneableBool eventData( on );
        NotifyObservers( CapsLockStateChangedEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::AddCodePoints( const char* text, UInt32 size )
{GUCEF_TRACE;

    const UInt8* bytes = reinterpret_cast< const UInt8* >( text );

    // first look at all of it: text that is broken is not taken at all, and so is text that has nothing to take
    UInt32 accepted = 0;
    UInt32 position = 0;
    while ( position < size )
    {
        UInt32 codePoint = 0;
        const UInt32 taken = ReadCodePoint( bytes + position, size - position, codePoint );
        if ( 0 == taken )
            return false;
        position += taken;
        if ( !IsControlCodePoint( codePoint ) )
            ++accepted;
    }
    if ( 0 == accepted )
        return false;

    // text that comes in one piece is a paste, whatever way it got here
    if ( accepted > 1 && !m_settings.allowPaste )
        return false;

    if ( GetBoolProperty( EWidgetProperty_Selected ) )
        WipeValue();

    const UInt32 maxLength = GetEffectiveMaxLength();
    UInt32 lastBytes = 0;
    position = 0;
    while ( position < size && m_length < maxLength )
    {
        UInt32 codePoint = 0;
        const UInt32 taken = ReadCodePoint( bytes + position, size - position, codePoint );
        position += taken;
        if ( IsControlCodePoint( codePoint ) )
            continue;
        if ( !m_value.Append( bytes + position - taken, taken ) )
            break;
        ++m_length;
        lastBytes = taken;
    }
    if ( lastBytes > 0 )
    {
        m_lastCharBytes = lastBytes;
        m_lastCharRemainingInMs = m_settings.lastCharRevealInMs;
        SetBoolProperty( EWidgetProperty_CaretVisible, true );
        ValueChanged( true );
    }
    else
    {
        // the value did not change but a selection that was replaced is gone
        UpdateMask();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::Paste( void )
{GUCEF_TRACE;

    if ( !m_settings.allowPaste || GetBoolProperty( EWidgetProperty_ReadOnly ) )
        return false;
    TClipboardReader reader = GUCEF_NULL != g_clipboardReader ? g_clipboardReader : &DefaultClipboardRead;

    // what is read goes into a block that is wiped when this function ends
    CORE::CSecureBuffer scratch( m_settings.maxLength * 4 + PasteSlackInBytes );
    UInt32 written = 0;
    if ( !reader( reinterpret_cast< char* >( scratch.GetWritableData() ), scratch.GetCapacity() + 1, &written ) )
        return false;
    if ( written > scratch.GetCapacity() )
        written = scratch.GetCapacity();
    if ( written == scratch.GetCapacity() )
        written = WithoutIncompleteTail( scratch.GetData(), written );
    scratch.SetSize( written );
    return AddCodePoints( reinterpret_cast< const char* >( scratch.GetData() ), scratch.GetSize() );
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::Copy( bool cut )
{GUCEF_TRACE;

    if ( !m_settings.allowCopy || 0 == m_length )
        return false;
    TClipboardWriter writer = GUCEF_NULL != g_clipboardWriter ? g_clipboardWriter : &DefaultClipboardWrite;
    const bool done = writer( reinterpret_cast< const char* >( m_value.GetData() ) );
    if ( done && cut && !GetBoolProperty( EWidgetProperty_ReadOnly ) )
        ClearByUser();
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::HandleKey( INPUT::KeyCode keyCode, UInt32 modifiers )
{GUCEF_TRACE;

    const bool control = 0 != ( modifiers & INPUT::KEYMOD_CTRL ) && 0 == ( modifiers & INPUT::KEYMOD_ALT );
    const bool shift = 0 != ( modifiers & INPUT::KEYMOD_SHIFT );
    const bool readOnly = GetBoolProperty( EWidgetProperty_ReadOnly );

    if ( control )
    {
        switch ( keyCode )
        {
            case INPUT::KEYCODE_A:
            {
                if ( m_length > 0 )
                    SetBoolProperty( EWidgetProperty_Selected, true );
                return true;
            }
            case INPUT::KEYCODE_C:
            case INPUT::KEYCODE_INSERT:
            {
                Copy( false );
                return true;
            }
            case INPUT::KEYCODE_X:
            {
                Copy( true );
                return true;
            }
            case INPUT::KEYCODE_V:
            {
                Paste();
                return true;
            }
            case INPUT::KEYCODE_BACK:
            {
                // a secret has no words: the whole value goes
                if ( !readOnly )
                    ClearByUser();
                return true;
            }
            default: break;
        }
    }
    else
    if ( shift )
    {
        if ( INPUT::KEYCODE_INSERT == keyCode )
        {
            Paste();
            return true;
        }
        if ( INPUT::KEYCODE_DELETE == keyCode )
        {
            Copy( true );
            return true;
        }
    }

    switch ( keyCode )
    {
        case INPUT::KEYCODE_BACK:
        {
            if ( readOnly )
                return true;
            if ( GetBoolProperty( EWidgetProperty_Selected ) )
            {
                ClearByUser();
            }
            else
            if ( m_length > 0 )
            {
                RemoveLastCodePoint();
                ValueChanged( true );
            }
            return true;
        }
        case INPUT::KEYCODE_DELETE:
        {
            // the caret is at the end: there is nothing after it, unless the whole value is selected
            if ( !readOnly && GetBoolProperty( EWidgetProperty_Selected ) )
                ClearByUser();
            return true;
        }
        case INPUT::KEYCODE_HOME:
        case INPUT::KEYCODE_END:
        case INPUT::KEYCODE_LEFT:
        case INPUT::KEYCODE_RIGHT:
        {
            SetBoolProperty( EWidgetProperty_Selected, false );
            return true;
        }
        case INPUT::KEYCODE_RETURN:
        case INPUT::KEYCODE_NUMPADENTER:
        {
            NotifyObservers( TextEnteredEvent );
            return true;
        }
        default: break;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( TextInputEvent == eventId )
    {
        CTextEventData* textData = CORE::CICloneable::CloneableAs< CTextEventData >( &eventData );
        if ( GUCEF_NULL == textData )
            return false;
        bool taken = false;
        if ( !GetBoolProperty( EWidgetProperty_ReadOnly ) )
            taken = AddCodePoints( textData->text.C_String(), (UInt32) ::strlen( textData->text.C_String() ) );

        // the event data is the context's: whoever else saw the text, nobody sees it after this
        CORE::SecureWipeString( textData->text );
        return taken;
    }
    if ( KeyDownEvent == eventId )
    {
        CKeyEventData* key = CORE::CICloneable::CloneableAs< CKeyEventData >( &eventData );
        if ( GUCEF_NULL == key )
            return false;
        ReportCapsLock( key->modifiers );
        return HandleKey( key->keyCode, key->modifiers );
    }
    if ( KeyboardFocusLostEvent == eventId )
    {
        CEditbox::OnDefaultAction( eventId, eventData );
        EndReveal();
        if ( m_settings.clearOnFocusLoss && ( m_length > 0 || !m_value.IsEmpty() ) )
        {
            WipeValue();
            ValueChanged( false );
        }
        return true;
    }
    return CEditbox::OnDefaultAction( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( EWidgetProperty_Text == property && !m_updatingMask )
    {
        // the text property is what is drawn: it is not a way to set the value and does not take anything else
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CSecretInput: A value was written to the Text property of the secret input \"" << GetName() << "\" and is refused, SetValue() sets the value" );
        CString mask;
        BuildMask( mask, false );
        m_updatingMask = true;
        SetProperty( EWidgetProperty_Text, mask );
        m_updatingMask = false;
        return;
    }
    if ( EWidgetProperty_Visible == property )
        CheckVisibility();
    CEditbox::OnPropertyChanged( property, oldValue, newValue );
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::CheckVisibility( void )
{GUCEF_TRACE;

    const bool visible = IsEffectivelyVisible();
    if ( m_wasVisible && !visible )
    {
        EndReveal();
        if ( m_settings.clearOnHide && m_length > 0 )
        {
            WipeValue();
            ValueChanged( false );
        }
    }
    m_wasVisible = visible;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInput::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return CEditbox::NeedsUpdate() || m_wasVisible || IsEffectivelyVisible();
}

/*-------------------------------------------------------------------------*/

void
CSecretInput::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    CheckVisibility();
    CEditbox::OnUpdate( deltaInMs );

    if ( m_revealed )
    {
        if ( deltaInMs >= m_revealRemainingInMs )
            EndReveal();
        else
            m_revealRemainingInMs -= deltaInMs;
    }
    if ( m_lastCharRemainingInMs > 0 )
    {
        m_lastCharRemainingInMs = deltaInMs >= m_lastCharRemainingInMs ? 0 : m_lastCharRemainingInMs - deltaInMs;
        if ( 0 == m_lastCharRemainingInMs )
            Touch( EWidgetProperty_Text );
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CSecretInput::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CSecretInput";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
