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

#ifndef GUCEF_DRN_CDRNRESOURCEINFO_H
#include "gucefDRN_CDRNResourceInfo.h"
#define GUCEF_DRN_CDRNRESOURCEINFO_H
#endif /* GUCEF_DRN_CDRNRESOURCEINFO_H ? */

#include "gucefDRN_CDRNResourceHashTree.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char DigestLabel[] = "GUCEF.DRN.RES.TREE.1";

/** The largest level a CDynamicBuffer holds, in hashes */
static const UInt64 MaxLevelHashes = ( (UInt64) 0xFFFFFFFF ) / CDRNResourceHashTree::HashSize;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNResourceHashTree::CDRNResourceHashTree( void )
    : m_levels()
    , m_resourceSize( 0 )
    , m_leafCount( 0 )
    , m_complete( false )
{GUCEF_TRACE;

    memset( m_emptyRoot, 0, sizeof( m_emptyRoot ) );
}

/*-------------------------------------------------------------------------*/

CDRNResourceHashTree::CDRNResourceHashTree( const CDRNResourceHashTree& src )
    : m_levels( src.m_levels )
    , m_resourceSize( src.m_resourceSize )
    , m_leafCount( src.m_leafCount )
    , m_complete( src.m_complete )
{GUCEF_TRACE;

    memcpy( m_emptyRoot, src.m_emptyRoot, sizeof( m_emptyRoot ) );
}

/*-------------------------------------------------------------------------*/

