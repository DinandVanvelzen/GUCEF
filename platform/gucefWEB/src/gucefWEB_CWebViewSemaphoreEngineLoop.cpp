/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#include "gucefWEB_CWebViewSemaphoreEngineLoop.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewSemaphoreEngineLoop::CWebViewSemaphoreEngineLoop( void )
    : CIWebViewEngineLoop()
    , m_wake( 0 )
    , m_posted( 0 )
    , m_sleeps( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewSemaphoreEngineLoop::~CWebViewSemaphoreEngineLoop()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewSemaphoreEngineLoop::OnLoopStart( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewSemaphoreEngineLoop::RunOnce( UInt32 maxWaitMs )
{GUCEF_TRACE;

    ++m_sleeps;
    const UInt32 wait = WaitForever == maxWaitMs ? GUCEF_MT_INFINITE_LOCK_TIMEOUT : maxWaitMs;
    if ( 0 == m_wake.WaitOrTimeout( wait ) )
        m_posted.Store( 0 );
}

/*-------------------------------------------------------------------------*/

void
CWebViewSemaphoreEngineLoop::Wake( void )
{GUCEF_TRACE;

    if ( 0 == m_posted.Exchange( 1 ) )
        m_wake.Post();
}

/*-------------------------------------------------------------------------*/

void
CWebViewSemaphoreEngineLoop::OnLoopStop( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewSemaphoreEngineLoop::GetSleepCount( void ) const
{GUCEF_TRACE;

    return m_sleeps.Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
