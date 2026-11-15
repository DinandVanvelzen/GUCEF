/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_MATH_CMATHBATCHDISPATCHER_H
#include "gucefMATH_CMathBatchDispatcher.h"
#define GUCEF_MATH_CMATHBATCHDISPATCHER_H
#endif /* GUCEF_MATH_CMATHBATCHDISPATCHER_H ? */

#include "gucefGEOMETRY_CGeometryGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Defined in gucefGEOMETRY_BatchKernels.cpp: registers the batch kernels and their providers with a dispatcher */
bool GeometryRegisterBatchKernels( MATH::CMathBatchDispatcher& dispatcher, const void* owner );

/*-------------------------------------------------------------------------*/

/** The instance, published with a release store only after Initialize() completed */
static MT::CTAtomic< CGeometryGlobal* > g_instance;

/** 1 while one thread creates the instance; the others wait for the publication */
static MT::CTAtomic< UInt32 > g_creating;

/*-------------------------------------------------------------------------*/

CGeometryGlobal*
CGeometryGlobal::Instance( void )
{
    // the path of every call after the first: one acquire load, no lock, no tracing
    CGeometryGlobal* instance = g_instance.Load( MT::MEMORDER_ACQUIRE );
    if ( GUCEF_NULL != instance )
        return instance;
    return CreateInstance();
}

/*-------------------------------------------------------------------------*/

CGeometryGlobal*
CGeometryGlobal::CreateInstance( void )
{GUCEF_TRACE;

    UInt32 spins = 0;
    for ( ;; )
    {
        CGeometryGlobal* instance = g_instance.Load( MT::MEMORDER_ACQUIRE );
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
                    // Ensure the systems of the modules this module depends on are initialized first
                    CORE::CCoreGlobal::Instance();
                    MATH::CMathGlobal::Instance();

                    instance = GUCEF_NEW CGeometryGlobal();
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
CGeometryGlobal::Deinstance( void )
{GUCEF_TRACE;

    // module unload: no call can be running any more
    CGeometryGlobal* instance = g_instance.Exchange( GUCEF_NULL, MT::MEMORDER_ACQ_REL );
    GUCEF_DELETE instance;
}

/*-------------------------------------------------------------------------*/

CGeometryGlobal::CGeometryGlobal( void )
    : m_batchDispatcher( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGeometryGlobal::~CGeometryGlobal()
{GUCEF_TRACE;

    // in the reverse order of Initialize(): the providers are removed (this waits for the calls that still run in them)
    // while the dispatcher of gucefMATH, which this module depends on and which is unloaded after it, still exists
    if ( GUCEF_NULL != m_batchDispatcher )
    {
        m_batchDispatcher->UnregisterProviders( this );
        m_batchDispatcher = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CGeometryGlobal::Initialize( void )
{GUCEF_TRACE;

    // the batch kernels go into the dispatcher of gucefMATH, which exists: Instance() created CMathGlobal first
    MATH::CMathBatchDispatcher& dispatcher = MATH::CMathGlobal::Instance()->GetBatchDispatcher();
    if ( GeometryRegisterBatchKernels( dispatcher, this ) )
        m_batchDispatcher = &dispatcher;
    else
        dispatcher.UnregisterProviders( this );
}

/*-------------------------------------------------------------------------*/

bool
CGeometryGlobal::AreBatchKernelsRegistered( void ) const
{
    return GUCEF_NULL != m_batchDispatcher;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
