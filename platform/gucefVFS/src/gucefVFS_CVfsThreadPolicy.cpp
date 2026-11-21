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

#include "gucefVFS_CVfsThreadPolicy.h"

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

CVfsThreadPolicy::CVfsThreadPolicy( void )
    : missAction( MISSACTION_WAIT )
    , serveStaleMetadata( true )
    , openRequiresCompleteFile( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicy::CVfsThreadPolicy( EMissAction action )
    : missAction( action )
    , serveStaleMetadata( true )
    , openRequiresCompleteFile( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicy::CVfsThreadPolicy( const CVfsThreadPolicy& src )
    : missAction( src.missAction )
    , serveStaleMetadata( src.serveStaleMetadata )
    , openRequiresCompleteFile( src.openRequiresCompleteFile )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicy&
CVfsThreadPolicy::operator=( const CVfsThreadPolicy& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        missAction = src.missAction;
        serveStaleMetadata = src.serveStaleMetadata;
        openRequiresCompleteFile = src.openRequiresCompleteFile;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CVfsThreadPolicy::NeverWaits( void ) const
{GUCEF_TRACE;

    return MISSACTION_WAIT != missAction;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
