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

#ifndef GUCEF_AUDIO_TESTAPP_TESTAUDIOCODECSUPPORT_H
#define GUCEF_AUDIO_TESTAPP_TESTAUDIOCODECSUPPORT_H

/*
 *  What the tests of the compressed codec plugins (FLAC, MP3, Ogg) share: loading the plugins, test signals made in
 *  code (no stored files), measurements on decoded audio, checks against hostile input and the harness that proves
 *  streams of one plugin decode at the same time.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <string.h>

#ifndef GUCEFAUDIO_H
#include "gucefAUDIO.h"
#define GUCEFAUDIO_H
#endif /* GUCEFAUDIO_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#include "gucefAUDIO_CAudioCodecPluginManager.h"
#define GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#endif /* GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CodecTest {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      PLUGINS                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef AUDIO::CAudioCodecRegistry::TAudioCodecPtr TCodecPtr;

inline TCodecPtr
FindCodec( const char* type )
{
    TCodecPtr codec;
    AUDIO::CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodec( type, codec );
    return codec;
}

/*-------------------------------------------------------------------------*/

/** Loads the plugin from the directory of the test application, true when it is there and registered 'probeType' */
inline bool
LoadPlugin( const char* pluginName, const char* group, const char* probeType )
{
    if ( !FindCodec( probeType ).IsNULL() )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString first = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    const CORE::CString second = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    #else
    const CORE::CString first = CORE::CString( "$MODULEDIR$/" ) + pluginName;
    const CORE::CString second = CORE::CString( "$MODULEDIR$/" ) + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return ( pluginControl.AddPluginFromDir( first, group, true ) || pluginControl.AddPluginFromDir( second, group, true ) ) && !FindCodec( probeType ).IsNULL();
}

/*-------------------------------------------------------------------------*/

inline bool
UnloadPlugin( const char* group, const char* probeType )
{
    return CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( group ) && FindCodec( probeType ).IsNULL();
}

/*-------------------------------------------------------------------------*/

