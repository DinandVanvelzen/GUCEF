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

#include "gucefDRN_CDRNResourceTransferOptions.h"

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

CDRNResourceTransferOptions::CDRNResourceTransferOptions( void )
    : verifyMode( VERIFY_LINK )
    , hasTreeDigest( false )
    , hasSha256( false )
    , expectedRevision()
    , windowChunks( 16 )
    , openTimeoutMs( 30000 )
    , idleTimeoutMs( 30000 )
    , corruptChunkRetries( 3 )
    , resume( true )
    , restartIfChanged( true )
{GUCEF_TRACE;

    memset( treeDigest, 0, sizeof( treeDigest ) );
    memset( sha256, 0, sizeof( sha256 ) );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferOptions::CDRNResourceTransferOptions( const CDRNResourceTransferOptions& src )
    : verifyMode( src.verifyMode )
    , hasTreeDigest( src.hasTreeDigest )
    , hasSha256( src.hasSha256 )
    , expectedRevision( src.expectedRevision )
    , windowChunks( src.windowChunks )
    , openTimeoutMs( src.openTimeoutMs )
    , idleTimeoutMs( src.idleTimeoutMs )
    , corruptChunkRetries( src.corruptChunkRetries )
    , resume( src.resume )
    , restartIfChanged( src.restartIfChanged )
{GUCEF_TRACE;

    memcpy( treeDigest, src.treeDigest, sizeof( treeDigest ) );
    memcpy( sha256, src.sha256, sizeof( sha256 ) );
}

/*-------------------------------------------------------------------------*/

CDRNResourceTransferOptions&
CDRNResourceTransferOptions::operator=( const CDRNResourceTransferOptions& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        verifyMode = src.verifyMode;
        hasTreeDigest = src.hasTreeDigest;
        hasSha256 = src.hasSha256;
        expectedRevision = src.expectedRevision;
        windowChunks = src.windowChunks;
        openTimeoutMs = src.openTimeoutMs;
        idleTimeoutMs = src.idleTimeoutMs;
        corruptChunkRetries = src.corruptChunkRetries;
        resume = src.resume;
        restartIfChanged = src.restartIfChanged;
        memcpy( treeDigest, src.treeDigest, sizeof( treeDigest ) );
        memcpy( sha256, src.sha256, sizeof( sha256 ) );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransferOptions::RequiresTree( void ) const
{GUCEF_TRACE;

    return VERIFY_TREE == verifyMode || hasTreeDigest;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceTransferOptions::WantsTree( void ) const
{GUCEF_TRACE;

    return RequiresTree() || VERIFY_TREE_IF_AVAILABLE == verifyMode;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
