/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#include "gucefVFS_CVfsCallResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsCallResult::CVfsCallResult( void )
    : result( CORE::FILESYSTEMACCESS_SUCCESS )
    , message()
    , fetchStarted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsCallResult::CVfsCallResult( CORE::TFileSystemAccessResult callResult, const CORE::CString& callMessage, bool callFetchStarted )
    : result( callResult )
    , message( callMessage )
    , fetchStarted( callFetchStarted )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsCallResult::CVfsCallResult( const CVfsCallResult& src )
    : result( src.result )
    , message( src.message )
    , fetchStarted( src.fetchStarted )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsCallResult&
CVfsCallResult::operator=( const CVfsCallResult& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        result = src.result;
        message = src.message;
        fetchStarted = src.fetchStarted;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CVfsCallResult::IsSuccess( void ) const
{GUCEF_TRACE;

    return CORE::FileSystemAccessSucceeded( result );
}

/*-------------------------------------------------------------------------*/

bool
CVfsCallResult::IsTemporary( void ) const
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_REMOTE_NOT_YET_AVAILABLE == result ||
           CORE::FILESYSTEMACCESS_REMOTE_CONNECTION_PENDING == result ||
           CORE::FILESYSTEMACCESS_REMOTE_CAPLIMIT_RETRY_LATER == result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsCallResult::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString text = CORE::FileSystemAccessResultToString( result );
    if ( !message.IsNULLOrEmpty() )
        text += ": " + message;
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
