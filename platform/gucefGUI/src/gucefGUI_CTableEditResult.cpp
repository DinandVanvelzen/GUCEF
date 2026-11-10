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

#include "gucefGUI_CTableEditResult.h"

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

CTableEditResult::CTableEditResult( void )
    : status( STATUS_ACCEPTED )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTableEditResult::CTableEditResult( EStatus resultStatus, const CString& resultMessage )
    : status( resultStatus )
    , message( resultMessage )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTableEditResult
CTableEditResult::Accepted( void )
{GUCEF_TRACE;

    return CTableEditResult( STATUS_ACCEPTED, CString::Empty );
}

/*-------------------------------------------------------------------------*/

CTableEditResult
CTableEditResult::Refused( const CString& reason )
{GUCEF_TRACE;

    return CTableEditResult( STATUS_REFUSED, reason );
}

/*-------------------------------------------------------------------------*/

CTableEditResult
CTableEditResult::ReadOnly( void )
{GUCEF_TRACE;

    return CTableEditResult( STATUS_READ_ONLY, "The cell is read only" );
}

/*-------------------------------------------------------------------------*/

bool
CTableEditResult::IsAccepted( void ) const
{GUCEF_TRACE;

    return STATUS_ACCEPTED == status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
