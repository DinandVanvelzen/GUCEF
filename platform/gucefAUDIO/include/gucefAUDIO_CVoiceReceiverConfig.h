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

#ifndef GUCEF_AUDIO_CVOICERECEIVERCONFIG_H
#define GUCEF_AUDIO_CVOICERECEIVERCONFIG_H

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
 *  What a CVoiceReceiver is made with: the codec (it must match the sender's), the playout profile with its margins
 *  and window, the sizes of the inbound queue and of the jitter buffer, the concealment limit and the stream it
 *  accepts. A value class.
 *
 *  SetPlayoutProfile() sets the profile together with its margins and window (section 4.8 of the design); the margins
 *  can be changed after it. Saved as the section "VoiceReceiver" with the attributes playoutProfile (by name),
 *  initialMarginMs, minMarginMs, maxMarginMs, windowS, latePercent, inboundQueueFrames, slotCount, concealLimit, streamId and
 *  redPayloadType and the child section "VoiceCodec" (CVoiceCodecConfig).
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceReceiverConfig : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;             /**< "VoiceReceiver" */

    static const UInt32 DefaultInboundQueueFrames = 32;
    static const UInt32 MaxInboundQueueFrames = 4096;
    static const UInt32 DefaultSlotCount = 64;
    static const UInt32 DefaultConcealLimit = 6;
    static const UInt32 MaxConcealLimit = 100;
    static const UInt8 DefaultRedPayloadType = 113;
    static const Float64 MaxLatePercentage;             /**< 20 */

    CVoiceReceiverConfig( void );

    CVoiceReceiverConfig( const CVoiceReceiverConfig& src );

    CVoiceReceiverConfig& operator=( const CVoiceReceiverConfig& src );

    virtual ~CVoiceReceiverConfig() GUCEF_VIRTUAL_OVERRIDE;

    /** The decoder: the same codec, frame duration and payload type as the sender's encoder. Its sample rate is the rate of the playout; best the mix rate of the engine */
    const CVoiceCodecConfig& GetCodecConfig( void ) const;
    CVoiceCodecConfig& GetCodecConfig( void );
    void SetCodecConfig( const CVoiceCodecConfig& codecConfig );

    /**
     *  The profile and its margins, window and late percentage: wired 4 / 2 / 60 ms, 2 s and 0 %, wireless (default)
     *  20 / 6 / 200 ms, 4 s and 1 %
     */
    TVoicePlayoutProfile GetPlayoutProfile( void ) const;
    void SetPlayoutProfile( TVoicePlayoutProfile profile );

    /** The margin M at the start of the playout, between the minimum and the maximum margin */
    Float64 GetInitialMarginInMs( void ) const;
    void SetInitialMarginInMs( Float64 milliseconds );

    /** M never decays below this, 0 to 1000 ms */
    Float64 GetMinMarginInMs( void ) const;
    void SetMinMarginInMs( Float64 milliseconds );

    /** M never grows above this, at least the minimum, at most 2000 ms */
    Float64 GetMaxMarginInMs( void ) const;
    void SetMaxMarginInMs( Float64 milliseconds );

    /** The window of the minimum buffer level, 0.1 to 60 s */
    Float64 GetWindowInS( void ) const;
    void SetWindowInS( Float64 seconds );

    /**
     *  The share of frames, in percent (0 to 20), that may come too late for their first render request: the margin is
     *  kept beyond this quantile of the slack of the window. 0 keeps it beyond the smallest slack, and a late frame
     *  grows the margin by its lateness (the wired profile); 1 (wireless) lets the playout delay follow the bulk of a
     *  heavy-tailed Wi-Fi delay instead of its rare worst frames, which are concealed or stretched for
     */
    Float64 GetLatePercentage( void ) const;
    void SetLatePercentage( Float64 percentage );

    /** Frames the inbound queue from the network thread holds, 2 to 4096 (default 32) */
    UInt32 GetInboundQueueFrames( void ) const;
    void SetInboundQueueFrames( UInt32 frames );

    /** Frame slots of the jitter buffer, a power of two from 2 to 4096 (default 64); other values are rounded up */
    UInt32 GetSlotCount( void ) const;
    void SetSlotCount( UInt32 slots );

    /** Concealed frames with nothing newer that end a talkspurt, 1 to 100 (default 6) */
    UInt32 GetConcealLimit( void ) const;
    void SetConcealLimit( UInt32 frames );

    /** The RTP SSRC accepted; 0 (default) accepts the first stream that arrives (until Reset()) */
    UInt32 GetStreamId( void ) const;
    void SetStreamId( UInt32 streamId );

    /** The payload type of packets with redundancy, for the CRtpVoicePacketCodec of the network side, 0 to 127 (default 113) */
    UInt8 GetRedPayloadType( void ) const;
    void SetRedPayloadType( UInt8 payloadType );

    /** "wireless" or "wired" */
    static const char* PlayoutProfileToString( TVoicePlayoutProfile profile );

    /** The reverse of PlayoutProfileToString() without regard for case. False, and 'profile' as it was, for other text */
    static bool StringToPlayoutProfile( const CString& text, TVoicePlayoutProfile& profile );

    bool operator==( const CVoiceReceiverConfig& other ) const;
    bool operator!=( const CVoiceReceiverConfig& other ) const;

    /** Writes the section "VoiceReceiver" (kept when it is there) with the codec configuration as its child */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads the section from the tree, or the tree itself when it is that section. False when there is no such section.
     *  The profile is applied first, the margins and the window of the section after it; what the section does not have
     *  keeps its value; values out of range are limited; an unknown profile name is logged and ignored
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void LimitMargins( void );

    private:

    CVoiceCodecConfig m_codecConfig;
    TVoicePlayoutProfile m_playoutProfile;
    Float64 m_initialMarginInMs;
    Float64 m_minMarginInMs;
    Float64 m_maxMarginInMs;
    Float64 m_windowInS;
    Float64 m_latePercentage;
    UInt32 m_inboundQueueFrames;
    UInt32 m_slotCount;
    UInt32 m_concealLimit;
    UInt32 m_streamId;
    UInt8 m_redPayloadType;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICERECEIVERCONFIG_H ? */
