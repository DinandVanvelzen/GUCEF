/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#include "authpluginANDROID_CAndroidRedirectReceiver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAndroidRedirectReceiver::CAndroidRedirectReceiver( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidRedirectReceiver::~CAndroidRedirectReceiver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEngineRedirectReceiver::CEngineRedirectReceiver( void )
    : CAndroidRedirectReceiver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEngineRedirectReceiver::~CEngineRedirectReceiver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEngineRedirectReceiver::OnAuthRedirect( const CORE::CString& redirectUrl )
{GUCEF_TRACE;

    AUTH::IAuthFlowEngine* engine = AUTH::CAuthGlobal::Instance()->GetFlowEngine();
    if ( GUCEF_NULL == engine )
        return false;
    return engine->SubmitRedirectUrl( redirectUrl );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
