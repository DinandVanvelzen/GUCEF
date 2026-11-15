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

#ifndef GUCEF_COMPUTE_CCOMPUTEFENCE_H
#define GUCEF_COMPUTE_CCOMPUTEFENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMPUTE_MACROS_H
#include "gucefCOMPUTE_macros.h"
#define GUCEF_COMPUTE_MACROS_H
#endif /* GUCEF_COMPUTE_MACROS_H ? */

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
 *  A point on the timeline of a queue: the queue and the timeline value of a submission. Values grow per queue.
 *  The fence is signalled once the queue completed the submission. A default constructed fence (no queue) and a
 *  fence of value 0 are always signalled. A fence of a device that is lost reports COMPUTERESULT_DEVICE_LOST for
 *  work that had not completed when the device was lost. Copying is cheap; the fence keeps its queue alive.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeFence
{
    public:

    CComputeFence( void );

    CComputeFence( const TComputeQueuePtr& queue, UInt64 value );

    CComputeFence( const CComputeFence& src );

    ~CComputeFence();

    CComputeFence& operator=( const CComputeFence& src );

    bool HasQueue( void ) const;

    const TComputeQueuePtr& GetQueue( void ) const;

    UInt64 GetValue( void ) const;

    /** True once the queue completed the submission */
    bool IsSignalled( void ) const;

    /**
     *  Waits on the calling thread until the fence is signalled, at most 'timeoutInMs' milliseconds.
     *  COMPUTERESULT_SUCCESS, COMPUTERESULT_TIMEOUT or COMPUTERESULT_DEVICE_LOST.
     */
    TComputeResult Wait( UInt32 timeoutInMs = GUCEF_COMPUTE_INFINITE_TIMEOUT ) const;

    private:

    TComputeQueuePtr m_queue;
    UInt64 m_value;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTEFENCE_H ? */
