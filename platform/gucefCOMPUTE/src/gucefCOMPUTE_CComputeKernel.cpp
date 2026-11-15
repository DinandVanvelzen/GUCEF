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

#ifndef GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#include "gucefCOMPUTE_CComputeDevice.h"
#define GUCEF_COMPUTE_CCOMPUTEDEVICE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDEVICE_H ? */

#include "gucefCOMPUTE_CComputeKernel.h"

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

namespace {

const CComputeKernelDecl g_emptyDecl;
const CComputeKernelArtefact g_emptyArtefact;

}

/*-------------------------------------------------------------------------*/

CComputeKernel::CComputeKernel( void )
    : m_device()
    , m_decl()
    , m_artefact()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeKernel::~CComputeKernel()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CComputeKernel::GetKernelId( void ) const
{GUCEF_TRACE;

    return GetDecl().GetKernelId();
}

/*-------------------------------------------------------------------------*/

const CComputeKernelDecl&
CComputeKernel::GetDecl( void ) const
{GUCEF_TRACE;

    return m_decl.IsNULL() ? g_emptyDecl : *m_decl;
}

/*-------------------------------------------------------------------------*/

const CComputeKernelSignature&
CComputeKernel::GetSignature( void ) const
{GUCEF_TRACE;

    return GetDecl().GetSignature();
}

/*-------------------------------------------------------------------------*/

const CComputeKernelArtefact&
CComputeKernel::GetArtefact( void ) const
{GUCEF_TRACE;

    return m_artefact.IsNULL() ? g_emptyArtefact : *m_artefact;
}

/*-------------------------------------------------------------------------*/

CComputeDevice*
CComputeKernel::GetDevice( void ) const
{GUCEF_TRACE;

    return const_cast< CComputeDevice* >( m_device.GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernel::IsBound( void ) const
{GUCEF_TRACE;

    return !m_device.IsNULL() && !m_decl.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CComputeKernel::Bind( const TComputeDevicePtr& device, const TComputeKernelDeclPtr& decl, const TComputeKernelArtefactPtr& artefact )
{GUCEF_TRACE;

    m_device = device;
    m_decl = decl;
    m_artefact = artefact;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
