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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefAUTH_CAuthSettingsProfile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAuthSettingsProfile::ProfileChangedEvent = "GUCEF::AUTH::CAuthSettingsProfile::ProfileChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CAuthSettingsProfile::RegisterEvents( void )
{GUCEF_TRACE;

    ProfileChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAuthSettingsProfile::CAuthSettingsProfile( void )
    : CORE::CObservingNotifier()
    , IAccountProfile()
    , m_manager( GUCEF_NULL )
    , m_lastName()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CAuthSettingsProfile::CAuthSettingsProfile( CAuthManager* manager )
    : CORE::CObservingNotifier()
    , IAccountProfile()
    , m_manager( GUCEF_NULL )
    , m_lastName()
{GUCEF_TRACE;

    RegisterEvents();
    SetAuthManager( manager );
}

/*-------------------------------------------------------------------------*/

CAuthSettingsProfile::~CAuthSettingsProfile()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CAuthSettingsProfile::SetAuthManager( CAuthManager* manager )
{GUCEF_TRACE;

    if ( manager == m_manager )
        return;
    if ( GUCEF_NULL != m_manager )
        UnsubscribeFrom( m_manager );
    m_manager = manager;
    m_lastName = GetDisplayName();
    if ( GUCEF_NULL != m_manager )
    {
        TEventCallback settingsCallback( this, &CAuthSettingsProfile::OnSettingsChanged );
        SubscribeTo( m_manager, CAuthManager::SettingsChangedEvent, settingsCallback );
    }
    NotifyObservers( ProfileChangedEvent );
}

/*-------------------------------------------------------------------------*/

CAuthManager*
CAuthSettingsProfile::GetAuthManager( void ) const
{GUCEF_TRACE;

    return m_manager;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthSettingsProfile::GetDisplayName( void )
{GUCEF_TRACE;

    return GUCEF_NULL != m_manager ? m_manager->GetSettings().displayName : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettingsProfile::SetDisplayName( const CORE::CString& displayName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager || displayName.IsNULLOrEmpty() )
        return false;
    if ( m_manager->GetSettings().displayName == displayName )
        return true;
    CAuthSettings settings( m_manager->GetSettings() );
    settings.displayName = displayName;
    m_manager->SetSettings( settings );
    m_lastName = displayName;
    NotifyObservers( ProfileChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthSettingsProfile::GetProfileId( void )
{GUCEF_TRACE;

    return GUCEF_NULL != m_manager ? m_manager->GetProfileKeyId() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CNotifier*
CAuthSettingsProfile::GetProfileNotifier( void )
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

const CORE::CEvent&
CAuthSettingsProfile::GetProfileChangedEvent( void ) const
{GUCEF_TRACE;

    return ProfileChangedEvent;
}

/*-------------------------------------------------------------------------*/

void
CAuthSettingsProfile::OnNotify( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_manager || notifier != static_cast< CORE::CNotifier* >( m_manager ) )
        return;
    if ( CORE::CINotifier::DestructionEvent == eventId )
    {
        m_manager = GUCEF_NULL;
        m_lastName.Clear();
        NotifyObservers( ProfileChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CAuthSettingsProfile::OnSettingsChanged( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the settings also change when a login was used last: only a new name is the profile's
    const CORE::CString name = GetDisplayName();
    if ( name != m_lastName )
    {
        m_lastName = name;
        NotifyObservers( ProfileChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthSettingsProfile::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthSettingsProfile";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
