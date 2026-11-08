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

#include <string.h>

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#include "gucefAUDIO_CAudioCodecRegistry.h"
#define GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOCODECREGISTRY_H ? */

#include "gucefAUDIO_CAudioClip.h"

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

CAudioClip::CAudioClip( void )
    : m_audio( GUCEF_NEW CAudioBuffer() )
    , m_name()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioClip::CAudioClip( const CAudioBuffer& audio, const CString& name )
    : m_audio( GUCEF_NEW CAudioBuffer( audio ) )
    , m_name( name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioClip::CAudioClip( const TAudioBufferPtr& audio, const CString& name )
    : m_audio( audio )
    , m_name( name )
{GUCEF_TRACE;

    if ( m_audio.IsNULL() )
        m_audio = TAudioBufferPtr( GUCEF_NEW CAudioBuffer() );
}

/*-------------------------------------------------------------------------*/

CAudioClip::CAudioClip( const CAudioClip& src )
    : m_audio( src.m_audio )
    , m_name( src.m_name )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioClip&
CAudioClip::operator=( const CAudioClip& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_audio = src.m_audio;
        m_name = src.m_name;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioClip::~CAudioClip()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioClip::GetFormat( void ) const
{GUCEF_TRACE;

    return m_audio->GetFormat();
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioClip::GetFrameCount( void ) const
{GUCEF_TRACE;

    return m_audio->GetFrameCount();
}

/*-------------------------------------------------------------------------*/

Float64
CAudioClip::GetDurationInSeconds( void ) const
{GUCEF_TRACE;

    return m_audio->GetDurationInSeconds();
}

/*-------------------------------------------------------------------------*/

bool
CAudioClip::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_audio->IsEmpty();
}

/*-------------------------------------------------------------------------*/

const CAudioBuffer&
CAudioClip::GetBuffer( void ) const
{GUCEF_TRACE;

    return *m_audio;
}

/*-------------------------------------------------------------------------*/

const TAudioBufferPtr&
CAudioClip::GetSharedBuffer( void ) const
{GUCEF_TRACE;

    return m_audio;
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioClip::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CAudioClip::SetName( const CString& name )
{GUCEF_TRACE;

    m_name = name;
}

/*-------------------------------------------------------------------------*/

bool
CAudioClip::Decode( const CString& codecType, CORE::CIOAccess& encodedInput, TAudioClipPtr& clip, const CORE::CValueList& options, const CString& name )
{GUCEF_TRACE;

    CAudioCodecRegistry::TAudioCodecPtr codec;
    if ( !CAudioGlobal::Instance()->GetAudioCodecRegistry().TryFindCodec( codecType, codec ) || codec.IsNULL() )
        return false;

    CAudioBuffer audio;
    if ( !codec->DecodeAll( encodedInput, audio, options ) )
        return false;

    clip = TAudioClipPtr( GUCEF_NEW CAudioClip( audio, name ) );
    return true;
}

/*-------------------------------------------------------------------------*/

TAudioSourcePtr
CAudioClip::CreateSource( void ) const
{GUCEF_TRACE;

    return TAudioSourcePtr( GUCEF_NEW CAudioClipSource( m_audio ) );
}

/*-------------------------------------------------------------------------*/

CAudioClipSource::CAudioClipSource( const TAudioBufferPtr& audio )
    : IAudioSource()
    , m_audio( audio )
    , m_buffer( GUCEF_NULL )
    , m_position( 0 )
{GUCEF_TRACE;

    if ( m_audio.IsNULL() )
        m_audio = TAudioBufferPtr( GUCEF_NEW CAudioBuffer() );
    m_buffer = m_audio.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

CAudioClipSource::~CAudioClipSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioClipSource::GetFormat( void ) const
{GUCEF_TRACE;

    return m_audio->GetFormat();
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioClipSource::ReadFrames( void* destination, UInt32 frameCapacity )
{
    // no trace macro: this runs where the audio is mixed

    const UInt32 left = m_buffer->GetFrameCount() - m_position;
    const UInt32 count = ( frameCapacity < left ) ? frameCapacity : left;
    if ( count > 0 && GUCEF_NULL != destination )
    {
        memcpy( destination, m_buffer->GetDataPtr( m_position ), (size_t) count * m_buffer->GetFormat().GetBytesPerFrame() );
        m_position += count;
        return count;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CAudioClipSource::IsEndOfStream( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_position >= m_buffer->GetFrameCount();
}

/*-------------------------------------------------------------------------*/

bool
CAudioClipSource::IsRealtimeSafe( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioClipSource::IsSeekable( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioClipSource::SeekToFrame( UInt64 frame )
{
    // no trace macro: this runs where the audio is mixed

    if ( frame > m_buffer->GetFrameCount() )
        return false;
    m_position = (UInt32) frame;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioClipSource::GetPositionInFrames( void ) const
{GUCEF_TRACE;

    return m_position;
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioClipSource::GetLengthInFrames( void ) const
{GUCEF_TRACE;

    return m_audio->GetFrameCount();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