CDRNResourceHashTree::~CDRNResourceHashTree()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceHashTree&
CDRNResourceHashTree::operator=( const CDRNResourceHashTree& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_levels = src.m_levels;
        m_resourceSize = src.m_resourceSize;
        m_leafCount = src.m_leafCount;
        m_complete = src.m_complete;
        memcpy( m_emptyRoot, src.m_emptyRoot, sizeof( m_emptyRoot ) );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHashTree::Clear( void )
{GUCEF_TRACE;

    m_levels.clear();
    m_resourceSize = 0;
    m_leafCount = 0;
    m_complete = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::Reset( UInt64 resourceSize )
{GUCEF_TRACE;

    Clear();
    UInt64 leafCount = CDRNResourceInfo::ChunkCountFor( resourceSize );
    if ( leafCount > MaxLevelHashes )
        return false;
    m_resourceSize = resourceSize;
    m_leafCount = leafCount;
    m_levels.push_back( CORE::CDynamicBuffer() );
    if ( leafCount > 0 )
    {
        if ( !m_levels[ 0 ].SetBufferSize( (UInt32) ( leafCount * HashSize ), false, true ) )
        {
            Clear();
            return false;
        }
        m_levels[ 0 ].SetDataSize( (UInt32) ( leafCount * HashSize ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::SetLeafHash( UInt64 leafIndex, const UInt8* leafHash )
{GUCEF_TRACE;

    if ( m_levels.empty() || leafIndex >= m_leafCount )
        return false;
    m_levels[ 0 ].CopyFrom( (UInt32) ( leafIndex * HashSize ), HashSize, leafHash );
    m_complete = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::Finish( CDRNResourceHasher& hasher )
{GUCEF_TRACE;

    if ( m_levels.empty() )
        return false;
    m_levels.resize( 1 );
    GetEmptyRoot( hasher, m_emptyRoot );

    UInt64 count = m_leafCount;
    while ( count > 1 )
    {
        UInt64 parentCount = ( count + 1 ) / 2;
        m_levels.push_back( CORE::CDynamicBuffer() );
        CORE::CDynamicBuffer& parent = m_levels.back();
        const CORE::CDynamicBuffer& child = m_levels[ m_levels.size()-2 ];
        if ( !parent.SetBufferSize( (UInt32) ( parentCount * HashSize ), false, true ) )
            return false;
        parent.SetDataSize( (UInt32) ( parentCount * HashSize ) );
        const UInt8* childHashes = static_cast< const UInt8* >( child.GetConstBufferPtr() );
        UInt8* parentHashes = static_cast< UInt8* >( parent.GetBufferPtr() );
        for ( UInt64 i=0; i<parentCount; ++i )
        {
            UInt64 left = 2 * i;
            if ( left + 1 < count )
                hasher.HashNode( childHashes + left * HashSize, childHashes + ( left + 1 ) * HashSize, parentHashes + i * HashSize );
            else
                memcpy( parentHashes + i * HashSize, childHashes + left * HashSize, HashSize );   // carried up unchanged
        }
        count = parentCount;
    }
    m_complete = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::IsComplete( void ) const
{GUCEF_TRACE;

    return m_complete;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceHashTree::GetResourceSize( void ) const
{GUCEF_TRACE;

    return m_resourceSize;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceHashTree::GetLeafCount( void ) const
{GUCEF_TRACE;

    return m_leafCount;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::GetLeafHash( UInt64 leafIndex, UInt8* leafHash ) const
{GUCEF_TRACE;

    if ( m_levels.empty() || leafIndex >= m_leafCount )
        return false;
    memcpy( leafHash, static_cast< const UInt8* >( m_levels[ 0 ].GetConstBufferPtr() ) + leafIndex * HashSize, HashSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::GetRoot( UInt8* root ) const
{GUCEF_TRACE;

    if ( !m_complete )
        return false;
    if ( 0 == m_leafCount )
    {
        memcpy( root, m_emptyRoot, HashSize );
        return true;
    }
    memcpy( root, m_levels.back().GetConstBufferPtr(), HashSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::GetDigest( CDRNResourceHasher& hasher, UInt8* digest ) const
{GUCEF_TRACE;

    UInt8 root[ HashSize ];
    if ( !GetRoot( root ) )
        return false;
    ComputeDigest( hasher, m_resourceSize, root, digest );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNResourceHashTree::GetAuditPath( UInt64 leafIndex, UInt8* path ) const
{GUCEF_TRACE;

    if ( !m_complete || leafIndex >= m_leafCount )
        return 0;
    UInt32 length = 0;
    UInt64 index = leafIndex;
    for ( UInt32 level=0; level+1<m_levels.size() && length<MaxAuditPathLength; ++level )
    {
        UInt64 count = m_levels[ level ].GetDataSize() / HashSize;
        UInt64 sibling = index ^ 1;
        if ( sibling < count )
        {
            memcpy( path + length * HashSize, static_cast< const UInt8* >( m_levels[ level ].GetConstBufferPtr() ) + sibling * HashSize, HashSize );
            ++length;
        }
        index >>= 1;
    }
    return length;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNResourceHashTree::GetMemoryBytes( void ) const
{GUCEF_TRACE;

    UInt64 bytes = 0;
    for ( UInt32 i=0; i<m_levels.size(); ++i )
        bytes += m_levels[ i ].GetBufferSize();
    return bytes;
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHashTree::GetEmptyRoot( CDRNResourceHasher& hasher, UInt8* root )
{GUCEF_TRACE;

    hasher.Hash( GUCEF_NULL, 0, root );
}

/*-------------------------------------------------------------------------*/

void
CDRNResourceHashTree::ComputeDigest( CDRNResourceHasher& hasher, UInt64 resourceSize, const UInt8* root, UInt8* digest )
{GUCEF_TRACE;

    UInt8 size[ 8 ];
    for ( UInt32 i=0; i<8; ++i )
        size[ i ] = (UInt8) ( resourceSize >> ( 56 - 8 * i ) );
    hasher.Begin();
    hasher.Update( DigestLabel, (UInt32) ( sizeof( DigestLabel ) - 1 ) );
    hasher.Update( size, sizeof( size ) );
    hasher.Update( root, HashSize );
    hasher.Final( digest );
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceHashTree::VerifyAuditPath( CDRNResourceHasher& hasher ,
                                       UInt64 leafIndex           ,
                                       UInt64 leafCount           ,
                                       const UInt8* leafHash      ,
                                       const UInt8* path          ,
                                       UInt32 pathLength          ,
                                       const UInt8* root          )
{GUCEF_TRACE;

    if ( leafIndex >= leafCount || pathLength > MaxAuditPathLength )
        return false;

    UInt64 fn = leafIndex;
    UInt64 sn = leafCount - 1;
    UInt8 r[ HashSize ];
    memcpy( r, leafHash, HashSize );
    for ( UInt32 i=0; i<pathLength; ++i )
    {
        const UInt8* p = path + i * HashSize;
        if ( 0 == sn )
            return false;
        if ( 0 != ( fn & 1 ) || fn == sn )
        {
            hasher.HashNode( p, r, r );
            if ( 0 == ( fn & 1 ) )
            {
                while ( 0 == ( fn & 1 ) && 0 != fn )
                {
                    fn >>= 1;
                    sn >>= 1;
                }
            }
        }
        else
        {
            hasher.HashNode( r, p, r );
        }
        fn >>= 1;
        sn >>= 1;
    }
    return 0 == sn && CORE::CSha256::ConstantTimeEquals( r, root, HashSize );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
