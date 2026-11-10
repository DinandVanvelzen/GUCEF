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

#ifndef GUCEF_AUDIO_CWAVAUDIOCODEC_H
#define GUCEF_AUDIO_CWAVAUDIOCODEC_H

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
 *  WAVE files (RIFF) with plain samples: 8, 16, 24 and 32 bit integer and 32 and 64 bit float, any number
 *  of channels, also in the extensible form that carries the speaker layout. Compressed WAVE variants
 *  (ADPCM, A-law, mu-law, ...) are not handled here, a codec plugin can take those on.
 *  Reading is streaming and seekable when the input is, writing makes the form every player understands.
 *  This is the codec type "wav".
 */
class GUCEF_AUDIO_PUBLIC_CPP CWavAudioCodec : public CIAudioCodec
{
    public:

    /** What the header of a WAVE file says */
    struct SWavInfo
    {
        CAudioFormat format;
        UInt64 dataOffset;              /**< where the first frame is */
        UInt64 frameCount;              /**< GUCEF_AUDIO_LENGTH_UNKNOWN when the file does not tell */
    };

    CWavAudioCodec( void );

    CWavAudioCodec( const CWavAudioCodec& src );

    virtual ~CWavAudioCodec() GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the header and leaves the input at the start of the audio. False if this is not a WAVE file this codec can read */
    static bool ParseHeader( CORE::CIOAccess& input, SWavInfo& info );

    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanEncode( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Encode( const CAudioBuffer& audio, CORE::CIOAccess& encodedOutput, const CORE::CValueList& options = CORE::CValueList() ) GUCEF_VIRTUAL_OVERRIDE;

    using CIAudioCodec::Encode;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see CORE::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );

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

#endif /* GUCEF_AUDIO_CWAVAUDIOCODEC_H ? */
