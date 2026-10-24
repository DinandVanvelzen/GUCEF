/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#include "gucefDRN_CDRNBulkTransfer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Chunks a transfer sends per tick at most */
static const UInt32 MaxChunksPerTick = 8;

/** Reliable messages a datagram link may have unacknowledged before a transfer waits */
static const UInt32 MaxUnackedForChunks = 64;

/** Block size of the hash task reading a file */
static const UInt32 HashReadSize = 64 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CDRNBulkTransfer::ProgressEvent = "GUCEF::DRN::CDRNBulkTransfer::ProgressEvent";
const CORE::CEvent CDRNBulkTransfer::CompletedEvent = "GUCEF::DRN::CDRNBulkTransfer::CompletedEvent";
const CORE::CEvent CDRNBulkTransfer::FailedEvent = "GUCEF::DRN::CDRNBulkTransfer::FailedEvent";
const CORE::CEvent CDRNBulkTransferManager::IncomingTransferEvent = "GUCEF::DRN::CDRNBulkTransferManager::IncomingTransferEvent";
const CORE::CString CDRNHashTaskConsumer::TaskType = "GUCEF::DRN::CDRNHashTaskConsumer";

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::RegisterEvents( void )
{GUCEF_TRACE;

    ProgressEvent.Initialize();
    CompletedEvent.Initialize();
    FailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransferManager::RegisterEvents( void )
{GUCEF_TRACE;

    IncomingTransferEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransferInfo::CDRNBulkTransferInfo( void )
    : peerId( 0 )
    , transferId( 0 )
    , name()
    , size( 0 )
    , isIncoming( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNHashTaskData::CDRNHashTaskData( void )
    : CORE::CICloneable()
    , fromFile( false )
    , filePath()
    , data()
    , size( 0 )
    , succeeded( false )
{GUCEF_TRACE;

    memset( digest, 0, sizeof( digest ) );
}

/*-------------------------------------------------------------------------*/

CDRNHashTaskData::CDRNHashTaskData( const CDRNHashTaskData& src )
    : CORE::CICloneable( src )
    , fromFile( src.fromFile )
    , filePath( src.filePath )
    , data( src.data )
    , size( src.size )
    , succeeded( src.succeeded )
{GUCEF_TRACE;

    memcpy( digest, src.digest, sizeof( digest ) );
}

/*-------------------------------------------------------------------------*/

CDRNHashTaskData::~CDRNHashTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDRNHashTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDRNHashTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CDRNHashTaskConsumer::CDRNHashTaskConsumer( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNHashTaskConsumer::~CDRNHashTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNHashTaskConsumer::OnTaskEnded( CORE::CTaskPtr /* task */, bool /* wasForced */ )
{GUCEF_TRACE;

    // the task data stays: it has the digest for the transfer
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNHashTaskConsumer::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CDRNHashTaskConsumer::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CDRNHashTaskData* data = static_cast< CDRNHashTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return true;

    CORE::CSha256 sha;
    data->succeeded = false;
    data->size = 0;
    if ( data->fromFile )
    {
        CORE::CFileAccess file( data->filePath, "rb" );
        if ( !file.Opened() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNHashTask: Cannot open \"" << data->filePath << "\"" );
            return true;
        }
        CORE::CDynamicBuffer block( HashReadSize, true );
        while ( !file.Eof() )
        {
            if ( task->IsCancellationRequested() )
                return true;
            UInt32 read = file.Read( block.GetBufferPtr(), 1, HashReadSize );
            if ( 0 == read )
                break;
            sha.Update( block.GetConstBufferPtr(), read );
            data->size += read;
        }
        file.Close();
    }
    else
    {
        sha.Update( data->data.GetConstBufferPtr(), data->data.GetDataSize() );
        data->size = data->data.GetDataSize();
    }
    sha.Final( data->digest );
    data->succeeded = true;
    return true;
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer::CDRNBulkTransfer( CDRNBulkTransferManager& manager )
    : CORE::CTSGNotifier( manager.GetSession().GetNode().GetAppPulseGenerator(), true, true )
    , m_manager( &manager )
    , m_info()
    , m_state( TRANSFERSTATE_OFFERED )
    , m_toFile( false )
    , m_filePath()
    , m_file()
    , m_data()
    , m_resumeOffset( 0 )
    , m_nextOffset( 0 )
    , m_bytesDone( 0 )
    , m_progressDue( false )
    , m_hashFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
    , m_failureReason()
    , m_chunk()
{GUCEF_TRACE;

    RegisterEvents();
    memset( m_sha256, 0, sizeof( m_sha256 ) );
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer::~CDRNBulkTransfer()
{GUCEF_TRACE;

    m_file.Close();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNBulkTransfer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNBulkTransfer";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_manager->GetSession().GetNode().IsRunningOwnThread() )
        NotifyObserversFromThread( eventid, eventData );
    else
        NotifyObservers( eventid, eventData );
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer::TTransferState
CDRNBulkTransfer::GetState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    return m_state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransfer::IsFinished( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    return TRANSFERSTATE_COMPLETED == m_state || TRANSFERSTATE_FAILED == m_state;
}

/*-------------------------------------------------------------------------*/

const CDRNBulkTransferInfo&
CDRNBulkTransfer::GetInfo( void ) const
{GUCEF_TRACE;

    return m_info;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNBulkTransfer::GetBytesDone( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    return m_bytesDone;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNBulkTransfer::GetResumeOffset( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    return m_resumeOffset;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNBulkTransfer::GetFailureReason( void ) const
{GUCEF_TRACE;

    return m_failureReason;
}

/*-------------------------------------------------------------------------*/

const CORE::CDynamicBuffer&
CDRNBulkTransfer::GetReceivedData( void ) const
{GUCEF_TRACE;

    return m_data;
}

/*-------------------------------------------------------------------------*/

const UInt8*
CDRNBulkTransfer::GetSha256( void ) const
{GUCEF_TRACE;

    return m_sha256;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransfer::StartHashing( void )
{GUCEF_TRACE;

    CDRNHashTaskData taskData;
    taskData.fromFile = m_toFile;
    taskData.filePath = m_filePath;
    if ( !m_toFile )
        taskData.data = m_data;

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( CORE::CTaskManager::DefaultThreadPoolName );
    if ( threadPool.IsNULL() )
        return false;
    m_hashFuture = threadPool->QueueTask( CDRNHashTaskConsumer::TaskType, &taskData );
    return m_hashFuture.HasAFuture();
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransfer::TryTakeHash( UInt8* digest, UInt64& size, bool& ok )
{GUCEF_TRACE;

    ok = false;
    if ( m_hashFuture.HasNoFuture() )
        return true;
    if ( !m_hashFuture.IsReady() )
        return false;
    CORE::CTaskPtr task = m_hashFuture.GetResult();
    m_hashFuture = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    if ( task.IsNULL() )
        return true;
    const CDRNHashTaskData* data = static_cast< const CDRNHashTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data || !data->succeeded )
        return true;
    memcpy( digest, data->digest, CORE::CSha256::DigestSize );
    size = data->size;
    ok = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransfer::Accept( const CORE::CString& targetFilePath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    if ( !m_info.isIncoming || TRANSFERSTATE_OFFERED != m_state )
        return false;

    m_toFile = true;
    m_filePath = targetFilePath;
    m_resumeOffset = CORE::FileExists( targetFilePath ) ? CORE::FileSize( targetFilePath ) : 0;
    if ( m_resumeOffset > m_info.size )
        m_resumeOffset = 0;
    if ( !m_file.Open( targetFilePath, 0 == m_resumeOffset ? "w+b" : "r+b" ) )
    {
        Fail( "cannot write \"" + targetFilePath + "\"", true );
        return false;
    }
    m_bytesDone = m_resumeOffset;

    UInt8 body[ 16 ];
    UInt32 bodySize = CDRNWireWriter::EncodeVarUInt64( m_resumeOffset, body );
    m_manager->Send( m_info.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_BULK_ACCEPT, m_info.peerId, m_info.transferId, body, bodySize );
    m_state = TRANSFERSTATE_TRANSFERRING;
    if ( m_bytesDone >= m_info.size )
    {
        m_file.Close();
        m_state = TRANSFERSTATE_VERIFYING;
        if ( !StartHashing() )
            Fail( "cannot hash what arrived", true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransfer::Accept( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    if ( !m_info.isIncoming || TRANSFERSTATE_OFFERED != m_state )
        return false;

    m_toFile = false;
    m_data.SetDataSize( 0 );
    m_resumeOffset = 0;
    m_bytesDone = 0;
    UInt8 body[ 16 ];
    UInt32 bodySize = CDRNWireWriter::EncodeVarUInt64( 0, body );
    m_manager->Send( m_info.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_BULK_ACCEPT, m_info.peerId, m_info.transferId, body, bodySize );
    m_state = TRANSFERSTATE_TRANSFERRING;
    if ( 0 == m_info.size )
    {
        m_state = TRANSFERSTATE_VERIFYING;
        if ( !StartHashing() )
            Fail( "cannot hash what arrived", true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::Reject( const CORE::CString& reason )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    if ( !m_info.isIncoming || TRANSFERSTATE_OFFERED != m_state )
        return;
    Fail( reason.IsNULLOrEmpty() ? CORE::CString( "rejected" ) : reason, true );
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::Cancel( const CORE::CString& reason )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_manager->GetSession().GetNode().GetLock() );
    if ( TRANSFERSTATE_COMPLETED == m_state || TRANSFERSTATE_FAILED == m_state )
        return;
    Fail( reason.IsNULLOrEmpty() ? CORE::CString( "cancelled" ) : reason, true );
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::Fail( const CORE::CString& reason, bool tellPeer )
{GUCEF_TRACE;

    if ( TRANSFERSTATE_COMPLETED == m_state || TRANSFERSTATE_FAILED == m_state )
        return;
    m_state = TRANSFERSTATE_FAILED;
    m_failureReason = reason;
    m_file.Close();
    if ( tellPeer )
    {
        m_manager->m_txBuffer.SetDataSize( 0 );
        CDRNWireWriter writer( m_manager->m_txBuffer );
        writer.WriteString( reason );
        CORE::CDynamicBuffer body;
        body.CopyFrom( writer.GetOffset(), m_manager->m_txBuffer.GetConstBufferPtr() );
        UInt32 senderPeerId = m_info.isIncoming ? m_info.peerId : m_manager->GetSession().GetLocalPeerId();
        m_manager->Send( m_info.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_BULK_CANCEL, senderPeerId, m_info.transferId, body.GetConstBufferPtr(), body.GetDataSize() );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNBulkTransfer: \"" << m_info.name << "\" " << ( m_info.isIncoming ? "from " : "to " ) << m_info.peerId << " failed: " << reason );
    CORE::TCloneableString eventData( reason );
    Emit( FailedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::Complete( void )
{GUCEF_TRACE;

    m_state = TRANSFERSTATE_COMPLETED;
    m_file.Close();
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNBulkTransfer: \"" << m_info.name << "\" " << ( m_info.isIncoming ? "from " : "to " ) << m_info.peerId << " completed, " << m_info.size << " bytes" );
    Emit( CompletedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransfer::ReadSource( UInt64 offset, UInt32 size, CORE::CDynamicBuffer& dest )
{GUCEF_TRACE;
    dest.SetDataSize( 0 );
    if ( !m_toFile )
    {
        if ( offset + size > m_data.GetDataSize() )
            return false;
        dest.CopyFrom( size, m_data.GetConstBufferPtr( (UInt32) offset ) );
        return true;
    }
    if ( !m_file.Opened() && !m_file.Open( m_filePath, "rb" ) )
        return false;
    if ( 0 != m_file.Setpos( offset ) )
        return false;
    if ( dest.GetBufferSize() < size )
        dest.SetBufferSize( size, false );
    UInt32 read = m_file.Read( dest.GetBufferPtr(), 1, size );
    dest.SetDataSize( read );
    return read == size;
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::SendChunks( void )
{GUCEF_TRACE;
    UInt32 localPeerId = m_manager->GetSession().GetLocalPeerId();
    for ( UInt32 n=0; n<MaxChunksPerTick && m_nextOffset < m_info.size; ++n )
    {
        if ( !m_manager->CanSendChunk( m_info.peerId ) )
            break;
        UInt64 left = m_info.size - m_nextOffset;
        UInt32 size = left < ChunkSize ? (UInt32) left : ChunkSize;
        if ( !ReadSource( m_nextOffset, size, m_chunk ) )
        {
            Fail( "cannot read the source", true );
            return;
        }
        m_manager->m_txBuffer.SetDataSize( 0 );
        CDRNWireWriter writer( m_manager->m_txBuffer );
        writer.WriteVarUInt64( m_nextOffset );
        writer.WriteBytes( m_chunk.GetConstBufferPtr(), size );
        if ( !m_manager->Send( m_info.peerId, DRN_LANE_RELIABLE_UNORDERED, DRN_BULK_CHUNK, localPeerId, m_info.transferId, m_manager->m_txBuffer.GetConstBufferPtr(), writer.GetOffset() ) )
            break;
        m_nextOffset += size;
        m_bytesDone = m_nextOffset;
        m_progressDue = true;
    }
    if ( m_nextOffset >= m_info.size )
    {
        // everything went out: the receiver confirms with DONE once it checked the hash
        m_file.Close();
        m_state = TRANSFERSTATE_VERIFYING;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::OnChunk( CDRNWireReader& reader )
{GUCEF_TRACE;
    UInt64 offset = 0;
    if ( TRANSFERSTATE_TRANSFERRING != m_state || !reader.ReadVarUInt64( offset ) )
        return;
    const UInt8* bytes = reader.GetCurrentPtr();
    UInt32 size = reader.GetRemaining();
    if ( offset < m_resumeOffset || offset + size > m_info.size )
    {
        Fail( "a chunk outside of the transfer", true );
        return;
    }
    if ( m_toFile )
    {
        if ( 0 != m_file.Setpos( offset ) || size != m_file.Write( bytes, 1, size ) )
        {
            Fail( "cannot write \"" + m_filePath + "\"", true );
            return;
        }
    }
    else
    {
        m_data.CopyFrom( (UInt32) offset, size, bytes );
    }
    m_bytesDone += size;
    m_progressDue = true;

    if ( m_bytesDone >= m_info.size )
    {
        m_file.Close();
        m_state = TRANSFERSTATE_VERIFYING;
        if ( !StartHashing() )
            Fail( "cannot hash what arrived", true );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::OnMessage( UInt32 opcode, CDRNWireReader& reader )
{GUCEF_TRACE;

    switch ( opcode )
    {
        case DRN_BULK_ACCEPT:
        {
            UInt64 resumeOffset = 0;
            if ( m_info.isIncoming || TRANSFERSTATE_OFFERED != m_state || !reader.ReadVarUInt64( resumeOffset ) )
                return;
            m_resumeOffset = resumeOffset > m_info.size ? m_info.size : resumeOffset;
            m_nextOffset = m_resumeOffset;
            m_bytesDone = m_resumeOffset;
            m_state = TRANSFERSTATE_TRANSFERRING;
            if ( m_nextOffset >= m_info.size )
                m_state = TRANSFERSTATE_VERIFYING;
            break;
        }
        case DRN_BULK_CHUNK:
        {
            if ( m_info.isIncoming )
                OnChunk( reader );
            break;
        }
        case DRN_BULK_DONE:
        {
            UInt8 hashOk = 0;
            if ( m_info.isIncoming || TRANSFERSTATE_VERIFYING != m_state || !reader.ReadUInt8( hashOk ) )
                return;
            if ( 0 != hashOk )
                Complete();
            else
                Fail( "the receiver got another hash", false );
            break;
        }
        case DRN_BULK_CANCEL:
        {
            CORE::CString reason;
            reader.ReadString( reason );
            Fail( "the other end stopped: " + reason, false );
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransfer::Tick( UInt64 /* nodeNow */ )
{GUCEF_TRACE;
    if ( TRANSFERSTATE_HASHING == m_state || ( TRANSFERSTATE_VERIFYING == m_state && m_info.isIncoming ) )
    {
        UInt8 digest[ CORE::CSha256::DigestSize ];
        UInt64 size = 0;
        bool ok = false;
        if ( TryTakeHash( digest, size, ok ) )
        {
            if ( TRANSFERSTATE_HASHING == m_state )
            {
                if ( !ok )
                {
                    Fail( "cannot read the source", false );
                    return;
                }
                memcpy( m_sha256, digest, sizeof( m_sha256 ) );
                m_info.size = size;
                m_state = TRANSFERSTATE_OFFERED;

                m_manager->m_txBuffer.SetDataSize( 0 );
                CDRNWireWriter writer( m_manager->m_txBuffer );
                writer.WriteString( m_info.name );
                writer.WriteVarUInt64( m_info.size );
                writer.WriteBytes( m_sha256, sizeof( m_sha256 ) );
                m_manager->Send( m_info.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_BULK_OFFER, m_manager->GetSession().GetLocalPeerId(), m_info.transferId,
                                 m_manager->m_txBuffer.GetConstBufferPtr(), writer.GetOffset() );
            }
            else
            {
                bool match = ok && size == m_info.size && CORE::CSha256::ConstantTimeEquals( digest, m_sha256, CORE::CSha256::DigestSize );
                UInt8 body = match ? 1 : 0;
                m_manager->Send( m_info.peerId, DRN_LANE_RELIABLE_ORDERED, DRN_BULK_DONE, m_info.peerId, m_info.transferId, &body, 1 );
                if ( match )
                    Complete();
                else
                    Fail( "what arrived has another hash", false );
                return;
            }
        }
    }

    if ( TRANSFERSTATE_TRANSFERRING == m_state && !m_info.isIncoming )
        SendChunks();

    if ( m_progressDue )
    {
        m_progressDue = false;
        CORE::TCloneableUInt64 eventData( m_bytesDone );
        Emit( ProgressEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransferManager::CDRNBulkTransferManager( CDRNSession& session )
    : CORE::CTSGNotifier( session.GetNode().GetAppPulseGenerator(), true, true )
    , CIDRNSessionComponent()
    , m_session( &session )
    , m_transfers()
    , m_nextTransferId( 1 )
    , m_txBuffer()
{GUCEF_TRACE;

    RegisterEvents();
    CDRNBulkTransfer::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransferManager::~CDRNBulkTransferManager()
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        GUCEF_DELETE m_transfers[ i ];
    m_transfers.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNBulkTransferManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNBulkTransferManager";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CDRNSession&
CDRNBulkTransferManager::GetSession( void ) const
{GUCEF_TRACE;

    return *m_session;
}

/*-------------------------------------------------------------------------*/

UInt8
CDRNBulkTransferManager::GetMessageClass( void ) const
{GUCEF_TRACE;

    return DRN_MSGCLASS_BULK;
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransferManager::Emit( const CORE::CEvent& eventid, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_session->GetNode().IsRunningOwnThread() )
        NotifyObserversFromThread( eventid, eventData );
    else
        NotifyObservers( eventid, eventData );
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransferManager::Reset( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        GUCEF_DELETE m_transfers[ i ];
    m_transfers.clear();
    m_nextTransferId = 1;
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer*
CDRNBulkTransferManager::AddOutgoing( UInt32 toPeerId, const CORE::CString& name )
{GUCEF_TRACE;

    if ( !m_session->IsActive() || toPeerId == m_session->GetLocalPeerId() || !m_session->HasMember( toPeerId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNBulkTransferManager: Cannot offer \"" << name << "\" to member " << toPeerId );
        return GUCEF_NULL;
    }
    CDRNBulkTransfer* transfer = GUCEF_NEW CDRNBulkTransfer( *this );
    transfer->m_info.peerId = toPeerId;
    transfer->m_info.transferId = m_nextTransferId++;
    transfer->m_info.name = name;
    transfer->m_info.isIncoming = false;
    transfer->m_state = CDRNBulkTransfer::TRANSFERSTATE_HASHING;
    m_transfers.push_back( transfer );
    return transfer;
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer*
CDRNBulkTransferManager::OfferFile( UInt32 toPeerId, const CORE::CString& name, const CORE::CString& filePath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    if ( !CORE::FileExists( filePath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNBulkTransferManager: There is no file \"" << filePath << "\" to offer" );
        return GUCEF_NULL;
    }
    CDRNBulkTransfer* transfer = AddOutgoing( toPeerId, name );
    if ( GUCEF_NULL == transfer )
        return GUCEF_NULL;
    transfer->m_toFile = true;
    transfer->m_filePath = filePath;
    if ( !transfer->StartHashing() )
        transfer->Fail( "cannot hash the source", false );
    return transfer;
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer*
CDRNBulkTransferManager::OfferData( UInt32 toPeerId, const CORE::CString& name, const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    CDRNBulkTransfer* transfer = AddOutgoing( toPeerId, name );
    if ( GUCEF_NULL == transfer )
        return GUCEF_NULL;
    transfer->m_toFile = false;
    transfer->m_data.CopyFrom( data.GetDataSize(), data.GetConstBufferPtr() );
    if ( !transfer->StartHashing() )
        transfer->Fail( "cannot hash the source", false );
    return transfer;
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer*
CDRNBulkTransferManager::FindTransfer( UInt32 peerId, UInt32 transferId, bool incoming ) const
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_transfers.size(); ++i )
    {
        CDRNBulkTransfer* transfer = m_transfers[ i ];
        if ( transfer->m_info.transferId == transferId && transfer->m_info.peerId == peerId && transfer->m_info.isIncoming == incoming )
            return transfer;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CDRNBulkTransfer*
CDRNBulkTransferManager::GetIncoming( UInt32 fromPeerId, UInt32 transferId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return FindTransfer( fromPeerId, transferId, true );
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransferManager::Release( CDRNBulkTransfer* transfer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    for ( UInt32 i=0; i<m_transfers.size(); ++i )
    {
        if ( m_transfers[ i ] == transfer )
        {
            if ( CDRNBulkTransfer::TRANSFERSTATE_COMPLETED != transfer->m_state && CDRNBulkTransfer::TRANSFERSTATE_FAILED != transfer->m_state )
                return false;
            m_transfers.erase( m_transfers.begin() + i );
            GUCEF_DELETE transfer;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransferManager::Send( UInt32 toPeerId, UInt8 lane, UInt32 opcode, UInt32 senderPeerId, UInt32 transferId, const void* body, UInt32 bodySize )
{GUCEF_TRACE;
    // directly when there is a link with the member, else through the sequencer
    CDRNPeerLinkPtr link = m_session->GetLinkToMember( toPeerId );
    if ( link.IsNULL() && !m_session->IsSequencer() )
        link = m_session->GetLinkToMember( m_session->GetSequencerPeerId() );
    if ( link.IsNULL() )
        return false;

    UInt8 header[ 24 ];
    UInt32 headerSize = CDRNWireWriter::EncodeVarUInt64( m_session->GetLocalPeerId(), header );
    headerSize += CDRNWireWriter::EncodeVarUInt64( toPeerId, header + headerSize );
    headerSize += CDRNWireWriter::EncodeVarUInt64( senderPeerId, header + headerSize );
    headerSize += CDRNWireWriter::EncodeVarUInt64( transferId, header + headerSize );
    return link->SendGather( lane, DRN_MSGCLASS_BULK, opcode, header, headerSize, body, bodySize );
}

/*-------------------------------------------------------------------------*/

bool
CDRNBulkTransferManager::CanSendChunk( UInt32 toPeerId ) const
{GUCEF_TRACE;
    CDRNPeerLinkPtr link = m_session->GetLinkToMember( toPeerId );
    if ( link.IsNULL() && !m_session->IsSequencer() )
        link = m_session->GetLinkToMember( m_session->GetSequencerPeerId() );
    if ( link.IsNULL() )
        return false;
    return link->GetUnackedCount() < MaxUnackedForChunks;
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransferManager::OnSessionMessage( CDRNSession& session, CDRNPeerLink& /* link */, UInt32 linkPeerId, const CDRNLinkMessage& message )
{GUCEF_TRACE;

    CDRNWireReader reader( message.payload );
    UInt32 fromPeerId = 0;
    UInt32 toPeerId = 0;
    UInt32 senderPeerId = 0;
    UInt32 transferId = 0;
    if ( !reader.ReadVarUInt32( fromPeerId ) || !reader.ReadVarUInt32( toPeerId ) || !reader.ReadVarUInt32( senderPeerId ) || !reader.ReadVarUInt32( transferId ) )
        return;

    // a member speaks for itself, the sequencer passes on what others send
    if ( fromPeerId != linkPeerId && linkPeerId != session.GetSequencerPeerId() )
        return;

    if ( toPeerId != session.GetLocalPeerId() )
    {
        if ( session.IsSequencer() )
        {
            CDRNPeerLinkPtr target = session.GetLinkToMember( toPeerId );
            if ( !target.IsNULL() )
                target->Send( message.lane, DRN_MSGCLASS_BULK, message.opcode, message.payload.GetConstBufferPtr(), message.payload.GetDataSize() );
        }
        return;
    }

    bool incoming = senderPeerId != session.GetLocalPeerId();
    if ( incoming && senderPeerId != fromPeerId )
        return;

    if ( DRN_BULK_OFFER == message.opcode )
    {
        if ( !incoming || GUCEF_NULL != FindTransfer( senderPeerId, transferId, true ) )
            return;
        CDRNBulkTransfer* transfer = GUCEF_NEW CDRNBulkTransfer( *this );
        transfer->m_info.peerId = senderPeerId;
        transfer->m_info.transferId = transferId;
        transfer->m_info.isIncoming = true;
        if ( !reader.ReadString( transfer->m_info.name ) || !reader.ReadVarUInt64( transfer->m_info.size ) ||
             !reader.ReadBytes( transfer->m_sha256, CORE::CSha256::DigestSize ) )
        {
            GUCEF_DELETE transfer;
            return;
        }
        transfer->m_state = CDRNBulkTransfer::TRANSFERSTATE_OFFERED;
        m_transfers.push_back( transfer );

        CDRNBulkTransferEventData eventData( transfer->m_info );
        Emit( IncomingTransferEvent, &eventData );
        return;
    }

    CDRNBulkTransfer* transfer = FindTransfer( incoming ? senderPeerId : fromPeerId, transferId, incoming );
    if ( GUCEF_NULL != transfer )
        transfer->OnMessage( message.opcode, reader );
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransferManager::OnSessionTick( CDRNSession& /* session */, UInt64 nodeNow )
{GUCEF_TRACE;
    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        m_transfers[ i ]->Tick( nodeNow );
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransferManager::OnMemberLeft( CDRNSession& /* session */, UInt32 peerId )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        if ( m_transfers[ i ]->m_info.peerId == peerId )
            m_transfers[ i ]->Fail( "the member left", false );
}

/*-------------------------------------------------------------------------*/

void
CDRNBulkTransferManager::OnSessionClosed( CDRNSession& /* session */ )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<m_transfers.size(); ++i )
        m_transfers[ i ]->Fail( "the session closed", false );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
