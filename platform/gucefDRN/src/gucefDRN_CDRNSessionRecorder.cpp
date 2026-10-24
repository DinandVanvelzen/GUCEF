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

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#include "gucefDRN_CDRNSessionRecorder.h"

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

static const char RecordingMagic[ 8 ] = { 'D', 'R', 'N', 'R', 'E', 'C', '0', '1' };
static const UInt8 RecordTypeMessage = 1;
static const UInt8 RecordFlagOutgoing = 0x01;
static const UInt32 FlushThreshold = 64 * 1024;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNRecordedMessage::CDRNRecordedMessage( void )
    : timeUs( 0 )
    , outgoing( false )
    , peerId( 0 )
    , linkId( 0 )
    , lane( 0 )
    , msgClass( 0 )
    , opcode( 0 )
    , payload()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSessionRecorder::CDRNSessionRecorder( void )
    : m_session( GUCEF_NULL )
    , m_filePath()
    , m_classMask( DefaultClassMask )
    , m_startTime( 0 )
    , m_recordCount( 0 )
    , m_failed( false )
    , m_pending()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSessionRecorder::~CDRNSessionRecorder()
{GUCEF_TRACE;

    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRecorder::Start( CDRNSession& session, const CORE::CString& filePath, UInt32 classMask )
{GUCEF_TRACE;

    Stop();

    MT::CScopeMutex lock( session.GetNode().GetLock() );

    m_filePath = filePath;
    m_classMask = classMask;
    m_startTime = CDRNClock::NowInMicroseconds();
    m_recordCount = 0;
    m_failed = false;

    CORE::CDynamicBuffer header;
    CDRNWireWriter writer( header );
    writer.WriteBytes( RecordingMagic, sizeof( RecordingMagic ) );
    writer.WriteUInt8( FileVersion );
    writer.WriteString( session.GetName() );
    writer.WriteVarUInt64( m_startTime );
    writer.WriteVarUInt32( session.GetLocalPeerId() );
    if ( !header.WriteContentToFile( m_filePath, 0, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSessionRecorder: Cannot write the recording " << m_filePath );
        return false;
    }

    m_pending.SetDataSize( 0 );
    m_session = &session;
    session.SetRecorder( this );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNSessionRecorder: Recording session \"" << session.GetName() << "\" to " << m_filePath );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNSessionRecorder::Stop( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_session )
        return;

    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    m_session->SetRecorder( GUCEF_NULL );
    m_session = GUCEF_NULL;
    FlushImp();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNSessionRecorder: Recorded " << m_recordCount << " messages to " << m_filePath );
}

/*-------------------------------------------------------------------------*/

void
CDRNSessionRecorder::OnSessionGone( void )
{GUCEF_TRACE;

    m_session = GUCEF_NULL;
    FlushImp();
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRecorder::IsRecording( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_session;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRecorder::Flush( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_session )
        return FlushImp();
    MT::CScopeMutex lock( m_session->GetNode().GetLock() );
    return FlushImp();
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRecorder::FlushImp( void )
{GUCEF_TRACE;

    if ( 0 == m_pending.GetDataSize() )
        return !m_failed;
    if ( !m_pending.AppendContentToFile( m_filePath ) )
    {
        if ( !m_failed )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSessionRecorder: Cannot append to the recording " << m_filePath );
        m_failed = true;
    }
    m_pending.SetDataSize( 0 );
    return !m_failed;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNSessionRecorder::GetRecordCount( void ) const
{GUCEF_TRACE;

    return m_recordCount;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNSessionRecorder::GetFilePath( void ) const
{GUCEF_TRACE;

    return m_filePath;
}

/*-------------------------------------------------------------------------*/

void
CDRNSessionRecorder::OnTraffic( const CDRNPeerLink& link       ,
                                bool outgoing                  ,
                                UInt8 lane                     ,
                                UInt8 msgClass                 ,
                                UInt32 opcode                  ,
                                const void* part1              ,
                                UInt32 size1                   ,
                                const void* part2              ,
                                UInt32 size2                   )
{GUCEF_TRACE;

    if ( msgClass >= 32 || 0 == ( m_classMask & ( 1u << msgClass ) ) )
        return;

    UInt64 now = CDRNClock::NowInMicroseconds();
    CDRNWireWriter writer( m_pending, m_pending.GetDataSize() );
    writer.WriteUInt8( RecordTypeMessage );
    writer.WriteVarUInt64( now >= m_startTime ? now - m_startTime : 0 );
    writer.WriteUInt8( outgoing ? RecordFlagOutgoing : 0 );
    writer.WriteVarUInt32( link.GetSessionPeerId() );
    writer.WriteVarUInt32( link.GetLinkId() );
    writer.WriteUInt8( lane );
    writer.WriteUInt8( msgClass );
    writer.WriteVarUInt32( opcode );
    writer.WriteVarUInt32( size1 + size2 );
    if ( size1 > 0 )
        writer.WriteBytes( part1, size1 );
    if ( size2 > 0 )
        writer.WriteBytes( part2, size2 );
    ++m_recordCount;

    if ( m_pending.GetDataSize() >= FlushThreshold )
        FlushImp();
}

/*-------------------------------------------------------------------------*/

CDRNSessionRecordingReader::CDRNSessionRecordingReader( void )
    : m_data()
    , m_offset( 0 )
    , m_sessionName()
    , m_startTime( 0 )
    , m_localPeerId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRecordingReader::Open( const CORE::CString& filePath )
{GUCEF_TRACE;

    m_data.Clear();
    m_offset = 0;
    m_sessionName.Clear();
    m_startTime = 0;
    m_localPeerId = 0;

    if ( !m_data.LoadContentFromFile( filePath ) )
        return false;

    CDRNWireReader reader( m_data );
    char magic[ sizeof( RecordingMagic ) ];
    UInt8 version = 0;
    if ( !reader.ReadBytes( magic, sizeof( magic ) ) || 0 != memcmp( magic, RecordingMagic, sizeof( magic ) ) ||
         !reader.ReadUInt8( version ) || CDRNSessionRecorder::FileVersion != version ||
         !reader.ReadString( m_sessionName ) || !reader.ReadVarUInt64( m_startTime ) || !reader.ReadVarUInt32( m_localPeerId ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNSessionRecordingReader: " << filePath << " is not a DRN session recording" );
        m_data.Clear();
        return false;
    }
    m_offset = reader.GetPosition();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRecordingReader::ReadNext( CDRNRecordedMessage& message )
{GUCEF_TRACE;

    if ( IsAtEnd() )
        return false;

    CDRNWireReader reader( m_data, m_offset );
    UInt8 type = 0;
    UInt8 flags = 0;
    const UInt8* payload = GUCEF_NULL;
    UInt32 payloadSize = 0;
    if ( !reader.ReadUInt8( type ) || RecordTypeMessage != type ||
         !reader.ReadVarUInt64( message.timeUs ) || !reader.ReadUInt8( flags ) ||
         !reader.ReadVarUInt32( message.peerId ) || !reader.ReadVarUInt32( message.linkId ) ||
         !reader.ReadUInt8( message.lane ) || !reader.ReadUInt8( message.msgClass ) ||
         !reader.ReadVarUInt32( message.opcode ) || !reader.ReadSizedBytes( payload, payloadSize ) )
    {
        m_offset = m_data.GetDataSize();
        return false;
    }
    message.outgoing = 0 != ( flags & RecordFlagOutgoing );
    message.payload.SetDataSize( 0 );
    if ( payloadSize > 0 )
        message.payload.CopyFrom( 0, payloadSize, payload );
    m_offset = reader.GetPosition();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionRecordingReader::IsAtEnd( void ) const
{GUCEF_TRACE;

    return m_offset >= m_data.GetDataSize();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNSessionRecordingReader::GetSessionName( void ) const
{GUCEF_TRACE;

    return m_sessionName;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNSessionRecordingReader::GetStartTime( void ) const
{GUCEF_TRACE;

    return m_startTime;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNSessionRecordingReader::GetLocalPeerId( void ) const
{GUCEF_TRACE;

    return m_localPeerId;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
