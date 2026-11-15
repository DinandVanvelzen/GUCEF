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

#ifndef GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H
#define GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H
#include "gucefCOMPUTE_CComputeArguments.h"
#define GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H
#endif /* GUCEF_COMPUTE_CCOMPUTEARGUMENTS_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H
#include "gucefCOMPUTE_CComputeDispatchSize.h"
#define GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEDISPATCHSIZE_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEFENCE_H
#include "gucefCOMPUTE_CComputeFence.h"
#define GUCEF_COMPUTE_CCOMPUTEFENCE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEFENCE_H ? */

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

class CComputeDevice;

/**
 *  A backend-neutral recording of work for the queues of one device: writes from host memory, reads to host memory,
 *  copies, fills, dispatches and waits for fences of other queues. The commands are recorded compactly into one
 *  CDynamicBuffer; the list holds references to the buffers, kernels and queues it uses, so they stay alive while
 *  the list, or work submitted from it, exists. Clear() keeps the allocated memory for the next recording.
 *
 *  A list can be submitted many times and to any queue of the device it was recorded for (the device of the first
 *  object recorded). It is recorded by one thread at a time. Recording checks its arguments: a failed command is
 *  not recorded, its result is returned and kept (GetRecordingResult()) and queues refuse the list.
 *
 *  Commands of one queue take effect in submission and recording order. Callers never write barriers: backends derive
 *  them from the access each command declares.
 */
class GUCEF_COMPUTE_PUBLIC_CPP CComputeCommandList
{
    public:

    enum ECommandType
    {
        COMMANDTYPE_NONE = 0          ,
        COMMANDTYPE_WRITE_FROM_HOST   ,
        COMMANDTYPE_READ_TO_HOST      ,
        COMMANDTYPE_COPY              ,
        COMMANDTYPE_FILL              ,
        COMMANDTYPE_DISPATCH          ,
        COMMANDTYPE_WAIT_FOR
    };
    typedef enum ECommandType TCommandType;

    /** A buffer bound to a slot of a dispatch, as backends read it */
    struct SDispatchBinding
    {
        UInt32 slot;
        TComputeAccess access;
        CComputeBuffer* buffer;
        UInt64 offset;
        UInt64 size;            /**< resolved: never 0 */
    };

    /** One command as backends read it back with ReadCommand(); only the fields of its type are set */
    struct SCommand
    {
        TCommandType type;
        CComputeBuffer* buffer;             /**< write, read, fill: the buffer; copy: the destination */
        UInt64 offset;                      /**< write, read, fill: offset in 'buffer'; copy: destination offset */
        UInt64 size;                        /**< bytes */
        CComputeBuffer* sourceBuffer;       /**< copy */
        UInt64 sourceOffset;                /**< copy */
        const void* hostSource;             /**< write: the data, inside the list */
        void* hostDestination;              /**< read */
        UInt32 fillPattern;                 /**< fill */
        CComputeKernel* kernel;             /**< dispatch */
        UInt32 bindingCount;                /**< dispatch */
        SDispatchBinding bindings[ GUCEF_COMPUTE_MAX_BINDINGS ];
        UInt32 constantsSize;               /**< dispatch, including the 8 byte header */
        UInt64 constants[ GUCEF_COMPUTE_MAX_CONSTANTS_SIZE / 8 ];     /**< dispatch, 8 byte aligned */
        CComputeDispatchSize dispatchSize;  /**< dispatch */
        CComputeQueue* waitQueue;           /**< wait */
        UInt64 waitValue;                   /**< wait */

        SCommand( void );
    };

    CComputeCommandList( void );

    CComputeCommandList( const CComputeCommandList& src );

    ~CComputeCommandList();

    CComputeCommandList& operator=( const CComputeCommandList& src );

    /**
     *  Writes 'size' bytes of host memory into the buffer at 'offset'. The data is copied into the list now, so the
     *  memory of the caller is free at once. The buffer needs BUFFERUSAGE_COPY_DST or BUFFERUSAGE_STORAGE.
     */
    TComputeResult WriteFromHost( const TComputeBufferPtr& buffer, UInt64 offset, const void* data, UInt64 size );

    /** Reads 'size' bytes of the buffer at 'offset' into host memory, which must stay valid until the submission completes */
    TComputeResult ReadToHost( const TComputeBufferPtr& buffer, UInt64 offset, void* destination, UInt64 size );

    /** Copies between buffers of the device; ranges in one buffer must not overlap */
    TComputeResult Copy( const TComputeBufferPtr& source, UInt64 sourceOffset, const TComputeBufferPtr& destination, UInt64 destinationOffset, UInt64 size );

    /** Fills a range with a 32 bit pattern; offset and size are multiples of 4 */
    TComputeResult Fill( const TComputeBufferPtr& buffer, UInt64 offset, UInt64 size, UInt32 pattern );

    /**
     *  Runs a kernel. Every slot of its signature must be bound in 'arguments' (and no other), the constants must have
     *  the size of the signature (or be empty for a signature with only the 8 byte header), and the bound ranges must
     *  lie inside their buffers. The arguments are copied.
     */
    TComputeResult Dispatch( const TComputeKernelPtr& kernel, const CComputeArguments& arguments, const CComputeDispatchSize& size );

    /** Later commands of the queue wait for the fence of another queue of the same device */
    TComputeResult WaitFor( const CComputeFence& fence );

    /** Forgets the commands and references, keeps the memory */
    void Clear( void );

    UInt32 GetCommandCount( void ) const;

    bool IsEmpty( void ) const;

    /** COMPUTERESULT_SUCCESS, or the result of the first command that failed to record since the last Clear() */
    TComputeResult GetRecordingResult( void ) const;

    /** The device of the objects recorded; null for an empty list */
    const CComputeDevice* GetDevice( void ) const;

    /**
     *  Reads the next command. Start with a cursor of 0; false at the end. For backends.
     *  'command' is large (it holds a whole dispatch): reuse one per list.
     */
    bool ReadCommand( UInt32& cursor, SCommand& command ) const;

    /** The distinct buffers the list uses */
    UInt32 GetBufferCount( void ) const;
    CComputeBuffer* GetBuffer( UInt32 index ) const;

    /** The bytes of the recording (the size of the command stream) */
    UInt32 GetRecordingSizeInBytes( void ) const;

    private:

    UInt32 AddBuffer( const TComputeBufferPtr& buffer );
    UInt32 AddKernel( const TComputeKernelPtr& kernel );
    UInt32 AddQueue( const TComputeQueuePtr& queue );
    bool CheckDevice( const CComputeDevice* device );
    TComputeResult Fail( TComputeResult result );
    void AppendRecord( UInt32 type, const void* record, UInt32 recordSize, const void* payload, UInt32 payloadSize );

    private:

    CORE::CDynamicBuffer m_stream;
    GUCEF::vector< TComputeBufferPtr > m_buffers;
    GUCEF::vector< TComputeKernelPtr > m_kernels;
    GUCEF::vector< TComputeQueuePtr > m_queues;
    UInt32 m_commandCount;
    TComputeResult m_recordingResult;
    const CComputeDevice* m_device;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPUTE_CCOMPUTECOMMANDLIST_H ? */
