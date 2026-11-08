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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefAUDIO_CAudioFormat.h"

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

CAudioFormat::CAudioFormat( void )
    : m_sampleRate( 0 )
    , m_channelCount( 0 )
    , m_sampleType( SAMPLETYPE_UNKNOWN )
    , m_channelLayout( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioFormat::CAudioFormat( UInt32 sampleRate      ,
                            UInt32 channelCount    ,
                            TSampleType sampleType ,
                            UInt32 channelLayout   )
    : m_sampleRate( sampleRate )
    , m_channelCount( channelCount )
    , m_sampleType( sampleType )
    , m_channelLayout( 0 )
{GUCEF_TRACE;

    if ( 0 != channelLayout )
        SetChannelLayout( channelLayout );
}

/*-------------------------------------------------------------------------*/

CAudioFormat::CAudioFormat( const CAudioFormat& src )
    : m_sampleRate( src.m_sampleRate )
    , m_channelCount( src.m_channelCount )
    , m_sampleType( src.m_sampleType )
    , m_channelLayout( src.m_channelLayout )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioFormat&
CAudioFormat::operator=( const CAudioFormat& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_sampleRate = src.m_sampleRate;
        m_channelCount = src.m_channelCount;
        m_sampleType = src.m_sampleType;
        m_channelLayout = src.m_channelLayout;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CAudioFormat::~CAudioFormat()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAudioFormat::SetSampleRate( UInt32 sampleRate )
{GUCEF_TRACE;

    m_sampleRate = sampleRate;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetSampleRate( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_sampleRate;
}

/*-------------------------------------------------------------------------*/

void
CAudioFormat::SetChannelCount( UInt32 channelCount )
{GUCEF_TRACE;

    m_channelCount = channelCount;
    if ( 0 != m_channelLayout && GetChannelCountOfLayout( m_channelLayout ) != channelCount )
        m_channelLayout = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetChannelCount( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_channelCount;
}

/*-------------------------------------------------------------------------*/

void
CAudioFormat::SetSampleType( TSampleType sampleType )
{GUCEF_TRACE;

    m_sampleType = sampleType;
}

/*-------------------------------------------------------------------------*/

TSampleType
CAudioFormat::GetSampleType( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_sampleType;
}

/*-------------------------------------------------------------------------*/

void
CAudioFormat::SetChannelLayout( UInt32 channelLayout )
{GUCEF_TRACE;

    m_channelLayout = channelLayout;
    if ( 0 != channelLayout )
        m_channelCount = GetChannelCountOfLayout( channelLayout );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetChannelLayout( void ) const
{
    // no trace macro: this runs where the audio is mixed

    if ( 0 != m_channelLayout )
        return m_channelLayout;
    return GetDefaultChannelLayout( m_channelCount );
}

/*-------------------------------------------------------------------------*/

bool
CAudioFormat::HasExplicitChannelLayout( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return 0 != m_channelLayout;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetBytesPerSample( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return GetBytesPerSample( m_sampleType );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetBytesPerFrame( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return GetBytesPerSample( m_sampleType ) * m_channelCount;
}

/*-------------------------------------------------------------------------*/

bool
CAudioFormat::IsValid( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_sampleRate > 0 && m_channelCount > 0 && GetBytesPerSample( m_sampleType ) > 0;
}

/*-------------------------------------------------------------------------*/

bool
CAudioFormat::operator==( const CAudioFormat& other ) const
{
    // no trace macro: this runs where the audio is mixed

    return m_sampleRate == other.m_sampleRate     &&
           m_channelCount == other.m_channelCount &&
           m_sampleType == other.m_sampleType     &&
           GetChannelLayout() == other.GetChannelLayout();
}

/*-------------------------------------------------------------------------*/

bool
CAudioFormat::operator!=( const CAudioFormat& other ) const
{
    // no trace macro: this runs where the audio is mixed

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CString
CAudioFormat::ToString( void ) const
{GUCEF_TRACE;

    return CORE::UInt32ToString( m_sampleRate ) + " Hz, " + CORE::UInt32ToString( m_channelCount ) + " ch, " + SampleTypeToString( m_sampleType );
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetBytesPerSample( TSampleType sampleType )
{
    // no trace macro: this runs where the audio is mixed

    switch ( sampleType )
    {
        case SAMPLETYPE_UINT8   : return 1;
        case SAMPLETYPE_INT16   : return 2;
        case SAMPLETYPE_INT24   : return 3;
        case SAMPLETYPE_INT32   : return 4;
        case SAMPLETYPE_FLOAT32 : return 4;
        case SAMPLETYPE_FLOAT64 : return 8;
        default                 : return 0;
    }
}

/*-------------------------------------------------------------------------*/

const char*
CAudioFormat::SampleTypeToString( TSampleType sampleType )
{GUCEF_TRACE;

    switch ( sampleType )
    {
        case SAMPLETYPE_UINT8   : return "uint8";
        case SAMPLETYPE_INT16   : return "int16";
        case SAMPLETYPE_INT24   : return "int24";
        case SAMPLETYPE_INT32   : return "int32";
        case SAMPLETYPE_FLOAT32 : return "float32";
        case SAMPLETYPE_FLOAT64 : return "float64";
        default                 : return "unknown";
    }
}

/*-------------------------------------------------------------------------*/

TSampleType
CAudioFormat::SampleTypeFromString( const CString& text )
{GUCEF_TRACE;

    const CString name = text.Lowercase();
    for ( UInt32 t=SAMPLETYPE_UINT8; t<=SAMPLETYPE_FLOAT64; ++t )
        if ( name == SampleTypeToString( (TSampleType) t ) )
            return (TSampleType) t;
    return SAMPLETYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetDefaultChannelLayout( UInt32 channelCount )
{
    // no trace macro: this runs where the audio is mixed

    switch ( channelCount )
    {
        case 0 : return 0;
        case 1 : return AUDIOSPEAKER_FRONT_CENTER;
        case 2 : return AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_FRONT_RIGHT;
        case 3 : return AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_FRONT_RIGHT | AUDIOSPEAKER_FRONT_CENTER;
        case 4 : return AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_FRONT_RIGHT | AUDIOSPEAKER_BACK_LEFT | AUDIOSPEAKER_BACK_RIGHT;
        case 5 : return AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_FRONT_RIGHT | AUDIOSPEAKER_FRONT_CENTER | AUDIOSPEAKER_BACK_LEFT | AUDIOSPEAKER_BACK_RIGHT;
        case 6 : return AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_FRONT_RIGHT | AUDIOSPEAKER_FRONT_CENTER | AUDIOSPEAKER_LOW_FREQUENCY | AUDIOSPEAKER_BACK_LEFT | AUDIOSPEAKER_BACK_RIGHT;
        case 7 : return AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_FRONT_RIGHT | AUDIOSPEAKER_FRONT_CENTER | AUDIOSPEAKER_LOW_FREQUENCY | AUDIOSPEAKER_BACK_CENTER | AUDIOSPEAKER_SIDE_LEFT | AUDIOSPEAKER_SIDE_RIGHT;
        case 8 : return AUDIOSPEAKER_FRONT_LEFT | AUDIOSPEAKER_FRONT_RIGHT | AUDIOSPEAKER_FRONT_CENTER | AUDIOSPEAKER_LOW_FREQUENCY | AUDIOSPEAKER_BACK_LEFT | AUDIOSPEAKER_BACK_RIGHT | AUDIOSPEAKER_SIDE_LEFT | AUDIOSPEAKER_SIDE_RIGHT;
        default :
        {
            // More channels than there are named speakers in a common layout: the lowest positions in order
            UInt32 layout = 0;
            for ( UInt32 i=0; i<channelCount && i<18; ++i )
                layout |= ( 1U << i );
            return layout;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioFormat::GetChannelCountOfLayout( UInt32 channelLayout )
{
    // no trace macro: this runs where the audio is mixed

    UInt32 count = 0;
    while ( 0 != channelLayout )
    {
        count += ( channelLayout & 1U );
        channelLayout >>= 1;
    }
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
