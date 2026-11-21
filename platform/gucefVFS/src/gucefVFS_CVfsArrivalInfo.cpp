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

#include "gucefVFS_CVfsArrivalInfo.h"

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

CVfsArrivalInfo::CVfsArrivalInfo( void )
    : CORE::CICloneable()
    , kind( KIND_FILE )
    , path()
    , revision()
    , oldRevision()
    , firstByte( 0 )
    , byteCount( 0 )
    , fileComplete( false )
    , result( CORE::FILESYSTEMACCESS_SUCCESS )
    , message()
    , jobKind( FETCHJOB_STAT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsArrivalInfo::CVfsArrivalInfo( EKind infoKind, const CORE::CString& infoPath )
    : CORE::CICloneable()
    , kind( infoKind )
    , path( infoPath )
    , revision()
    , oldRevision()
    , firstByte( 0 )
    , byteCount( 0 )
    , fileComplete( false )
    , result( CORE::FILESYSTEMACCESS_SUCCESS )
    , message()
    , jobKind( FETCHJOB_STAT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsArrivalInfo::CVfsArrivalInfo( const CVfsArrivalInfo& src )
    : CORE::CICloneable( src )
    , kind( src.kind )
    , path( src.path )
    , revision( src.revision )
    , oldRevision( src.oldRevision )
    , firstByte( src.firstByte )
    , byteCount( src.byteCount )
    , fileComplete( src.fileComplete )
    , result( src.result )
    , message( src.message )
    , jobKind( src.jobKind )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsArrivalInfo::~CVfsArrivalInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsArrivalInfo&
CVfsArrivalInfo::operator=( const CVfsArrivalInfo& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CICloneable::operator=( src );
        kind = src.kind;
        path = src.path;
        revision = src.revision;
        oldRevision = src.oldRevision;
        firstByte = src.firstByte;
        byteCount = src.byteCount;
        fileComplete = src.fileComplete;
        result = src.result;
        message = src.message;
        jobKind = src.jobKind;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CVfsArrivalInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CVfsArrivalInfo( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
