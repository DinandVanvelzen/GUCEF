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

#ifndef GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#define GUCEF_GUI_CDEVELOPERCONSOLEFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

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
 *  A developer console in the style of the classic shooters: a panel that slides down from the top over
 *  the application when its key is pressed (the key left of 1 by default) and slides back up on the
 *  same key, with the output above a command line. Widgets (by name): DeveloperConsole (the panel),
 *  Output (a text box), CommandLine (an edit box).
 *
 *  The key is watched on the gucefINPUT keyboards themselves, so it works whatever has the focus and
 *  whichever layer of the window takes the input; the character the key types is not typed into the
 *  command line. Put the console in a GUI context on a layer above everything else (CGUIContext::SetLayer())
 *  so it covers the application and takes the input while it is open.
 *
 *  A command entered is put in the output and notified with CommandEnteredEvent. What the commands do is
 *  up to the application, for example passing them to the system console (CSysConsole). One command the console
 *  knows itself: "gui.lint on|off|overlay|log|overlay,log|report" switches the layout debug overlay
 *  (CLayoutDebugOverlay) of every GUI layer of its window, or writes what the layout lint found into the output.
 */
class GUCEF_GUI_PUBLIC_CPP CDeveloperConsoleForm : public CFormEx
{
    public:

    static const CORE::CEvent CommandEnteredEvent;  /**< TCommandEnteredEventData: the command line */
    static const CORE::CEvent OpenedEvent;          /**< the console started to drop down */
    static const CORE::CEvent ClosedEvent;          /**< the console is gone */

    typedef CORE::TCloneableString TCommandEnteredEventData;

    static void RegisterEvents( void );

    /** Watches the keyboards of every input context */
    static const Int32 AnyInputContext;

    public:

    CDeveloperConsoleForm( void );

    CDeveloperConsoleForm( CIGUIContext* context );

    virtual ~CDeveloperConsoleForm();

    /** Makes the panel, hidden */
    bool Build( void );

    /** Whether the key opens the console; switched off it closes */
    void SetEnabled( bool enabled );

    bool IsEnabled( void ) const;

    void SetConsoleKey( INPUT::KeyCode key );

    INPUT::KeyCode GetConsoleKey( void ) const;

    /**
     *  Whether the console watches the keyboards of gucefINPUT itself, which is the default. Off, the application hands it the keys
     *  and the text that reach its GUI (HandleKey(), HandleText()), as it does when its keys come as events of the widgets
     */
    void SetWatchesInputDevices( bool watches );

    bool WatchesInputDevices( void ) const;

    /**
     *  A key down that reached the GUI: the console key opens or closes the console, unless the console is closed and a text field
     *  other than its own command line has the keyboard focus (the key is typed there). True when the key was the console key and the
     *  console took it
     */
    bool HandleKey( INPUT::KeyCode key, UInt32 modifiers );

    /**
     *  Text that reached the GUI: the character the console key typed is swallowed (the key event came first), and for the console key
     *  Grave the character itself ("`", "~") opens or closes the console when no key event did, which is how a layout that has the
     *  character on another key delivers it; not while another text field has the focus and the console is closed. True when the text is consumed
     */
    bool HandleText( const CString& text );

    /** Only keys of this gucefINPUT context (the window) count; AnyInputContext by default */
    void SetInputContextId( Int32 inputContextId );

    Int32 GetInputContextId( void ) const;

    /** How long dropping down and going back up take, 0 for at once */
    void SetSlideTimeInMs( UInt32 slideTimeInMs );

    UInt32 GetSlideTimeInMs( void ) const;

    /** How much of the height of the context the console covers, 0 .. 1 */
    void SetHeight( Float32 fractionOfContext );

    Float32 GetHeight( void ) const;

    /** How many lines of output are kept */
    void SetMaxLines( UInt32 maxLines );

    bool Open( void );

    bool Close( void );

    bool Toggle( void );

    /** Open, also while it still drops down */
    bool IsOpen( void ) const;

    /** How far the console is down: 0 gone, 1 all the way */
    Float32 GetSlidePosition( void ) const;

    /**
     *  A command as if it was typed: echoed in the output, the layout lint command handled, and CommandEnteredEvent raised. For the
     *  application's own shortcuts to commands (a menu entry that does what a command does)
     */
    void ExecuteCommand( const CString& command );

    void AppendLine( const CString& line );

    void ClearOutput( void );

    /** Moves the slide on, done on every frame of the context */
    void Update( UInt32 deltaInMs );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CDeveloperConsoleForm > TEventCallback;

    /** A key of a keyboard changed state: the console key toggles the console */
    void OnKeyStateChanged( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    /** A keyboard was attached: its keys are watched too */
    void OnKeyboardAttached( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    /** Text typed in the console: the character of the console key is swallowed */
    void OnTextInput( CORE::CNotifier* notifier    ,
                      const CORE::CEvent& eventId  ,
                      CORE::CICloneable* eventData );

    /** Enter in the command line */
    void OnCommandLineTextEntered( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

    /** A frame of the context starts: the slide moves on */
    void OnFrameStarted( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void AttachToKeyboards( void );

    /** Whether a text field other than the command line of the console has the keyboard focus */
    bool IsOtherTextFieldFocused( void ) const;

    /** A key down of the console key from either route: the toggle with the swallow of its character */
    bool ToggleOnKey( INPUT::KeyCode key );

    /** The key down of a widget of the console: for an application that does not let the console watch the devices */
    void OnRootKeyDown( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    void ApplySlidePosition( void );

    void OnCommandEntered( void );

    /** "gui.lint ...": the layout debug overlay of the GUI layers of the window; false for another command */
    bool HandleLayoutLintCommand( const CString& command );

    CDeveloperConsoleForm( const CDeveloperConsoleForm& src );
    CDeveloperConsoleForm& operator=( const CDeveloperConsoleForm& src );

    private:

    bool m_enabled;
    bool m_open;
    bool m_swallowText;
    bool m_keyboardsAttached;
    bool m_watchesInputDevices;
    INPUT::KeyCode m_consoleKey;
    Int32 m_inputContextId;
    UInt32 m_slideTimeInMs;
    Float32 m_height;
    Float32 m_slide;
    UInt32 m_maxLines;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CDEVELOPERCONSOLEFORM_H ? */
