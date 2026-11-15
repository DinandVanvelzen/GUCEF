/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefASSETSTORE_CAssetToolsExposureControl.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolsExposureControl::CAssetToolsExposureControl( void )
    : m_lock()
    , m_path()
    , m_state()
    , m_seen( false )
    , m_seenExists( false )
    , m_seenSize( 0 )
    , m_seenTime()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetToolsExposureControl::SetFile( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_path = path;
    m_seen = false;
    ReadLocked();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetToolsExposureControl::GetFile( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_path;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposureControl::ReadLocked( void )
{GUCEF_TRACE;

    CAssetToolsExposure next;
    m_seen = true;
    m_seenExists = !m_path.IsNULLOrEmpty() && CORE::FileExists( m_path );
    m_seenSize = m_seenExists ? CORE::FileSize( m_path ) : 0;
    m_seenTime = m_seenExists ? CORE::GetFileModificationTime( m_path ) : CORE::CDateTime();
    if ( !m_path.IsNULLOrEmpty() )
    {
        CORE::CString problem;
        bool missing = false;
        if ( !next.LoadFromFile( m_path, problem, missing ) )
        {
            next = CAssetToolsExposure();
            if ( !missing )
            {
                next.problem = problem;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetToolsExposure: The exposure file is refused, the closed defaults apply: " << problem );
            }
        }
    }
    const bool changed = next != m_state;
    if ( changed )
    {
        m_state = next;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetToolsExposure: In force from " << m_state.origin << ": mcpEnabled " << CORE::ToString( m_state.mcpEnabled ) <<
            ", convert " << CORE::ToString( m_state.convertEnabled ) << ", author " << CORE::ToString( m_state.authorEnabled ) <<
            ", store " << CORE::ToString( m_state.storeEnabled ) );
    }
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposureControl::Reload( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return ReadLocked();
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposureControl::CheckForChange( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_path.IsNULLOrEmpty() )
        return false;
    const bool exists = CORE::FileExists( m_path );
    if ( m_seen && exists == m_seenExists )
    {
        if ( !exists )
            return false;
        if ( CORE::FileSize( m_path ) == m_seenSize && CORE::GetFileModificationTime( m_path ) == m_seenTime )
            return false;
    }
    return ReadLocked();
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolsExposureControl::SetState( const CAssetToolsExposure& state )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const bool changed = state != m_state;
    m_state = state;
    return changed;
}

/*-------------------------------------------------------------------------*/

CAssetToolsExposure
CAssetToolsExposureControl::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_state;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
