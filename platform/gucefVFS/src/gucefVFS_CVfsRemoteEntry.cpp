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

#include "gucefVFS_CVfsRemoteEntry.h"

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

CVfsRemoteEntry::CVfsRemoteEntry( void )
    : name()
    , exists( false )
    , isDirectory( false )
    , size( 0 )
    , revision()
    , digest()
    , modifiedUtcMs( 0 )
    , attributesKnown( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsRemoteEntry::CVfsRemoteEntry( const CVfsRemoteEntry& src )
    : name( src.name )
    , exists( src.exists )
    , isDirectory( src.isDirectory )
    , size( src.size )
    , revision( src.revision )
    , digest( src.digest )
    , modifiedUtcMs( src.modifiedUtcMs )
    , attributesKnown( src.attributesKnown )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsRemoteEntry&
CVfsRemoteEntry::operator=( const CVfsRemoteEntry& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        name = src.name;
        exists = src.exists;
        isDirectory = src.isDirectory;
        size = src.size;
        revision = src.revision;
        digest = src.digest;
        modifiedUtcMs = src.modifiedUtcMs;
        attributesKnown = src.attributesKnown;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteEntry::IsSameAs( const CVfsRemoteEntry& other ) const
{GUCEF_TRACE;

    // an entry whose attributes are not known on either side says no more than that the name is there and what kind it is
    if ( !attributesKnown || !other.attributesKnown )
        return name == other.name && exists == other.exists && isDirectory == other.isDirectory;

    return name == other.name                 &&
           exists == other.exists             &&
           isDirectory == other.isDirectory   &&
           size == other.size                 &&
           revision == other.revision         &&
           digest.algorithm == other.digest.algorithm &&
           digest.value == other.digest.value &&
           modifiedUtcMs == other.modifiedUtcMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
