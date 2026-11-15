/*
 *  computebridgeMATH: GUCEF compute bridge plugin of gucefMATH: multi-core and accelerator providers for the batch kernels
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

#include "computebridgeMATH_SMathBridgeDiagnostics.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTEBRIDGEMATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SMathBridgeDiagnostics::SMathBridgeDiagnostics( void )
    : forcedMultiCoreFailures( 0 )
    , grainOverrideItems( 0 )
    , acceleratorDevice( GUCEF_NULL )
    , acceleratorGeneration( 0 )
    , liveKernels( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
SMathBridgeDiagnostics::TakeForcedMultiCoreFailure( void )
{GUCEF_TRACE;

    UInt32 remaining = forcedMultiCoreFailures.Load( MT::MEMORDER_RELAXED );
    while ( remaining > 0 )
    {
        if ( forcedMultiCoreFailures.CompareExchangeWeak( remaining, remaining - 1, MT::MEMORDER_ACQ_REL, MT::MEMORDER_RELAXED ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
SMathBridgeDiagnostics::Reset( void )
{GUCEF_TRACE;

    for ( UInt32 k=0; k<MATHBATCHKERNEL_SLOTS; ++k )
    {
        multiCoreCalls[ k ].Store( 0 );
        multiCoreFailures[ k ].Store( 0 );
        acceleratorCalls[ k ].Store( 0 );
        acceleratorFailures[ k ].Store( 0 );
    }
    forcedMultiCoreFailures.Store( 0 );
    grainOverrideItems.Store( 0 );
    acceleratorDevice.Store( GUCEF_NULL );
    acceleratorGeneration.Store( 0 );
}

/*-------------------------------------------------------------------------*/

SMathBridgeProviderContext::SMathBridgeProviderContext( void )
    : kernel( MATH::MATHBATCHKERNEL_TRANSFORM_POINTS3_F32 )
    , tier( MATH::MATHBATCHTIER_SCALAR )
    , diagnostics( GUCEF_NULL )
    , info( GUCEF_NULL )
    , accelerator( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTEBRIDGEMATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
