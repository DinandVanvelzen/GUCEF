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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUTH_CIAuthProvider.h"

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

const CORE::CEvent CIAuthProvider::ActionRequiredEvent = "GUCEF::AUTH::CIAuthProvider::ActionRequiredEvent";
const CORE::CEvent CIAuthProvider::LoginSucceededEvent = "GUCEF::AUTH::CIAuthProvider::LoginSucceededEvent";
const CORE::CEvent CIAuthProvider::LoginFailedEvent = "GUCEF::AUTH::CIAuthProvider::LoginFailedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIAuthProvider::RegisterEvents( void )
{GUCEF_TRACE;

    ActionRequiredEvent.Initialize();
    LoginSucceededEvent.Initialize();
    LoginFailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIAuthProvider::CIAuthProvider( void )
    : CORE::CTSGNotifier()
    , CORE::CIConfigurable()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIAuthProvider::~CIAuthProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIAuthProvider::Refresh( const CAccountRef& account, const CSecretString& refreshToken, CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIAuthProvider::Logout( const CAccountRef& account, const CSecretString& refreshToken )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIAuthProvider::Verify( const CAuthProof& proof, CAuthResult& result )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIAuthProvider::GetUnavailability( CAuthUnavailableProvider& unavailable ) const
{GUCEF_TRACE;

    unavailable.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIAuthProvider::GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const
{GUCEF_TRACE;

    displayName.Clear();
    isRunning = false;
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIAuthProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CIAuthProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CIAuthProvider::CompleteLogin( CORE::CFutureResultSource& source, const CAuthResult& result )
{GUCEF_TRACE;

    if ( !result.IsValid() )
        return FailLogin( source, AUTHFAIL_INVALID_RESPONSE, "The provider gave no valid account" );

    CAuthResult stamped( result );
    stamped.providerName = GetProviderName();
    if ( !source.Complete( stamped.Clone(), true ) )
        return false;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthProvider: " << stamped.providerName << " logged in " << stamped.accountRef.ToString() );
    NotifyObserversFromThread( LoginSucceededEvent, &stamped );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIAuthProvider::FailLogin( CORE::CFutureResultSource& source, TAuthFailureReason reason, const CORE::CString& message )
{GUCEF_TRACE;

    bool ended = false;
    if ( AUTHFAIL_CANCELLED == reason )
    {
        ended = source.Cancel();
    }
    else
    {
        CORE::CString extraInfo = AuthFailureReasonToString( reason );
        extraInfo += ": ";
        extraInfo += message;
        ended = source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, extraInfo );
    }
    if ( !ended )
        return false;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AuthProvider: " << GetProviderName() << " login failed (" << AuthFailureReasonToString( reason ) << "): " << message );
    CAuthFailure failure( GetProviderName(), reason, message );
    NotifyObserversFromThread( LoginFailedEvent, &failure );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CIAuthProvider::RaiseActionRequired( const CAuthActionRequired& action )
{GUCEF_TRACE;

    CAuthActionRequired stamped( action );
    stamped.providerName = GetProviderName();
    NotifyObserversFromThread( ActionRequiredEvent, &stamped );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
