/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#include "guidriverOgre_COgreSurfaceState.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreSurfaceState::COgreSurfaceState( void )
    : m_state( SURFACESTATE_NO_WINDOW )
    , m_lossCount( 0 )
    , m_restoreCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
COgreSurfaceState::OnRenderWindowCreated( void )
{GUCEF_TRACE;

    m_state = SURFACESTATE_READY;
}

/*-------------------------------------------------------------------------*/

void
COgreSurfaceState::OnRenderWindowDestroyed( void )
{GUCEF_TRACE;

    m_state = SURFACESTATE_NO_WINDOW;
}

/*-------------------------------------------------------------------------*/

COgreSurfaceState::EAction
COgreSurfaceState::OnSurfaceLost( void )
{GUCEF_TRACE;

    if ( SURFACESTATE_READY != m_state )
        return SURFACEACTION_NONE;
    m_state = SURFACESTATE_LOST;
    ++m_lossCount;
    return SURFACEACTION_RELEASE_SURFACE;
}

/*-------------------------------------------------------------------------*/

COgreSurfaceState::EAction
COgreSurfaceState::OnSurfaceRestored( bool hasNativeWindow )
{GUCEF_TRACE;

    if ( SURFACESTATE_LOST != m_state || !hasNativeWindow )
        return SURFACEACTION_NONE;
    m_state = SURFACESTATE_READY;
    ++m_restoreCount;
    return SURFACEACTION_ADOPT_SURFACE;
}

/*-------------------------------------------------------------------------*/

bool
COgreSurfaceState::CanDraw( void ) const
{GUCEF_TRACE;

    return SURFACESTATE_READY == m_state;
}

/*-------------------------------------------------------------------------*/

COgreSurfaceState::EState
COgreSurfaceState::GetState( void ) const
{GUCEF_TRACE;

    return m_state;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreSurfaceState::GetLossCount( void ) const
{GUCEF_TRACE;

    return m_lossCount;
}

/*-------------------------------------------------------------------------*/

UInt32
COgreSurfaceState::GetRestoreCount( void ) const
{GUCEF_TRACE;

    return m_restoreCount;
}

/*-------------------------------------------------------------------------*/

const char*
COgreSurfaceState::ToString( EState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case SURFACESTATE_NO_WINDOW: return "NoWindow";
        case SURFACESTATE_READY: return "Ready";
        case SURFACESTATE_LOST: return "Lost";
        default: return "Unknown";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
