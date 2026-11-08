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

#ifndef GUCEF_AUDIO_CAUDIOCLIP_H
#define GUCEF_AUDIO_CAUDIOCLIP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#include "gucefAUDIO_CAudioBuffer.h"
#define GUCEF_AUDIO_CAUDIOBUFFER_H
#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

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
 *  A short sound held in memory, fully decoded: the usual form for effects. One clip is shared by any number
 *  of sounds playing at once, each with its own source (and so its own position) made with CreateSource().
 *  The audio of a clip never changes after it is made, so sharing needs no locking.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioClip
{
    public:

    typedef CORE::CTSharedPtr< CAudioClip, MT::CMutex > TAudioClipPtr;

    CAudioClip( void );

    /** A clip with a copy of the audio */
    explicit CAudioClip( const CAudioBuffer& audio, const CString& name = CString() );

    /** A clip sharing the buffer, which must not be changed afterwards */
    explicit CAudioClip( const TAudioBufferPtr& audio, const CString& name = CString() );

    CAudioClip( const CAudioClip& src );

    CAudioClip& operator=( const CAudioClip& src );

    ~CAudioClip();

    const CAudioFormat& GetFormat( void ) const;

    UInt32 GetFrameCount( void ) const;

    Float64 GetDurationInSeconds( void ) const;

    bool IsEmpty( void ) const;

    const CAudioBuffer& GetBuffer( void ) const;

    const TAudioBufferPtr& GetSharedBuffer( void ) const;

    const CString& GetName( void ) const;

    void SetName( const CString& name );

    /**
     *  Decodes a whole file into a clip with the codec of that type name ("wav", ...), looked up in the audio codec registry.
     *  The options are those of the codec. Returns false when there is no such codec or the data cannot be decoded.
     */
    static bool Decode( const CString& codecType, CORE::CIOAccess& encodedInput, TAudioClipPtr& clip, const CORE::CValueList& options = CORE::CValueList(), const CString& name = CString() );

    /** A new source that plays the clip from the start. Many sources of one clip can exist at once */
    TAudioSourcePtr CreateSource( void ) const;

    private:

    TAudioBufferPtr m_audio;
    CString m_name;
};

typedef CAudioClip::TAudioClipPtr TAudioClipPtr;

/*-------------------------------------------------------------------------*/

/** Reads the audio of a clip: nothing is copied until it is asked for */
class GUCEF_AUDIO_PUBLIC_CPP CAudioClipSource : public IAudioSource
{
    public:

    explicit CAudioClipSource( const TAudioBufferPtr& audio );

    virtual ~CAudioClipSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 ReadFrames( void* destination, UInt32 frameCapacity ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsEndOfStream( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsRealtimeSafe( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSeekable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SeekToFrame( UInt64 frame ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetPositionInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetLengthInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAudioClipSource( const CAudioClipSource& src );            /**< not implemented */
    CAudioClipSource& operator=( const CAudioClipSource& src ); /**< not implemented */

    private:

    TAudioBufferPtr m_audio;
    const CAudioBuffer* m_buffer;       /**< the buffer for the audio thread: reaching it through the shared pointer is not real time safe */
    UInt32 m_position;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOCLIP_H ? */
