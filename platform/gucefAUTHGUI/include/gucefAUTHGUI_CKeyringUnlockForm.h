/*
 *  gucefAUTHGUI: GUCEF module providing prebaked account and login user interface forms
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

#ifndef GUCEF_AUTHGUI_CKEYRINGUNLOCKFORM_H
#define GUCEF_AUTHGUI_CKEYRINGUNLOCKFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CSECUREBUFFER_H
#include "gucefCORE_CSecureBuffer.h"
#define GUCEF_CORE_CSECUREBUFFER_H
#endif /* GUCEF_CORE_CSECUREBUFFER_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CSECRETINPUTSETTINGS_H
#include "gucefGUI_CSecretInputSettings.h"
#define GUCEF_GUI_CSECRETINPUTSETTINGS_H
#endif /* GUCEF_GUI_CSECRETINPUTSETTINGS_H ? */

#ifndef GUCEF_AUTH_CKEYRINGSESSION_H
#include "gucefAUTH_CKeyringSession.h"
#define GUCEF_AUTH_CKEYRINGSESSION_H
#endif /* GUCEF_AUTH_CKEYRINGSESSION_H ? */

#ifndef GUCEF_AUTHGUI_MACROS_H
#include "gucefAUTHGUI_macros.h"
#define GUCEF_AUTHGUI_MACROS_H
#endif /* GUCEF_AUTHGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The window that asks for the keyring code of the saved credentials (gucefAUTH, CFileCredentialStore in the keyring
 *  code mode), in one of three modes:
 *
 *  - MODE_UNLOCK: "Keyring code" and the buttons Unlock and Cancel.
 *  - MODE_SET_UP: a new code, the same code again and a hint of its strength; Set up and Cancel. A session that has no
 *    keyring yet opens in this mode when MODE_UNLOCK is asked for.
 *  - MODE_CHANGE_CODE: the current code, the new code, the new code again and the hint; Change code and Cancel.
 *
 *  The codes are typed in GUI::CSecretInput fields (SetCodeSettings(); the secure defaults otherwise, with notifyCapsLock
 *  on so that the line above the buttons says "Caps Lock is on."): the value is never a property of a widget, it is taken
 *  out of the fields when the person submits and the fields are wiped then, when the window closes and when the form goes.
 *  Nothing of a code is logged or shown.
 *
 *  With a session (SetSession(), not owned) the form does the work: it runs the operation on the pool of the session
 *  (UnlockWithCodeAsync() and the others), shows that it runs, shows a wrong code ("Wrong code.") apart from the other
 *  failures, shows the wait before the next try and counts it down (the button is disabled meanwhile), and closes when the
 *  keyring is unlocked, set up or has its new code (DoneEvent). Without a session an application does the work itself:
 *  it takes the codes in its handler of UnlockRequestedEvent (GetCode(), GetNewCode()) and reports with ShowResult().
 *
 *  It is a modal window in the middle of the screen, nine tenths of it up to 520 pixels wide and as high as the mode
 *  needs. Enter in a field submits, Escape cancels.
 *
 *  Widgets: Prompt, Note, CurrentLabel, Current, CodeLabel, Code, ConfirmLabel, Confirm, Strength, ButtonRow, Message,
 *  Submit, Cancel (the ones a mode does not use are not made).
 */
