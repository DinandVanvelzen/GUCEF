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

#include "gucefDRN_CDRNNamespaceResult.h"

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

CDRNNamespaceResult::CDRNNamespaceResult( void )
    : code( DRN_NSRESULT_OK )
    , status( DRN_RESSTATUS_OK )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNNamespaceResult::CDRNNamespaceResult( TDRNNamespaceResultCode resultCode, TDRNResourceStatus resultStatus, const CORE::CString& text )
    : code( resultCode )
    , status( resultStatus )
    , message( text )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceResult::IsOk( void ) const
{GUCEF_TRACE;

    return DRN_NSRESULT_OK == code || DRN_NSRESULT_UNCHANGED == code;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceResult::IsUnchanged( void ) const
{GUCEF_TRACE;

    return DRN_NSRESULT_UNCHANGED == code;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceResult::IsRetryable( void ) const
{GUCEF_TRACE;

    return DRN_NSRESULT_BUSY == code || DRN_NSRESULT_TIMEOUT == code || DRN_NSRESULT_UNAVAILABLE == code;
}

/*-------------------------------------------------------------------------*/

TDRNNamespaceResultCode
CDRNNamespaceResult::CodeFromStatus( TDRNResourceStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case DRN_RESSTATUS_OK:                  return DRN_NSRESULT_OK;
        case DRN_RESSTATUS_UNCHANGED:           return DRN_NSRESULT_UNCHANGED;
        case DRN_RESSTATUS_NOT_FOUND:           return DRN_NSRESULT_NOT_FOUND;
        case DRN_RESSTATUS_NOT_A_DIRECTORY:     return DRN_NSRESULT_NOT_A_DIRECTORY;
        case DRN_RESSTATUS_FORBIDDEN:           return DRN_NSRESULT_ACCESS_DENIED;
        case DRN_RESSTATUS_INSECURE_LINK:       return DRN_NSRESULT_ACCESS_DENIED;
        case DRN_RESSTATUS_INVALID_NAME:        return DRN_NSRESULT_INVALID_NAME;
        case DRN_RESSTATUS_NO_SUCH_PROVIDER:    return DRN_NSRESULT_NO_SUCH_SHARE;
        case DRN_RESSTATUS_BUSY:                return DRN_NSRESULT_BUSY;
        case DRN_RESSTATUS_TIMEOUT:             return DRN_NSRESULT_TIMEOUT;
        case DRN_RESSTATUS_CANCELLED:           return DRN_NSRESULT_UNAVAILABLE;
        case DRN_RESSTATUS_LINK_LOST:           return DRN_NSRESULT_UNAVAILABLE;
        case DRN_RESSTATUS_CLOSED:              return DRN_NSRESULT_UNAVAILABLE;
        case DRN_RESSTATUS_UNSUPPORTED:         return DRN_NSRESULT_UNSUPPORTED;
        case DRN_RESSTATUS_TOO_LARGE:           return DRN_NSRESULT_TOO_LARGE;
        case DRN_RESSTATUS_CURSOR_EXPIRED:      return DRN_NSRESULT_CURSOR_EXPIRED;
        case DRN_RESSTATUS_CHANGED:             return DRN_NSRESULT_CHANGED;
        case DRN_RESSTATUS_PROTOCOL_ERROR:      return DRN_NSRESULT_INTEGRITY;
        case DRN_RESSTATUS_DIGEST_MISMATCH:     return DRN_NSRESULT_INTEGRITY;
        case DRN_RESSTATUS_READ_ONLY:           return DRN_NSRESULT_READ_ONLY;
        default:                                return DRN_NSRESULT_IO_ERROR;
    }
}

/*-------------------------------------------------------------------------*/

const char*
CDRNNamespaceResult::CodeToString( TDRNNamespaceResultCode code )
{GUCEF_TRACE;

    switch ( code )
    {
        case DRN_NSRESULT_OK:               return "OK";
        case DRN_NSRESULT_UNCHANGED:        return "UNCHANGED";
        case DRN_NSRESULT_NOT_FOUND:        return "NOT_FOUND";
        case DRN_NSRESULT_NOT_A_DIRECTORY:  return "NOT_A_DIRECTORY";
        case DRN_NSRESULT_ACCESS_DENIED:    return "ACCESS_DENIED";
        case DRN_NSRESULT_INVALID_NAME:     return "INVALID_NAME";
        case DRN_NSRESULT_BUSY:             return "BUSY";
        case DRN_NSRESULT_TIMEOUT:          return "TIMEOUT";
        case DRN_NSRESULT_UNAVAILABLE:      return "UNAVAILABLE";
        case DRN_NSRESULT_UNSUPPORTED:      return "UNSUPPORTED";
        case DRN_NSRESULT_NO_SUCH_SHARE:    return "NO_SUCH_SHARE";
        case DRN_NSRESULT_TOO_LARGE:        return "TOO_LARGE";
        case DRN_NSRESULT_CURSOR_EXPIRED:   return "CURSOR_EXPIRED";
        case DRN_NSRESULT_CHANGED:          return "CHANGED";
        case DRN_NSRESULT_INTEGRITY:        return "INTEGRITY";
        case DRN_NSRESULT_WRONG_THREAD:     return "WRONG_THREAD";
        case DRN_NSRESULT_READ_ONLY:        return "READ_ONLY";
        case DRN_NSRESULT_IO_ERROR:         return "IO_ERROR";
        default:                            return "UNKNOWN";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
