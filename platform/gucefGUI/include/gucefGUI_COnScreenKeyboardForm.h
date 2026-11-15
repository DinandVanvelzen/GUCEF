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

#ifndef GUCEF_GUI_CONSCREENKEYBOARDFORM_H
#define GUCEF_GUI_CONSCREENKEYBOARDFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_INPUT_CVIRTUALINPUTDRIVER_H
#include "gucefINPUT_CVirtualInputDriver.h"
#define GUCEF_INPUT_CVIRTUALINPUTDRIVER_H
#endif /* GUCEF_INPUT_CVIRTUALINPUTDRIVER_H ? */

#ifndef GUCEF_GUI_CKEYBOARDLAYOUT_H
#include "gucefGUI_CKeyboardLayout.h"
#define GUCEF_GUI_CKEYBOARDLAYOUT_H
#endif /* GUCEF_GUI_CKEYBOARDLAYOUT_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

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

/**
 *  An on screen keyboard: a panel at the bottom of the context with a button for every key of a layout
 *  (CKeyboardLayout, chosen by name from the CKeyboardLayoutRegistry, "US" by default). Widgets (by name): the
 *  panel OnScreenKeyboard and a Button per key, Key_<row>_<index> with the indices of the layout.
 *
 *  The keys are sent through gucefINPUT, not to the widgets of the GUI: the form owns a virtual keyboard, a device of
 *  an INPUT::CVirtualInputDriver, and sends what a physical keyboard sends (CKeyboard::KeyStateChangedEvent with the
 *  modifier state, then CKeyboard::TextInputEvent for keys that type text, then the key up). The input adapter of the GUI
 *  (CGUIInputAdapter), the developer console, an input action map and every other observer of the keyboards get those
 *  events as they get the events of a keyboard of the hardware, and gucefGUI's own routing then delivers them to the
 *  widget that has the focus. The keys take no keyboard focus themselves, so the edit box that is being typed in keeps it.
 *
 *  The events carry the input context id that was set (SetInputContextId(): the context of the window the GUI draws
 *  in, or AnyInputContext for observers that do not filter), as events of a keyboard of that window do. A GUI context on a
 *  layer above an application that gets the keys of this keyboard has to pass the keyboard on
 *  (CGUIContext::SetKeyboardInputPassThrough()), else its own layer takes the keys, which have nowhere to go there.
 *
 *  Modifiers latch, as on a touch keyboard: a tap on Shift, Ctrl or Alt sends the key down and keeps it down until the next
 *  key has been pressed and released, which sends their key up after its own; a second tap within the double tap time
 *  locks the modifier until it is tapped again. Caps lock is a key as on a keyboard: tapping it sends the key and switches
 *  the case of the keys marked for it in the layout. With Ctrl, Alt or Meta down a key types no text, as on a keyboard.
 *
 *  A key held down repeats after the repeat delay, each repeat interval, as the operating system does: the key goes down
 *  again with its text and goes up when it is released. Time is what Update() is given, which the form does on every
 *  frame of its context, so tests with a manual clock drive it with Update().
 */
class GUCEF_GUI_PUBLIC_CPP COnScreenKeyboardForm : public CFormEx
{
    public:

    /** Sends for keyboards of every input context */
    static const Int32 AnyInputContext;

    enum EModifierState
    {
        MODIFIER_OFF        = 0 ,
        MODIFIER_LATCHED    = 1 ,   /**< down until the next key has been pressed and released */
        MODIFIER_LOCKED     = 2     /**< down until tapped again */
    };
    typedef enum EModifierState TModifierState;

    COnScreenKeyboardForm( void );

    COnScreenKeyboardForm( CIGUIContext* context );

    virtual ~COnScreenKeyboardForm();

    /** Makes the keyboard device and the panel with its keys of the layout, hidden */
    bool Build( void );

    /** Takes the layout from the registry by name; false (and nothing changes) when there is none. Rebuilds the keys when they exist */
    bool SetLayout( const CString& layoutName );

    /** Takes a layout that is not registered. Rebuilds the keys when they exist */
    bool SetLayout( const CKeyboardLayout& layout );

    const CKeyboardLayout& GetLayout( void ) const;

    /** The input context id the events of the keyboard carry; AnyInputContext by default */
    void SetInputContextId( Int32 inputContextId );

    Int32 GetInputContextId( void ) const;

    /** How much of the height of the context the keyboard covers, 0.1 .. 1; 0.38 by default */
    void SetHeight( Float32 fractionOfContext );

