/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CVOICESENDERCONFIG_H
#define GUCEF_AUDIO_CVOICESENDERCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUDIO_CVOICECODECCONFIG_H
#include "gucefAUDIO_CVoiceCodecConfig.h"
#define GUCEF_AUDIO_CVOICECODECCONFIG_H
#endif /* GUCEF_AUDIO_CVOICECODECCONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a CVoiceSender is made with: the codec, when it transmits, the pre-roll and hangover of the voice activated
 *  mode, redundancy, the size of its outbound queue and the RTP identity of the stream. A value class.
 *
 *  Saved as the section "VoiceSender" with the attributes transmitMode (by name), vadHangoverMs, preRollFrames,
 *  redundancy, redPayloadType, outboundQueueFrames, streamId, firstSequenceNumber and firstTimestamp (the last two only
 *  when they are set) and the child section "VoiceCodec" (CVoiceCodecConfig).
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceSenderConfig : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;             /**< "VoiceSender" */

    static const UInt32 DefaultVadHangoverInMs = 300;
    static const UInt32 MaxVadHangoverInMs = 5000;
    static const UInt32 DefaultPreRollFrames = 2;
    static const UInt32 MaxPreRollFrames = 8;
    static const UInt32 DefaultOutboundQueueFrames = 16;
    static const UInt32 MaxOutboundQueueFrames = 4096;
    static const UInt8 DefaultRedPayloadType = 113;

    CVoiceSenderConfig( void );

    CVoiceSenderConfig( const CVoiceSenderConfig& src );

    CVoiceSenderConfig& operator=( const CVoiceSenderConfig& src );

    virtual ~CVoiceSenderConfig() GUCEF_VIRTUAL_OVERRIDE;

    /** The encoder: its sample rate must be the processing rate of the capture path */
    const CVoiceCodecConfig& GetCodecConfig( void ) const;
    CVoiceCodecConfig& GetCodecConfig( void );
    void SetCodecConfig( const CVoiceCodecConfig& codecConfig );

    /** When frames are sent (default continuous) */
    TVoiceTransmitMode GetTransmitMode( void ) const;
    void SetTransmitMode( TVoiceTransmitMode mode );

    /** Voice activated mode: how long frames are still sent after the detector stopped hearing a voice, 0 to 5000 ms (default 300) */
    UInt32 GetVadHangoverInMs( void ) const;
    void SetVadHangoverInMs( UInt32 milliseconds );

    /** Voice activated mode: encoded frames kept before an onset and sent first, 0 to 8 (default 2) */
    UInt32 GetPreRollFrames( void ) const;
    void SetPreRollFrames( UInt32 frames );

    /** Redundant frames per packet (RFC 2198): 0 or 1, the previous frame travels with each frame (default 0) */
    UInt32 GetRedundancy( void ) const;
    void SetRedundancy( UInt32 redundancy );

    /** The payload type of packets with redundancy, for the CRtpVoicePacketCodec of the network side, 0 to 127 (default 113) */
    UInt8 GetRedPayloadType( void ) const;
    void SetRedPayloadType( UInt8 payloadType );

    /** Frames the outbound queue holds before the newest one is dropped, 2 to 4096 (default 16) */
    UInt32 GetOutboundQueueFrames( void ) const;
    void SetOutboundQueueFrames( UInt32 frames );

    /** The RTP SSRC of the stream, chosen by the network layer; 0 (default) makes the sender pick a random one */
    UInt32 GetStreamId( void ) const;
    void SetStreamId( UInt32 streamId );

    /** The sequence number of the first frame. Random (RFC 3550) unless set */
    bool HasFirstSequenceNumber( void ) const;
    UInt16 GetFirstSequenceNumber( void ) const;
    void SetFirstSequenceNumber( UInt16 sequenceNumber );
    void ClearFirstSequenceNumber( void );

    /** The RTP timestamp of the first frame. Random (RFC 3550) unless set */
    bool HasFirstTimestamp( void ) const;
    UInt32 GetFirstTimestamp( void ) const;
    void SetFirstTimestamp( UInt32 timestamp );
    void ClearFirstTimestamp( void );

    /** "continuous", "voiceActivated" or "pushToTalk" */
    static const char* TransmitModeToString( TVoiceTransmitMode mode );

    /** The reverse of TransmitModeToString() without regard for case. False, and 'mode' as it was, for other text */
    static bool StringToTransmitMode( const CString& text, TVoiceTransmitMode& mode );

    bool operator==( const CVoiceSenderConfig& other ) const;
    bool operator!=( const CVoiceSenderConfig& other ) const;

    /** Writes the section "VoiceSender" (kept when it is there) with the codec configuration as its child */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads the section from the tree, or the tree itself when it is that section. False when there is no such section.
     *  What the section does not have keeps its value; values out of range are limited, an unknown mode name is
     *  logged and ignored
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVoiceCodecConfig m_codecConfig;
    TVoiceTransmitMode m_transmitMode;
    UInt32 m_vadHangoverInMs;
    UInt32 m_preRollFrames;
    UInt32 m_redundancy;
    UInt8 m_redPayloadType;
    UInt32 m_outboundQueueFrames;
    UInt32 m_streamId;
    bool m_hasFirstSequenceNumber;
    UInt16 m_firstSequenceNumber;
    bool m_hasFirstTimestamp;
    UInt32 m_firstTimestamp;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICESENDERCONFIG_H ? */