class GUCEF_AUTHGUI_EXPORT_CPP CKeyringUnlockForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent UnlockRequestedEvent;     /**< no data: the person submitted codes that pass the checks of the form, before anything is done with them */
    static const CORE::CEvent CancelledEvent;           /**< no data: Cancel or Escape; the window is closed */
    static const CORE::CEvent DoneEvent;                /**< no data: the session unlocked, set up or changed the code; the window is closed */

    static void RegisterEvents( void );

    enum EMode
    {
        MODE_UNLOCK = 0,
        MODE_SET_UP,
        MODE_CHANGE_CODE
    };

    /** The hint of the strength of a new code */
    enum EStrength
    {
        STRENGTH_NONE = 0,      /**< nothing typed */
        STRENGTH_TOO_SHORT,     /**< below the minimum length */
        STRENGTH_WEAK,
        STRENGTH_FAIR,
        STRENGTH_GOOD,
        STRENGTH_STRONG
    };

    static const UInt32 DefaultMinimumCodeLength = 8;

    CKeyringUnlockForm( void );
    CKeyringUnlockForm( GUI::CIGUIContext* context );
    virtual ~CKeyringUnlockForm();

    /** Makes the window of the current mode, hidden */
    bool Build( void );

    /** Shows the window (modal) in the mode, empty; a session without a keyring turns MODE_UNLOCK into MODE_SET_UP */
    bool Open( EMode mode = MODE_UNLOCK );

    /** Wipes the fields and hides the window; no event */
    void Close( void );

    bool IsOpen( void ) const;
    EMode GetMode( void ) const;

    /** The session the form unlocks with; not owned, it has to outlive the form or be taken away with GUCEF_NULL first */
    void SetSession( AUTH::CKeyringSession* session );
    AUTH::CKeyringSession* GetSession( void ) const;

    /** The settings of the secret fields (length, mask, reveal, paste, ...) */
    void SetCodeSettings( const GUI::CSecretInputSettings& settings );
    const GUI::CSecretInputSettings& GetCodeSettings( void ) const;

    /** The fewest characters (code points) of a new code, 1 or more */
    void SetMinimumCodeLength( UInt32 minimumCodeLength );
    UInt32 GetMinimumCodeLength( void ) const;

    /** @name What the buttons do */
    ///@{

    /**
     *  As the button Unlock, Set up or Change code: checks the fields (a code typed, the new code at least the minimum length
     *  and the same twice), notifies UnlockRequestedEvent and, with a session, starts its operation. The fields are wiped
     *  afterwards. False when nothing was started: a check failed (the message says which), the session waits after wrong
     *  codes or is busy (the future of the session ended in an error status). True when it started, also when the pool was
     *  done with it before the call returned: the form waits for the outcome, which comes as the events of the session
     */
    bool Submit( void );

    /** As the button Cancel: closes and notifies CancelledEvent */
    void Cancel( void );
    ///@}

    /** @name For the handler of UnlockRequestedEvent of an application without a session */
    ///@{

    /** The code typed (MODE_UNLOCK, MODE_SET_UP) or the current code (MODE_CHANGE_CODE); false when there is none */
    bool GetCode( AUTH::CSecretString& code ) const;

    /** The new code (MODE_SET_UP, MODE_CHANGE_CODE); false when there is none */
    bool GetNewCode( AUTH::CSecretString& newCode ) const;

    /** Shows the outcome of an operation: KEYRING_OK closes the window with DoneEvent, the others show their message */
    void ShowResult( AUTH::TKeyringResult result, UInt32 retryWaitInMs = 0 );
    ///@}

    /** @name What the form shows */
    ///@{
    CORE::CString GetMessageText( void ) const;     /**< the line above the buttons: what went wrong, what runs, the wait; empty for nothing */
    CORE::CString GetStrengthText( void ) const;    /**< the hint of the strength of the new code, empty outside MODE_SET_UP and MODE_CHANGE_CODE */
    EStrength GetStrength( void ) const;
    bool IsBusy( void ) const;                      /**< an operation of the form runs */
    bool IsWaiting( void ) const;                   /**< the next try has to wait after wrong codes */
    ///@}

    /** Wipes every field */
    void ClearSecrets( void );

    /** The strength of a code: its length in code points and the kinds of characters in it (lower and upper case letters, digits, others) */
    static EStrength RateCode( const CORE::CSecureBuffer& code, UInt32 minimumCodeLength );

    /** "Too short", "Weak", "Fair", "Good", "Strong", empty for STRENGTH_NONE */
    static CORE::CString StrengthToText( EStrength strength );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CKeyringUnlockForm > TEventCallback;

    CKeyringUnlockForm( const CKeyringUnlockForm& src );
    CKeyringUnlockForm& operator=( const CKeyringUnlockForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    bool Rebuild( void );
    void SubscribeToSession( void );
    void ApplyCodeSettings( void );
    void SetMessage( const CORE::CString& text, bool isError );
    void UpdateState( void );
    void UpdateStrength( void );
    void FinishDone( void );
    CORE::CString MakeWaitText( UInt32 retryWaitInMs ) const;
    bool ReadField( const CORE::CString& name, AUTH::CSecretString& value ) const;
    UInt32 GetFieldLength( const CORE::CString& name ) const;

    void OnSubmitActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFieldTextEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCodeChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCapsLockChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionDone( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionWrongCode( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSessionRetryAllowed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    AUTH::CKeyringSession* m_session;
    GUI::CSecretInputSettings m_codeSettings;
    UInt32 m_minimumCodeLength;
    EMode m_mode;
    EMode m_builtMode;
    bool m_built;
    bool m_open;
    bool m_busy;                    /**< an operation the form started runs */
    bool m_waiting;                 /**< the next try waits after wrong codes */
    bool m_subscribedToFrames;
    CORE::CString m_messageText;
    bool m_messageIsError;
    EStrength m_strength;
    UInt32 m_shownWaitSeconds;
    UInt64 m_waitUntilMs;           /**< monotonic time (AUTH::GetMonotonicTimeInMs()) the wait ends */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_CKEYRINGUNLOCKFORM_H ? */
