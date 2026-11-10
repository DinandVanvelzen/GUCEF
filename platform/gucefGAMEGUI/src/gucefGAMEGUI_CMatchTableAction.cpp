/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
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

#include "gucefGAMEGUI_CMatchTableAction.h"

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

CMatchTableAction::CMatchTableAction( void ) : CORE::CICloneable(), text() {GUCEF_TRACE;}
CMatchTableAction::CMatchTableAction( const CORE::CString& actionText ) : CORE::CICloneable(), text( actionText ) {GUCEF_TRACE;}
CMatchTableAction::CMatchTableAction( const CMatchTableAction& src ) : CORE::CICloneable( src ), text( src.text ) {GUCEF_TRACE;}
CMatchTableAction::~CMatchTableAction() {GUCEF_TRACE;}

/*-------------------------------------------------------------------------*/

CMatchTableAction&
CMatchTableAction::operator=( const CMatchTableAction& src )
{GUCEF_TRACE;

    if ( &src != this )
        text = src.text;
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMatchTableAction::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMatchTableAction( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMatchTableAction::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMatchTableAction::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::GAMEGUI::CMatchTableAction";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
