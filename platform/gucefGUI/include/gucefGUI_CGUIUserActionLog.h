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

#ifndef GUCEF_GUI_CGUIUSERACTIONLOG_H
#define GUCEF_GUI_CGUIUSERACTIONLOG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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

class CGUIContext;
class CWidget;

/**
 *  Writes what the user does in GUI contexts to the user log (GUCEF_USER_LOG, LOG_USER), the log that
 *  CLogManager::SetUserActionLogEnabled() keeps in a file of its own and that CLogManager::SetUserLogEnabled()
 *  switches on and off as a whole (CGeneralSettings::SetUserActionLog()). The context itself already logs the
 *  mouse buttons and the wheel; this class adds, for every context it is attached to:
 *
 *      - a form (a root widget) that is shown or hidden, a popup included: "opened" and "closed" with its name
 *      - a key that is pressed, with its modifiers, and the widget it goes to (keys that only modify are left out)
 *      - text that is typed, with the widget it goes to
 *      - a widget that is activated (a button clicked or accepted with Enter), with its text
 *      - a value the user changed (a check box, a list, a slider), with the property and its new value
 *      - an item of a list, menu or tree that was chosen
 *
 *  Secret inputs (the widget type "SecretInput" and every type derived from it, CSecretInput) are never logged with
 *  what is typed into them: their text and their value changes are left out, and of their keys only Tab, Enter and
 *  Escape are logged, so neither the secret nor its length can be read from the log. The value changes of edit boxes
 *  and text boxes are left out as well, the typed text already says what changed.
 *
 *  Each line starts with the name the context was attached with ("GUI main: key Ctrl+R in Desk/Pages (TabControl)").
 *  Nothing is made while the user log is off. The class observes the routed events at the root widgets of the
 *  context, in the tunnel phase, so an observer of a widget that stops an event does not keep it out of the log.
 *  A context and its root widgets may go away while attached: the roots are forgotten when they notify their destruction, the
 *  context (which takes its observers away before it goes) when its shared pointer, held weakly here, has expired.
 */
class GUCEF_GUI_PUBLIC_CPP CGUIUserActionLog : public CORE::CObserver
{
    public:

    CGUIUserActionLog( void );

    virtual ~CGUIUserActionLog();

    /**
     *  Starts logging what the user does in the context, the lines named by 'contextName' ("main", "settings"). Attaching a
     *  context that is attached already only changes its name. The forms shown at the time are watched too. False when the
     *  pointer holds no CGUIContext
     */
    bool Attach( const TGuiContextPtr& context, const CString& contextName );

    /** Stops logging the context */
    void Detach( const TGuiContextPtr& context );

    /** Stops logging every context */
    void DetachAll( void );

    bool IsAttached( const TGuiContextPtr& context );

    /** The number of contexts the log is attached to that still exist */
    UInt32 GetAttachedContextCount( void );

    /** Whether the widget is a secret input or inside one: what is typed into it is never logged */
    static bool IsSecretInput( const CWidget* widget );

    /** How the log names a widget: its path in the tree and its type, "Desk/Header/Refresh (Button)"; "nothing" for NULL */
    static CString DescribeWidget( const CWidget* widget );

    /** How the log names a key with its modifiers, "Ctrl+Shift+F5" */
    static CString DescribeKey( INPUT::KeyCode keyCode, UInt32 modifiers );

    /** Whether the key only modifies others (Shift, Ctrl, Alt, the system keys and the locks) */
    static bool IsModifierKey( INPUT::KeyCode keyCode );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** The destruction of a context or of a root widget that is watched: the standard events reach only this */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CGUIUserActionLog > TEventCallback;

    void OnFormShown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFormHidden( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTextInput( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    /** Watches the root widgets of the context it does not watch yet */
    void WatchRoots( CGUIContext& context );

    void WatchRoot( CWidget& root, CGUIContext& context );

    /** The name of the context a watched root or context belongs to; empty when the notifier is not watched */
    CString GetContextName( CORE::CNotifier* notifier ) const;

    /** Forgets the contexts that went away, with the roots of theirs that are left */
    void ForgetExpiredContexts( void );

    /** Stops watching the context of the entry and its roots, and forgets them */
    void ForgetContext( CORE::CNotifier* contextNotifier, bool unsubscribe );

    CGUIUserActionLog( const CGUIUserActionLog& src );              /**< not implemented */
    CGUIUserActionLog& operator=( const CGUIUserActionLog& src );   /**< not implemented */

    private:

    struct SAttachedContext
    {
        CORE::CTWeakPtr< CIGUIContext, MT::CMutex > weak;   /**< the context does not notify its destruction: this says it went */
        CGUIContext* context;
        CString name;
    };

    struct SWatchedRoot
    {
        CWidget* root;
        CGUIContext* context;
    };

    typedef GUCEF::map< CORE::CNotifier*, SAttachedContext > TContextMap;
    typedef GUCEF::map< CORE::CNotifier*, SWatchedRoot > TRootMap;

    TContextMap m_contexts;
    TRootMap m_roots;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGUIUSERACTIONLOG_H ? */
