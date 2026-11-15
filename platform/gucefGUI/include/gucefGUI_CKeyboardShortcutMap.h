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

#ifndef GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#define GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

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
 *  Key chords bound to actions of an application: a key with its modifiers (Shift, Ctrl, Alt, Meta; the lock keys do
 *  not count) per action id. HandleKey() raises ShortcutActivatedEvent with the id of the action of a chord that is
 *  bound, so the application does the action in one place, whether it came from a key or from a menu. The text of a
 *  chord ("Ctrl+1", "Alt+Left", "Shift+F10") is what menus and tooltips show next to the action.
 *
 *  The map does not watch widgets: the application gives it the keys it wants handled (for example in the
 *  KeyDownEvent of its root widget), so the order against other handling of keys stays the application's.
 */
class GUCEF_GUI_PUBLIC_CPP CKeyboardShortcutMap : public CORE::CNotifier
{
    public:

    static const CORE::CEvent ShortcutActivatedEvent;   /**< CORE::TCloneableString: the id of the action */

    static void RegisterEvents( void );

    CKeyboardShortcutMap( void );

    virtual ~CKeyboardShortcutMap();

    /**
     *  Binds a chord to an action: one chord per action (a new one replaces the old one), one action per chord (the chord
     *  is taken from an action that had it). False for an empty id or KEYCODE_UNASSIGNED
     */
    bool Bind( const CString& actionId, INPUT::KeyCode key, UInt32 modifiers );

    /** False when the action had no chord */
    bool Unbind( const CString& actionId );

    void Clear( void );

    /** The action of a chord, empty for none */
    CString FindAction( INPUT::KeyCode key, UInt32 modifiers ) const;

    /** The chord of an action; false when it has none */
    bool GetChord( const CString& actionId, INPUT::KeyCode& key, UInt32& modifiers ) const;

    UInt32 GetBindingCount( void ) const;

    /** Raises ShortcutActivatedEvent for the action of the chord; false when no action has it */
    bool HandleKey( INPUT::KeyCode key, UInt32 modifiers );

    bool HandleKeyEvent( const CKeyEventData& key );

    /** The text of the chord of an action ("Ctrl+1"), empty for none */
    CString GetShortcutText( const CString& actionId ) const;

    /** The text of a chord: the modifiers in the order Ctrl, Alt, Shift, Meta, then the key, joined by '+' */
    static CString DescribeChord( INPUT::KeyCode key, UInt32 modifiers );

    /** The name of a key as a chord shows it: "A", "1", "F10", "Left", "Tab", "Enter" ... */
    static CString DescribeKey( INPUT::KeyCode key );

    /** Only the flags of Shift, Ctrl, Alt and Meta */
    static UInt32 NormalizeModifiers( UInt32 modifiers );

    private:

    struct SBinding
    {
        CString actionId;
        INPUT::KeyCode key;
        UInt32 modifiers;
    };
    typedef struct SBinding TBinding;
    typedef GUCEF::vector< TBinding > TBindingVector;

    CKeyboardShortcutMap( const CKeyboardShortcutMap& src );
    CKeyboardShortcutMap& operator=( const CKeyboardShortcutMap& src );

    private:

    TBindingVector m_bindings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H ? */
