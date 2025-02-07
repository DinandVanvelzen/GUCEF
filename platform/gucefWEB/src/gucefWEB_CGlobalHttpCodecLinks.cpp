/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CSTDCODECPLUGINMANAGER_H
#include "CStdCodecPluginManager.h"
#define GUCEF_CORE_CSTDCODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CSTDCODECPLUGINMANAGER_H ? */

#include "gucefWEB_CGlobalHttpCodecLinks.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CGlobalHttpCodecLinks::MimeCodecsChangedEvent = "GUCEF::WEB::CGlobalHttpCodecLinks::MimeCodecsChangedEvent";
const CORE::CEvent CGlobalHttpCodecLinks::EncodingCodecsChangedEvent = "GUCEF::WEB::CGlobalHttpCodecLinks::EncodingCodecsChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGlobalHttpCodecLinks::CGlobalHttpCodecLinks( void )
    : CHttpCodecLinks()
    , CORE::CTSGNotifier()
{GUCEF_TRACE;

    RegisterEvents();
    
    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
    CORE::CGUCEFApplication& app = CORE::CCoreGlobal::Instance()->GetApplication();
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    CORE::CStdCodecPluginManager& codecPluginMngr = CORE::CCoreGlobal::Instance()->GetStdCodecPluginManager();

    TEventCallback callback( this, &CGlobalHttpCodecLinks::OnSomeShutdownEvent );
    SubscribeTo( &app, CORE::CGUCEFApplication::AppShutdownEvent, callback );

    TEventCallback callback2( this, &CGlobalHttpCodecLinks::OnEventThatMightHaveChangedCodecs );
    SubscribeTo( &configStore, configStore.GlobalBootstrapConfigLoadCompletedEvent, callback2 );

    TEventCallback callback3( this, &CGlobalHttpCodecLinks::OnEventThatMightHaveChangedCodecs );
    SubscribeTo( &configStore, configStore.GlobalConfigLoadCompletedEvent, callback3 );

    TEventCallback callback4( this, &CGlobalHttpCodecLinks::OnEventThatMightHaveChangedCodecs );
    SubscribeTo( &app, app.FirstCycleEvent, callback4 );

    TEventCallback callback5( this, &CGlobalHttpCodecLinks::OnEventThatMightHaveChangedEncodeCodecs );
    SubscribeTo( &codecPluginMngr, codecPluginMngr.StdCodecRegisteredEvent, callback5 );

    TEventCallback callback6( this, &CGlobalHttpCodecLinks::OnEventThatMightHaveChangedEncodeCodecs );
    SubscribeTo( &codecPluginMngr, codecPluginMngr.StdCodecUnregisteredEvent, callback6 );

    TEventCallback callback7( this, &CGlobalHttpCodecLinks::OnSomeShutdownEvent );
    SubscribeTo( &pluginControl, CORE::CPluginControl::UnregisterOfAllPluginsStartedEvent, callback7 );
}

/*-------------------------------------------------------------------------*/

CGlobalHttpCodecLinks::~CGlobalHttpCodecLinks()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CGlobalHttpCodecLinks::RegisterEvents( void )
{GUCEF_TRACE;

    MimeCodecsChangedEvent.Initialize();
    EncodingCodecsChangedEvent.Initialize();    
}

/*-------------------------------------------------------------------------*/

void 
CGlobalHttpCodecLinks::OnEventThatMightHaveChangedMimeCodecs( CORE::CNotifier* notifier    ,
                                                              const CORE::CEvent& eventid  ,
                                                              CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    InitMimeCodecLinks();
}

/*-------------------------------------------------------------------------*/

void 
CGlobalHttpCodecLinks::OnEventThatMightHaveChangedEncodeCodecs( CORE::CNotifier* notifier    ,
                                                                const CORE::CEvent& eventid  ,
                                                                CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    InitEncodingCodecLinks();
}

/*-------------------------------------------------------------------------*/

void 
CGlobalHttpCodecLinks::OnEventThatMightHaveChangedCodecs( CORE::CNotifier* notifier    ,
                                                          const CORE::CEvent& eventid  ,
                                                          CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    InitMimeCodecLinks();
    InitEncodingCodecLinks();
}

/*-------------------------------------------------------------------------*/

void 
CGlobalHttpCodecLinks::OnSomeShutdownEvent( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventid  ,
                                            CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    RemoveEncodingCodecLinks();
    RemoveMimeCodecLinks(); 
}

/*-------------------------------------------------------------------------*/

const CString& 
CGlobalHttpCodecLinks::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CGlobalHttpCodecLinks";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CGlobalHttpCodecLinks::InitMimeCodecLinks( void ) 
{GUCEF_TRACE;

    if ( CHttpCodecLinks::InitMimeCodecLinks() )
    {
        NotifyObserversFromThread( MimeCodecsChangedEvent );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGlobalHttpCodecLinks::InitEncodingCodecLinks( void ) 
{GUCEF_TRACE;

    if ( CHttpCodecLinks::InitEncodingCodecLinks() )
    {
        NotifyObserversFromThread( EncodingCodecsChangedEvent );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CGlobalHttpCodecLinks::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return CTSGNotifier::Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CGlobalHttpCodecLinks::Unlock( void ) const
{GUCEF_TRACE;

    return CTSGNotifier::Unlock();
}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CGlobalHttpCodecLinks::AsLockable( void ) const
{GUCEF_TRACE;

    return CTSGNotifier::AsLockable();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
