/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CAUTHSETTINGSPROFILE_H
#define GUCEF_AUTH_CAUTHSETTINGSPROFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_AUTH_IACCOUNTPROFILE_H
#include "gucefAUTH_IAccountProfile.h"
#define GUCEF_AUTH_IACCOUNTPROFILE_H
#endif /* GUCEF_AUTH_IACCOUNTPROFILE_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The profile of a tool that has none of its own: the name is the setting displayName of the manager (the config section
 *  "Auth", so the tool keeps it with its other settings) and the id is the profile key id of the manager. The manager is
 *  not owned; it has to outlive the profile or be taken away with SetAuthManager( GUCEF_NULL ) first.
 *  A name that was set is kept in the settings of the manager (CAuthManager::GetSettings(), saved with the section "Auth") and
 *  this object raises ProfileChangedEvent, on which the tool saves its configuration.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthSettingsProfile : public CORE::CObservingNotifier ,
                                                   public IAccountProfile
{
    public:

    static const CORE::CEvent ProfileChangedEvent;      /**< no data: the name or the id changed */

    static void RegisterEvents( void );

    CAuthSettingsProfile( void );

    explicit CAuthSettingsProfile( CAuthManager* manager );

    virtual ~CAuthSettingsProfile();

    void SetAuthManager( CAuthManager* manager );

    CAuthManager* GetAuthManager( void ) const;

    virtual CORE::CString GetDisplayName( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetDisplayName( const CORE::CString& displayName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetProfileId( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CNotifier* GetProfileNotifier( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CEvent& GetProfileChangedEvent( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** The standard events of the manager, its destruction */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAuthSettingsProfile > TEventCallback;

    void OnSettingsChanged( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    CAuthSettingsProfile( const CAuthSettingsProfile& src );
    CAuthSettingsProfile& operator=( const CAuthSettingsProfile& src );

    private:

    CAuthManager* m_manager;
    CORE::CString m_lastName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHSETTINGSPROFILE_H ? */
