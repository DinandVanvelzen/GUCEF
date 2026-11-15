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

#ifndef GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H
#define GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#include "gucefCOMPUTE_CComputeCommandList.h"
#define GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#endif /* GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One pending submission of a queue: its timeline value, the source of the future of ExecuteAsync() (when there is
 *  one) and a copy of the command list, whose references keep the buffers and kernels alive until the work completed
 *  (deferred release) and which backends that execute on the host (the CPU backend) run from.
 *  Owned by the completion task of the queue once enqueued, or in cooperative mode by the queue until
 *  CComputeGlobal::Update() (or a wait on the calling thread) processed it.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputePendingWork
{
    public:

    /** Work from Submit(): no future */
    CComputePendingWork( UInt64 fenceValue, const CComputeCommandList& commands );

    /** Work from ExecuteAsync(): completes or fails the future of 'source' */
    CComputePendingWork( UInt64 fenceValue, const CComputeCommandList& commands, const CORE::CFutureResultSource& source );

    ~CComputePendingWork();

    UInt64 GetFenceValue( void ) const;

    const CComputeCommandList& GetCommands( void ) const;

    bool HasFuture( void ) const;

    CORE::CFutureResult GetFuture( void ) const;

    /** Completes the future (COMPUTERESULT_SUCCESS) or fails it with the result and the error text. Only the first call counts */
    void Finish( TComputeResult result, const CString& errorText );

    bool IsFinished( void ) const;

    private:

    CComputePendingWork( const CComputePendingWork& src );            /**< not implemented */
    CComputePendingWork& operator=( const CComputePendingWork& src ); /**< not implemented */

    private:

    UInt64 m_fenceValue;
    CComputeCommandList m_commands;
    CORE::CFutureResultSource m_source;
    bool m_hasFuture;
    bool m_finished;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CComputePendingWork* > TComputePendingWorkVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEPENDINGWORK_H ? */
