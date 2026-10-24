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

#ifndef GUCEF_DRN_CDRNSESSIONRECORDER_H
#define GUCEF_DRN_CDRNSESSIONRECORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;
class CDRNPeerLink;

/*-------------------------------------------------------------------------*/

/** One message of a recording */
class GUCEF_DRN_EXPORT_CPP CDRNRecordedMessage
{
    public:

    UInt64 timeUs;              /**< microseconds since the recording started */
    bool outgoing;              /**< sent by the recording node, else received */
    UInt32 peerId;              /**< the session peer id of the other end of the link, 0 before it is known */
    UInt32 linkId;
    UInt8 lane;
    UInt8 msgClass;
    UInt32 opcode;
    CORE::CDynamicBuffer payload;

    CDRNRecordedMessage( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Records the traffic of a session to a file for review: what the members sent and received on the links of the
 *  session, by message class (by default the grids, the streams and the application messages: REPL, STREAM, APP).
 *
 *  The file, in the encodings of the DRN wire format (varints, strings as varint length + UTF-8):
 *      file    := "DRNREC01" (8 bytes), u8 version (1), string sessionName, varint64 start (CDRNClock microseconds),
 *                 varint localPeerId, record*
 *      record  := u8 type (1 = message), varint64 timeUs, u8 flags (bit 0: outgoing), varint peerId, varint linkId,
 *                 u8 lane, u8 class, varint opcode, varint size, payload
 *  Records are collected in memory and appended to the file every 64 KB, on Flush() and on Stop().
 *  Read back with CDRNSessionRecordingReader. Used on the thread of the node, Start() and Stop() take its lock.
 */
class GUCEF_DRN_EXPORT_CPP CDRNSessionRecorder
{
    public:

    static const UInt32 DefaultClassMask = ( 1u << DRN_MSGCLASS_REPL ) | ( 1u << DRN_MSGCLASS_STREAM ) | ( 1u << DRN_MSGCLASS_APP );
    static const UInt32 AllClassesMask = 0xFFFFFFFE;
    static const UInt8 FileVersion = 1;

    CDRNSessionRecorder( void );

    virtual ~CDRNSessionRecorder();

    /**
     *  Starts recording the session to the file, which is replaced
     *  @param classMask bit n = record message class n
     */
    bool Start( CDRNSession& session, const CORE::CString& filePath, UInt32 classMask = DefaultClassMask );

    void Stop( void );

    bool IsRecording( void ) const;

    /** Writes what was collected so far */
    bool Flush( void );

    UInt64 GetRecordCount( void ) const;

    const CORE::CString& GetFilePath( void ) const;

    private:

    friend class CDRNSession;

    CDRNSessionRecorder( const CDRNSessionRecorder& src );
    CDRNSessionRecorder& operator=( const CDRNSessionRecorder& src );

    /** Called by the session for every message of its links */
    void OnTraffic( const CDRNPeerLink& link, bool outgoing, UInt8 lane, UInt8 msgClass, UInt32 opcode,
                    const void* part1, UInt32 size1, const void* part2, UInt32 size2 );

    /** The session goes away */
    void OnSessionGone( void );

    bool FlushImp( void );

    CDRNSession* m_session;
    CORE::CString m_filePath;
    UInt32 m_classMask;
    UInt64 m_startTime;
    UInt64 m_recordCount;
    bool m_failed;
    CORE::CDynamicBuffer m_pending;
};

/*-------------------------------------------------------------------------*/

/**
 *  Reads a recording of CDRNSessionRecorder
 */
class GUCEF_DRN_EXPORT_CPP CDRNSessionRecordingReader
{
    public:

    CDRNSessionRecordingReader( void );

    /** Loads the recording, false if it is not one */
    bool Open( const CORE::CString& filePath );

    /** The next message, false at the end or when the rest of the file is damaged */
    bool ReadNext( CDRNRecordedMessage& message );

    bool IsAtEnd( void ) const;

    const CORE::CString& GetSessionName( void ) const;

    UInt64 GetStartTime( void ) const;

    UInt32 GetLocalPeerId( void ) const;

    private:

    CORE::CDynamicBuffer m_data;
    UInt32 m_offset;
    CORE::CString m_sessionName;
    UInt64 m_startTime;
    UInt32 m_localPeerId;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSESSIONRECORDER_H ? */
