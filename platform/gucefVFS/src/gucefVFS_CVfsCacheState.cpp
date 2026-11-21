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

#include "gucefVFS_CVfsCacheState.h"

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

CVfsCacheState::CVfsCacheState( void )
    : cached( false )
    , complete( false )
    , pinned( false )
    , superseded( false )
    , staleRevision( false )
    , digestVerified( false )
    , fileSize( 0 )
    , bytesHeld( 0 )
    , revision()
    , digest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsCacheState::CVfsCacheState( const CVfsCacheState& src )
    : cached( src.cached )
    , complete( src.complete )
    , pinned( src.pinned )
    , superseded( src.superseded )
    , staleRevision( src.staleRevision )
    , digestVerified( src.digestVerified )
    , fileSize( src.fileSize )
    , bytesHeld( src.bytesHeld )
    , revision( src.revision )
    , digest( src.digest )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsCacheState&
CVfsCacheState::operator=( const CVfsCacheState& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        cached = src.cached;
        complete = src.complete;
        pinned = src.pinned;
        superseded = src.superseded;
        staleRevision = src.staleRevision;
        digestVerified = src.digestVerified;
        fileSize = src.fileSize;
        bytesHeld = src.bytesHeld;
        revision = src.revision;
        digest = src.digest;
    }
    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
