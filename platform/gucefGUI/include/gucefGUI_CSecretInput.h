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

#ifndef GUCEF_GUI_CSECRETINPUT_H
#define GUCEF_GUI_CSECRETINPUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSECUREBUFFER_H
#include "gucefCORE_CSecureBuffer.h"
#define GUCEF_CORE_CSECUREBUFFER_H
#endif /* GUCEF_CORE_CSECUREBUFFER_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CSECRETINPUTSETTINGS_H
#include "gucefGUI_CSecretInputSettings.h"
#define GUCEF_GUI_CSECRETINPUTSETTINGS_H
#endif /* GUCEF_GUI_CSECRETINPUTSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CSecretInputTestAccess;

/**
 *  One line the user types a secret in (a password, a token). Registered as "SecretInput", an "Editbox": it
 *  looks, focuses and navigates like one, and it can be used wherever an Editbox is expected, but the secret
 *  is not text of the widget:
 *   - It is kept in a fixed block of memory (CORE::CSecureBuffer) that is overwritten when the value is cleared,
 *     replaced or shortened and when the widget goes away. The value is never a property: the Text property holds
 *     what is drawn (mask characters), so property dumps, saved layouts, change notifications, drivers, themes
 *     and automation only ever see the mask. Writing the Text property from outside is refused.
 *   - GetText() is the mask, GetSelectedText() says there is nothing to export, SetText() is SetValue().
 *   - The value is read with GetValue(), which hands out a scoped copy that wipes itself.
 *   - What is drawn is decided here: mask characters (as many as the value has, or a fixed number so the length
 *     is not shown), or the value itself while it is revealed. CWidgetPainter asks GetDisplayText() for that.
 *   - Typing, Backspace, Delete, Home, End, Ctrl+A, Ctrl+Backspace, paste, copy and cut are handled here and obey the
 *     CSecretInputSettings, whose defaults are the strictest: copy and cut do nothing, nothing can be revealed, the
 *     value is wiped when the widget is hidden, the length is not shown. Nothing the user types is logged and the
 *     text of the input events is overwritten once it is taken.
 *   - There is no undo history, no autocomplete, no suggestion and no drag and drop of text.
 *
 *  Events: SecretChangedEvent (the value changed, no data), RevealStateChangedEvent (shown or hidden, no data),
 *  CapsLockStateChangedEvent (when the setting asks for it, CORE::TCloneableBool with the new state), and of CEditbox
 *  TextEnteredEvent (Enter), TextChangedEvent (the drawn mask changed) and SelectionChangedEvent.
 *  Runtime: Selected (the whole value is selected, as in CEditbox), CaretVisible.
 */
class GUCEF_GUI_PUBLIC_CPP CSecretInput : public CEditbox
{
    public:

    static const CORE::CEvent SecretChangedEvent;
    static const CORE::CEvent RevealStateChangedEvent;
    static const CORE::CEvent CapsLockStateChangedEvent;    /**< CORE::TCloneableBool: Caps Lock is on */

    static void RegisterEvents( void );

    /** Takes text from the clipboard: writes at most 'size' - 1 bytes of UTF-8 and a terminator at 'dest', the number of bytes in 'written'; false when there is nothing */
    typedef bool ( *TClipboardReader )( char* dest, UInt32 size, UInt32* written );

    /** Puts text (UTF-8, terminated) on the clipboard; false when that failed */
    typedef bool ( *TClipboardWriter )( const char* text );

    /**
     *  The clipboard the keys use. The defaults are the O/S clipboard of gucefCORE (which exists where gucefCORE
     *  has one); a backend with a clipboard of its own, and tests, set theirs. NULL restores the default.
     */
    static void SetClipboardReader( TClipboardReader reader );
    static void SetClipboardWriter( TClipboardWriter writer );

