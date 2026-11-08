/*
 *  gucefAUDIO_TestApp: GUCEF test application for the AUDIO module
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

#include <math.h>
#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUDIO_CPACKETLOSSCONCEALER_H
#include "gucefAUDIO_CPacketLossConcealer.h"
#define GUCEF_AUDIO_CPACKETLOSSCONCEALER_H
#endif /* GUCEF_AUDIO_CPACKETLOSSCONCEALER_H ? */

#ifndef GUCEF_AUDIO_CPCMVOICEDECODER_H
#include "gucefAUDIO_CPcmVoiceDecoder.h"
#define GUCEF_AUDIO_CPCMVOICEDECODER_H
#endif /* GUCEF_AUDIO_CPCMVOICEDECODER_H ? */

#ifndef GUCEF_AUDIO_CVOICEFRAME_H
#include "gucefAUDIO_CVoiceFrame.h"
#define GUCEF_AUDIO_CVOICEFRAME_H
#endif /* GUCEF_AUDIO_CVOICEFRAME_H ? */

#include "TestAudioVoiceCodecSupport.h"
#include "TestAudioVoiceCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace VoiceCodecTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

AUDIO::CVoiceCodecRegistry&
Registry( void )
{
    return AUDIO::CAudioGlobal::Instance()->GetVoiceCodecRegistry();
}

/*-------------------------------------------------------------------------*/

/** A codec of no use that exists to be registered: counts through the factory like a plugin codec does */
class CNoneVoiceEncoder : public AUDIO::CVoiceEncoder
{
    public:
    CNoneVoiceEncoder( const AUDIO::CVoiceCodecConfig& config ) : AUDIO::CVoiceEncoder(), m_config( config ) {}
    virtual const AUDIO::CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_config; }
    virtual UInt32 GetMaxPayloadSize( void ) const GUCEF_VIRTUAL_OVERRIDE { return 1; }
    virtual UInt32 GetLookaheadInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual UInt32 GetTimestampRate( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_config.GetSampleRate(); }
    virtual Int32 Encode( const Float32* frame, UInt8* payload, UInt32 capacity ) GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE {}
    private:
    AUDIO::CVoiceCodecConfig m_config;
};

class CNoneVoiceDecoder : public AUDIO::CVoiceDecoder
{
    public:
    CNoneVoiceDecoder( const AUDIO::CVoiceCodecConfig& config ) : AUDIO::CVoiceDecoder(), m_config( config ) {}
    virtual const AUDIO::CVoiceCodecConfig& GetConfig( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_config; }
    virtual UInt32 GetOutputRate( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_config.GetSampleRate(); }
    virtual Int32 Decode( const UInt8* payload, UInt32 size, Float32* pcm, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual Int32 Conceal( Float32* pcm, UInt32 frames ) GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE {}
    private:
    AUDIO::CVoiceCodecConfig m_config;
};

class CNoneVoiceCodecFactory : public AUDIO::CVoiceCodecFactory
{
    public:
    CNoneVoiceCodecFactory( const char* name ) : AUDIO::CVoiceCodecFactory(), m_name( name ) {}
    virtual const CORE::CString& GetCodecTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name; }
    virtual bool IsSupported( const AUDIO::CVoiceCodecConfig& config ) const GUCEF_VIRTUAL_OVERRIDE { return config.GetSampleRate() == 16000; }
    protected:
    virtual AUDIO::CVoiceEncoder* OnCreateEncoder( const AUDIO::CVoiceCodecConfig& config ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CNoneVoiceEncoder( config ); }
    virtual AUDIO::CVoiceDecoder* OnCreateDecoder( const AUDIO::CVoiceCodecConfig& config ) GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CNoneVoiceDecoder( config ); }
    private:
    CORE::CString m_name;
};

/*-------------------------------------------------------------------------*/

bool
MakePcm( UInt32 sampleRate, UInt32 frameDurationInUs, AUDIO::TVoiceEncoderPtr& encoder, AUDIO::TVoiceDecoderPtr& decoder )
{
    const AUDIO::CVoiceCodecConfig config = MakeConfig( "pcm", sampleRate, frameDurationInUs );
    return Registry().CreateEncoder( config, encoder ) && Registry().CreateDecoder( config, decoder );
}

/*-------------------------------------------------------------------------*/

/** Runs a signal through the pcm codec frame by frame. A frame whose index is in 'lost' is concealed instead of decoded */
TSamples
RunPcm( const TSamples& input, UInt32 sampleRate, UInt32 frameDurationInUs, const GUCEF::vector< UInt32 >& lost, AUDIO::TVoiceDecoderPtr* keepDecoder = GUCEF_NULL )
{
    AUDIO::TVoiceEncoderPtr encoder;
    AUDIO::TVoiceDecoderPtr decoder;
    TSamples output;
    if ( !MakePcm( sampleRate, frameDurationInUs, encoder, decoder ) )
        return output;

    const UInt32 frameSize = encoder->GetFrameSizeInFrames();
    const UInt32 frames = (UInt32) ( input.size() / frameSize );
    output.assign( (size_t) frames * frameSize, 0.0f );
    UInt8 payload[ AUDIO::CVoiceFrame::MaxPayloadSize ];
    for ( UInt32 f=0; f<frames; ++f )
    {
        bool isLost = false;
        for ( size_t l=0; l<lost.size(); ++l )
            if ( lost[ l ] == f )
                isLost = true;

        const Int32 size = encoder->Encode( &input[ (size_t) f * frameSize ], payload, sizeof( payload ) );
        if ( size <= 0 )
            return TSamples();
        if ( isLost )
            decoder->Conceal( &output[ (size_t) f * frameSize ], frameSize );
        else
        if ( (Int32) frameSize != decoder->Decode( payload, (UInt32) size, &output[ (size_t) f * frameSize ], frameSize ) )
            return TSamples();
    }
    if ( GUCEF_NULL != keepDecoder )
        *keepDecoder = decoder;
    return output;
}

