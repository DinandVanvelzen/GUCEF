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

#include "gucefCOMPUTE_CComputeKernelDecl.h"

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

CComputeKernelDecl::CComputeKernelDecl( void )
    : m_kernelId()
    , m_interfaceVersion( 1 )
    , m_signature()
    , m_determinism( DETERMINISM_FLOAT_STRICT )
    , m_tolerance()
    , m_preferredGrain( 0 )
    , m_description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeKernelDecl::CComputeKernelDecl( const CString& kernelId, UInt32 interfaceVersion )
    : m_kernelId( kernelId )
    , m_interfaceVersion( interfaceVersion )
    , m_signature()
    , m_determinism( DETERMINISM_FLOAT_STRICT )
    , m_tolerance()
    , m_preferredGrain( 0 )
    , m_description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CComputeKernelDecl::GetKernelId( void ) const
{GUCEF_TRACE;

    return m_kernelId;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelDecl::SetKernelId( const CString& kernelId )
{GUCEF_TRACE;

    m_kernelId = kernelId;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelDecl::GetInterfaceVersion( void ) const
{GUCEF_TRACE;

    return m_interfaceVersion;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelDecl::SetInterfaceVersion( UInt32 interfaceVersion )
{GUCEF_TRACE;

    m_interfaceVersion = interfaceVersion;
}

/*-------------------------------------------------------------------------*/

const CComputeKernelSignature&
CComputeKernelDecl::GetSignature( void ) const
{GUCEF_TRACE;

    return m_signature;
}

/*-------------------------------------------------------------------------*/

CComputeKernelSignature&
CComputeKernelDecl::GetSignature( void )
{GUCEF_TRACE;

    return m_signature;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelDecl::SetSignature( const CComputeKernelSignature& signature )
{GUCEF_TRACE;

    m_signature = signature;
}

/*-------------------------------------------------------------------------*/

TComputeDeterminism
CComputeKernelDecl::GetDeterminism( void ) const
{GUCEF_TRACE;

    return m_determinism;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelDecl::SetDeterminism( TComputeDeterminism determinism )
{GUCEF_TRACE;

    m_determinism = determinism;
}

/*-------------------------------------------------------------------------*/

const CComputeTolerance&
CComputeKernelDecl::GetTolerance( void ) const
{GUCEF_TRACE;

    return m_tolerance;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelDecl::SetTolerance( const CComputeTolerance& tolerance )
{GUCEF_TRACE;

    m_tolerance = tolerance;
}

/*-------------------------------------------------------------------------*/

TComputeFeatures
CComputeKernelDecl::GetRequiredFeatures( void ) const
{GUCEF_TRACE;

    return m_signature.GetRequiredFeatures();
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelDecl::GetPreferredGrain( void ) const
{GUCEF_TRACE;

    return m_preferredGrain;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelDecl::SetPreferredGrain( UInt32 groupsPerChunk )
{GUCEF_TRACE;

    m_preferredGrain = groupsPerChunk;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeKernelDecl::GetDescription( void ) const
{GUCEF_TRACE;

    return m_description;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelDecl::SetDescription( const CString& description )
{GUCEF_TRACE;

    m_description = description;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelDecl::IsValid( void ) const
{GUCEF_TRACE;

    CString::StringVector parts = m_kernelId.ParseElements( '.', true );
    if ( 3 != parts.size() )
        return false;
    for ( size_t i=0; i<parts.size(); ++i )
        if ( parts[ i ].IsNULLOrEmpty() )
            return false;
    return m_signature.GetInvocationsPerGroup() > 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
