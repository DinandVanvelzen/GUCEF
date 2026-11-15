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

#ifndef GUCEF_AUDIO_CPANNINGSPATIALIZER_H
#define GUCEF_AUDIO_CPANNINGSPATIALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_IAUDIOSPATIALIZER_H
#include "gucefAUDIO_IAudioSpatializer.h"
#define GUCEF_AUDIO_IAUDIOSPATIALIZER_H
#endif /* GUCEF_AUDIO_IAUDIOSPATIALIZER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPANNER_H
#include "gucefAUDIO_CAudioPanner.h"
#define GUCEF_AUDIO_CAUDIOPANNER_H
#endif /* GUCEF_AUDIO_CAUDIOPANNER_H ? */

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
 *  The built-in spatializer, type name "panning" (design 5.6): amplitude panning with CAudioPanner, the stereo law for a
 *  stereo mix and pairwise panning on the horizontal ring for surround, widening toward all speakers near the listener,
 *  the front and back cue of stereo output, and the optional feed of the low frequency channel. The elevation is not
 *  rendered (top speakers are on the ring at their azimuth).
 *
 *  The gain of every speaker is ramped linearly over the block from the gains of the previous block, as the output stage
 *  of a non spatial voice does. Two speaker tables are prepared, with and without the centre speakers, so switching
 *  CAudioSpatialParams::SetUseCenterSpeaker() while voices play needs nothing but a table choice on the audio thread.
 *
 *  A centred source on a stereo mix has the gain 1 on both speakers (to Float32 rounding): the same as a non spatial mono voice
 *  at pan 0.
 */
class GUCEF_AUDIO_PUBLIC_CPP CPanningSpatializer : public IAudioSpatializer
{
    public:

    static const CString TypeName;      /**< "panning" */

    static const CString& GetStaticSpatializerTypeName( void );

    CPanningSpatializer( void );

    virtual ~CPanningSpatializer() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetSpatializerTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** False for a format of more than CAudioPanner::MaxChannels channels */
    virtual bool Prepare( const CAudioFormat& mixFormat, UInt32 maxBlockFrames ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT;

    virtual void Render( const Float32* mono, UInt32 frames, const SSpatialFrame& frame, Float32* output ) GUCEF_VIRTUAL_OVERRIDE GUCEF_AUDIO_REALTIME_CONTEXT;

    /** The speaker table in use with or without the centre speakers, for inspection */
    const CAudioPanner& GetPanner( bool useCenterSpeaker ) const;

    private:

    CPanningSpatializer( const CPanningSpatializer& src );              /**< not implemented */
    CPanningSpatializer& operator=( const CPanningSpatializer& src );   /**< not implemented */

    private:

    CAudioPanner m_withCenter;
    CAudioPanner m_withoutCenter;
    UInt32 m_channelCount;
    Int32 m_lfeChannel;
    bool m_prepared;
    bool m_gainsKnown;
    Float32 m_lastGains[ CAudioPanner::MaxChannels ];      /**< the gain of every channel at the end of the previous block */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CPANNINGSPATIALIZER_H ? */
