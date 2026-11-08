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

#ifndef GUCEF_AUDIO_CVOICECODECCONFIG_H
#define GUCEF_AUDIO_CVOICECODECCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

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
 *  What a voice encoder or decoder is made for: the codec, the format of the audio, the frame duration and the
 *  codec settings. A value class: copy it freely. It makes no promise that a codec supports the combination, the
 *  factory of the codec decides that (CVoiceCodecFactory::IsSupported()).
 *
 *  The configuration can be saved to and loaded from a configuration tree (CORE::CIConfigurable), the section
 *  "VoiceCodec": every field has an attribute, the codec specific options are the attributes of the child "Options".
 *  The profile is written by name.
 */
class GUCEF_AUDIO_PUBLIC_CPP CVoiceCodecConfig : public CORE::CIConfigurable
{
    public:

    static const CString ConfigSectionName;         /**< "VoiceCodec" */

    CVoiceCodecConfig( void );

    CVoiceCodecConfig( const CVoiceCodecConfig& src );

    CVoiceCodecConfig& operator=( const CVoiceCodecConfig& src );

    virtual ~CVoiceCodecConfig() GUCEF_VIRTUAL_OVERRIDE;

    /** The codec type name, the key in the voice codec registry ("opus", "pcm"). Default "opus" */
    const CString& GetCodec( void ) const;
    void SetCodec( const CString& codec );

    /** Samples per second of the audio the encoder takes and the decoder makes (default 48000) */
    UInt32 GetSampleRate( void ) const;
    void SetSampleRate( UInt32 sampleRate );

    /** Channels: 1 is the only value supported in this design (default 1) */
    UInt32 GetChannels( void ) const;
    void SetChannels( UInt32 channels );

    /** Duration of one frame in microseconds (default 10000) */
    UInt32 GetFrameDurationInUs( void ) const;
    void SetFrameDurationInUs( UInt32 microseconds );

    /** Samples in one frame at the sample rate: rate * duration, rounded to the nearest sample */
    UInt32 GetFrameSizeInFrames( void ) const;

    /** Target bit rate of codecs that have one, in bits per second (default 48000) */
    UInt32 GetBitrate( void ) const;
    void SetBitrate( UInt32 bitsPerSecond );

    /** The application profile (default lowDelay). Codecs without profiles ignore it */
    TVoiceCodecProfile GetProfile( void ) const;
    void SetProfile( TVoiceCodecProfile profile );

    /**
     *  Sets the profile together with the values that belong to it: lowDelay is frames of 10 ms, 48 kbit/s, no forward
     *  error correction and no expected loss; robust is frames of 20 ms, 32 kbit/s, forward error correction and 10 %
     *  expected loss. The complexity, the payload type and the options stay as they are
     */
    void SetProfileDefaults( TVoiceCodecProfile profile );

    /** Encoder effort, 0 (cheapest) to 10 (default 5) */
    UInt32 GetComplexity( void ) const;
    void SetComplexity( UInt32 complexity );

    /** True to have the encoder add forward error correction data (default false) */
    bool GetInbandFec( void ) const;
    void SetInbandFec( bool enabled );

    /** The packet loss the encoder prepares for, in percent 0 to 100 (default 0) */
    UInt32 GetExpectedLossPercentage( void ) const;
    void SetExpectedLossPercentage( UInt32 percentage );

    /**
     *  The RTP payload type of the encoded frames, 0 to 127. Until it is set it is the default of the codec: 111 for
     *  Opus and 112 for PCM (the dynamic range is 96 to 127, signalling overrides it), 111 for any other codec
     */
    UInt8 GetPayloadType( void ) const;
    void SetPayloadType( UInt8 payloadType );

    /** The payload type of a codec type name when none was set */
    static UInt8 GetDefaultPayloadType( const CString& codec );

    /** Codec specific settings by key */
    const CORE::CValueList& GetOptions( void ) const;
    CORE::CValueList& GetOptions( void );

    /** "lowDelay" or "robust" */
    static const char* ProfileToString( TVoiceCodecProfile profile );

    /** The reverse of ProfileToString() without regard for case. False, and 'profile' as it was, for text that is not a profile */
    static bool StringToProfile( const CString& text, TVoiceCodecProfile& profile );

    /** Whether all fields are the same */
    bool operator==( const CVoiceCodecConfig& other ) const;
    bool operator!=( const CVoiceCodecConfig& other ) const;

    /**
     *  Writes the section "VoiceCodec" (kept when it is there): attributes codec, sampleRate, channels, frameDurationUs,
     *  bitrate, profile, complexity, inbandFec, expectedLoss and payloadType (the effective one); and the child "Options"
     *  with one attribute per option
     */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads the section from the tree, or the tree itself when it is that section. False when there is no such section.
     *  What the section does not have keeps its value, a profile name that is not valid is logged and ignored
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString m_codec;
    UInt32 m_sampleRate;
    UInt32 m_channels;
    UInt32 m_frameDurationInUs;
    UInt32 m_bitrate;
    TVoiceCodecProfile m_profile;
    UInt32 m_complexity;
    bool m_inbandFec;
    UInt32 m_expectedLossPercentage;
    UInt8 m_payloadType;
    bool m_payloadTypeIsSet;
    CORE::CValueList m_options;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CVOICECODECCONFIG_H ? */
