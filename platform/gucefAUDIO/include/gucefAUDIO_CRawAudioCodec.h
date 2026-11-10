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

#ifndef GUCEF_AUDIO_CRAWAUDIOCODEC_H
#define GUCEF_AUDIO_CRAWAUDIOCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CIAUDIOCODEC_H
#include "gucefAUDIO_CIAudioCodec.h"
#define GUCEF_AUDIO_CIAUDIOCODEC_H
#endif /* GUCEF_AUDIO_CIAUDIOCODEC_H ? */

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
 *  Samples without any header: nothing in the data says what it is, so the format comes from the options.
 *  This is what sound data of old games and hardware dumps usually look like. The codec type is "raw".
 *
 *  Options:
 *    sampleRate     frames per second (default 44100)
 *    channels       number of channels (default 1)
 *    sampleType     uint8, int16, int24, int32, float32 or float64 (default int16)
 *    channelLayout  optional bit mask of speakers
 *    bigEndian      true when multi byte samples have the most significant byte first (default false)
 *    headerBytes    bytes to skip at the start (default 0)
 *    frames         number of frames to read, default is until the data ends
 */
class GUCEF_AUDIO_PUBLIC_CPP CRawAudioCodec : public CIAudioCodec
{
    public:

    CRawAudioCodec( void );

    CRawAudioCodec( const CRawAudioCodec& src );

    virtual ~CRawAudioCodec() GUCEF_VIRTUAL_OVERRIDE;

    /** Raw data has no metadata: always false */
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanEncode( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Writes the samples as they are in the buffer (byte order of 'bigEndian' option, default little endian) */
    virtual bool Encode( const CAudioBuffer& audio, CORE::CIOAccess& encodedOutput, const CORE::CValueList& options = CORE::CValueList() ) GUCEF_VIRTUAL_OVERRIDE;

    using CIAudioCodec::Encode;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TAudioSourcePtr& source ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CRAWAUDIOCODEC_H ? */
