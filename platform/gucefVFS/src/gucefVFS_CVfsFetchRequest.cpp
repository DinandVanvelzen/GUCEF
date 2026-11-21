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

#include "gucefVFS_CVfsFetchRequest.h"

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

CVfsFetchRequest::CVfsFetchRequest( void )
    : files()
    , directories()
    , recursive( false )
    , maxDirectories( 1000 )
    , pin( false )
    , verifyDigest( true )
    , maxParallelFiles( 4 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchRequest::CVfsFetchRequest( const CVfsFetchRequest& src )
    : files( src.files )
    , directories( src.directories )
    , recursive( src.recursive )
    , maxDirectories( src.maxDirectories )
    , pin( src.pin )
    , verifyDigest( src.verifyDigest )
    , maxParallelFiles( src.maxParallelFiles )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsFetchRequest&
CVfsFetchRequest::operator=( const CVfsFetchRequest& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        files = src.files;
        directories = src.directories;
        recursive = src.recursive;
        maxDirectories = src.maxDirectories;
        pin = src.pin;
        verifyDigest = src.verifyDigest;
        maxParallelFiles = src.maxParallelFiles;
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
