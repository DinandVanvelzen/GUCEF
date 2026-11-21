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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefDRN_CDRNResourceInfo.h"

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

CDRNResourceInfo::CDRNResourceInfo( void )
    : name()
    , size( 0 )
    , modifiedUtcMs( 0 )
    , revision()
    , hasSha256( false )
    , hasTreeRoot( false )
{GUCEF_TRACE;

    memset( sha256, 0, sizeof( sha256 ) );
    memset( treeRoot, 0, sizeof( treeRoot ) );
}

/*-------------------------------------------------------------------------*/

CDRNResourceInfo::CDRNResourceInfo( const CDRNResourceInfo& src )
    : name( src.name )
    , size( src.size )
    , modifiedUtcMs( src.modifiedUtcMs )
    , revision( src.revision )
    , hasSha256( src.hasSha256 )
    , hasTreeRoot( src.hasTreeRoot )
{GUCEF_TRACE;

    memcpy( sha256, src.sha256, sizeof( sha256 ) );
    memcpy( treeRoot, src.treeRoot, sizeof( treeRoot ) );
}

/*-------------------------------------------------------------------------*/

CDRNResourceInfo&
CDRNResourceInfo::operator=( const CDRNResourceInfo& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        name = src.name;
        size = src.size;
        modifiedUtcMs = src.modifiedUtcMs;
        revision = src.revision;
        hasSha256 = src.hasSha256;
        hasTreeRoot = src.hasTreeRoot;
        memcpy( sha256, src.sha256, sizeof( sha256 ) );
        memcpy( treeRoot, src.treeRoot, sizeof( treeRoot ) );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceInfo::Clear( void )
{GUCEF_TRACE;

    *this = CDRNResourceInfo();
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceInfo::ChunkCountFor( UInt64 resourceSize )
{GUCEF_TRACE;

    return ( resourceSize + DRN_RESOURCE_CHUNK_SIZE - 1 ) / DRN_RESOURCE_CHUNK_SIZE;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceInfo::GetChunkCount( void ) const
{GUCEF_TRACE;

    return ChunkCountFor( size );
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceInfo::GetChunkBytes( UInt64 chunkIndex ) const
{GUCEF_TRACE;

    UInt64 start = chunkIndex * DRN_RESOURCE_CHUNK_SIZE;
    if ( chunkIndex >= GetChunkCount() || start >= size )
        return 0;
    UInt64 left = size - start;
    return left < DRN_RESOURCE_CHUNK_SIZE ? (UInt32) left : DRN_RESOURCE_CHUNK_SIZE;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceInfo::IsSameRevisionAs( const CDRNResourceInfo& other ) const
{GUCEF_TRACE;

    if ( size != other.size || revision != other.revision )
        return false;
    if ( hasTreeRoot && other.hasTreeRoot && 0 != memcmp( treeRoot, other.treeRoot, DigestSize ) )
        return false;
    if ( hasSha256 && other.hasSha256 && 0 != memcmp( sha256, other.sha256, DigestSize ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNResourceInfo::MakeRevision( UInt64 resourceSize, UInt64 modifiedMs )
{GUCEF_TRACE;

    return CORE::ToString( resourceSize ) + "-" + CORE::ToString( modifiedMs );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
