/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#include "gucefCOMPUTE_CCpuDispatchContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCpuDispatchContext::SDispatchData::SDispatchData( void )
    : constantsSize( 0 )
    , totalGroups( 0 )
{GUCEF_TRACE;

    memset( bindings, 0, sizeof( bindings ) );
    memset( constants, 0, sizeof( constants ) );
    for ( UInt32 i=0; i<3; ++i )
    {
        groupCount[ i ] = 0;
        workgroupSize[ i ] = 1;
    }
}

/*-------------------------------------------------------------------------*/

CCpuDispatchContext::CCpuDispatchContext( const SDispatchData& data, void* scratch, UInt32 scratchSizeInBytes )
    : m_data( &data )
    , m_scratch( scratch )
    , m_scratchSize( scratchSizeInBytes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const void*
CCpuDispatchContext::GetConstants( void ) const
{GUCEF_TRACE;

    return m_data->constants;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetConstantsSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_data->constantsSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetBaseItem( void ) const
{GUCEF_TRACE;

    UInt32 header[ 2 ];
    memcpy( header, m_data->constants, sizeof( header ) );
    return header[ 0 ];
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetItemCount( void ) const
{GUCEF_TRACE;

    UInt32 header[ 2 ];
    memcpy( header, m_data->constants, sizeof( header ) );
    return header[ 1 ];
}

/*-------------------------------------------------------------------------*/

void*
CCpuDispatchContext::GetBufferData( UInt32 slot ) const
{GUCEF_TRACE;

    return slot < GUCEF_COMPUTE_MAX_BINDINGS ? m_data->bindings[ slot ].data : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt64
CCpuDispatchContext::GetBufferSizeInBytes( UInt32 slot ) const
{GUCEF_TRACE;

    return slot < GUCEF_COMPUTE_MAX_BINDINGS ? m_data->bindings[ slot ].sizeInBytes : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetGroupCount( UInt32 axis ) const
{GUCEF_TRACE;

    return axis < 3 ? m_data->groupCount[ axis ] : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetTotalGroupCount( void ) const
{GUCEF_TRACE;

    return m_data->totalGroups;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetWorkgroupSize( UInt32 axis ) const
{GUCEF_TRACE;

    return axis < 3 ? m_data->workgroupSize[ axis ] : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetInvocationsPerGroup( void ) const
{GUCEF_TRACE;

    return m_data->workgroupSize[ 0 ] * m_data->workgroupSize[ 1 ] * m_data->workgroupSize[ 2 ];
}

/*-------------------------------------------------------------------------*/

void*
CCpuDispatchContext::GetScratch( void ) const
{GUCEF_TRACE;

    return m_scratch;
}

/*-------------------------------------------------------------------------*/

UInt32
CCpuDispatchContext::GetScratchSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_scratchSize;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