    Float32 GetHeight( void ) const;

    /** How long a key is held before it repeats (500 by default) */
    void SetRepeatDelayInMs( UInt32 delayInMs );

    UInt32 GetRepeatDelayInMs( void ) const;

    /** The time between repeats (33 by default) */
    void SetRepeatIntervalInMs( UInt32 intervalInMs );

    UInt32 GetRepeatIntervalInMs( void ) const;

    /** The time within which a second tap on a latched modifier locks it (400 by default) */
    void SetDoubleTapTimeInMs( UInt32 timeInMs );

    UInt32 GetDoubleTapTimeInMs( void ) const;

    /** The device id of the keyboard in the input controller (INPUT::CInputController::GetKeyboard()); -1 before Build() */
    Int32 GetKeyboardDeviceId( void ) const;

    /** The key widget of the first key of a layout that sends the key code, NULL when there is none */
    CWidget* GetKeyWidget( INPUT::KeyCode keyCode );

    CWidget* GetKeyWidget( UInt32 row, UInt32 index );

    /**
     *  The press of a key, what a pointer going down on its button does: a key goes down (with its text when it has any),
     *  a modifier latches, locks or lets go, caps lock switches. Only one key is held at a time: another is released first.
     *  False when the keyboard has no key of that code
     */
    bool PressKey( INPUT::KeyCode keyCode );

    /** The release of a key, what the pointer going up does: the key goes up, then the latched modifiers let go */
    bool ReleaseKey( INPUT::KeyCode keyCode );

    /** Lets go of the held key and of the latched and locked modifiers, each with its key up; the case set by caps lock stays */
    void ReleaseAll( void );

    /** Moves the repeat of a held key on; done on every frame of the context */
    void Update( UInt32 deltaInMs );

    /** The key held now (by a press without its release); KEYCODE_UNASSIGNED when none */
    INPUT::KeyCode GetHeldKey( void ) const;

    /** The state of a modifier key (a shift, control, alt or windows key); MODIFIER_OFF for any other key */
    TModifierState GetModifierState( INPUT::KeyCode modifierKey ) const;

    bool IsShiftActive( void ) const;

    bool IsCapsLockActive( void ) const;

    virtual bool Hide( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPreLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< COnScreenKeyboardForm > TEventCallback;

    struct SKeyRef
    {
        UInt32 row;
        UInt32 index;
    };
    typedef GUCEF::map< CWidget*, SKeyRef > TKeyWidgetMap;

    struct SModifier
    {
        INPUT::KeyCode keyCode;
        TModifierState state;
        UInt64 tapTimeInMs;     /**< when it was latched, for the double tap */
    };
    typedef GUCEF::vector< SModifier > TModifierVector;

    void OnKeyWidgetDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyWidgetUp( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void Initialize( void );
    bool EnsureKeyboard( void );
    const CKeyboardLayout::TKey* FindKey( INPUT::KeyCode keyCode ) const;
    SModifier* FindModifier( INPUT::KeyCode keyCode );
    const SModifier* FindModifier( INPUT::KeyCode keyCode ) const;
    bool IsModifierActive( UInt32 modifierMask ) const;
    void SendKeyEvent( INPUT::KeyCode keyCode, bool pressed );
    void SendKeyWithText( const CKeyboardLayout::TKey& key );
    CString GetTextOf( const CKeyboardLayout::TKey& key ) const;
    void ReleaseLatchedModifiers( void );
    void RefreshKeys( void );
    void ApplyHeight( void );

    COnScreenKeyboardForm( const COnScreenKeyboardForm& src );
    COnScreenKeyboardForm& operator=( const COnScreenKeyboardForm& src );

    private:

    INPUT::CVirtualInputDriver m_driver;
    Int32 m_deviceId;
    Int32 m_inputContextId;
    CKeyboardLayout m_layout;
    Float32 m_height;
    UInt32 m_repeatDelayInMs;
    UInt32 m_repeatIntervalInMs;
    UInt32 m_doubleTapTimeInMs;
    UInt64 m_timeInMs;
    TKeyWidgetMap m_keyWidgets;
    TModifierVector m_modifiers;
    bool m_capsLock;
    INPUT::KeyCode m_heldKey;
    UInt32 m_heldTimeInMs;
    UInt32 m_repeatCount;
    bool m_frameSubscribed;
    bool m_built;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CONSCREENKEYBOARDFORM_H ? */
