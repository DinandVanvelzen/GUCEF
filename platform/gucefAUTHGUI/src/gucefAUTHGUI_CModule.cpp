/*
 *  gucefAUTHGUI: GUCEF module providing the user interface for logging in with external identities and the account page of settings
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

#ifndef GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H
#include "gucefAUTHGUI_CAuthGuiGlobal.h"
#define GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H
#endif /* GUCEF_AUTHGUI_CAUTHGUIGLOBAL_H ? */

#include "gucefAUTHGUI_CModule.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CModule::Load( void )
{GUCEF_TRACE;

    CAuthGuiGlobal::Instance();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CModule::Unload( void )
{GUCEF_TRACE;

    CAuthGuiGlobal::Deinstance();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTHGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
