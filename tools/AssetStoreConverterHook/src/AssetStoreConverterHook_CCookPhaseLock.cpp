/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#include "AssetStoreConverterHook_CCookPhaseLock.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The lock of the cook phase of the process: made when the module loads, before any run */
MT::CMutex g_cookLock;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCookPhaseLock::CCookPhaseLock( void )
    : m_held( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCookPhaseLock::~CCookPhaseLock()
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

void
CCookPhaseLock::OnPhase( ASSETCONVERTER::TPipelinePhase phase )
{GUCEF_TRACE;

    if ( ASSETCONVERTER::PIPELINEPHASE_COOK == phase )
    {
        if ( !m_held )
        {
            const MT::TLockStatus status = g_cookLock.Lock( GUCEF_MT_INFINITE_LOCK_TIMEOUT );
            m_held = MT::LOCKSTATUS_OPERATION_SUCCESS == status || MT::LOCKSTATUS_ABANDONED == status;
        }
        return;
    }
    Release();
}

/*-------------------------------------------------------------------------*/

bool
CCookPhaseLock::IsHeld( void ) const
{GUCEF_TRACE;

    return m_held;
}

/*-------------------------------------------------------------------------*/

void
CCookPhaseLock::Release( void )
{GUCEF_TRACE;

    if ( m_held )
    {
        g_cookLock.Unlock();
        m_held = false;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
