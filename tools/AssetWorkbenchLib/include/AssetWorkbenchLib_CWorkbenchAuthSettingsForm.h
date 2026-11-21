/*
 *  AssetWorkbenchLib: the settings page Authentication: the keyring that protects the saved passwords of the online stores
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHAUTHSETTINGSFORM_H
#define ASSETWORKBENCHLIB_CWORKBENCHAUTHSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#include "AssetWorkbenchLib_CWorkbenchSignIn.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The settings page Authentication: the state of the keyring that protects the passwords the owner lets the workbench save for the
 *  online stores, with the actions that belong to the state (set the code up, unlock, lock, change the code), and the sign in settings of
 *  every online store (the window of CWorkbenchSignIn). The windows are those of the sign in of the sources tab: nothing is made twice.
 *  The page has no value of its own to apply. Widgets (by name): KeyringState, KeyringAction, LockKeyring, ChangeCode, StoreChoice,
 *  StoreSettings.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchAuthSettingsForm : public GUI::CSettingsPageForm
{
    public:

    static const CORE::CString PageTitle;       /**< "Authentication" */

    CWorkbenchAuthSettingsForm( void );

    virtual ~CWorkbenchAuthSettingsForm() GUCEF_VIRTUAL_OVERRIDE;

    /** The sign in the page works with (not owned; NULL before the sources are made, the page is then disabled) */
    void SetSignIn( CWorkbenchSignIn* signIn );

    /** What the state line says now: the state of the keyring in words and what to do */
    CORE::CString GetKeyringStateText( void );

    /** The text of the action button of the state ("Set up the keyring...", "Unlock the keyring..."), empty when there is none */
    CORE::CString GetKeyringActionText( void );

    virtual CORE::CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchAuthSettingsForm > TEventCallback;

    void OnKeyringActionActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLockActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnChangeCodeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoreSettingsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyringStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ShowState( void );

    CWorkbenchAuthSettingsForm( const CWorkbenchAuthSettingsForm& src );
    CWorkbenchAuthSettingsForm& operator=( const CWorkbenchAuthSettingsForm& src );

    private:

    CWorkbenchSignIn* m_signIn;
    CORE::CString::StringVector m_storeIds;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHAUTHSETTINGSFORM_H ? */
