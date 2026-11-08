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

#ifndef GUCEF_AUDIO_CIAUDIOCODEC_H
#define GUCEF_AUDIO_CIAUDIOCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICODEC_H
#include "CICodec.h"
#define GUCEF_CORE_CICODEC_H
#endif /* GUCEF_CORE_CICODEC_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#include "gucefAUDIO_CAudioBuffer.h"
#define GUCEF_AUDIO_CAUDIOBUFFER_H
#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#include "gucefAUDIO_IAudioSource.h"
#define GUCEF_AUDIO_IAUDIOSOURCE_H
#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */

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
 *  Turns an encoded audio file format (wav, ogg, flac, ...) into audio and back. Like the image codecs the audio
 *  codecs are registered by name (the type, usually the file extension) in the CAudioCodecRegistry. Codecs
 *  that ship with the library are built in, others come from codec plugins.
 *
 *  A codec offers streaming: OpenSource() gives an IAudioSource that decodes while it is read, so music of
 *  any length plays without being held in memory. DecodeAll() is the convenience for short sounds.
 *
 *  The 'options' are hints for the codec as name / value pairs. What they mean is up to the codec (the raw
 *  codec needs rate, channels and sample type, a lossy codec might take a quality), unknown ones are ignored.
 */
class GUCEF_AUDIO_PUBLIC_CPP CIAudioCodec : public CORE::CICodec
{
    public:

    typedef CORE::CIOAccess::IOAccessPtr    TIOAccessPtr;

    static const CString AudioCodecFamilyName;

    CIAudioCodec( void );

    CIAudioCodec( const CIAudioCodec& src );

    CIAudioCodec& operator=( const CIAudioCodec& src );

    virtual ~CIAudioCodec() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Starts decoding the given data. The source reads from the input while it plays, so the input must stay
     *  valid and not be used by anyone else for as long as the source exists.
     */
    bool OpenSource( CORE::CIOAccess& encodedInput, TAudioSourcePtr& source, const CORE::CValueList& options = CORE::CValueList() );

    /** Same, but the source keeps the input alive itself */
    bool OpenSource( const TIOAccessPtr& encodedInput, TAudioSourcePtr& source, const CORE::CValueList& options = CORE::CValueList() );

    /** Decodes everything into one buffer */
    bool DecodeAll( CORE::CIOAccess& encodedInput, CAudioBuffer& audio, const CORE::CValueList& options = CORE::CValueList() );

    /**
     *  Reads what the data says about itself without decoding the audio: "sampleRate", "channels", "sampleType",
     *  "channelLayout", "frames" and "durationInMs" when known, and format specific tags.
     */
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) = 0;

    /**
     *  Content sniffing: tells how well the data at the current position of the input looks like the format of
     *  this codec, 'confidence' from 0 (not at all) to 100 (certain). The input position is the same afterwards.
     *  Returns false when the codec cannot tell, which is what a codec without this ability does. Cheap: reads a
     *  header, not the whole input. See CAudioCodecRegistry::TryFindCodecForData().
     */
    virtual bool Probe( CORE::CIOAccess& encodedInput, UInt32& confidence );

    /** True when the codec can write its format */
    virtual bool CanEncode( void ) const;

    /** Writes the audio in the format of the codec. False when it cannot (see CanEncode) or when the audio does not fit the format */
    virtual bool Encode( const CAudioBuffer& audio, CORE::CIOAccess& encodedOutput, const CORE::CValueList& options = CORE::CValueList() );

    /** Generic codec interface: writes the decoded audio as plain interleaved samples in the format that DecodeMetaData() describes */
    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE;

    /** Generic codec interface: not used by audio codecs, there is no plain byte form of audio to start from. Use Encode( CAudioBuffer, ... ) */
    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  What a codec implements to open a source. 'owner' is set when the source has to keep the input alive,
     *  'input' is the access to read from in both cases.
     */
    virtual bool OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TAudioSourcePtr& source ) = 0;
};

typedef CORE::CTSharedPtr< CIAudioCodec, MT::CMutex > TAudioCodecPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CIAUDIOCODEC_H ? */