    /**
     *  The text on the clipboard that SetClipboardReader() and SetClipboardWriter() select (the O/S clipboard by default),
     *  for the widgets that copy and paste text and have no clipboard of their own (the cells of a grid). ReadClipboardText()
     *  gives false when there is no text; both give false when the clipboard is not available. A text of more than
     *  16 MiB is cut at that size on reading.
     */
    static bool ReadClipboardText( CString& text );
    static bool WriteClipboardText( const CString& text );

    public:

    CSecretInput( void );

    virtual ~CSecretInput();

    /** Applies the settings (sanitized); a smaller maximum length cuts the value, a reveal that is not allowed any more ends */
    void SetSettings( const CSecretInputSettings& settings );

    const CSecretInputSettings& GetSettings( void ) const;

    /**
     *  Makes the given text the value. False (and the value is empty) when it has more code points than the settings
     *  allow, when it holds a control character or is not valid UTF-8. The text itself stays the business of the caller.
     */
    bool SetValue( const CString& value );

    bool SetValue( const CORE::CSecureBuffer& value );

    /** A copy of the value that wipes itself when it goes out of scope; empty when there is no value */
    CORE::CSecureBuffer GetValue( void ) const;

    bool HasValue( void ) const;

    /** The number of code points of the value */
    UInt32 GetValueLength( void ) const;

    /** Wipes the value */
    void ClearValue( void );

    /**
     *  Shows or hides the value; showing needs allowReveal in the settings and false is returned without it. A revealed
     *  value is hidden again after revealTimeoutInMs, when the widget loses the focus and when it stops being visible.
     */
    bool SetRevealed( bool revealed );

    bool IsRevealed( void ) const;

    /** Caps Lock as the last key event said, always false while notifyCapsLock is off */
    bool IsCapsLockOn( void ) const;

    /** Whether GetDisplayText() is something other than mask characters (revealed, or the code point typed last) */
    bool IsDisplayingClearText( void ) const;

    /** What is drawn: mask characters, or the value (or part of it) while that is shown */
    void GetDisplayText( CString& text ) const;

    /** The text property: the mask characters, never the value */
    virtual CString GetText( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The same as SetValue() */
    virtual bool SetText( const CString& text ) GUCEF_VIRTUAL_OVERRIDE;

    /** Replaces the selection, or when nothing is selected adds to the value */
    virtual bool SetSelectedText( const CString& text ) GUCEF_VIRTUAL_OVERRIDE;

    /** Always false and an empty text: the selection of a secret is not exported */
    virtual bool GetSelectedText( CString& text ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The same as ClearValue() */
    virtual void Clear( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "SecretInput"; }

    static CWidget* Create( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CSecretInputTestAccess;

    CSecretInput( const CSecretInput& src );
    CSecretInput& operator=( const CSecretInput& src );

    UInt32 GetEffectiveMaxLength( void ) const;
    bool AddCodePoints( const char* text, UInt32 size );
    bool ReplaceValue( const char* text, UInt32 size );
    void RemoveLastCodePoint( void );
    void ClearByUser( void );
    void WipeValue( void );
    void ValueChanged( bool byUser );
    void TruncateToCodePoints( UInt32 count );
    void UpdateMask( void );
    void BuildMask( CString& mask, bool showLastCodePoint ) const;
    void EndReveal( void );
    bool HandleKey( INPUT::KeyCode keyCode, UInt32 modifiers );
    bool Paste( void );
    bool Copy( bool cut );
    void ReportCapsLock( UInt32 modifiers );
    void CheckVisibility( void );

    private:

    CSecretInputSettings m_settings;
    CORE::CSecureBuffer m_value;
    UInt32 m_length;                    /**< code points in the value */
    bool m_revealed;
    UInt32 m_revealRemainingInMs;
    UInt32 m_lastCharRemainingInMs;
    UInt32 m_lastCharBytes;             /**< bytes of the code point typed last */
    bool m_capsLockOn;
    bool m_capsLockKnown;
    bool m_wasVisible;
    bool m_updatingMask;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSECRETINPUT_H ? */
