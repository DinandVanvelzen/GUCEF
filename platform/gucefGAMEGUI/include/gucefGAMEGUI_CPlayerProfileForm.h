/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H
#define GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#ifndef GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H
#include "gucefMULTIPLAY_CIIdentityProvider.h"
#define GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIIDENTITYPROVIDER_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The profile of the local player: the name the others see, and the id of the profile (read only). Save checks
 *  the name, hands it to the identity provider when there is one, and notifies ProfileChangedEvent with the
 *  identity as it is now (a CPlayerIdentity); a name that did not change is not a change.
 *  The layout is built in code and made for BuildDarkTheme().
 */
class GUCEF_GAMEGUI_EXPORT_CPP CPlayerProfileForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent ProfileChangedEvent;      /**< MULTIPLAY::CPlayerIdentity */
    static const CORE::CEvent CancelledEvent;           /**< no data */

    static void RegisterEvents( void );

    /** The most characters of a display name */
    static const UInt32 MaxDisplayNameLength = 32;

    CPlayerProfileForm( void );

    CPlayerProfileForm( GUI::CIGUIContext* context );

    virtual ~CPlayerProfileForm();

    /** Makes the window, hidden */
    bool Build( void );

    /** Shows the window, modal, with the identity in the fields */
    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** The identity the form shows; it replaces what the provider gave */
    void SetIdentity( const MULTIPLAY::CPlayerIdentity& identity );

    /** The identity as it was saved last, or set */
    const MULTIPLAY::CPlayerIdentity& GetIdentity( void ) const;

    /** The provider the identity comes from and the name is saved with; not owned. Takes its identity at once. */
    void SetIdentityProvider( MULTIPLAY::CIIdentityProvider* provider );

    MULTIPLAY::CIIdentityProvider* GetIdentityProvider( void ) const;

    /** The name as typed */
    CORE::CString GetDisplayNameText( void );

    void SetDisplayNameText( const CORE::CString& name );

    /**
     *  Checks the name and saves it.
     *
     *  @return false when the name is not good (it is empty) or the provider did not take it
     */
    bool Save( void );

    /** Puts the saved identity back in the fields and closes */
    void Cancel( void );

    CORE::CString GetErrorText( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void BuildLayout( CORE::CDataNode& layout );
    void ShowIdentity( void );

    CPlayerProfileForm( const CPlayerProfileForm& src );
    CPlayerProfileForm& operator=( const CPlayerProfileForm& src );

    private:

    MULTIPLAY::CPlayerIdentity m_identity;
    MULTIPLAY::CIIdentityProvider* m_provider;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CPLAYERPROFILEFORM_H ? */
