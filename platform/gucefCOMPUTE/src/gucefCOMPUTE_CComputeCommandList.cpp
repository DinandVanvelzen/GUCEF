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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#include "gucefCOMPUTE_CComputeBuffer.h"
#define GUCEF_COMPUTE_CCOMPUTEBUFFER_H
#endif /* GUCEF_COMPUTE_CCOMPUTEBUFFER_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#include "gucefCOMPUTE_CComputeKernel.h"
#define GUCEF_COMPUTE_CCOMPUTEKERNEL_H
#endif /* GUCEF_COMPUTE_CCOMPUTEKERNEL_H ? */

#ifndef GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#include "gucefCOMPUTE_CComputeQueue.h"
#define GUCEF_COMPUTE_CCOMPUTEQUEUE_H
#endif /* GUCEF_COMPUTE_CCOMPUTEQUEUE_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeCommandList.h"

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

/*
 *  The records of the command stream. Each record is a header followed by the record of its type and, for a write,
 *  the data padded to 8 bytes. Records are read back with memcpy, so the stream needs no alignment.
 */

struct SRecordHeader
{
    UInt32 type;
    UInt32 totalSize;       /**< header, record and payload */
};

struct SBufferRangeRecord      // write, read, fill
{
    UInt32 bufferIndex;
    UInt32 fillPattern;
    UInt64 offset;
    UInt64 size;
    void* hostDestination;
};

struct SCopyRecord
{
    UInt32 sourceIndex;
    UInt32 destinationIndex;
    UInt64 sourceOffset;
    UInt64 destinationOffset;
    UInt64 size;
};

struct SBindingRecord
{
    UInt32 slot;
    UInt32 access;
    UInt32 bufferIndex;
    UInt32 padding;
    UInt64 offset;
    UInt64 size;
};

struct SDispatchRecord
{
    UInt32 kernelIndex;
    UInt32 bindingCount;
    SBindingRecord bindings[ GUCEF_COMPUTE_MAX_BINDINGS ];
    UInt32 constantsSize;
    UInt32 isItems;
    UInt32 items;
    UInt32 groups[ 3 ];
    UInt64 constants[ GUCEF_COMPUTE_MAX_CONSTANTS_SIZE / 8 ];
};

struct SWaitRecord
{
    UInt32 queueIndex;
    UInt32 padding;
    UInt64 value;
};

inline UInt32 PadTo8( UInt64 size ) { return static_cast< UInt32 >( ( size + 7 ) & ~static_cast< UInt64 >( 7 ) ); }

bool
IsRangeInside( const CComputeBuffer& buffer, UInt64 offset, UInt64 size )
{
    const UInt64 bufferSize = buffer.GetSizeInBytes();
    return offset <= bufferSize && size <= bufferSize - offset;
}

}

/*-------------------------------------------------------------------------*/

CComputeCommandList::SCommand::SCommand( void )
    : type( COMMANDTYPE_NONE )
    , buffer( GUCEF_NULL )
    , offset( 0 )
    , size( 0 )
    , sourceBuffer( GUCEF_NULL )
    , sourceOffset( 0 )
    , hostSource( GUCEF_NULL )
    , hostDestination( GUCEF_NULL )
    , fillPattern( 0 )
    , kernel( GUCEF_NULL )
    , bindingCount( 0 )
    , constantsSize( 0 )
    , dispatchSize()
    , waitQueue( GUCEF_NULL )
    , waitValue( 0 )
{GUCEF_TRACE;

    memset( bindings, 0, sizeof( bindings ) );
    memset( constants, 0, sizeof( constants ) );
}

/*-------------------------------------------------------------------------*/

