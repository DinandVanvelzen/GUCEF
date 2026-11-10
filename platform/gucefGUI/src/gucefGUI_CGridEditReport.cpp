/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include "gucefGUI_CGridEditReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridEditReport::CGridEditReport( void )
    : operation( OPERATION_EDIT )
    , row( -1 )
    , column( -1 )
    , cellsApplied( 0 )
    , cellsRefused( 0 )
    , cellsReadOnly( 0 )
    , cellsSkipped( 0 )
    , refused( false )
    , message()
    , problemRow( -1 )
    , problemColumn( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridEditReport::CGridEditReport( EOperation editOperation )
    : operation( editOperation )
    , row( -1 )
    , column( -1 )
    , cellsApplied( 0 )
    , cellsRefused( 0 )
    , cellsReadOnly( 0 )
    , cellsSkipped( 0 )
    , refused( false )
    , message()
    , problemRow( -1 )
    , problemColumn( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CGridEditReport::GetCellCount( void ) const
{GUCEF_TRACE;

    return cellsApplied + cellsRefused + cellsReadOnly + cellsSkipped;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
