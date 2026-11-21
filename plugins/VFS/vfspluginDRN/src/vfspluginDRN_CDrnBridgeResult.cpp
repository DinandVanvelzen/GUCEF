/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#include "vfspluginDRN_CDrnBridgeResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeResult::CDrnBridgeResult( void )
    : fileSystemResult( CORE::FILESYSTEMACCESS_SUCCESS )
    , code( DRN::DRN_NSRESULT_OK )
    , status( DRN::DRN_RESSTATUS_OK )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult::CDrnBridgeResult( DRN::TDRNNamespaceResultCode resultCode, DRN::TDRNResourceStatus resultStatus, const CORE::CString& text )
    : fileSystemResult( MapToFileSystemResult( resultCode, resultStatus ) )
    , code( resultCode )
    , status( resultStatus )
    , message( text )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeResult::IsOk( void ) const
{GUCEF_TRACE;

    return DRN::DRN_NSRESULT_OK == code || DRN::DRN_NSRESULT_UNCHANGED == code;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeResult::IsUnreachable( void ) const
{GUCEF_TRACE;

    // a call that answered "not yet available" met a link that may be fine: it is not an outage
    if ( CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == fileSystemResult )
        return false;
    return DRN::DRN_NSRESULT_UNAVAILABLE == code || DRN::DRN_NSRESULT_TIMEOUT == code || DRN::DRN_NSRESULT_WRONG_THREAD == code;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeResult::IsNotYetAvailable( void ) const
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == fileSystemResult;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeResult::IsChanged( void ) const
{GUCEF_TRACE;

    return DRN::DRN_NSRESULT_CHANGED == code;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeResult::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString text = DRN::CDRNNamespaceResult::CodeToString( code );
    if ( !message.IsNULLOrEmpty() )
        text += ": " + message;
    return text;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeResult::FromNamespaceResult( const DRN::CDRNNamespaceResult& result )
{GUCEF_TRACE;

    return CDrnBridgeResult( result.code, result.status, result.message );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeResult::FromTransferResult( const DRN::CDRNResourceTransferResult& result )
{GUCEF_TRACE;

    CORE::CString text = result.message;
    if ( text.IsNULLOrEmpty() && !result.IsOk() )
        text = DRN::CDRNResourceTransferResult::StatusToString( result.status );
    return CDrnBridgeResult( DRN::CDRNNamespaceResult::CodeFromStatus( result.status ), result.status, text );
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeResult::NotYetAvailable( const CORE::CString& text )
{GUCEF_TRACE;

    CDrnBridgeResult result( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_CLOSED, text );
    result.fileSystemResult = CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE;
    return result;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeResult
CDrnBridgeResult::FromCallResult( const VFS::CVfsCallResult& result )
{GUCEF_TRACE;

    CDrnBridgeResult converted;
    switch ( result.result )
    {
        case CORE::FILESYSTEMACCESS_SUCCESS:                        return converted;
        case CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE:       return NotYetAvailable( result.message );
        case CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING:      converted = CDrnBridgeResult( DRN::DRN_NSRESULT_UNAVAILABLE, DRN::DRN_RESSTATUS_LINK_LOST, result.message ); break;
        case CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED:        converted = CDrnBridgeResult( DRN::DRN_NSRESULT_CHANGED, DRN::DRN_RESSTATUS_CHANGED, result.message ); break;
        case CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER:    converted = CDrnBridgeResult( DRN::DRN_NSRESULT_BUSY, DRN::DRN_RESSTATUS_BUSY, result.message ); break;
        case CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT:               converted = CDrnBridgeResult( DRN::DRN_NSRESULT_INTEGRITY, DRN::DRN_RESSTATUS_DIGEST_MISMATCH, result.message ); break;
        case CORE::FILESYSTEMACCESS_REMOTE_PROTOCOL_ERROR:          converted = CDrnBridgeResult( DRN::DRN_NSRESULT_INTEGRITY, DRN::DRN_RESSTATUS_PROTOCOL_ERROR, result.message ); break;
        case CORE::FILESYSTEMACCESS_NOT_FOUND:                      converted = CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_FOUND, DRN::DRN_RESSTATUS_NOT_FOUND, result.message ); break;
        case CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY:                converted = CDrnBridgeResult( DRN::DRN_NSRESULT_NOT_A_DIRECTORY, DRN::DRN_RESSTATUS_NOT_A_DIRECTORY, result.message ); break;
        case CORE::FILESYSTEMACCESS_ACCESS_DENIED:                  converted = CDrnBridgeResult( DRN::DRN_NSRESULT_ACCESS_DENIED, DRN::DRN_RESSTATUS_FORBIDDEN, result.message ); break;
        case CORE::FILESYSTEMACCESS_INVALID_PATH:                   converted = CDrnBridgeResult( DRN::DRN_NSRESULT_INVALID_NAME, DRN::DRN_RESSTATUS_INVALID_NAME, result.message ); break;
        case CORE::FILESYSTEMACCESS_READ_ONLY:                      converted = CDrnBridgeResult( DRN::DRN_NSRESULT_READ_ONLY, DRN::DRN_RESSTATUS_READ_ONLY, result.message ); break;
        case CORE::FILESYSTEMACCESS_TIMEOUT:                        converted = CDrnBridgeResult( DRN::DRN_NSRESULT_TIMEOUT, DRN::DRN_RESSTATUS_TIMEOUT, result.message ); break;
        case CORE::FILESYSTEMACCESS_NOT_SUPPORTED:                  converted = CDrnBridgeResult( DRN::DRN_NSRESULT_UNSUPPORTED, DRN::DRN_RESSTATUS_UNSUPPORTED, result.message ); break;
        case CORE::FILESYSTEMACCESS_TOO_LONG:                       converted = CDrnBridgeResult( DRN::DRN_NSRESULT_TOO_LARGE, DRN::DRN_RESSTATUS_TOO_LARGE, result.message ); break;
        default:                                                    converted = CDrnBridgeResult( DRN::DRN_NSRESULT_IO_ERROR, DRN::DRN_RESSTATUS_IO_ERROR, result.message ); break;
    }

    // the value of the generic layer is the one that was meant, whatever the nearest code is
    converted.fileSystemResult = result.result;
    return converted;
}

/*-------------------------------------------------------------------------*/

VFS::CVfsCallResult
CDrnBridgeResult::ToCallResult( void ) const
{GUCEF_TRACE;

    return VFS::CVfsCallResult( fileSystemResult, message );
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CDrnBridgeResult::MapToFileSystemResult( DRN::TDRNNamespaceResultCode code, DRN::TDRNResourceStatus status )
{GUCEF_TRACE;

    switch ( code )
    {
        case DRN::DRN_NSRESULT_OK:              return CORE::FILESYSTEMACCESS_SUCCESS;
        case DRN::DRN_NSRESULT_UNCHANGED:       return CORE::FILESYSTEMACCESS_SUCCESS;
        case DRN::DRN_NSRESULT_NOT_FOUND:       return CORE::FILESYSTEMACCESS_NOT_FOUND;
        case DRN::DRN_NSRESULT_NO_SUCH_SHARE:   return CORE::FILESYSTEMACCESS_NOT_FOUND;
        case DRN::DRN_NSRESULT_NOT_A_DIRECTORY: return CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY;
        case DRN::DRN_NSRESULT_ACCESS_DENIED:   return CORE::FILESYSTEMACCESS_ACCESS_DENIED;
        case DRN::DRN_NSRESULT_INVALID_NAME:    return CORE::FILESYSTEMACCESS_INVALID_PATH;
        case DRN::DRN_NSRESULT_READ_ONLY:       return CORE::FILESYSTEMACCESS_READ_ONLY;
        case DRN::DRN_NSRESULT_BUSY:            return CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER;
        case DRN::DRN_NSRESULT_UNAVAILABLE:     return CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING;
        case DRN::DRN_NSRESULT_CHANGED:         return CORE::FILESYSTEMACCESS_REMOTE_RESOURCE_CHANGED;
        case DRN::DRN_NSRESULT_INTEGRITY:       return DRN::DRN_RESSTATUS_DIGEST_MISMATCH == status ? CORE::FILESYSTEMACCESS_RESOURCE_CORRUPT : CORE::FILESYSTEMACCESS_REMOTE_PROTOCOL_ERROR;
        case DRN::DRN_NSRESULT_TIMEOUT:         return CORE::FILESYSTEMACCESS_TIMEOUT;
        case DRN::DRN_NSRESULT_UNSUPPORTED:     return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
        case DRN::DRN_NSRESULT_TOO_LARGE:       return CORE::FILESYSTEMACCESS_TOO_LONG;
        default:                                return CORE::FILESYSTEMACCESS_FAILED;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
