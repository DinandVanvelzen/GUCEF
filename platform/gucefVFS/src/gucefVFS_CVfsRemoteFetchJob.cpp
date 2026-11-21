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

#include "gucefVFS_CVfsRemoteFetchJob.h"

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

CVfsRemoteFetchJob::CVfsRemoteFetchJob( const CVfsRemoteArchiveCorePtr& core ,
                                        EVfsFetchJobKind kind                ,
                                        const CORE::CString& path            ,
                                        const CVfsRemoteEntry& entry         ,
                                        UInt32 firstBlock                    ,
                                        UInt32 blockCount                    )
    : CIVfsFetchJob()
    , m_core( core )
    , m_kind( kind )
    , m_path( path )
    , m_entry( entry )
    , m_firstBlock( firstBlock )
    , m_blockCount( blockCount )
    , m_silent( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsRemoteFetchJob::~CVfsRemoteFetchJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

EVfsFetchJobKind
CVfsRemoteFetchJob::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemoteFetchJob::MakeKey( EVfsFetchJobKind kind, const void* owner, const CORE::CString& path, UInt32 firstBlock )
{GUCEF_TRACE;

    return CORE::ToString( (UInt32) kind ) + '|' + CORE::ToString( (UInt64) reinterpret_cast< size_t >( owner ) ) + '|' + path + '|' + CORE::ToString( firstBlock );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemoteFetchJob::GetKey( void ) const
{GUCEF_TRACE;

    return MakeKey( m_kind, m_core.GetPointerAlways(), m_path, FETCHJOB_BLOCKS == m_kind ? m_firstBlock : 0 );
}

/*-------------------------------------------------------------------------*/

const void*
CVfsRemoteFetchJob::GetOwner( void ) const
{GUCEF_TRACE;

    return m_core.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

CVfsCallResult
CVfsRemoteFetchJob::Run( const MT::CTAtomic< bool >& cancelRequested, CVfsArrivalInfo& outcome )
{GUCEF_TRACE;

    return m_core->RunJob( m_kind, m_path, m_entry, m_firstBlock, m_blockCount, cancelRequested, outcome, m_silent );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteFetchJob::OnEnded( const CVfsCallResult& result, const CVfsArrivalInfo& outcome )
{GUCEF_TRACE;

    m_core->OnJobEnded( m_kind, m_path, result, outcome, m_silent );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
