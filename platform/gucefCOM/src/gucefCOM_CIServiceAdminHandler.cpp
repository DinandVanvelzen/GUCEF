/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#include "gucefCOM_CIServiceAdminHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
ServiceAdminStatusToString( TServiceAdminStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case SERVICEADMIN_OK: return "OK";
        case SERVICEADMIN_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case SERVICEADMIN_NOT_IMPLEMENTED: return "NOT_IMPLEMENTED";
        case SERVICEADMIN_FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

CIServiceAdminHandler::CIServiceAdminHandler( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIServiceAdminHandler::~CIServiceAdminHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
