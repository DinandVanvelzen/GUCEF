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

#ifndef GUCEF_AUDIO_CAUDIOCONVERTER_H
#include "gucefAUDIO_CAudioConverter.h"
#define GUCEF_AUDIO_CAUDIOCONVERTER_H
#endif /* GUCEF_AUDIO_CAUDIOCONVERTER_H ? */

#include "gucefAUDIO_CAudioBuffer.h"

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

CAudioBuffer::CAudioBuffer( void )
    : m_format()
    , m_data()
    , m_dataBase( GUCEF_NULL )
    , m_frameCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioBuffer::CAudioBuffer( const CAudioFormat& format, UInt32 frameCount )
    : m_format( format )
    , m_data()
    , m_dataBase( GUCEF_NULL )
    , m_frameCount( 0 )
{GUCEF_TRACE;

    Resize( frameCount );
}

/*-------------------------------------------------------------------------*/

CAudioBuffer::CAudioBuffer( const CAudioFormat& format, const void* frames, UInt32 frameCount )
    : m_format( format )
    , m_data()
    , m_dataBase( GUCEF_NULL )
    , m_frameCount( 0 )
{GUCEF_TRACE;

    if ( Resize( frameCount ) && GUCEF_NULL != frames && frameCount > 0 )
        memcpy( m_data.GetBufferPtr(), frames, (size_t) frameCount * m_format.GetBytesPerFrame() );
}

/*-------------------------------------------------------------------------*/

CAudioBuffer::CAudioBuffer( const CAudioBuffer& src )
    : m_format( src.m_format )
    , m_data()
    , m_dataBase( GUCEF_NULL )
    , m_frameCount( 0 )
{GUCEF_TRACE;

    if ( Resize( src.m_frameCount ) && src.m_frameCount > 0 )
        memcpy( m_data.GetBufferPtr(), src.m_data.GetConstBufferPtr(), (size_t) src.m_frameCount * m_format.GetBytesPerFrame() );
}

/*-------------------------------------------------------------------------*/

