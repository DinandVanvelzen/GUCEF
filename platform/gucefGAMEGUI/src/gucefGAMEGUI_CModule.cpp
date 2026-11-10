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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GAMEGUI_CGAMEGUIGLOBAL_H
#include "gucefGAMEGUI_CGameGuiGlobal.h"
#define GUCEF_GAMEGUI_CGAMEGUIGLOBAL_H
#endif /* GUCEF_GAMEGUI_CGAMEGUIGLOBAL_H ? */

#include "gucefGAMEGUI_CModule.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CModule::Load( void )
{GUCEF_TRACE;

    CGameGuiGlobal::Instance();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CModule::Unload( void )
{GUCEF_TRACE;

    CGameGuiGlobal::Deinstance();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
