/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#include "gucefMATH_CMathGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The instance, published with a release store only after Initialize() completed */
static MT::CTAtomic< CMathGlobal* > g_instance;

/** 1 while one thread creates the instance; the others wait for the publication */
static MT::CTAtomic< UInt32 > g_creating;

/*-------------------------------------------------------------------------*/

CMathGlobal*
CMathGlobal::Instance( void )
{
    // the path of every batch call: one acquire load, no lock, no tracing
    CMathGlobal* instance = g_instance.Load( MT::MEMORDER_ACQUIRE );
    if ( GUCEF_NULL != instance )
        return instance;
    return CreateInstance();
}

/*-------------------------------------------------------------------------*/

CMathGlobal*
CMathGlobal::CreateInstance( void )
{GUCEF_TRACE;

    UInt32 spins = 0;
    for ( ;; )
    {
        CMathGlobal* instance = g_instance.Load( MT::MEMORDER_ACQUIRE );
        if ( GUCEF_NULL != instance )
            return instance;

        UInt32 idle = 0;
        if ( g_creating.CompareExchangeStrong( idle, 1, MT::MEMORDER_ACQ_REL, MT::MEMORDER_ACQUIRE ) )
        {
            // this thread creates it; another one may have finished between the load above and the exchange
            instance = g_instance.Load( MT::MEMORDER_ACQUIRE );
            if ( GUCEF_NULL == instance )
            {
                try
                {
                    // Ensure the Core systems are initialized first since this module depends on Core
                    CORE::CCoreGlobal::Instance();

                    instance = GUCEF_NEW CMathGlobal();
                    instance->Initialize();
                }
                catch ( ... )
                {
                    GUCEF_DELETE instance;
                    g_creating.Store( 0, MT::MEMORDER_RELEASE );
                    throw;
                }
                g_instance.Store( instance, MT::MEMORDER_RELEASE );
            }
            g_creating.Store( 0, MT::MEMORDER_RELEASE );
            return instance;
        }

        // another thread creates it: wait for the publication (or retry if that thread failed)
        if ( ++spins < 1000 )
            MT::AtomicCpuRelax();
        else
            MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CMathGlobal::Deinstance( void )
{GUCEF_TRACE;

    // module unload: no call can be running any more
    CMathGlobal* instance = g_instance.Exchange( GUCEF_NULL, MT::MEMORDER_ACQ_REL );
    GUCEF_DELETE instance;
}

/*-------------------------------------------------------------------------*/

CMathGlobal::CMathGlobal( void )
    : m_batchDispatcher( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMathGlobal::~CMathGlobal()
{GUCEF_TRACE;

    // in the reverse order of Initialize()
    GUCEF_DELETE m_batchDispatcher;
    m_batchDispatcher = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CMathGlobal::Initialize( void )
{GUCEF_TRACE;

    // 1. the batch dispatch table, with the providers of this module (the CPU features are read here, once)
    m_batchDispatcher = GUCEF_NEW CMathBatchDispatcher();
    m_batchDispatcher->RegisterBuiltInProviders();
}

/*-------------------------------------------------------------------------*/

CMathBatchDispatcher&
CMathGlobal::GetBatchDispatcher( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    return *m_batchDispatcher;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