/*-------------------------------------------------------------------------*/

bool
IsBigEndianL16( const UInt8* bytes, Int32 expected )
{
    return bytes[ 0 ] == (UInt8) ( ( expected >> 8 ) & 0xFF ) && bytes[ 1 ] == (UInt8) ( expected & 0xFF );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_AUDIO_VOICETEST_RTSAN_BUILD
const bool IsRtsanBuild = true;
#else
const bool IsRtsanBuild = false;
#endif

/*-------------------------------------------------------------------------*/

void
PerformConfigTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The configuration has the documented defaults and the payload type follows the codec until it is set" )
    try
    {
        AUDIO::CVoiceCodecConfig config;
        ASSERT_TRUE( config.GetCodec() == "opus" );
        ASSERT_TRUE( 48000 == config.GetSampleRate() && 1 == config.GetChannels() && 10000 == config.GetFrameDurationInUs() );
        ASSERT_TRUE( 48000 == config.GetBitrate() && AUDIO::VOICECODECPROFILE_LOWDELAY == config.GetProfile() && 5 == config.GetComplexity() );
        ASSERT_TRUE( !config.GetInbandFec() && 0 == config.GetExpectedLossPercentage() && 111 == config.GetPayloadType() );
        ASSERT_TRUE( 480 == config.GetFrameSizeInFrames() );

        config.SetCodec( "pcm" );
        ASSERT_TRUE( 112 == config.GetPayloadType() );
        config.SetPayloadType( 100 );
        config.SetCodec( "opus" );
        ASSERT_TRUE( 100 == config.GetPayloadType() );

        config.SetSampleRate( 16000 );
        config.SetFrameDurationInUs( 2500 );
        ASSERT_TRUE( 40 == config.GetFrameSizeInFrames() );
        config.SetFrameDurationInUs( 20000 );
        ASSERT_TRUE( 320 == config.GetFrameSizeInFrames() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The configuration survives a save and a load through a configuration tree, options included" )
    try
    {
        AUDIO::CVoiceCodecConfig config = MakeConfig( "opus", 24000, 20000 );
        config.SetBitrate( 32000 );
        config.SetProfile( AUDIO::VOICECODECPROFILE_ROBUST );
        config.SetComplexity( 9 );
        config.SetInbandFec( true );
        config.SetExpectedLossPercentage( 15 );
        config.SetPayloadType( 97 );
        config.GetOptions().Set( "vendorKey", "some value" );
        config.GetOptions().Set( "other", "7" );

        CORE::CDataNode tree;
        ASSERT_TRUE( config.SaveConfig( tree ) );
        ASSERT_TRUE( GUCEF_NULL != tree.FindChild( "VoiceCodec" ) );

        AUDIO::CVoiceCodecConfig loaded;
        ASSERT_TRUE( loaded.LoadConfig( tree ) );
        ASSERT_TRUE( loaded == config );
        ASSERT_TRUE( loaded.GetOptions().GetValueAlways( "vendorKey" ).AsString() == "some value" );
        ASSERT_TRUE( loaded.GetProfile() == AUDIO::VOICECODECPROFILE_ROBUST && 97 == loaded.GetPayloadType() );

        // the section itself loads too, and a different value shows
        CORE::CDataNode* section = tree.FindChild( "VoiceCodec" );
        AUDIO::CVoiceCodecConfig fromSection;
        ASSERT_TRUE( fromSection.LoadConfig( *section ) );
        ASSERT_TRUE( fromSection == config );
        AUDIO::CVoiceCodecConfig changed = config;
        changed.SetBitrate( 33000 );
        ASSERT_TRUE( changed != config );

        // a pcm configuration saves its effective payload type and loads it back
        AUDIO::CVoiceCodecConfig pcm = MakeConfig( "pcm", 16000, 10000 );
        CORE::CDataNode pcmTree;
        ASSERT_TRUE( pcm.SaveConfig( pcmTree ) );
        AUDIO::CVoiceCodecConfig pcmLoaded;
        ASSERT_TRUE( pcmLoaded.LoadConfig( pcmTree ) );
        ASSERT_TRUE( pcmLoaded == pcm && 112 == pcmLoaded.GetPayloadType() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: Loading a configuration without its section fails, an unknown profile is ignored and absent values keep theirs" )
    try
    {
        CORE::CDataNode empty;
        AUDIO::CVoiceCodecConfig config = MakeConfig( "pcm", 8000, 5000 );
        ASSERT_FALSE( config.LoadConfig( empty ) );
        ASSERT_TRUE( config == MakeConfig( "pcm", 8000, 5000 ) );

        CORE::CDataNode tree;
        CORE::CDataNode* section = tree.Structure( "VoiceCodec", '/' );
        ASSERT_TRUE( GUCEF_NULL != section );
        section->SetAttribute( "profile", CORE::CString( "bogus" ) );
        section->SetAttribute( "bitrate", (UInt32) 12345 );
        ASSERT_TRUE( config.LoadConfig( tree ) );
        ASSERT_TRUE( AUDIO::VOICECODECPROFILE_LOWDELAY == config.GetProfile() );
        ASSERT_TRUE( 12345 == config.GetBitrate() && 8000 == config.GetSampleRate() && config.GetCodec() == "pcm" );

        AUDIO::TVoiceCodecProfile profile = AUDIO::VOICECODECPROFILE_LOWDELAY;
        ASSERT_TRUE( AUDIO::CVoiceCodecConfig::StringToProfile( "ROBUST", profile ) && AUDIO::VOICECODECPROFILE_ROBUST == profile );
        ASSERT_FALSE( AUDIO::CVoiceCodecConfig::StringToProfile( "nope", profile ) );
        ASSERT_TRUE( AUDIO::VOICECODECPROFILE_ROBUST == profile );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformRegistryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The registry has the pcm codec, finds it without regard for case and makes encoders and decoders of it" )
    try
    {
        AUDIO::CVoiceCodecRegistry::TRegisteredFactoryPtr factory;
        ASSERT_TRUE( Registry().TryFindFactory( "pcm", factory ) && !factory.IsNULL() );
        ASSERT_TRUE( factory->GetCodecTypeName() == "pcm" );
        ASSERT_TRUE( Registry().TryFindFactory( "PCM", factory ) );
        ASSERT_FALSE( Registry().TryFindFactory( "nosuchcodec", factory ) );
        ASSERT_TRUE( Registry().TryFindFactory( "pcm", factory ) && !factory.IsNULL() );

        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        AUDIO::CVoiceCodecConfig config = MakeConfig( "PcM", 16000, 10000 );
        ASSERT_TRUE( Registry().CreateEncoder( config, encoder ) && !encoder.IsNULL() );
        ASSERT_TRUE( Registry().CreateDecoder( config, decoder ) && !decoder.IsNULL() );
        ASSERT_TRUE( encoder->GetConfig() == config && decoder->GetConfig() == config );
        ASSERT_TRUE( 2 == factory->GetLiveInstanceCount() );
        encoder.Unlink();
        decoder.Unlink();
        ASSERT_TRUE( 0 == factory->GetLiveInstanceCount() );

        // an unknown codec and an unsupported configuration make nothing
        AUDIO::CVoiceCodecConfig unknown = MakeConfig( "nosuchcodec", 16000, 10000 );
        ASSERT_FALSE( Registry().CreateEncoder( unknown, encoder ) );
        ASSERT_FALSE( Registry().CreateDecoder( unknown, decoder ) );
        ASSERT_TRUE( encoder.IsNULL() && decoder.IsNULL() );
        ASSERT_FALSE( Registry().CreateEncoder( MakeConfig( "pcm", 16000, 12345 ), encoder ) );
        ASSERT_TRUE( encoder.IsNULL() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: A factory registers under its lower case name, a name is taken once and a factory with live instances stays registered" )
    try
    {
        ASSERT_FALSE( Registry().RegisterFactory( AUDIO::TVoiceCodecFactoryPtr() ) );
        ASSERT_FALSE( Registry().RegisterFactory( AUDIO::TVoiceCodecFactoryPtr( GUCEF_NEW CNoneVoiceCodecFactory( "" ) ) ) );

        AUDIO::TVoiceCodecFactoryPtr none( GUCEF_NEW CNoneVoiceCodecFactory( "NoneTestCodec" ) );
        ASSERT_TRUE( Registry().RegisterFactory( none ) );
        ASSERT_FALSE( Registry().RegisterFactory( AUDIO::TVoiceCodecFactoryPtr( GUCEF_NEW CNoneVoiceCodecFactory( "nonetestcodec" ) ) ) );
        ASSERT_FALSE( Registry().RegisterFactory( AUDIO::TVoiceCodecFactoryPtr( GUCEF_NEW CNoneVoiceCodecFactory( "pcm" ) ) ) );

        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        ASSERT_TRUE( Registry().CreateEncoder( MakeConfig( "nonetestcodec", 16000, 10000 ), encoder ) );
        ASSERT_TRUE( Registry().CreateDecoder( MakeConfig( "NONETESTCODEC", 16000, 10000 ), decoder ) );
        AUDIO::TVoiceEncoderPtr refused;
        ASSERT_FALSE( Registry().CreateEncoder( MakeConfig( "nonetestcodec", 8000, 10000 ), refused ) );
        ASSERT_TRUE( refused.IsNULL() );
        ASSERT_TRUE( 2 == none->GetLiveInstanceCount() );

        // the plugin rule: no unregistering while encoders or decoders live
        ASSERT_FALSE( Registry().TryUnregisterFactory( "nonetestcodec" ) );
        encoder.Unlink();
        ASSERT_FALSE( Registry().TryUnregisterFactory( "nonetestcodec" ) );
        decoder.Unlink();
        ASSERT_TRUE( 0 == none->GetLiveInstanceCount() );
        ASSERT_TRUE( Registry().TryUnregisterFactory( "NoneTestCodec" ) );
        AUDIO::CVoiceCodecRegistry::TRegisteredFactoryPtr found;
        ASSERT_FALSE( Registry().TryFindFactory( "nonetestcodec", found ) );
        ASSERT_FALSE( Registry().TryUnregisterFactory( "nonetestcodec" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformPcmTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The pcm codec round trip is bit-exact for every 16 bit value, at several rates and frame durations" )
    try
    {
        const UInt32 rates[] = { 8000, 16000, 48000 };
        const UInt32 durations[] = { 2500, 5000, 10000 };
        for ( UInt32 r=0; r<3; ++r )
        {
            for ( UInt32 d=0; d<3; ++d )
            {
                AUDIO::TVoiceEncoderPtr encoder;
                AUDIO::TVoiceDecoderPtr decoder;
                ASSERT_TRUE( MakePcm( rates[ r ], durations[ d ], encoder, decoder ) );
                const UInt32 frameSize = encoder->GetFrameSizeInFrames();
                ASSERT_TRUE( frameSize == (UInt32) ( (UInt64) rates[ r ] * durations[ d ] / 1000000ULL ) );
                ASSERT_TRUE( 2 * frameSize == encoder->GetMaxPayloadSize() );

                TSamples input( frameSize, 0.0f ), output( frameSize, 0.0f );
                UInt8 payload[ 2048 ];
                bool exact = true;
                Int32 next = -32768;
                while ( next < 32768 && exact )
                {
                    for ( UInt32 i=0; i<frameSize; ++i )
                        input[ i ] = (Float32) ( next + (Int32) i < 32768 ? next + (Int32) i : 32767 ) / 32768.0f;
                    const Int32 size = encoder->Encode( &input[ 0 ], payload, sizeof( payload ) );
                    exact = exact && size == (Int32) ( 2 * frameSize );
                    exact = exact && (Int32) frameSize == decoder->Decode( payload, (UInt32) size, &output[ 0 ], frameSize );
                    exact = exact && 0 == memcmp( &input[ 0 ], &output[ 0 ], frameSize * sizeof( Float32 ) );
                    next += (Int32) frameSize;
                }
                ASSERT_TRUE( exact );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The pcm payload is 16 bit signed big endian with rounding to the nearest and saturation, and a NaN is silence" )
    try
    {
        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        ASSERT_TRUE( MakePcm( 8000, 2500, encoder, decoder ) );
        ASSERT_TRUE( 20 == encoder->GetFrameSizeInFrames() );

        TSamples frame( 20, 0.0f );
        frame[ 0 ] = 4660.0f / 32768.0f;            // 0x1234
        frame[ 1 ] = -1.0f / 32768.0f;              // 0xFFFF
        frame[ 2 ] = 1.5f;                          // saturates to 0x7FFF
        frame[ 3 ] = -2.0f;                         // saturates to 0x8000
        frame[ 4 ] = 0.5f / 32768.0f;               // half rounds away from zero: 1
        frame[ 5 ] = -0.5f / 32768.0f;              // -1
        frame[ 6 ] = 0.4f / 32768.0f;               // 0
        frame[ 7 ] = -0.4f / 32768.0f;              // 0
        frame[ 8 ] = 1.0f;                          // 0x7FFF
        frame[ 9 ] = -1.0f;                         // 0x8000
        frame[ 10 ] = (Float32) sqrt( (Float64) -1.0 );     // NaN
        frame[ 11 ] = 3.5f / 32768.0f;              // 4
        frame[ 12 ] = -3.5f / 32768.0f;             // -4
        UInt8 payload[ 64 ];
        ASSERT_TRUE( 40 == encoder->Encode( &frame[ 0 ], payload, sizeof( payload ) ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 0, 0x1234 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 2, -1 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 4, 0x7FFF ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 6, -32768 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 8, 1 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 10, -1 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 12, 0 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 14, 0 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 16, 0x7FFF ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 18, -32768 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 20, 0 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 22, 4 ) );
        ASSERT_TRUE( IsBigEndianL16( payload + 24, -4 ) );

        // too small a capacity is refused without writing
        UInt8 small[ 40 ];
        memset( small, 0x55, sizeof( small ) );
        ASSERT_TRUE( encoder->Encode( &frame[ 0 ], small, 39 ) < 0 );
        ASSERT_TRUE( 0x55 == small[ 0 ] && 0x55 == small[ 39 ] );
        ASSERT_TRUE( 40 == encoder->Encode( &frame[ 0 ], small, 40 ) );

        // and the decoder reads it back
        TSamples decoded( 20, 0.0f );
        ASSERT_TRUE( 20 == decoder->Decode( payload, 40, &decoded[ 0 ], 20 ) );
        ASSERT_TRUE( decoded[ 0 ] == 4660.0f / 32768.0f && decoded[ 1 ] == -1.0f / 32768.0f );
        ASSERT_TRUE( decoded[ 2 ] == 32767.0f / 32768.0f && decoded[ 3 ] == -1.0f && decoded[ 10 ] == 0.0f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The pcm decoder refuses payloads that are not whole samples or do not fit the output" )
    try
    {
        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        ASSERT_TRUE( MakePcm( 16000, 10000, encoder, decoder ) );
        UInt8 payload[ 400 ];
        memset( payload, 0, sizeof( payload ) );
        TSamples out( 200, 0.5f );
        ASSERT_TRUE( decoder->Decode( payload, 0, &out[ 0 ], 200 ) < 0 );
        ASSERT_TRUE( decoder->Decode( payload, 319, &out[ 0 ], 200 ) < 0 );
        ASSERT_TRUE( decoder->Decode( payload, 320, &out[ 0 ], 159 ) < 0 );
        ASSERT_TRUE( 0.5f == out[ 0 ] && 0.5f == out[ 159 ] );
        ASSERT_TRUE( 160 == decoder->Decode( payload, 320, &out[ 0 ], 160 ) );
        ASSERT_TRUE( 0.0f == out[ 0 ] && 0.5f == out[ 160 ] );
        // a payload of another size than the frame decodes as what it is
        ASSERT_TRUE( 10 == decoder->Decode( payload, 20, &out[ 0 ], 200 ) );
        ASSERT_TRUE( 0 == decoder->GetConfig().GetChannels() - 1 && 16000 == decoder->GetOutputRate() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The pcm codec supports 8 to 48 kHz frames of 2.5, 5, 10 and 20 ms that are whole samples and fit 1440 bytes" )
    try
    {
        AUDIO::CVoiceCodecRegistry::TRegisteredFactoryPtr pcm;
        ASSERT_TRUE( Registry().TryFindFactory( "pcm", pcm ) );

        // supported
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 8000, 2500 ) ) );     // 20 samples
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 16000, 20000 ) ) );   // 320 samples, 640 bytes
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 24000, 20000 ) ) );   // 480 samples, 960 bytes
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 32000, 20000 ) ) );   // 640 samples, 1280 bytes
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 48000, 2500 ) ) );
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 48000, 5000 ) ) );
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 48000, 10000 ) ) );   // 480 samples, 960 bytes
        ASSERT_TRUE( pcm->IsSupported( MakeConfig( "pcm", 44100, 10000 ) ) );   // 441 samples

        // too long a payload: 48 kHz, 20 ms is 1920 bytes
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 48000, 20000 ) ) );
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 44100, 20000 ) ) );  // 882 samples, 1764 bytes
        // durations that are not offered
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 16000, 15000 ) ) );
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 16000, 40000 ) ) );
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 16000, 1250 ) ) );
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 16000, 0 ) ) );
        // not a whole number of samples
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 44100, 2500 ) ) );   // 110.25
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 44100, 5000 ) ) );   // 220.5
        // rates out of range
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 7999, 10000 ) ) );
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 48001, 10000 ) ) );
        ASSERT_FALSE( pcm->IsSupported( MakeConfig( "pcm", 96000, 5000 ) ) );
        // one channel only
        AUDIO::CVoiceCodecConfig stereo = MakeConfig( "pcm", 16000, 10000 );
        stereo.SetChannels( 2 );
        ASSERT_FALSE( pcm->IsSupported( stereo ) );
        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        ASSERT_FALSE( Registry().CreateEncoder( stereo, encoder ) || Registry().CreateDecoder( stereo, decoder ) );

        // the largest frames that fit are exactly at the limit
        AUDIO::TVoiceEncoderPtr widest;
        AUDIO::TVoiceDecoderPtr widestDecoder;
        ASSERT_TRUE( MakePcm( 32000, 20000, widest, widestDecoder ) );
        ASSERT_TRUE( widest->GetMaxPayloadSize() <= AUDIO::CVoiceFrame::MaxPayloadSize );
        TSamples frame( 640, 0.25f );
        UInt8 payload[ AUDIO::CVoiceFrame::MaxPayloadSize ];
        ASSERT_TRUE( 1280 == widest->Encode( &frame[ 0 ], payload, sizeof( payload ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The pcm codec reports its timing: no delay, timestamps at the sample rate, no forward error correction" )
    try
    {
        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        ASSERT_TRUE( MakePcm( 24000, 5000, encoder, decoder ) );
        ASSERT_TRUE( 0 == encoder->GetLookaheadInFrames() && 24000 == encoder->GetTimestampRate() && 24000 == decoder->GetOutputRate() );
        ASSERT_TRUE( 120 == encoder->GetFrameSizeInFrames() );
        ASSERT_FALSE( decoder->SupportsFec() );
        UInt8 payload[ 240 ];
        TSamples out( 120, 0.0f );
        ASSERT_TRUE( decoder->DecodeFec( payload, 240, &out[ 0 ], 120 ) < 0 );
        encoder->SetBitrate( 1000 );                    // no effect, no failure
        encoder->SetExpectedLossPercentage( 50 );
        encoder->Reset();
        decoder->Reset();
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: Encode, Decode, Conceal and Reset do nothing a real time thread must not do (RealtimeSanitizer build)" )
    if ( !IsRtsanBuild )
    {
        GUCEF_TESTFW_SKIP( "not a RealtimeSanitizer build (-fsanitize=realtime)" );
    }
    GUCEF_TESTFW_SKIP_IF( GUCEF_TESTFW_GETENV( "GUCEF_TEST_AUDIO_RTSAN_CLEAN" ) == "0", "switched off with GUCEF_TEST_AUDIO_RTSAN_CLEAN=0" );
    try
    {
        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        ASSERT_TRUE( MakePcm( 16000, 10000, encoder, decoder ) );
        const TSamples input = MakeHarmonic( 160 * 30, 150.0, 16000, 0.5f );
        TSamples output( 160, 0.0f );
        UInt8 payload[ 512 ];
        for ( UInt32 f=0; f<30; ++f )
        {
            const Int32 size = EncodeRealtime( *encoder, &input[ (size_t) f * 160 ], payload, sizeof( payload ) );
            ASSERT_TRUE( 320 == size );
            const bool isLost = 20 == f || 21 == f || 22 == f;
            const Int32 got = isLost ? ConcealRealtime( *decoder, &output[ 0 ], 160 ) : DecodeRealtime( *decoder, payload, (UInt32) size, &output[ 0 ], 160 );
            ASSERT_TRUE( 160 == got );
        }
        // the long loss, past the silence, and the way back
        for ( UInt32 f=0; f<12; ++f )
            ConcealRealtime( *decoder, &output[ 0 ], 160 );
        ASSERT_TRUE( 160 == DecodeRealtime( *decoder, payload, 320, &output[ 0 ], 160 ) );
        ResetRealtime( *encoder, *decoder );
        ASSERT_TRUE( 160 == ConcealRealtime( *decoder, &output[ 0 ], 160 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** The concealed samples that replace frames [firstLost, firstLost + lostCount) of a stream of the pcm codec */
void
PerformConcealmentTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: A concealed periodic signal goes on with its period (zero crossings within 2 %) at 8, 16 and 48 kHz" )
    try
    {
        const UInt32 rates[] = { 8000, 16000, 48000 };
        const Float64 frequencies[] = { 120.0, 160.0, 220.0 };
        for ( UInt32 r=0; r<3; ++r )
        {
            const UInt32 rate = rates[ r ];
            const UInt32 frameSize = rate / 100;
            const Float64 period = (Float64) rate / frequencies[ r ];
            const TSamples input = MakeHarmonic( frameSize * 30, frequencies[ r ], rate, 0.5f );

            // frames 12 to 16 are lost: 50 ms
            GUCEF::vector< UInt32 > lost;
            for ( UInt32 f=12; f<17; ++f )
                lost.push_back( f );
            AUDIO::TVoiceDecoderPtr decoder;
            const TSamples output = RunPcm( input, rate, 10000, lost, &decoder );
            ASSERT_TRUE( output.size() == input.size() );

            // the first 30 ms of the loss, where the level is still above 80 %
            const size_t from = (size_t) 12 * frameSize, to = (size_t) 15 * frameSize;
            size_t firstCross = 0, lastCross = 0;
            const UInt32 crossings = CountZeroCrossings( output, from, to, firstCross, lastCross );
            size_t firstRef = 0, lastRef = 0;
            const UInt32 reference = CountZeroCrossings( input, from, to, firstRef, lastRef );
            const Float64 measured = crossings > 1 ? (Float64) ( lastCross - firstCross ) / (Float64) ( crossings - 1 ) : 0.0;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodec: " << rate << " Hz, period " << period << " samples: " << crossings << " crossings in 30 ms of concealment against " << reference << " in the signal, mean period " << measured );
            ASSERT_TRUE( crossings >= 3 && ( crossings == reference || crossings + 1 == reference || crossings == reference + 1 ) );
            ASSERT_TRUE( fabs( measured - period ) / period < 0.02 );

            // the signal itself continues: correlation with the real continuation over the first period and a half
            const size_t compare = (size_t) ( period * 1.5 );
            ASSERT_TRUE( Correlation( input, from, output, from, compare ) > 0.99 );

            // the concealer found the period (to a sample, up to the pitch that is a multiple of it)
            const AUDIO::CPcmVoiceDecoder* pcm = static_cast< const AUDIO::CPcmVoiceDecoder* >( decoder.GetPointerAlways() );
            const Float64 found = (Float64) pcm->GetConcealer().GetPitchPeriod();
            ASSERT_TRUE( fabs( found - period ) <= 1.0 || fabs( found - 2.0 * period ) <= 2.0 || fabs( found - 3.0 * period ) <= 3.0 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: Concealment is full level for 10 ms, then turned down by 20 % per 10 ms (schedule within 0.04 of the level)" )
    try
    {
        const UInt32 rate = 16000;
        const UInt32 frameSize = 160;
        const TSamples input = AudioEffectTest::MakeSine( frameSize * 24, 1, 200.0, 0.5f, 16000.0 );
        GUCEF::vector< UInt32 > lost;
        for ( UInt32 f=10; f<18; ++f )
            lost.push_back( f );
        const TSamples output = RunPcm( input, rate, 10000, lost );
        ASSERT_TRUE( output.size() == input.size() );

        const Float64 reference = Rms( input, (size_t) 8 * frameSize, (size_t) 10 * frameSize );
        const Float64 expected[ 8 ] = { 1.0, 0.9, 0.7, 0.5, 0.3, 0.1, 0.0, 0.0 };
        for ( UInt32 chunk=0; chunk<8; ++chunk )
        {
            const size_t from = (size_t) ( 10 + chunk ) * frameSize;
            const Float64 level = Rms( output, from, from + frameSize ) / reference;
            ASSERT_TRUE( fabs( level - expected[ chunk ] ) < 0.04 );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: Concealment is silent from 60 ms on, exactly, and does not depend on the frame size it is asked for" )
    try
    {
        const UInt32 rate = 48000;
        const TSamples input = MakeHarmonic( 480 * 20, 130.0, rate, 0.6f );
        AUDIO::TVoiceEncoderPtr encoder;
        AUDIO::TVoiceDecoderPtr decoder;
        ASSERT_TRUE( MakePcm( rate, 10000, encoder, decoder ) );
        UInt8 payload[ 1024 ];
        TSamples scratch( 480, 0.0f );
        for ( UInt32 f=0; f<10; ++f )
        {
            const Int32 size = encoder->Encode( &input[ (size_t) f * 480 ], payload, sizeof( payload ) );
            ASSERT_TRUE( 480 == decoder->Decode( payload, (UInt32) size, &scratch[ 0 ], 480 ) );
        }

        // 100 ms asked for in pieces of 2.5 ms (120 samples), then the same asked in one piece by a second decoder
        TSamples pieces( 4800, 1.0f );
        for ( UInt32 p=0; p<40; ++p )
            ASSERT_TRUE( 120 == decoder->Conceal( &pieces[ (size_t) p * 120 ], 120 ) );

        AUDIO::TVoiceEncoderPtr encoder2;
        AUDIO::TVoiceDecoderPtr decoder2;
        ASSERT_TRUE( MakePcm( rate, 10000, encoder2, decoder2 ) );
        for ( UInt32 f=0; f<10; ++f )
        {
            const Int32 size = encoder2->Encode( &input[ (size_t) f * 480 ], payload, sizeof( payload ) );
            decoder2->Decode( payload, (UInt32) size, &scratch[ 0 ], 480 );
        }
        TSamples whole( 4800, 1.0f );
        ASSERT_TRUE( 4800 == decoder2->Conceal( &whole[ 0 ], 4800 ) );
        ASSERT_TRUE( 0 == memcmp( &pieces[ 0 ], &whole[ 0 ], 4800 * sizeof( Float32 ) ) );

        // 60 ms is 2880 samples at 48 kHz
        bool silent = true;
        for ( size_t n=2880; n<4800; ++n )
            silent = silent && 0.0f == pieces[ n ];
        ASSERT_TRUE( silent );
        ASSERT_TRUE( PeakOfRange( pieces, 2700, 2880 ) > 0.0f && PeakOfRange( pieces, 2700, 2880 ) < 0.1f );
        ASSERT_TRUE( PeakOfRange( pieces, 0, 480 ) > 0.3f );
        const AUDIO::CPcmVoiceDecoder* pcm = static_cast< const AUDIO::CPcmVoiceDecoder* >( decoder.GetPointerAlways() );
        ASSERT_TRUE( pcm->GetConcealer().IsConcealing() && 4800 == pcm->GetConcealer().GetConcealedSampleCount() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The return from a concealed stretch is smooth: the second difference stays within twice that of the clean signal in the same stretch" )
    try
    {
        const UInt32 rates[] = { 8000, 16000, 48000 };
        const UInt32 lostFrames[] = { 1, 2, 3, 5, 9 };            // 10 ms up to 90 ms: past the silence too
        for ( UInt32 r=0; r<3; ++r )
        {
            const UInt32 rate = rates[ r ];
            const UInt32 frameSize = rate / 100;
            // a pitch that is not a whole number of samples and an amplitude that moves: the repetition is not exact
            const TSamples input = MakeSpeechLike( frameSize * 40, rate, 0.5f );
            const Float32 clean = MaxSecondDifference( input, 1, input.size() - 1 );
            for ( UInt32 l=0; l<5; ++l )
            {
                GUCEF::vector< UInt32 > lost;
                for ( UInt32 f=0; f<lostFrames[ l ]; ++f )
                    lost.push_back( 15 + f );
                const TSamples output = RunPcm( input, rate, 10000, lost );
                ASSERT_TRUE( output.size() == input.size() );

                // from the last good sample to the end of the cross-fade into the first good frame
                const size_t from = (size_t) 15 * frameSize - 1;
                const size_t to = (size_t) ( 15 + lostFrames[ l ] + 1 ) * frameSize + 1;
                const Float32 worst = MaxSecondDifference( output, from, to );
                const Float32 local = MaxSecondDifference( input, from, to );
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodec: speech-like at " << rate << " Hz, " << lostFrames[ l ] << " lost frames: second difference " << worst << " against " << local << " in the same stretch of the clean signal (" << clean << " at most)" );
                ASSERT_TRUE( worst <= 2.0f * local );
                ASSERT_TRUE( AllFinite( output ) );
            }
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: Good frames after the cross-fade are bit-exact, and without a loss the decoder passes frames through untouched" )
    try
    {
        const UInt32 rate = 16000;
        const TSamples input = MakeSpeechLike( 160 * 30, rate, 0.5f );
        GUCEF::vector< UInt32 > none;
        const TSamples clean = RunPcm( input, rate, 10000, none );
        ASSERT_TRUE( clean.size() == input.size() );
        // 16 bit quantised input
        for ( size_t n=0; n<clean.size(); ++n )
            ASSERT_TRUE( fabsf( clean[ n ] - input[ n ] ) <= 0.5f / 32768.0f + 1.0e-9f );

        GUCEF::vector< UInt32 > lost;
        lost.push_back( 12 );
        lost.push_back( 13 );
        AUDIO::TVoiceDecoderPtr keep;
        const TSamples output = RunPcm( input, rate, 10000, lost, &keep );
        const AUDIO::CPcmVoiceDecoder* pcm = static_cast< const AUDIO::CPcmVoiceDecoder* >( keep.GetPointerAlways() );
        // frames before the loss and from the second good frame after it: the same as without the loss
        ASSERT_TRUE( 0 == memcmp( &output[ 0 ], &clean[ 0 ], (size_t) 12 * 160 * sizeof( Float32 ) ) );
        ASSERT_TRUE( 0 == memcmp( &output[ (size_t) 15 * 160 ], &clean[ (size_t) 15 * 160 ], (size_t) 15 * 160 * sizeof( Float32 ) ) );
        // the first good frame is changed at its start only, over the cross-fade: half a pitch period after a loss of 20 ms
        size_t changedUpTo = 0;
        for ( size_t n=0; n<160; ++n )
            if ( output[ (size_t) 14 * 160 + n ] != clean[ (size_t) 14 * 160 + n ] )
                changedUpTo = n + 1;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AudioVoiceCodec: the first good frame after 20 ms of loss is changed over " << changedUpTo << " samples, the pitch period was " << pcm->GetConcealer().GetPitchPeriod() );
        ASSERT_TRUE( changedUpTo > 0 && changedUpTo <= pcm->GetConcealer().GetPitchPeriod() / 2 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AudioVoiceCodec: The concealer handles a stream start, silence, resets and bad rates" )
    try
    {
        AUDIO::CPacketLossConcealer concealer;
        ASSERT_FALSE( concealer.IsPrepared() );
        TSamples out( 160, 1.0f );
        concealer.Conceal( &out[ 0 ], 160 );                       // not prepared: silence
        ASSERT_TRUE( 0.0f == PeakOfRange( out, 0, 160 ) );
        ASSERT_FALSE( concealer.Prepare( 7999 ) );
        ASSERT_FALSE( concealer.Prepare( 192001 ) );
        ASSERT_FALSE( concealer.IsPrepared() );
        ASSERT_TRUE( concealer.Prepare( 16000 ) && concealer.IsPrepared() && 16000 == concealer.GetSampleRate() );

        // a loss before any good frame is silence, and not an error
        out.assign( 160, 1.0f );
        concealer.Conceal( &out[ 0 ], 160 );
        ASSERT_TRUE( 0.0f == PeakOfRange( out, 0, 160 ) && concealer.IsConcealing() );
        concealer.OnGoodFrame( &out[ 0 ], 160 );
        ASSERT_FALSE( concealer.IsConcealing() );

        // a silent history conceals as silence
        for ( UInt32 f=0; f<10; ++f )
        {
            out.assign( 160, 0.0f );
            concealer.OnGoodFrame( &out[ 0 ], 160 );
        }
        out.assign( 160, 1.0f );
        concealer.Conceal( &out[ 0 ], 160 );
        ASSERT_TRUE( 0.0f == PeakOfRange( out, 0, 160 ) );

        // a short history (less than the search needs) still gives a finite, bounded signal
        concealer.Reset();
        TSamples tone = MakeHarmonic( 160, 200.0, 16000, 0.5f );
        concealer.OnGoodFrame( &tone[ 0 ], 160 );
        out.assign( 160, 1.0f );
        concealer.Conceal( &out[ 0 ], 160 );
        ASSERT_TRUE( AllFinite( out ) && PeakOfRange( out, 0, 160 ) <= 1.0f );

        // Reset forgets the loss
        concealer.Reset();
        ASSERT_FALSE( concealer.IsConcealing() );
        ASSERT_TRUE( 0 == concealer.GetConcealedSampleCount() );

        // frames larger than the history, and a frame of zero samples
        TSamples big = MakeHarmonic( 16000, 200.0, 16000, 0.5f );
        concealer.OnGoodFrame( &big[ 0 ], 16000 );
        concealer.OnGoodFrame( &big[ 0 ], 0 );
        out.assign( 160, 1.0f );
        concealer.Conceal( &out[ 0 ], 160 );
        ASSERT_TRUE( PeakOfRange( out, 0, 160 ) > 0.3f );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

}

/*-------------------------------------------------------------------------*/

void
PerformAudioVoiceCodecTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING AudioVoiceCodec TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUDIO.VoiceCodec" );

    PerformConfigTests();
    PerformRegistryTests();
    PerformPcmTests();
    PerformConcealmentTests();
}

/*-------------------------------------------------------------------------*/