CAudioBuffer&
CAudioBuffer::operator=( const CAudioBuffer& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_format = src.m_format;
        m_frameCount = 0;
        m_data.Clear( true );
        RefreshDataBase();
        if ( Resize( src.m_frameCount ) && src.m_frameCount > 0 )
            memcpy( m_data.GetBufferPtr(), src.m_data.GetConstBufferPtr(), (size_t) src.m_frameCount * m_format.GetBytesPerFrame() );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioBuffer::~CAudioBuffer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioBuffer::GetFormat( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_format;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioBuffer::GetFrameCount( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_frameCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioBuffer::GetSizeInBytes( void ) const
{GUCEF_TRACE;

    return m_frameCount * m_format.GetBytesPerFrame();
}

/*-------------------------------------------------------------------------*/

bool
CAudioBuffer::IsEmpty( void ) const
{GUCEF_TRACE;

    return 0 == m_frameCount;
}

/*-------------------------------------------------------------------------*/

Float64
CAudioBuffer::GetDurationInSeconds( void ) const
{GUCEF_TRACE;

    if ( 0 == m_format.GetSampleRate() )
        return 0.0;
    return (Float64) m_frameCount / (Float64) m_format.GetSampleRate();
}

/*-------------------------------------------------------------------------*/

void*
CAudioBuffer::GetDataPtr( UInt32 frame )
{
    // no trace macro: this runs where the audio is mixed

    if ( frame > m_frameCount || GUCEF_NULL == m_dataBase )
        return GUCEF_NULL;
    return m_dataBase + (size_t) frame * m_format.GetBytesPerFrame();
}

/*-------------------------------------------------------------------------*/

const void*
CAudioBuffer::GetDataPtr( UInt32 frame ) const
{
    // no trace macro: this runs where the audio is mixed

    if ( frame > m_frameCount || GUCEF_NULL == m_dataBase )
        return GUCEF_NULL;
    return m_dataBase + (size_t) frame * m_format.GetBytesPerFrame();
}

/*-------------------------------------------------------------------------*/

void
CAudioBuffer::RefreshDataBase( void )
{GUCEF_TRACE;

    m_dataBase = static_cast< UInt8* >( m_data.GetBufferPtr() );
}

/*-------------------------------------------------------------------------*/

bool
CAudioBuffer::Resize( UInt32 frameCount )
{GUCEF_TRACE;

    const UInt32 frameSize = m_format.GetBytesPerFrame();
    if ( 0 == frameSize )
        return 0 == frameCount;

    const UInt64 newSize = (UInt64) frameCount * frameSize;
    if ( newSize > 0xFFFFFFFFULL )
        return false;

    const UInt32 oldFrameCount = m_frameCount;
    if ( !m_data.SetBufferSize( (UInt32) newSize ) )
        return false;
    m_data.SetDataSize( (UInt32) newSize );
    m_frameCount = frameCount;
    RefreshDataBase();

    if ( frameCount > oldFrameCount )
        FillSilence( oldFrameCount, frameCount - oldFrameCount );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioBuffer::Clear( void )
{GUCEF_TRACE;

    m_data.Clear( true );
    m_frameCount = 0;
    RefreshDataBase();
}

/*-------------------------------------------------------------------------*/

void
CAudioBuffer::FillSilence( UInt32 firstFrame, UInt32 count )
{
    // no trace macro: this runs where the audio is mixed

    if ( firstFrame >= m_frameCount )
        return;
    if ( 0 == count || firstFrame + count > m_frameCount )
        count = m_frameCount - firstFrame;

    // Unsigned 8 bit audio has its zero point in the middle, every other type has it at zero
    const int silenceByte = ( SAMPLETYPE_UINT8 == m_format.GetSampleType() ) ? 128 : 0;
    memset( m_dataBase + (size_t) firstFrame * m_format.GetBytesPerFrame(), silenceByte, (size_t) count * m_format.GetBytesPerFrame() );
}

/*-------------------------------------------------------------------------*/

bool
CAudioBuffer::Append( const CAudioBuffer& other )
{GUCEF_TRACE;

    if ( other.m_format != m_format )
        return false;
    if ( 0 == other.m_frameCount )
        return true;

    const UInt32 oldFrameCount = m_frameCount;
    if ( !Resize( oldFrameCount + other.m_frameCount ) )
        return false;
    memcpy( GetDataPtr( oldFrameCount ), other.GetDataPtr(), (size_t) other.m_frameCount * m_format.GetBytesPerFrame() );
    return true;
}

/*-------------------------------------------------------------------------*/

Float32
CAudioBuffer::GetSampleAsFloat( UInt32 frame, UInt32 channel ) const
{GUCEF_TRACE;

    if ( frame >= m_frameCount || channel >= m_format.GetChannelCount() )
        return 0.0f;

    Float32 value = 0.0f;
    const UInt8* sample = static_cast< const UInt8* >( GetDataPtr( frame ) ) + (size_t) channel * m_format.GetBytesPerSample();
    CAudioConverter::ConvertToFloat32( sample, m_format.GetSampleType(), &value, 1 );
    return value;
}

/*-------------------------------------------------------------------------*/

void
CAudioBuffer::SetSampleFromFloat( UInt32 frame, UInt32 channel, Float32 value )
{GUCEF_TRACE;

    if ( frame >= m_frameCount || channel >= m_format.GetChannelCount() )
        return;

    UInt8* sample = static_cast< UInt8* >( GetDataPtr( frame ) ) + (size_t) channel * m_format.GetBytesPerSample();
    CAudioConverter::ConvertFromFloat32( &value, sample, m_format.GetSampleType(), 1 );
}

/*-------------------------------------------------------------------------*/

bool
CAudioBuffer::ConvertFormatTo( const CAudioFormat& format )
{GUCEF_TRACE;

    if ( format == m_format )
        return true;

    CAudioBuffer converted;
    if ( !CAudioConverter::Convert( *this, format, converted ) )
        return false;
    *this = converted;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBuffer::ConvertFormatTo( const CAudioFormat& format, TAudioBufferPtr& newBuffer ) const
{GUCEF_TRACE;

    TAudioBufferPtr result( GUCEF_NEW CAudioBuffer() );
    if ( format == m_format )
        *result = *this;
    else if ( !CAudioConverter::Convert( *this, format, *result ) )
        return false;
    newBuffer = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioBuffer::ConvertFormatTo( TSampleType sampleType )
{GUCEF_TRACE;

    CAudioFormat format( m_format );
    format.SetSampleType( sampleType );
    return ConvertFormatTo( format );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
