/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include "gucefDRN_CDRNResourceTransferResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNResourceTransferResult::CDRNResourceTransferResult( void )
    : status( DRN_RESSTATUS_OK )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferResult::CDRNResourceTransferResult( TDRNResourceStatus statusCode, const CORE::CString& text )
    : status( statusCode )
    , message( text )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransferResult::IsOk( void ) const
{GUCEF_TRACE;

    return DRN_RESSTATUS_OK == status;
}

/*-------------------------------------------------------------------------*/

const char*
CDRNResourceTransferResult::StatusToString( TDRNResourceStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case DRN_RESSTATUS_OK:                  return "OK";
        case DRN_RESSTATUS_NOT_FOUND:           return "NOT_FOUND";
        case DRN_RESSTATUS_FORBIDDEN:           return "FORBIDDEN";
        case DRN_RESSTATUS_INVALID_NAME:        return "INVALID_NAME";
        case DRN_RESSTATUS_NO_SUCH_PROVIDER:    return "NO_SUCH_PROVIDER";
        case DRN_RESSTATUS_BUSY:                return "BUSY";
        case DRN_RESSTATUS_CHANGED:             return "CHANGED";
        case DRN_RESSTATUS_OUT_OF_RANGE:        return "OUT_OF_RANGE";
        case DRN_RESSTATUS_IO_ERROR:            return "IO_ERROR";
        case DRN_RESSTATUS_DIGEST_MISMATCH:     return "DIGEST_MISMATCH";
        case DRN_RESSTATUS_CANCELLED:           return "CANCELLED";
        case DRN_RESSTATUS_LINK_LOST:           return "LINK_LOST";
        case DRN_RESSTATUS_UNSUPPORTED:         return "UNSUPPORTED";
        case DRN_RESSTATUS_INSECURE_LINK:       return "INSECURE_LINK";
        case DRN_RESSTATUS_TIMEOUT:             return "TIMEOUT";
        case DRN_RESSTATUS_LOCAL_IO_ERROR:      return "LOCAL_IO_ERROR";
        case DRN_RESSTATUS_PROTOCOL_ERROR:      return "PROTOCOL_ERROR";
        case DRN_RESSTATUS_TOO_LARGE:           return "TOO_LARGE";
        case DRN_RESSTATUS_CLOSED:              return "CLOSED";
        case DRN_RESSTATUS_NO_TREE:             return "NO_TREE";
        case DRN_RESSTATUS_UNCHANGED:           return "UNCHANGED";
        case DRN_RESSTATUS_CURSOR_EXPIRED:      return "CURSOR_EXPIRED";
        case DRN_RESSTATUS_NOT_A_DIRECTORY:     return "NOT_A_DIRECTORY";
        case DRN_RESSTATUS_READ_ONLY:           return "READ_ONLY";
        case DRN_RESSTATUS_CONFLICT:            return "CONFLICT";
        case DRN_RESSTATUS_ALREADY_EXISTS:      return "ALREADY_EXISTS";
        case DRN_RESSTATUS_NOT_EMPTY:           return "NOT_EMPTY";
        case DRN_RESSTATUS_NO_SPACE:            return "NO_SPACE";
        default:                                return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CDRNResourceTransferResult::StatusFromWire( UInt32 value )
{GUCEF_TRACE;

    if ( value >= (UInt32) DRN_RESSTATUS_COUNT )
        return DRN_RESSTATUS_IO_ERROR;
    return (TDRNResourceStatus) value;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransferResult::IsRetryable( TDRNResourceStatus status )
{GUCEF_TRACE;

    return DRN_RESSTATUS_BUSY == status || DRN_RESSTATUS_LINK_LOST == status || DRN_RESSTATUS_TIMEOUT == status || DRN_RESSTATUS_CHANGED == status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
