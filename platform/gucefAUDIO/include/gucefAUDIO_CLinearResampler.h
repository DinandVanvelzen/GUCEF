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

#ifndef GUCEF_AUDIO_CLINEARRESAMPLER_H
#define GUCEF_AUDIO_CLINEARRESAMPLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_IAUDIORESAMPLER_H
#include "gucefAUDIO_IAudioResampler.h"
#define GUCEF_AUDIO_IAUDIORESAMPLER_H
#endif /* GUCEF_AUDIO_IAUDIORESAMPLER_H ? */

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
 *  The default resampler: linear interpolation between neighbouring frames. Cheap, stable and exact for the
 *  rate ratio 1, which is right for sound effects and for moderate pitch changes. It does not filter, so large
 *  downsampling of material with high frequencies aliases; a better resampler can replace it through the interface.
 */
class GUCEF_AUDIO_PUBLIC_CPP CLinearResampler : public IAudioResampler
{
    public:

    CLinearResampler( void );

    virtual ~CLinearResampler() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Configure( UInt32 channelCount, Float64 inputRate, Float64 outputRate ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetRates( Float64 inputRate, Float64 outputRate ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Reset( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Process( const Float32* input          ,
                          UInt32 inputFrames            ,
                          UInt32& inputFramesConsumed   ,
                          Float32* output               ,
                          UInt32 outputFrameCapacity    ,
                          UInt32& outputFramesProduced  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual IAudioResampler* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CLinearResampler( const CLinearResampler& src );            /**< not implemented, use Clone() */
    CLinearResampler& operator=( const CLinearResampler& src ); /**< not implemented */

    private:

    UInt32 m_channelCount;
    Float64 m_step;                     /**< input frames per output frame */
    Float64 m_position;                 /**< position of the next output, in input frames relative to the start of the next chunk, -1 is the last frame of the previous chunk */
    GUCEF::vector< Float32 > m_previous;    /**< the last frame of the previous chunk, one value per channel */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CLINEARRESAMPLER_H ? */
