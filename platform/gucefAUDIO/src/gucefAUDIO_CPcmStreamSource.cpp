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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#include "gucefAUDIO_CPcmStreamSource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPcmStreamSource::CPcmStreamSource( CORE::CIOAccess* input                    ,
                                    const CORE::CIOAccess::IOAccessPtr& owner ,
                                    const CAudioFormat& format                ,
                                    UInt64 dataOffset                         ,
                                    UInt64 lengthInFrames                     ,
                                    bool bigEndian                            )
    : IAudioSource()
    , m_input( input )
    , m_owner( owner )
    , m_format( format )
    , m_dataOffset( dataOffset )
    , m_length( lengthInFrames )
    , m_position( 0 )
    , m_swapBytes( bigEndian != IsHostBigEndian() && CAudioFormat::GetBytesPerSample( format.GetSampleType() ) > 1 )
    , m_atEnd( false )
    , m_canSeek( false )
{GUCEF_TRACE;

    // Whether the input can be positioned is found out once, by positioning it where the audio starts
    if ( GUCEF_NULL != m_input )
        m_canSeek = ( 0 == m_input->Seek( (Int64) m_dataOffset, SEEK_SET ) );
}

/*-------------------------------------------------------------------------*/

CPcmStreamSource::~CPcmStreamSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CPcmStreamSource::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

UInt32
CPcmStreamSource::ReadFrames( void* destination, UInt32 frameCapacity )
{GUCEF_TRACE;

    const UInt32 frameSize = m_format.GetBytesPerFrame();
    if ( GUCEF_NULL == m_input || GUCEF_NULL == destination || 0 == frameSize || m_atEnd )
        return 0;

    UInt32 wanted = frameCapacity;
    if ( GUCEF_AUDIO_LENGTH_UNKNOWN != m_length )
    {
        const UInt64 left = ( m_position < m_length ) ? ( m_length - m_position ) : 0;
        if ( left < wanted )
            wanted = (UInt32) left;
    }
    if ( 0 == wanted )
    {
        m_atEnd = true;
        return 0;
    }

    const UInt32 read = m_input->Read( destination, frameSize, wanted );
    if ( read < wanted )
        m_atEnd = true;                     // the input ended before the length it promised, or it is simply as long as it is
    if ( m_swapBytes && read > 0 )
        SwapSampleBytes( destination, read, m_format );

    m_position += read;
    if ( GUCEF_AUDIO_LENGTH_UNKNOWN != m_length && m_position >= m_length )
        m_atEnd = true;
    return read;
}

/*-------------------------------------------------------------------------*/

bool
CPcmStreamSource::IsEndOfStream( void ) const
{GUCEF_TRACE;

    return m_atEnd;
}

/*-------------------------------------------------------------------------*/

bool
CPcmStreamSource::IsSeekable( void ) const
{GUCEF_TRACE;

    return m_canSeek;
}

/*-------------------------------------------------------------------------*/

bool
CPcmStreamSource::SeekToFrame( UInt64 frame )
{GUCEF_TRACE;

    if ( !m_canSeek || GUCEF_NULL == m_input )
        return false;
    if ( GUCEF_AUDIO_LENGTH_UNKNOWN != m_length && frame > m_length )
        return false;
    if ( 0 != m_input->Seek( (Int64) ( m_dataOffset + frame * m_format.GetBytesPerFrame() ), SEEK_SET ) )
        return false;
    m_position = frame;
    m_atEnd = ( GUCEF_AUDIO_LENGTH_UNKNOWN != m_length && frame >= m_length );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CPcmStreamSource::GetPositionInFrames( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

UInt64
CPcmStreamSource::GetLengthInFrames( void ) const
{GUCEF_TRACE;

    return m_length;
}

/*-------------------------------------------------------------------------*/

bool
CPcmStreamSource::IsHostBigEndian( void )
{GUCEF_TRACE;

    const UInt16 probe = 1;
    return 0 == *reinterpret_cast< const UInt8* >( &probe );
}

/*-------------------------------------------------------------------------*/

void
CPcmStreamSource::SwapSampleBytes( void* frames, UInt32 frameCount, const CAudioFormat& format )
{GUCEF_TRACE;

    const UInt32 sampleSize = format.GetBytesPerSample();
    if ( sampleSize < 2 )
        return;

    UInt8* bytes = static_cast< UInt8* >( frames );
    const UInt64 sampleCount = (UInt64) frameCount * format.GetChannelCount();
    for ( UInt64 s=0; s<sampleCount; ++s )
    {
        UInt8* sample = bytes + s * sampleSize;
        for ( UInt32 i=0; i<sampleSize / 2; ++i )
        {
            const UInt8 t = sample[ i ];
            sample[ i ] = sample[ sampleSize - 1 - i ];
            sample[ sampleSize - 1 - i ] = t;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
