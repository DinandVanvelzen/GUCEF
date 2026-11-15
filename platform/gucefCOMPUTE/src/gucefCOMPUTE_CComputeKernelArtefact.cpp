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

#include "gucefCOMPUTE_CComputeKernelArtefact.h"

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

CComputeKernelArtefact::CComputeKernelArtefact( void )
    : m_backendName()
    , m_format( ARTEFACTFORMAT_UNKNOWN )
    , m_entryPoint()
    , m_staticData( GUCEF_NULL )
    , m_staticDataSize( 0 )
    , m_ownedData()
    , m_hasOwnedData( false )
    , m_cpuFactory( GUCEF_NULL )
    , m_interfaceVersion( 0 )
    , m_sourceHash()
    , m_compilerId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeKernelArtefact::CComputeKernelArtefact( const CComputeKernelArtefact& src )
    : m_backendName( src.m_backendName )
    , m_format( src.m_format )
    , m_entryPoint( src.m_entryPoint )
    , m_staticData( src.m_staticData )
    , m_staticDataSize( src.m_staticDataSize )
    , m_ownedData( src.m_ownedData )
    , m_hasOwnedData( src.m_hasOwnedData )
    , m_cpuFactory( src.m_cpuFactory )
    , m_interfaceVersion( src.m_interfaceVersion )
    , m_sourceHash( src.m_sourceHash )
    , m_compilerId( src.m_compilerId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeKernelArtefact&
CComputeKernelArtefact::operator=( const CComputeKernelArtefact& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_backendName = src.m_backendName;
        m_format = src.m_format;
        m_entryPoint = src.m_entryPoint;
        m_staticData = src.m_staticData;
        m_staticDataSize = src.m_staticDataSize;
        m_ownedData = src.m_ownedData;
        m_hasOwnedData = src.m_hasOwnedData;
        m_cpuFactory = src.m_cpuFactory;
        m_interfaceVersion = src.m_interfaceVersion;
        m_sourceHash = src.m_sourceHash;
        m_compilerId = src.m_compilerId;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CComputeKernelArtefact
CComputeKernelArtefact::ForCpu( TCpuComputeKernelFactory* factory )
{GUCEF_TRACE;

    CComputeKernelArtefact artefact;
    artefact.SetBackendName( "cpu" );
    artefact.SetFormat( ARTEFACTFORMAT_CPU );
    artefact.SetCpuFactory( factory );
    return artefact;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeKernelArtefact::GetBackendName( void ) const
{GUCEF_TRACE;

    return m_backendName;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetBackendName( const CString& backendName )
{GUCEF_TRACE;

    m_backendName = backendName.Lowercase();
}

/*-------------------------------------------------------------------------*/

TComputeArtefactFormat
CComputeKernelArtefact::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetFormat( TComputeArtefactFormat format )
{GUCEF_TRACE;

    m_format = format;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeKernelArtefact::GetEntryPoint( void ) const
{GUCEF_TRACE;

    return m_entryPoint;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetEntryPoint( const CString& entryPoint )
{GUCEF_TRACE;

    m_entryPoint = entryPoint;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetStaticData( const void* data, UInt64 sizeInBytes )
{GUCEF_TRACE;

    m_staticData = data;
    m_staticDataSize = sizeInBytes;
    m_ownedData.Clear( false );
    m_hasOwnedData = false;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetOwnedData( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    m_ownedData = data;
    m_hasOwnedData = true;
    m_staticData = GUCEF_NULL;
    m_staticDataSize = 0;
}

/*-------------------------------------------------------------------------*/

const void*
CComputeKernelArtefact::GetData( void ) const
{GUCEF_TRACE;

    if ( m_hasOwnedData )
        return m_ownedData.GetConstBufferPtr();
    return m_staticData;
}

/*-------------------------------------------------------------------------*/

UInt64
CComputeKernelArtefact::GetDataSizeInBytes( void ) const
{GUCEF_TRACE;

    if ( m_hasOwnedData )
        return m_ownedData.GetDataSize();
    return m_staticDataSize;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelArtefact::HasOwnedData( void ) const
{GUCEF_TRACE;

    return m_hasOwnedData;
}

/*-------------------------------------------------------------------------*/

TCpuComputeKernelFactory*
CComputeKernelArtefact::GetCpuFactory( void ) const
{GUCEF_TRACE;

    return m_cpuFactory;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetCpuFactory( TCpuComputeKernelFactory* factory )
{GUCEF_TRACE;

    m_cpuFactory = factory;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelArtefact::GetInterfaceVersion( void ) const
{GUCEF_TRACE;

    return m_interfaceVersion;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetInterfaceVersion( UInt32 interfaceVersion )
{GUCEF_TRACE;

    m_interfaceVersion = interfaceVersion;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeKernelArtefact::GetSourceHash( void ) const
{GUCEF_TRACE;

    return m_sourceHash;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetSourceHash( const CString& sourceHash )
{GUCEF_TRACE;

    m_sourceHash = sourceHash;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeKernelArtefact::GetCompilerId( void ) const
{GUCEF_TRACE;

    return m_compilerId;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelArtefact::SetCompilerId( const CString& compilerId )
{GUCEF_TRACE;

    m_compilerId = compilerId;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelArtefact::IsValid( void ) const
{GUCEF_TRACE;

    if ( m_backendName.IsNULLOrEmpty() )
        return false;
    if ( ARTEFACTFORMAT_CPU == m_format )
        return GUCEF_NULL != m_cpuFactory;
    return ARTEFACTFORMAT_UNKNOWN != m_format && !m_entryPoint.IsNULLOrEmpty() && GUCEF_NULL != GetData() && GetDataSizeInBytes() > 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
