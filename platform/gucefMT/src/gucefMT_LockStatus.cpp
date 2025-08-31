/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#include <cstring>

#include "gucefMT_LockStatus.h"  

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/
 
bool
LockStatusToLockSuccessStatusBool( TLockStatus threadStatus )
{GUCEF_TRACE;

    switch ( threadStatus )
    {
        case TLockStatus::LOCKSTATUS_OPERATION_SUCCESS:
        case TLockStatus::LOCKSTATUS_ABANDONED:        
        case TLockStatus::LOCKSTATUS_NOT_APPLICABLE:
        {
            return true;
        }
        default:
        case TLockStatus::LOCKSTATUS_OPERATION_FAILED:
        case TLockStatus::LOCKSTATUS_WAIT_TIMEOUT:
        case TLockStatus::LOCKSTATUS_UNDEFINED:
        {
            return false;
        }
    }
}

/*--------------------------------------------------------------------------*/

const char* 
LockStatusToLockStatusString( TLockStatus threadStatus )
{GUCEF_TRACE;

    switch ( threadStatus )
    {
        case TLockStatus::LOCKSTATUS_OPERATION_SUCCESS: return "OPERATION_SUCCESS";
        case TLockStatus::LOCKSTATUS_ABANDONED: return "ABANDONED";       
        case TLockStatus::LOCKSTATUS_NOT_APPLICABLE: return "NOT_APPLICABLE";         
        case TLockStatus::LOCKSTATUS_OPERATION_FAILED: return "OPERATION_FAILED";
        case TLockStatus::LOCKSTATUS_WAIT_TIMEOUT: return "WAIT_TIMEOUT";
        
        case TLockStatus::LOCKSTATUS_UNDEFINED:
        default:
        {
            return "<UNDEFINED>";
        }
    }    
}

/*-------------------------------------------------------------------------*/

TLockStatus 
LockStatusStringToLockStatus( const char* threadStatusStr )
{GUCEF_TRACE;

    if ( GUCEF_NULL == threadStatusStr )
        return LOCKSTATUS_UNDEFINED;

    if ( 0 == std::strcmp( threadStatusStr, "OPERATION_SUCCESS" ) )
        return LOCKSTATUS_OPERATION_SUCCESS;

    if ( 0 == std::strcmp( threadStatusStr, "WAIT_TIMEOUT" ) )
        return LOCKSTATUS_WAIT_TIMEOUT;

    if ( 0 == std::strcmp( threadStatusStr, "ABANDONED" ) )
        return LOCKSTATUS_ABANDONED;

    if ( 0 == std::strcmp( threadStatusStr, "OPERATION_FAILED" ) )
        return LOCKSTATUS_OPERATION_FAILED;

    if ( 0 == std::strcmp( threadStatusStr, "NOT_APPLICABLE" ) )
        return LOCKSTATUS_NOT_APPLICABLE;

    if ( 0 == std::strcmp( threadStatusStr, "UNDEFINED" ) || 0 == std::strcmp( threadStatusStr, "<UNDEFINED>" ) )
        return LOCKSTATUS_UNDEFINED;

    return LOCKSTATUS_UNDEFINED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