CComputeCommandList::CComputeCommandList( void )
    : m_stream( true )
    , m_buffers()
    , m_kernels()
    , m_queues()
    , m_commandCount( 0 )
    , m_recordingResult( COMPUTERESULT_SUCCESS )
    , m_device( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeCommandList::CComputeCommandList( const CComputeCommandList& src )
    : m_stream( src.m_stream )
    , m_buffers( src.m_buffers )
    , m_kernels( src.m_kernels )
    , m_queues( src.m_queues )
    , m_commandCount( src.m_commandCount )
    , m_recordingResult( src.m_recordingResult )
    , m_device( src.m_device )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeCommandList::~CComputeCommandList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeCommandList&
CComputeCommandList::operator=( const CComputeCommandList& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_stream = src.m_stream;
        m_buffers = src.m_buffers;
        m_kernels = src.m_kernels;
        m_queues = src.m_queues;
        m_commandCount = src.m_commandCount;
        m_recordingResult = src.m_recordingResult;
        m_device = src.m_device;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::Fail( TComputeResult result )
{GUCEF_TRACE;

    if ( COMPUTERESULT_SUCCESS == m_recordingResult )
        m_recordingResult = result;
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CComputeCommandList::CheckDevice( const CComputeDevice* device )
{GUCEF_TRACE;

    if ( GUCEF_NULL == device )
        return false;
    if ( GUCEF_NULL == m_device )
    {
        m_device = device;
        return true;
    }
    return m_device == device;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeCommandList::AddBuffer( const TComputeBufferPtr& buffer )
{GUCEF_TRACE;

    const CComputeBuffer* raw = buffer.GetPointerAlways();
    for ( UInt32 i=0; i<static_cast< UInt32 >( m_buffers.size() ); ++i )
        if ( m_buffers[ i ].GetPointerAlways() == raw )
            return i;
    m_buffers.push_back( buffer );
    return static_cast< UInt32 >( m_buffers.size() - 1 );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeCommandList::AddKernel( const TComputeKernelPtr& kernel )
{GUCEF_TRACE;

    const CComputeKernel* raw = kernel.GetPointerAlways();
    for ( UInt32 i=0; i<static_cast< UInt32 >( m_kernels.size() ); ++i )
        if ( m_kernels[ i ].GetPointerAlways() == raw )
            return i;
    m_kernels.push_back( kernel );
    return static_cast< UInt32 >( m_kernels.size() - 1 );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeCommandList::AddQueue( const TComputeQueuePtr& queue )
{GUCEF_TRACE;

    const CComputeQueue* raw = queue.GetPointerAlways();
    for ( UInt32 i=0; i<static_cast< UInt32 >( m_queues.size() ); ++i )
        if ( m_queues[ i ].GetPointerAlways() == raw )
            return i;
    m_queues.push_back( queue );
    return static_cast< UInt32 >( m_queues.size() - 1 );
}

/*-------------------------------------------------------------------------*/

void
CComputeCommandList::AppendRecord( UInt32 type, const void* record, UInt32 recordSize, const void* payload, UInt32 payloadSize )
{GUCEF_TRACE;

    SRecordHeader header;
    header.type = type;
    header.totalSize = static_cast< UInt32 >( sizeof( header ) ) + PadTo8( recordSize ) + PadTo8( payloadSize );

    static const UInt8 zeros[ 8 ] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    m_stream.Append( &header, sizeof( header ), true );
    m_stream.Append( record, recordSize, true );
    if ( PadTo8( recordSize ) != recordSize )
        m_stream.Append( zeros, PadTo8( recordSize ) - recordSize, true );
    if ( payloadSize > 0 )
    {
        m_stream.Append( payload, payloadSize, true );
        if ( PadTo8( payloadSize ) != payloadSize )
            m_stream.Append( zeros, PadTo8( payloadSize ) - payloadSize, true );
    }
    ++m_commandCount;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::WriteFromHost( const TComputeBufferPtr& buffer, UInt64 offset, const void* data, UInt64 size )
{GUCEF_TRACE;

    if ( buffer.IsNULL() || ( size > 0 && GUCEF_NULL == data ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !IsRangeInside( *buffer, offset, size ) || size > 0x7FFFFFF0u )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !buffer->GetDesc().HasUsage( BUFFERUSAGE_COPY_DST ) && !buffer->GetDesc().HasUsage( BUFFERUSAGE_STORAGE ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !CheckDevice( buffer->GetDevice() ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );

    SBufferRangeRecord record;
    memset( &record, 0, sizeof( record ) );
    record.bufferIndex = AddBuffer( buffer );
    record.offset = offset;
    record.size = size;
    AppendRecord( COMMANDTYPE_WRITE_FROM_HOST, &record, sizeof( record ), data, static_cast< UInt32 >( size ) );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::ReadToHost( const TComputeBufferPtr& buffer, UInt64 offset, void* destination, UInt64 size )
{GUCEF_TRACE;

    if ( buffer.IsNULL() || ( size > 0 && GUCEF_NULL == destination ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !IsRangeInside( *buffer, offset, size ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !buffer->GetDesc().HasUsage( BUFFERUSAGE_COPY_SRC ) && !buffer->GetDesc().HasUsage( BUFFERUSAGE_STORAGE ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !CheckDevice( buffer->GetDevice() ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );

    SBufferRangeRecord record;
    memset( &record, 0, sizeof( record ) );
    record.bufferIndex = AddBuffer( buffer );
    record.offset = offset;
    record.size = size;
    record.hostDestination = destination;
    AppendRecord( COMMANDTYPE_READ_TO_HOST, &record, sizeof( record ), GUCEF_NULL, 0 );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::Copy( const TComputeBufferPtr& source, UInt64 sourceOffset, const TComputeBufferPtr& destination, UInt64 destinationOffset, UInt64 size )
{GUCEF_TRACE;

    if ( source.IsNULL() || destination.IsNULL() )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !IsRangeInside( *source, sourceOffset, size ) || !IsRangeInside( *destination, destinationOffset, size ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( source.GetPointerAlways() == destination.GetPointerAlways() )
    {
        const bool disjoint = sourceOffset + size <= destinationOffset || destinationOffset + size <= sourceOffset;
        if ( !disjoint )
            return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    }
    if ( source->GetDevice() != destination->GetDevice() || !CheckDevice( source->GetDevice() ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );

    SCopyRecord record;
    memset( &record, 0, sizeof( record ) );
    record.sourceIndex = AddBuffer( source );
    record.destinationIndex = AddBuffer( destination );
    record.sourceOffset = sourceOffset;
    record.destinationOffset = destinationOffset;
    record.size = size;
    AppendRecord( COMMANDTYPE_COPY, &record, sizeof( record ), GUCEF_NULL, 0 );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::Fill( const TComputeBufferPtr& buffer, UInt64 offset, UInt64 size, UInt32 pattern )
{GUCEF_TRACE;

    if ( buffer.IsNULL() || 0 != ( offset % 4 ) || 0 != ( size % 4 ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !IsRangeInside( *buffer, offset, size ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !CheckDevice( buffer->GetDevice() ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );

    SBufferRangeRecord record;
    memset( &record, 0, sizeof( record ) );
    record.bufferIndex = AddBuffer( buffer );
    record.fillPattern = pattern;
    record.offset = offset;
    record.size = size;
    AppendRecord( COMMANDTYPE_FILL, &record, sizeof( record ), GUCEF_NULL, 0 );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::Dispatch( const TComputeKernelPtr& kernel, const CComputeArguments& arguments, const CComputeDispatchSize& size )
{GUCEF_TRACE;

    if ( kernel.IsNULL() || !kernel->IsBound() )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );

    const CComputeKernelSignature& signature = kernel->GetSignature();
    if ( arguments.GetBoundSlotMask() != signature.GetBoundSlotMask() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeCommandList: Dispatch of kernel \"" << kernel->GetKernelId() << "\" binds the slots of mask " <<
            arguments.GetBoundSlotMask() << ", its signature has mask " << signature.GetBoundSlotMask() );
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    }
    const UInt32 constantsSize = arguments.GetConstantsSizeInBytes();
    if ( constantsSize != signature.GetConstantsSizeInBytes() &&
         !( 0 == constantsSize && GUCEF_COMPUTE_CONSTANTS_HEADER_SIZE == signature.GetConstantsSizeInBytes() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeCommandList: Dispatch of kernel \"" << kernel->GetKernelId() << "\" with " << constantsSize <<
            " bytes of constants, its signature has " << signature.GetConstantsSizeInBytes() );
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    }
    if ( size.GetTotalGroupCount( signature ) > 0xFFFFFFFFu )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );
    if ( !CheckDevice( kernel->GetDevice() ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );

    SDispatchRecord record;
    memset( &record, 0, sizeof( record ) );
    record.bindingCount = signature.GetBindingCount();
    for ( UInt32 i=0; i<record.bindingCount; ++i )
    {
        CComputeKernelSignature::SBinding binding;
        signature.GetBinding( i, binding );
        const CComputeArguments::SBufferBinding& bound = arguments.GetBinding( binding.slot );
        const UInt64 rangeSize = 0 == bound.size ? ( bound.offset <= bound.buffer->GetSizeInBytes() ? bound.buffer->GetSizeInBytes() - bound.offset : 0 ) : bound.size;
        if ( 0 == rangeSize || !IsRangeInside( *bound.buffer, bound.offset, rangeSize ) )
            return Fail( COMPUTERESULT_INVALID_ARGUMENT );
        if ( !bound.buffer->GetDesc().HasUsage( BUFFERUSAGE_STORAGE ) )
            return Fail( COMPUTERESULT_INVALID_ARGUMENT );
        if ( bound.buffer->GetDevice() != kernel->GetDevice() )
            return Fail( COMPUTERESULT_INVALID_ARGUMENT );

        SBindingRecord& out = record.bindings[ i ];
        out.slot = binding.slot;
        out.access = static_cast< UInt32 >( binding.access );
        out.offset = bound.offset;
        out.size = rangeSize;
    }
    // only reference the buffers once every binding is valid
    for ( UInt32 i=0; i<record.bindingCount; ++i )
        record.bindings[ i ].bufferIndex = AddBuffer( arguments.GetBinding( record.bindings[ i ].slot ).buffer );

    record.kernelIndex = AddKernel( kernel );
    record.constantsSize = signature.GetConstantsSizeInBytes();
    if ( constantsSize > 0 )
        memcpy( record.constants, arguments.GetConstants(), constantsSize );
    record.isItems = size.IsItems() ? 1 : 0;
    record.items = size.IsItems() ? size.GetItemCount( signature ) : 0;
    for ( UInt32 axis=0; axis<3; ++axis )
        record.groups[ axis ] = size.GetGroupCount( axis, signature );

    AppendRecord( COMMANDTYPE_DISPATCH, &record, sizeof( record ), GUCEF_NULL, 0 );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::WaitFor( const CComputeFence& fence )
{GUCEF_TRACE;

    if ( !fence.HasQueue() )
        return COMPUTERESULT_SUCCESS;
    if ( !CheckDevice( fence.GetQueue()->GetDevice() ) )
        return Fail( COMPUTERESULT_INVALID_ARGUMENT );

    SWaitRecord record;
    memset( &record, 0, sizeof( record ) );
    record.queueIndex = AddQueue( fence.GetQueue() );
    record.value = fence.GetValue();
    AppendRecord( COMMANDTYPE_WAIT_FOR, &record, sizeof( record ), GUCEF_NULL, 0 );
    return COMPUTERESULT_SUCCESS;
}

/*-------------------------------------------------------------------------*/

void
CComputeCommandList::Clear( void )
{GUCEF_TRACE;

    m_stream.Clear( true );
    m_buffers.clear();
    m_kernels.clear();
    m_queues.clear();
    m_commandCount = 0;
    m_recordingResult = COMPUTERESULT_SUCCESS;
    m_device = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeCommandList::GetCommandCount( void ) const
{GUCEF_TRACE;

    return m_commandCount;
}

/*-------------------------------------------------------------------------*/

bool
CComputeCommandList::IsEmpty( void ) const
{GUCEF_TRACE;

    return 0 == m_commandCount;
}

/*-------------------------------------------------------------------------*/

TComputeResult
CComputeCommandList::GetRecordingResult( void ) const
{GUCEF_TRACE;

    return m_recordingResult;
}

/*-------------------------------------------------------------------------*/

const CComputeDevice*
CComputeCommandList::GetDevice( void ) const
{GUCEF_TRACE;

    return m_device;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeCommandList::GetBufferCount( void ) const
{GUCEF_TRACE;

    return static_cast< UInt32 >( m_buffers.size() );
}

/*-------------------------------------------------------------------------*/

CComputeBuffer*
CComputeCommandList::GetBuffer( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= m_buffers.size() )
        return GUCEF_NULL;
    return const_cast< CComputeBuffer* >( m_buffers[ index ].GetPointerAlways() );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeCommandList::GetRecordingSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_stream.GetDataSize();
}

/*-------------------------------------------------------------------------*/

bool
CComputeCommandList::ReadCommand( UInt32& cursor, SCommand& command ) const
{GUCEF_TRACE;

    const UInt32 streamSize = m_stream.GetDataSize();
    if ( cursor + sizeof( SRecordHeader ) > streamSize )
        return false;

    const UInt8* base = static_cast< const UInt8* >( m_stream.GetConstBufferPtr() );
    SRecordHeader header;
    memcpy( &header, base + cursor, sizeof( header ) );
    if ( header.totalSize < sizeof( header ) || cursor + header.totalSize > streamSize )
        return false;
    const UInt8* record = base + cursor + sizeof( header );

    command.type = static_cast< TCommandType >( header.type );
    switch ( header.type )
    {
        case COMMANDTYPE_WRITE_FROM_HOST:
        case COMMANDTYPE_READ_TO_HOST:
        case COMMANDTYPE_FILL:
        {
            SBufferRangeRecord range;
            memcpy( &range, record, sizeof( range ) );
            command.buffer = GetBuffer( range.bufferIndex );
            command.offset = range.offset;
            command.size = range.size;
            command.fillPattern = range.fillPattern;
            command.hostDestination = range.hostDestination;
            command.hostSource = record + PadTo8( sizeof( range ) );
            break;
        }
        case COMMANDTYPE_COPY:
        {
            SCopyRecord copy;
            memcpy( &copy, record, sizeof( copy ) );
            command.sourceBuffer = GetBuffer( copy.sourceIndex );
            command.sourceOffset = copy.sourceOffset;
            command.buffer = GetBuffer( copy.destinationIndex );
            command.offset = copy.destinationOffset;
            command.size = copy.size;
            break;
        }
        case COMMANDTYPE_DISPATCH:
        {
            SDispatchRecord dispatch;
            memcpy( &dispatch, record, sizeof( dispatch ) );
            command.kernel = const_cast< CComputeKernel* >( m_kernels[ dispatch.kernelIndex ].GetPointerAlways() );
            command.bindingCount = dispatch.bindingCount;
            for ( UInt32 i=0; i<dispatch.bindingCount; ++i )
            {
                SDispatchBinding& out = command.bindings[ i ];
                out.slot = dispatch.bindings[ i ].slot;
                out.access = static_cast< TComputeAccess >( dispatch.bindings[ i ].access );
                out.buffer = GetBuffer( dispatch.bindings[ i ].bufferIndex );
                out.offset = dispatch.bindings[ i ].offset;
                out.size = dispatch.bindings[ i ].size;
            }
            command.constantsSize = dispatch.constantsSize;
            memcpy( command.constants, dispatch.constants, sizeof( command.constants ) );
            command.dispatchSize = 0 != dispatch.isItems ? CComputeDispatchSize::ForItems( dispatch.items )
                                                         : CComputeDispatchSize::ForGroups( dispatch.groups[ 0 ], dispatch.groups[ 1 ], dispatch.groups[ 2 ] );
            break;
        }
        case COMMANDTYPE_WAIT_FOR:
        {
            SWaitRecord wait;
            memcpy( &wait, record, sizeof( wait ) );
            command.waitQueue = const_cast< CComputeQueue* >( m_queues[ wait.queueIndex ].GetPointerAlways() );
            command.waitValue = wait.value;
            break;
        }
        default:
        {
            return false;
        }
    }
    cursor += header.totalSize;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