inline UInt32
GetThreadingModel( const char* type )
{
    AUDIO::CAudioCodecPluginManager::TAudioCodecCPluginPtr plugin = AUDIO::CAudioGlobal::Instance()->GetAudioCodecPluginManager().GetPluginForCodecType( type );
    return plugin.IsNULL() ? 0xFFFFFFFF : plugin->GetThreadingModel();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      SIGNALS                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A xorshift generator, so the "random" tests are the same on every run and every platform */
class CRandom
{
    public:

    explicit CRandom( UInt32 seed = 2463534242UL ) : m_state( 0 == seed ? 1 : seed ) {}

    UInt32 Next( void )
    {
        m_state ^= m_state << 13;
        m_state ^= m_state >> 17;
        m_state ^= m_state << 5;
        return m_state;
    }

    /** A value from 0 to 'count' - 1 */
    UInt32 Below( UInt32 count ) { return 0 == count ? 0 : Next() % count; }

    /** A value from -1 to 1 */
    Float32 NextFloat( void ) { return (Float32) ( (Float64) Next() / 2147483648.0 - 1.0 ); }

    private:

    UInt32 m_state;
};

/*-------------------------------------------------------------------------*/

/** The frequency of the tone of a channel: the channels have different tones, so that mixing them up shows */
inline Float64
ToneOfChannel( UInt32 channel, Float64 baseFrequency )
{
    return baseFrequency * (Float64) ( channel + 1 );
}

/*-------------------------------------------------------------------------*/

/** Sines, channel c at baseFrequency * ( c + 1 ), with the given peak amplitude */
inline AUDIO::CAudioBuffer
MakeTones( const AUDIO::CAudioFormat& format, UInt32 frames, Float64 baseFrequency, Float64 amplitude )
{
    AUDIO::CAudioBuffer audio( format, frames );
    const Float64 twoPi = 6.283185307179586476925286766559;
    for ( UInt32 c=0; c<format.GetChannelCount(); ++c )
    {
        const Float64 step = twoPi * ToneOfChannel( c, baseFrequency ) / (Float64) format.GetSampleRate();
        for ( UInt32 f=0; f<frames; ++f )
            audio.SetSampleFromFloat( f, c, (Float32) ( amplitude * sin( step * (Float64) f ) ) );
    }
    return audio;
}

/*-------------------------------------------------------------------------*/

inline AUDIO::CAudioBuffer
MakeNoise( const AUDIO::CAudioFormat& format, UInt32 frames, Float32 amplitude, UInt32 seed )
{
    AUDIO::CAudioBuffer audio( format, frames );
    CRandom random( seed );
    for ( UInt32 f=0; f<frames; ++f )
        for ( UInt32 c=0; c<format.GetChannelCount(); ++c )
            audio.SetSampleFromFloat( f, c, amplitude * random.NextFloat() );
    return audio;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      DECODING                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CIOAccess::IOAccessPtr TInputPtr;

/** An input on the bytes of a buffer, which has to stay alive as long as the input is used */
inline TInputPtr
MakeInput( CORE::CDynamicBuffer& file )
{
    return TInputPtr( GUCEF_NEW CORE::CDynamicBufferAccess( file ) );
}

/*-------------------------------------------------------------------------*/

inline bool
OpenSource( TCodecPtr codec, CORE::CDynamicBuffer& file, AUDIO::TAudioSourcePtr& source, const CORE::CValueList& options = CORE::CValueList() )
{
    TInputPtr input = MakeInput( file );
    return codec->OpenSource( input, source, options );
}

/*-------------------------------------------------------------------------*/

inline bool
Encode( TCodecPtr codec, const AUDIO::CAudioBuffer& audio, CORE::CDynamicBuffer& file, const CORE::CValueList& options = CORE::CValueList() )
{
    file.Clear();
    CORE::CDynamicBufferAccess output( file );
    return codec->Encode( audio, output, options );
}

/*-------------------------------------------------------------------------*/

/** Reads from the source until it has nothing more, into a buffer of the format of the source. 'maxFrames' guards against a source that never ends */
inline bool
ReadAll( AUDIO::IAudioSource& source, AUDIO::CAudioBuffer& audio, UInt32 chunkFrames = 997, UInt64 maxFrames = 50000000 )
{
    const AUDIO::CAudioFormat& format = source.GetFormat();
    const UInt32 frameBytes = format.GetBytesPerFrame();
    CORE::CDynamicBuffer all;
    CORE::CDynamicBuffer chunk( chunkFrames * frameBytes, true );
    UInt64 total = 0;
    while ( total < maxFrames )
    {
        const UInt32 got = source.ReadFrames( chunk.GetBufferPtr(), chunkFrames );
        if ( 0 == got )
            break;
        all.Append( chunk.GetConstBufferPtr(), got * frameBytes );
        total += got;
    }
    audio = AUDIO::CAudioBuffer( format, all.GetConstBufferPtr(), (UInt32) total );
    return total < maxFrames;
}

/*-------------------------------------------------------------------------*/

/** Decodes a whole file through a stream of the codec */
inline bool
DecodeFile( TCodecPtr codec, CORE::CDynamicBuffer& file, AUDIO::CAudioBuffer& audio, const CORE::CValueList& options = CORE::CValueList() )
{
    AUDIO::TAudioSourcePtr source;
    if ( !OpenSource( codec, file, source, options ) )
        return false;
    return ReadAll( *source, audio );
}

/*-------------------------------------------------------------------------*/

inline bool
SameBytes( const AUDIO::CAudioBuffer& a, const AUDIO::CAudioBuffer& b )
{
    return a.GetFormat() == b.GetFormat() && a.GetFrameCount() == b.GetFrameCount() &&
           ( 0 == a.GetSizeInBytes() || 0 == memcmp( a.GetDataPtr(), b.GetDataPtr(), a.GetSizeInBytes() ) );
}

/*-------------------------------------------------------------------------*/

/** The same frames in the buffers 'a' from 'firstA' and 'b' from 'firstB', count frames of all channels, to within 'tolerance' */
inline bool
SameFrames( const AUDIO::CAudioBuffer& a, UInt32 firstA, const AUDIO::CAudioBuffer& b, UInt32 firstB, UInt32 count, Float32 tolerance )
{
    if ( a.GetFormat().GetChannelCount() != b.GetFormat().GetChannelCount() || firstA + count > a.GetFrameCount() || firstB + count > b.GetFrameCount() )
        return false;
    for ( UInt32 f=0; f<count; ++f )
        for ( UInt32 c=0; c<a.GetFormat().GetChannelCount(); ++c )
            if ( fabs( a.GetSampleAsFloat( firstA + f, c ) - b.GetSampleAsFloat( firstB + f, c ) ) > tolerance )
                return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MEASUREMENTS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The amplitude of the tone with the given frequency in the frames [first, first + count) of a channel */
inline Float64
ToneAmplitude( const AUDIO::CAudioBuffer& audio, UInt32 channel, Float64 frequency, UInt32 first, UInt32 count )
{
    const Float64 step = 6.283185307179586476925286766559 * frequency / (Float64) audio.GetFormat().GetSampleRate();
    Float64 inPhase = 0.0, quadrature = 0.0;
    for ( UInt32 f=0; f<count; ++f )
    {
        const Float64 sample = audio.GetSampleAsFloat( first + f, channel );
        inPhase += sample * sin( step * (Float64) f );
        quadrature += sample * cos( step * (Float64) f );
    }
    return 2.0 * sqrt( inPhase * inPhase + quadrature * quadrature ) / (Float64) count;
}

/*-------------------------------------------------------------------------*/

inline Float64
ToDecibel( Float64 ratio )
{
    return ratio <= 1.0e-12 ? -240.0 : 20.0 * log10( ratio );
}

/*-------------------------------------------------------------------------*/

/** Signal to noise ratio of 'test' against 'reference' for a channel, test delayed by 'lag' frames, over the frames [first, first + count) of the reference */
inline Float64
SnrInDb( const AUDIO::CAudioBuffer& reference, const AUDIO::CAudioBuffer& test, UInt32 channel, Int32 lag, UInt32 first, UInt32 count )
{
    Float64 signal = 0.0, noise = 0.0;
    for ( UInt32 f=first; f<first+count; ++f )
    {
        const Float64 r = reference.GetSampleAsFloat( f, channel );
        const Float64 t = test.GetSampleAsFloat( (UInt32) ( (Int32) f + lag ), channel );
        signal += r * r;
        noise += ( r - t ) * ( r - t );
    }
    return 0.0 == noise ? 240.0 : 10.0 * log10( signal / noise );
}

/*-------------------------------------------------------------------------*/

/** The delay of 'test' against 'reference' (in frames, within +-maxLag) at which they match best, by cross correlation */
inline Int32
BestLag( const AUDIO::CAudioBuffer& reference, const AUDIO::CAudioBuffer& test, UInt32 channel, Int32 maxLag, UInt32 first, UInt32 count )
{
    Int32 best = 0;
    Float64 bestValue = -1.0e300;
    for ( Int32 lag=-maxLag; lag<=maxLag; ++lag )
    {
        Float64 sum = 0.0;
        for ( UInt32 f=first; f<first+count; ++f )
            sum += reference.GetSampleAsFloat( f, channel ) * test.GetSampleAsFloat( (UInt32) ( (Int32) f + lag ), channel );
        if ( sum > bestValue )
        {
            bestValue = sum;
            best = lag;
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

/** The channel of a frame range that has the most of the tone of 'frequency' (the channels have different tones, this finds which one is where) */
inline UInt32
ChannelWithTone( const AUDIO::CAudioBuffer& audio, Float64 frequency, UInt32 first, UInt32 count )
{
    UInt32 best = 0;
    Float64 bestValue = -1.0;
    for ( UInt32 c=0; c<audio.GetFormat().GetChannelCount(); ++c )
    {
        const Float64 value = ToneAmplitude( audio, c, frequency, first, count );
        if ( value > bestValue )
        {
            bestValue = value;
            best = c;
        }
    }
    return best;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MD5                                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

/** MD5 (RFC 1321), for the checksum FLAC keeps of its audio */
class CMd5
{
    public:

    CMd5( void ) : m_byteCount( 0 )
    {
        m_state[ 0 ] = 0x67452301UL; m_state[ 1 ] = 0xefcdab89UL; m_state[ 2 ] = 0x98badcfeUL; m_state[ 3 ] = 0x10325476UL;
    }

    void Update( const void* data, size_t size )
    {
        const UInt8* bytes = (const UInt8*) data;
        while ( size > 0 )
        {
            const UInt32 used = (UInt32) ( m_byteCount % 64 );
            size_t take = 64 - used;
            if ( take > size )
                take = size;
            memcpy( m_block + used, bytes, take );
            m_byteCount += take;
            bytes += take;
            size -= take;
            if ( 64 == used + take )
                Process( m_block );
        }
    }

    void Final( UInt8* digest )
    {
        UInt8 padding[ 72 ];
        const UInt64 bitCount = m_byteCount * 8;
        const UInt32 used = (UInt32) ( m_byteCount % 64 );
        const UInt32 padSize = ( used < 56 ) ? 56 - used : 120 - used;
        memset( padding, 0, sizeof( padding ) );
        padding[ 0 ] = 0x80;
        for ( UInt32 i=0; i<8; ++i )
            padding[ padSize + i ] = (UInt8) ( ( bitCount >> ( 8 * i ) ) & 0xFF );
        Update( padding, padSize + 8 );
        for ( UInt32 i=0; i<16; ++i )
            digest[ i ] = (UInt8) ( ( m_state[ i / 4 ] >> ( 8 * ( i % 4 ) ) ) & 0xFF );
    }

    private:

    static UInt32 Rotate( UInt32 x, UInt32 n ) { return ( ( x << n ) | ( x >> ( 32 - n ) ) ) & 0xFFFFFFFFUL; }

    void Process( const UInt8* data )
    {
        static const UInt32 k[ 64 ] = {
            0xd76aa478UL, 0xe8c7b756UL, 0x242070dbUL, 0xc1bdceeeUL, 0xf57c0fafUL, 0x4787c62aUL, 0xa8304613UL, 0xfd469501UL,
            0x698098d8UL, 0x8b44f7afUL, 0xffff5bb1UL, 0x895cd7beUL, 0x6b901122UL, 0xfd987193UL, 0xa679438eUL, 0x49b40821UL,
            0xf61e2562UL, 0xc040b340UL, 0x265e5a51UL, 0xe9b6c7aaUL, 0xd62f105dUL, 0x02441453UL, 0xd8a1e681UL, 0xe7d3fbc8UL,
            0x21e1cde6UL, 0xc33707d6UL, 0xf4d50d87UL, 0x455a14edUL, 0xa9e3e905UL, 0xfcefa3f8UL, 0x676f02d9UL, 0x8d2a4c8aUL,
            0xfffa3942UL, 0x8771f681UL, 0x6d9d6122UL, 0xfde5380cUL, 0xa4beea44UL, 0x4bdecfa9UL, 0xf6bb4b60UL, 0xbebfbc70UL,
            0x289b7ec6UL, 0xeaa127faUL, 0xd4ef3085UL, 0x04881d05UL, 0xd9d4d039UL, 0xe6db99e5UL, 0x1fa27cf8UL, 0xc4ac5665UL,
            0xf4292244UL, 0x432aff97UL, 0xab9423a7UL, 0xfc93a039UL, 0x655b59c3UL, 0x8f0ccc92UL, 0xffeff47dUL, 0x85845dd1UL,
            0x6fa87e4fUL, 0xfe2ce6e0UL, 0xa3014314UL, 0x4e0811a1UL, 0xf7537e82UL, 0xbd3af235UL, 0x2ad7d2bbUL, 0xeb86d391UL };
        static const UInt8 s[ 64 ] = {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21 };
        UInt32 m[ 16 ];
        UInt32 a = m_state[ 0 ], b = m_state[ 1 ], c = m_state[ 2 ], d = m_state[ 3 ];
        for ( UInt32 i=0; i<16; ++i )
            m[ i ] = (UInt32) data[ i*4 ] | ( (UInt32) data[ i*4+1 ] << 8 ) | ( (UInt32) data[ i*4+2 ] << 16 ) | ( (UInt32) data[ i*4+3 ] << 24 );
        for ( UInt32 i=0; i<64; ++i )
        {
            UInt32 f, g;
            if ( i < 16 )      { f = ( b & c ) | ( ~b & d ); g = i; }
            else if ( i < 32 ) { f = ( d & b ) | ( ~d & c ); g = ( 5*i + 1 ) % 16; }
            else if ( i < 48 ) { f = b ^ c ^ d;              g = ( 3*i + 5 ) % 16; }
            else               { f = c ^ ( b | ( ~d & 0xFFFFFFFFUL ) ); g = ( 7*i ) % 16; }
            const UInt32 t = d;
            d = c;
            c = b;
            b = ( b + Rotate( ( a + f + k[ i ] + m[ g ] ) & 0xFFFFFFFFUL, s[ i ] ) ) & 0xFFFFFFFFUL;
            a = t;
        }
        m_state[ 0 ] = ( m_state[ 0 ] + a ) & 0xFFFFFFFFUL;
        m_state[ 1 ] = ( m_state[ 1 ] + b ) & 0xFFFFFFFFUL;
        m_state[ 2 ] = ( m_state[ 2 ] + c ) & 0xFFFFFFFFUL;
        m_state[ 3 ] = ( m_state[ 3 ] + d ) & 0xFFFFFFFFUL;
    }

    UInt32 m_state[ 4 ];
    UInt64 m_byteCount;
    UInt8 m_block[ 64 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      HOSTILE INPUT                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Opens and reads everything of one (damaged) file, seeks about in it and asks for its tags. Nothing may crash or
 *  go on for ever. False when it took too long or the source produced more audio than any real file of this test could.
 */
inline bool
UseDamagedFile( TCodecPtr codec, CORE::CDynamicBuffer& file, UInt64 frameLimit, UInt32 timeLimitInMs, UInt32 seed )
{
    const UInt64 startTick = MT::PrecisionTickCount();
    const UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    AUDIO::TAudioSourcePtr source;
    bool isOk = true;

    if ( OpenSource( codec, file, source ) )
    {
        const AUDIO::CAudioFormat format = source->GetFormat();
        const UInt32 chunk = 701;
        CORE::CDynamicBuffer buffer( chunk * format.GetBytesPerFrame(), true );
        UInt64 total = 0;
        CORE::CValueList tags;
        source->GetMetaData( tags );
        for ( ;; )
        {
            const UInt32 got = source->ReadFrames( buffer.GetBufferPtr(), chunk );
            if ( 0 == got )
                break;
            total += got;
            if ( total > frameLimit )
            {
                isOk = false;
                break;
            }
        }

        // seeking about in a file that is not what it was
        CRandom random( seed );
        const UInt64 length = source->GetLengthInFrames();
        for ( UInt32 i=0; i<4 && isOk; ++i )
        {
            const UInt64 target = ( GUCEF_AUDIO_LENGTH_UNKNOWN != length && length > 0 ) ? random.Next() % length : random.Next() % 5000;
            if ( source->SeekToFrame( target ) )
            {
                const UInt32 got = source->ReadFrames( buffer.GetBufferPtr(), chunk );
                isOk = got <= chunk;
            }
        }
        source->GetMetaData( tags );
    }

    CORE::CValueList metaData;
    CORE::CDynamicBufferAccess input( file );
    codec->DecodeMetaData( input, metaData );

    const UInt64 elapsedMs = ( MT::PrecisionTickCount() - startTick ) / ( 0 == ticksPerMs ? 1 : ticksPerMs );
    return isOk && elapsedMs <= timeLimitInMs;
}

/*-------------------------------------------------------------------------*/

/**
 *  Every length of the file from nothing to the whole of it, taking steps of 'step' bytes, and 'corruptions' files
 *  with some bytes changed (fixed seed). Returns the number of cases that went wrong (took too long, ran on for ever).
 */
inline UInt32
SweepDamage( TCodecPtr codec, const CORE::CDynamicBuffer& file, UInt32 step, UInt32 corruptions, UInt64 frameLimit, UInt32 seed )
{
    UInt32 failures = 0;
    const UInt32 size = file.GetDataSize();

    for ( UInt32 length=0; length<=size; length += step )
    {
        CORE::CDynamicBuffer cut( (const char*) file.GetConstBufferPtr(), length, true );
        if ( !UseDamagedFile( codec, cut, frameLimit, 5000, length + seed ) )
            ++failures;
    }

    CRandom random( seed );
    for ( UInt32 i=0; i<corruptions; ++i )
    {
        CORE::CDynamicBuffer damaged( (const char*) file.GetConstBufferPtr(), size, true );
        UInt8* bytes = (UInt8*) damaged.GetBufferPtr();
        const UInt32 changes = 1 + random.Below( 8 );
        for ( UInt32 n=0; n<changes; ++n )
        {
            // half of the damage lands in the first part of the file, where the headers are
            const UInt32 position = ( 0 == random.Below( 2 ) ) ? random.Below( size < 600 ? size : 600 ) : random.Below( size );
            switch ( random.Below( 3 ) )
            {
                case 0: bytes[ position ] = (UInt8) random.Next(); break;
                case 1: bytes[ position ] ^= (UInt8) ( 1U << random.Below( 8 ) ); break;
                default: bytes[ position ] = 0 == random.Below( 2 ) ? 0x00 : 0xFF; break;
            }
        }
        if ( !UseDamagedFile( codec, damaged, frameLimit, 5000, i + seed ) )
            ++failures;
    }
    return failures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONCURRENCY                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts how many reads of the inputs that use it are in progress at the same time */
class COverlapCounter
{
    public:

    COverlapCounter( void ) : m_inside( 0 ), m_maxInside( 0 ) {}

    /** Called by an input around a read. 'waitForPartner' holds the read until a second one is inside, or a while has passed */
    void Enter( bool waitForPartner )
    {
        const Int32 now = m_inside.FetchAdd( 1 ) + 1;
        Int32 seen = m_maxInside.Load();
        while ( now > seen && !m_maxInside.CompareExchangeWeak( seen, now ) ) {}

        if ( waitForPartner )
        {
            for ( UInt32 i=0; i<250 && m_inside.Load() < 2; ++i )
                MT::ThreadDelay( 1 );
        }
    }

    void Leave( void ) { m_inside.FetchSub( 1 ); }

    Int32 GetMaxInside( void ) const { return m_maxInside.Load(); }

    private:

    MT::CTAtomic< Int32 > m_inside;
    MT::CTAtomic< Int32 > m_maxInside;
};

/*-------------------------------------------------------------------------*/

/** An input that reports its reads to a counter, once it is armed (opening a stream reads the header, not counted) */
class COverlapInput : public CORE::CDynamicBufferAccess
{
    public:

    COverlapInput( CORE::CDynamicBuffer& buffer, COverlapCounter& counter )
        : CORE::CDynamicBufferAccess( buffer ), m_counter( counter ), m_isArmed( false ), m_isFirst( true ) {}

    void Arm( void ) { m_isArmed = true; }

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_isArmed )
            return CORE::CDynamicBufferAccess::Read( dest, esize, elements );

        m_counter.Enter( m_isFirst );
        m_isFirst = false;
        const UInt32 result = CORE::CDynamicBufferAccess::Read( dest, esize, elements );
        m_counter.Leave();
        return result;
    }

    private:

    COverlapCounter& m_counter;
    bool m_isArmed;
    bool m_isFirst;
};

/*-------------------------------------------------------------------------*/

/** Reads a source to its end on a thread of its own */
class CDecodeThread : public MT::CActiveObject
{
    public:

    explicit CDecodeThread( const AUDIO::TAudioSourcePtr& source )
        : MT::CActiveObject(), m_source( source ), m_audio(), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        ReadAll( *m_source, m_audio, 500 );
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitForDone( UInt32 timeoutInMs )
    {
        for ( UInt32 i=0; i<timeoutInMs && 0 == done.Load(); ++i )
            MT::ThreadDelay( 1 );
        return 0 != done.Load();
    }

    AUDIO::TAudioSourcePtr m_source;
    AUDIO::CAudioBuffer m_audio;
    MT::CTAtomic< UInt32 > done;
};

/*-------------------------------------------------------------------------*/

/**
 *  Decodes the same file through two sources of the codec at once, one thread each. 'maxOverlap' tells how many reads
 *  of the plugin were seen inside it at the same time. False when what the two threads decoded is not what a single
 *  decode gives ('reference').
 */
inline bool
DecodeTwoAtOnce( TCodecPtr codec, CORE::CDynamicBuffer& file, const AUDIO::CAudioBuffer& reference, Int32& maxOverlap )
{
    COverlapCounter counter;
    COverlapInput* inputs[ 2 ];
    AUDIO::TAudioSourcePtr sources[ 2 ];
    CDecodeThread* threads[ 2 ];
    for ( UInt32 i=0; i<2; ++i )
    {
        inputs[ i ] = GUCEF_NEW COverlapInput( file, counter );
        TInputPtr owner( inputs[ i ] );
        if ( !codec->OpenSource( owner, sources[ i ] ) )
            return false;
        inputs[ i ]->Arm();
        threads[ i ] = GUCEF_NEW CDecodeThread( sources[ i ] );
    }

    bool isOk = true;
    for ( UInt32 i=0; i<2; ++i )
        isOk = threads[ i ]->Activate() && isOk;
    for ( UInt32 i=0; i<2; ++i )
    {
        isOk = threads[ i ]->WaitForDone( 30000 ) && isOk;
        threads[ i ]->Deactivate( false, true );
    }
    for ( UInt32 i=0; i<2; ++i )
    {
        isOk = SameBytes( threads[ i ]->m_audio, reference ) && isOk;
        GUCEF_DELETE threads[ i ];
    }
    maxOverlap = counter.GetMaxInside();
    return isOk;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      STREAMING                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Plays the source through a stream that loops 'loops' times, filled by hand, and gives what came out */
inline bool
PlayLooped( const AUDIO::TAudioSourcePtr& source, UInt32 loops, AUDIO::CAudioBuffer& played, UInt64& underruns )
{
    AUDIO::CAudioStream stream( source, 200, loops, false );
    const AUDIO::CAudioFormat format = stream.GetFormat();
    const UInt32 chunk = 311;
    CORE::CDynamicBuffer all;
    CORE::CDynamicBuffer buffer( chunk * format.GetBytesPerFrame(), true );
    UInt64 total = 0;
    UInt32 idleRounds = 0;
    while ( !stream.IsEndOfStream() && total < 20000000 && idleRounds < 2000 )
    {
        stream.Fill();
        const UInt32 got = stream.ReadFrames( buffer.GetBufferPtr(), chunk );
        if ( 0 == got )
        {
            ++idleRounds;
            continue;
        }
        all.Append( buffer.GetConstBufferPtr(), got * format.GetBytesPerFrame() );
        total += got;
    }
    underruns = stream.GetUnderrunCount();
    played = AUDIO::CAudioBuffer( format, all.GetConstBufferPtr(), (UInt32) total );
    return idleRounds < 2000;
}

/*-------------------------------------------------------------------------*/

}; /* namespace CodecTest */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_TESTAPP_TESTAUDIOCODECSUPPORT_H ? */
