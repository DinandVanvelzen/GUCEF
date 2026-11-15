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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H
#include "gucefGUI_CKeyboardLayoutRegistry.h"
#define GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H
#endif /* GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#include "gucefGUI_COnScreenKeyboardForm.h"

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

const Int32 COnScreenKeyboardForm::AnyInputContext = -1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 KEY_PADDING_IN_PIXELS = 2;
const UInt32 MAX_REPEATS_PER_UPDATE = 16;

/** The modifier bit a key code of a modifier key stands for, 0 for every other key */
UInt32
ModifierMaskOf( INPUT::KeyCode keyCode )
{
    switch ( keyCode )
    {
        case INPUT::KEYCODE_LSHIFT:
        case INPUT::KEYCODE_RSHIFT: return INPUT::KEYMOD_SHIFT;
        case INPUT::KEYCODE_LCONTROL:
        case INPUT::KEYCODE_RCONTROL: return INPUT::KEYMOD_CTRL;
        case INPUT::KEYCODE_LMENU:
        case INPUT::KEYCODE_RMENU: return INPUT::KEYMOD_ALT;
        case INPUT::KEYCODE_LWIN:
        case INPUT::KEYCODE_RWIN: return INPUT::KEYMOD_META;
        default: return 0;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COnScreenKeyboardForm::COnScreenKeyboardForm( void )
    : CFormEx()
    , m_driver()
    , m_deviceId( -1 )
    , m_inputContextId( AnyInputContext )
    , m_layout()
    , m_height( 0.38f )
    , m_repeatDelayInMs( 500 )
    , m_repeatIntervalInMs( 33 )
    , m_doubleTapTimeInMs( 400 )
    , m_timeInMs( 0 )
    , m_keyWidgets()
    , m_modifiers()
    , m_capsLock( false )
    , m_heldKey( INPUT::KEYCODE_UNASSIGNED )
    , m_heldTimeInMs( 0 )
    , m_repeatCount( 0 )
    , m_frameSubscribed( false )
    , m_built( false )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

COnScreenKeyboardForm::COnScreenKeyboardForm( CIGUIContext* context )
    : CFormEx( context )
    , m_driver()
    , m_deviceId( -1 )
    , m_inputContextId( AnyInputContext )
    , m_layout()
    , m_height( 0.38f )
    , m_repeatDelayInMs( 500 )
    , m_repeatIntervalInMs( 33 )
    , m_doubleTapTimeInMs( 400 )
    , m_timeInMs( 0 )
    , m_keyWidgets()
    , m_modifiers()
    , m_capsLock( false )
    , m_heldKey( INPUT::KEYCODE_UNASSIGNED )
    , m_heldTimeInMs( 0 )
    , m_repeatCount( 0 )
    , m_frameSubscribed( false )
    , m_built( false )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::Initialize( void )
{GUCEF_TRACE;

    const INPUT::KeyCode modifierKeys[] = { INPUT::KEYCODE_LSHIFT, INPUT::KEYCODE_RSHIFT, INPUT::KEYCODE_LCONTROL, INPUT::KEYCODE_RCONTROL,
                                            INPUT::KEYCODE_LMENU, INPUT::KEYCODE_RMENU, INPUT::KEYCODE_LWIN, INPUT::KEYCODE_RWIN };
    for ( UInt32 i=0; i<sizeof( modifierKeys ) / sizeof( modifierKeys[ 0 ] ); ++i )
    {
        SModifier modifier;
        modifier.keyCode = modifierKeys[ i ];
        modifier.state = MODIFIER_OFF;
        modifier.tapTimeInMs = 0;
        m_modifiers.push_back( modifier );
    }

    if ( !CGuiGlobal::Instance()->GetKeyboardLayoutRegistry().Get( CKeyboardLayoutRegistry::DefaultLayoutName, m_layout ) )
        m_layout = CKeyboardLayout::MakeUS();
}

/*-------------------------------------------------------------------------*/

COnScreenKeyboardForm::~COnScreenKeyboardForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    // what a physical keyboard has down when it is unplugged is let go of first
    ReleaseAll();
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::EnsureKeyboard( void )
{GUCEF_TRACE;

    if ( m_deviceId < 0 )
        m_deviceId = m_driver.CreateKeyboard();
    return m_deviceId >= 0;
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::Build( void )
{GUCEF_TRACE;

    if ( !EnsureKeyboard() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "OnScreenKeyboardForm: The virtual keyboard could not be made" );
        return false;
    }

    const Float32 unit = m_layout.GetWidth();
    const UInt32 rowCount = m_layout.GetRowCount();
    if ( unit <= 0.0f || 0 == rowCount )
        return false;

    const CString pad = CORE::UInt32ToString( KEY_PADDING_IN_PIXELS );
    const CString negativePad = "-" + CORE::UInt32ToString( 2 * KEY_PADDING_IN_PIXELS );

    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    CORE::CDataNode* panel = CFormLayout::AddWidgetNode( layout, "Panel", "OnScreenKeyboard", "0,0 " + CORE::FloatToString( 1.0f - m_height ) + ",0", "1,0 " + CORE::FloatToString( m_height ) + ",0" );
    CFormLayout::SetWidgetNodeProperty( *panel, EWidgetProperty_BackgroundColour, "20,22,28,242" );
    CFormLayout::SetWidgetNodeProperty( *panel, EWidgetProperty_BorderColour, "74,136,232,255" );
    CFormLayout::SetWidgetNodeProperty( *panel, EWidgetProperty_BorderWidth, "1" );
    CFormLayout::SetWidgetNodeProperty( *panel, EWidgetProperty_Focusable, "false" );

    for ( UInt32 r=0; r<rowCount; ++r )
    {
        const CKeyboardLayout::TKeyRow& keys = m_layout.GetRow( r );
        const CString rowTop = CORE::FloatToString( (Float32) r / (Float32) rowCount );
        const CString rowHeight = CORE::FloatToString( 1.0f / (Float32) rowCount );
        Float32 offset = 0.0f;
        for ( UInt32 k=0; k<(UInt32) keys.size(); ++k )
        {
            const CKeyboardLayout::TKey& key = keys[ k ];
            if ( INPUT::KEYCODE_UNASSIGNED != key.keyCode )
            {
                CORE::CDataNode* button = CFormLayout::AddWidgetNode( *panel, "Button", "Key_" + CORE::UInt32ToString( r ) + "_" + CORE::UInt32ToString( k ),
                                              CORE::FloatToString( offset / unit ) + "," + pad + " " + rowTop + "," + pad,
                                              CORE::FloatToString( key.width / unit ) + "," + negativePad + " " + rowHeight + "," + negativePad );
                CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Text, key.label );
                CFormLayout::SetWidgetNodeProperty( *button, EWidgetProperty_Focusable, "false" );
            }
            offset += key.width;
        }
    }

    // a rebuilt keyboard keeps its visibility
    const bool wasVisible = m_built && IsVisible();
    if ( !LoadLayout( layout ) )
        return false;
    m_built = true;
    if ( wasVisible )
        Show();
    else
        Hide();
    RefreshKeys();
    return true;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::OnPreLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPreLayoutLoad();

    // the widgets that sent the press of the held key are about to go
    ReleaseAll();
    m_keyWidgets.clear();
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    TEventCallback downCallback( this, &COnScreenKeyboardForm::OnKeyWidgetDown );
    TEventCallback upCallback( this, &COnScreenKeyboardForm::OnKeyWidgetUp );
    for ( UInt32 r=0; r<m_layout.GetRowCount(); ++r )
    {
        const CKeyboardLayout::TKeyRow& keys = m_layout.GetRow( r );
        for ( UInt32 k=0; k<(UInt32) keys.size(); ++k )
        {
            if ( INPUT::KEYCODE_UNASSIGNED == keys[ k ].keyCode )
                continue;
            CWidget* widget = GetWidget( "Key_" + CORE::UInt32ToString( r ) + "_" + CORE::UInt32ToString( k ) );
            if ( GUCEF_NULL == widget )
                continue;
            SKeyRef ref;
            ref.row = r;
            ref.index = k;
            m_keyWidgets[ widget ] = ref;
            SubscribeTo( widget, CWidget::MouseDownEvent, downCallback );
            SubscribeTo( widget, CWidget::MouseUpEvent, upCallback );
            SubscribeTo( widget, CWidget::MouseCaptureLostEvent, upCallback );
        }
    }

    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback frameStartedCallback( this, &COnScreenKeyboardForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, frameStartedCallback );
    }
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::SetLayout( const CString& layoutName )
{GUCEF_TRACE;

    CKeyboardLayout layout;
    if ( !CGuiGlobal::Instance()->GetKeyboardLayoutRegistry().Get( layoutName, layout ) )
        return false;
    return SetLayout( layout );
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::SetLayout( const CKeyboardLayout& layout )
{GUCEF_TRACE;

    if ( !layout.IsValid() )
        return false;
    ReleaseAll();
    m_layout = layout;
    if ( m_built )
        return Build();
    return true;
}

/*-------------------------------------------------------------------------*/

const CKeyboardLayout&
COnScreenKeyboardForm::GetLayout( void ) const
{GUCEF_TRACE;

    return m_layout;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::SetInputContextId( Int32 inputContextId )
{GUCEF_TRACE;

    m_inputContextId = inputContextId;
}

/*-------------------------------------------------------------------------*/

Int32
COnScreenKeyboardForm::GetInputContextId( void ) const
{GUCEF_TRACE;

    return m_inputContextId;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::SetHeight( Float32 fractionOfContext )
{GUCEF_TRACE;

    if ( fractionOfContext < 0.1f )
        fractionOfContext = 0.1f;
    if ( fractionOfContext > 1.0f )
        fractionOfContext = 1.0f;
    m_height = fractionOfContext;
    CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        root->SetProperty( EWidgetProperty_Position, "0,0 " + CORE::FloatToString( 1.0f - m_height ) + ",0" );
        root->SetProperty( EWidgetProperty_Size, "1,0 " + CORE::FloatToString( m_height ) + ",0" );
    }
}

/*-------------------------------------------------------------------------*/

Float32
COnScreenKeyboardForm::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::SetRepeatDelayInMs( UInt32 delayInMs )
{GUCEF_TRACE;

    m_repeatDelayInMs = delayInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
COnScreenKeyboardForm::GetRepeatDelayInMs( void ) const
{GUCEF_TRACE;

    return m_repeatDelayInMs;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::SetRepeatIntervalInMs( UInt32 intervalInMs )
{GUCEF_TRACE;

    m_repeatIntervalInMs = intervalInMs > 0 ? intervalInMs : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
COnScreenKeyboardForm::GetRepeatIntervalInMs( void ) const
{GUCEF_TRACE;

    return m_repeatIntervalInMs;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::SetDoubleTapTimeInMs( UInt32 timeInMs )
{GUCEF_TRACE;

    m_doubleTapTimeInMs = timeInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
COnScreenKeyboardForm::GetDoubleTapTimeInMs( void ) const
{GUCEF_TRACE;

    return m_doubleTapTimeInMs;
}

/*-------------------------------------------------------------------------*/

Int32
COnScreenKeyboardForm::GetKeyboardDeviceId( void ) const
{GUCEF_TRACE;

    return m_deviceId;
}

/*-------------------------------------------------------------------------*/

CWidget*
COnScreenKeyboardForm::GetKeyWidget( INPUT::KeyCode keyCode )
{GUCEF_TRACE;

    UInt32 row = 0, index = 0;
    if ( !m_layout.FindKey( keyCode, row, index ) )
        return GUCEF_NULL;
    return GetKeyWidget( row, index );
}

/*-------------------------------------------------------------------------*/

CWidget*
COnScreenKeyboardForm::GetKeyWidget( UInt32 row, UInt32 index )
{GUCEF_TRACE;

    return GetWidget( "Key_" + CORE::UInt32ToString( row ) + "_" + CORE::UInt32ToString( index ) );
}

/*-------------------------------------------------------------------------*/

const CKeyboardLayout::TKey*
COnScreenKeyboardForm::FindKey( INPUT::KeyCode keyCode ) const
{GUCEF_TRACE;

    UInt32 row = 0, index = 0;
    if ( !m_layout.FindKey( keyCode, row, index ) )
        return GUCEF_NULL;
    return &m_layout.GetRow( row )[ index ];
}

/*-------------------------------------------------------------------------*/

COnScreenKeyboardForm::SModifier*
COnScreenKeyboardForm::FindModifier( INPUT::KeyCode keyCode )
{GUCEF_TRACE;

    for ( TModifierVector::iterator i=m_modifiers.begin(); i!=m_modifiers.end(); ++i )
        if ( (*i).keyCode == keyCode )
            return &(*i);
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const COnScreenKeyboardForm::SModifier*
COnScreenKeyboardForm::FindModifier( INPUT::KeyCode keyCode ) const
{GUCEF_TRACE;

    for ( TModifierVector::const_iterator i=m_modifiers.begin(); i!=m_modifiers.end(); ++i )
        if ( (*i).keyCode == keyCode )
            return &(*i);
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::IsModifierActive( UInt32 modifierMask ) const
{GUCEF_TRACE;

    for ( TModifierVector::const_iterator i=m_modifiers.begin(); i!=m_modifiers.end(); ++i )
        if ( MODIFIER_OFF != (*i).state && 0 != ( ModifierMaskOf( (*i).keyCode ) & modifierMask ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::SendKeyEvent( INPUT::KeyCode keyCode, bool pressed )
{GUCEF_TRACE;

    m_driver.SendKey( m_inputContextId, m_deviceId, keyCode, pressed );
}

/*-------------------------------------------------------------------------*/

CString
COnScreenKeyboardForm::GetTextOf( const CKeyboardLayout::TKey& key ) const
{GUCEF_TRACE;

    // with control, alt or the meta key down a key is a shortcut, which types nothing
    if ( IsModifierActive( INPUT::KEYMOD_CTRL | INPUT::KEYMOD_ALT | INPUT::KEYMOD_META ) )
        return CString::Empty;
    const bool shifted = IsShiftActive() != ( m_capsLock && key.capsAffected );
    return shifted && !key.shiftText.IsNULLOrEmpty() ? key.shiftText : key.text;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::SendKeyWithText( const CKeyboardLayout::TKey& key )
{GUCEF_TRACE;

    SendKeyEvent( key.keyCode, true );
    const CString text = GetTextOf( key );
    if ( !text.IsNULLOrEmpty() )
        m_driver.SendText( m_inputContextId, m_deviceId, text );
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::PressKey( INPUT::KeyCode keyCode )
{GUCEF_TRACE;

    const CKeyboardLayout::TKey* key = FindKey( keyCode );
    if ( GUCEF_NULL == key || !EnsureKeyboard() )
        return false;

    SModifier* modifier = FindModifier( keyCode );
    if ( GUCEF_NULL != modifier )
    {
        if ( MODIFIER_OFF == modifier->state )
        {
            modifier->state = MODIFIER_LATCHED;
            modifier->tapTimeInMs = m_timeInMs;
            SendKeyEvent( keyCode, true );
        }
        else
        if ( MODIFIER_LATCHED == modifier->state && m_timeInMs - modifier->tapTimeInMs <= (UInt64) m_doubleTapTimeInMs )
        {
            modifier->state = MODIFIER_LOCKED;
        }
        else
        {
            modifier->state = MODIFIER_OFF;
            SendKeyEvent( keyCode, false );
        }
        RefreshKeys();
        return true;
    }

    // one key at a time
    if ( INPUT::KEYCODE_UNASSIGNED != m_heldKey )
        ReleaseKey( m_heldKey );

    if ( INPUT::KEYCODE_CAPITAL == keyCode )
    {
        m_capsLock = !m_capsLock;
        SendKeyEvent( keyCode, true );
    }
    else
    {
        SendKeyWithText( *key );
    }
    m_heldKey = keyCode;
    m_heldTimeInMs = 0;
    m_repeatCount = 0;
    RefreshKeys();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::ReleaseKey( INPUT::KeyCode keyCode )
{GUCEF_TRACE;

    if ( INPUT::KEYCODE_UNASSIGNED == keyCode || keyCode != m_heldKey )
        return false;

    m_heldKey = INPUT::KEYCODE_UNASSIGNED;
    SendKeyEvent( keyCode, false );
    ReleaseLatchedModifiers();
    return true;
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::ReleaseLatchedModifiers( void )
{GUCEF_TRACE;

    bool changed = false;
    for ( TModifierVector::iterator i=m_modifiers.begin(); i!=m_modifiers.end(); ++i )
    {
        if ( MODIFIER_LATCHED == (*i).state )
        {
            (*i).state = MODIFIER_OFF;
            SendKeyEvent( (*i).keyCode, false );
            changed = true;
        }
    }
    if ( changed )
        RefreshKeys();
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::ReleaseAll( void )
{GUCEF_TRACE;

    if ( INPUT::KEYCODE_UNASSIGNED != m_heldKey )
    {
        const INPUT::KeyCode held = m_heldKey;
        m_heldKey = INPUT::KEYCODE_UNASSIGNED;
        SendKeyEvent( held, false );
    }
    bool changed = false;
    for ( TModifierVector::iterator i=m_modifiers.begin(); i!=m_modifiers.end(); ++i )
    {
        if ( MODIFIER_OFF != (*i).state )
        {
            (*i).state = MODIFIER_OFF;
            SendKeyEvent( (*i).keyCode, false );
            changed = true;
        }
    }
    if ( changed )
        RefreshKeys();
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    m_timeInMs += deltaInMs;

    // caps lock is a key that is pressed, not one that repeats
    if ( INPUT::KEYCODE_UNASSIGNED == m_heldKey || INPUT::KEYCODE_CAPITAL == m_heldKey )
        return;
    m_heldTimeInMs += deltaInMs;

    const CKeyboardLayout::TKey* key = FindKey( m_heldKey );
    if ( GUCEF_NULL == key )
        return;
    UInt32 sent = 0;
    while ( sent < MAX_REPEATS_PER_UPDATE && m_heldTimeInMs >= m_repeatDelayInMs + m_repeatCount * m_repeatIntervalInMs )
    {
        // as the system repeats a key: down again with its text, up when it is released
        SendKeyWithText( *key );
        ++m_repeatCount;
        ++sent;
    }
    if ( MAX_REPEATS_PER_UPDATE == sent )
        m_heldTimeInMs = m_repeatDelayInMs + m_repeatCount * m_repeatIntervalInMs;
}

/*-------------------------------------------------------------------------*/

INPUT::KeyCode
COnScreenKeyboardForm::GetHeldKey( void ) const
{GUCEF_TRACE;

    return m_heldKey;
}

/*-------------------------------------------------------------------------*/

COnScreenKeyboardForm::TModifierState
COnScreenKeyboardForm::GetModifierState( INPUT::KeyCode modifierKey ) const
{GUCEF_TRACE;

    const SModifier* modifier = FindModifier( modifierKey );
    return GUCEF_NULL != modifier ? modifier->state : MODIFIER_OFF;
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::IsShiftActive( void ) const
{GUCEF_TRACE;

    return IsModifierActive( INPUT::KEYMOD_SHIFT );
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::IsCapsLockActive( void ) const
{GUCEF_TRACE;

    return m_capsLock;
}

/*-------------------------------------------------------------------------*/

bool
COnScreenKeyboardForm::Hide( void )
{GUCEF_TRACE;

    // a keyboard that goes away does not leave a key or a modifier down
    ReleaseAll();
    return CFormEx::Hide();
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::RefreshKeys( void )
{GUCEF_TRACE;

    const bool shift = IsShiftActive();
    for ( TKeyWidgetMap::iterator i=m_keyWidgets.begin(); i!=m_keyWidgets.end(); ++i )
    {
        CWidget* widget = (*i).first;
        const CKeyboardLayout::TKey& key = m_layout.GetRow( (*i).second.row )[ (*i).second.index ];

        const bool shifted = shift != ( m_capsLock && key.capsAffected );
        const CString& label = shifted && !key.shiftLabel.IsNULLOrEmpty() ? key.shiftLabel : key.label;
        if ( widget->GetPropertyRef( EWidgetProperty_Text ) != label )
            widget->SetProperty( EWidgetProperty_Text, label );

        // a latched modifier is lit, a locked one and caps lock more so
        CString colour;
        const SModifier* modifier = FindModifier( key.keyCode );
        if ( GUCEF_NULL != modifier )
        {
            if ( MODIFIER_LATCHED == modifier->state )
                colour = "56,108,190,255";
            else
            if ( MODIFIER_LOCKED == modifier->state )
                colour = "74,136,232,255";
        }
        else
        if ( INPUT::KEYCODE_CAPITAL == key.keyCode && m_capsLock )
        {
            colour = "74,136,232,255";
        }
        if ( colour.IsNULLOrEmpty() )
        {
            if ( widget->IsPropertySet( EWidgetProperty_BackgroundColour ) )
                widget->ResetProperty( EWidgetProperty_BackgroundColour );
        }
        else
        if ( widget->GetPropertyRef( EWidgetProperty_BackgroundColour ) != colour )
        {
            widget->SetProperty( EWidgetProperty_BackgroundColour, colour );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::OnKeyWidgetDown( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CRoutedEventData* routed = CRoutedEventData::FromCloneable( eventData );
    if ( GUCEF_NULL != routed && routed->source != notifier )
        return;
    TKeyWidgetMap::const_iterator i = m_keyWidgets.find( static_cast< CWidget* >( notifier ) );
    if ( i != m_keyWidgets.end() )
        PressKey( m_layout.GetRow( (*i).second.row )[ (*i).second.index ].keyCode );
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::OnKeyWidgetUp( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CRoutedEventData* routed = CRoutedEventData::FromCloneable( eventData );
    if ( GUCEF_NULL != routed && routed->source != notifier )
        return;
    TKeyWidgetMap::const_iterator i = m_keyWidgets.find( static_cast< CWidget* >( notifier ) );
    if ( i != m_keyWidgets.end() )
        ReleaseKey( m_layout.GetRow( (*i).second.row )[ (*i).second.index ].keyCode );
}

/*-------------------------------------------------------------------------*/

void
COnScreenKeyboardForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

const CString&
COnScreenKeyboardForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::COnScreenKeyboardForm";
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
