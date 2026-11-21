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

#ifndef GUCEF_GUI_CINPUTSETTINGSFORM_H
#define GUCEF_GUI_CINPUTSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#include "gucefGUI_CKeyboardShortcutMap.h"
#define GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#endif /* GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H ? */

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

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
 *  The input settings: a prebaked page that shows the key chords of an application (a CKeyboardShortcutMap) and lets the person
 *  rebind them. The application declares its actions with AddAction() before the form that holds the page is built: the id of
 *  the action as the map has it, the text the list shows and the chord of the action as the application ships it.
 *
 *  Rebinding: the person selects an action and presses Change, then the chord (a key with the modifiers that are held) that
 *  the action gets; Escape alone cancels. A chord that another action has is refused, with the name of that action in the Status
 *  line, so no chord is ever taken from an action silently. Reset puts the shipped chord back on the action that is selected and
 *  Reset all on every action; they too are refused when the chord is taken by another action that has been rebound.
 *  Apply binds the chords in the map of the application and notifies SettingsAppliedEvent, for the application to keep them.
 *
 *  Widgets (by name): Actions (the list, a line per action: its text and its chord), Change, Reset, ResetAll, Status.
 */
class GUCEF_GUI_PUBLIC_CPP CInputSettingsForm : public CSettingsPageForm
{
    public:

    static const CString PageTitle;     /**< "Input" */

    CInputSettingsForm( void );

    virtual ~CInputSettingsForm();

    /** The map whose chords the page shows and rebinds; not owned, it has to be there as long as the page */
    void SetShortcutMap( CKeyboardShortcutMap* shortcuts );

    CKeyboardShortcutMap* GetShortcutMap( void ) const;

    /** Declares an action of the application, before the page is built; false for an id that is empty or declared already */
    bool AddAction( const CString& actionId, const CString& title, INPUT::KeyCode defaultKey, UInt32 defaultModifiers );

    UInt32 GetActionCount( void ) const;

    /** The line of the list for the action at the index, for tests: "title  chord" */
    CString GetActionLine( UInt32 index ) const;

    /** The chord of the action as the page has it now, not yet applied; false for an action that is not declared */
    bool GetPendingChord( const CString& actionId, INPUT::KeyCode& key, UInt32& modifiers ) const;

    /** Selects the action in the list */
    bool SelectAction( const CString& actionId );

    /** Starts the rebinding of the selected action: the next chord is its chord. False without a selection */
    bool BeginRebind( void );

    bool IsRebinding( void ) const;

    /**
     *  Gives the chord to the action; false (and the reason in the Status line, the chord that was there stays) when another
     *  action has the chord or the action is not declared
     */
    bool Rebind( const CString& actionId, INPUT::KeyCode key, UInt32 modifiers );

    /** The shipped chord back on the action, or on every action; false when another action has it */
    bool ResetAction( const CString& actionId );
    bool ResetAll( void );

    /** What the Status line says */
    CString GetStatusText( void );

    virtual CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CInputSettingsForm > TEventCallback;

    struct SAction
    {
        CString actionId;
        CString title;
        INPUT::KeyCode defaultKey;
        UInt32 defaultModifiers;
        INPUT::KeyCode key;
        UInt32 modifiers;
    };
    typedef struct SAction TAction;
    typedef GUCEF::vector< TAction > TActionVector;

    void OnChangeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResetActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResetAllActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRootKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    TAction* FindAction( const CString& actionId );
    const TAction* FindAction( const CString& actionId ) const;
    CString FindOwner( const CString& exceptActionId, INPUT::KeyCode key, UInt32 modifiers ) const;
    CString GetSelectedActionId( void );
    CString MakeLine( const TAction& action ) const;
    void ShowActions( void );
    void ShowStatus( const CString& text );

    CInputSettingsForm( const CInputSettingsForm& src );
    CInputSettingsForm& operator=( const CInputSettingsForm& src );

    private:

    CKeyboardShortcutMap* m_shortcuts;
    TActionVector m_actions;
    CString m_rebindingActionId;
    CString m_status;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CINPUTSETTINGSFORM_H ? */
