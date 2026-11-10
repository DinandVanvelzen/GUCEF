/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_VIDEO_CIVIDEOCODEC_H
#define GUCEF_VIDEO_CIVIDEOCODEC_H

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

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSOURCE_H
#include "gucefVIDEO_IVideoSource.h"
#define GUCEF_VIDEO_IVIDEOSOURCE_H
#endif /* GUCEF_VIDEO_IVIDEOSOURCE_H ? */

#ifndef GUCEF_VIDEO_IVIDEOSINK_H
#include "gucefVIDEO_IVideoSink.h"
#define GUCEF_VIDEO_IVIDEOSINK_H
#endif /* GUCEF_VIDEO_IVIDEOSINK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Turns an encoded video file format into frames and sound. Like the audio and image codecs the video codecs
 *  are registered by name (the type, usually the file extension) in the CVideoCodecRegistry; they come from
 *  codec plugins.
 *
 *  OpenSource() gives an IVideoSource that decodes while it is read, so videos of any length play without being
 *  held in memory. The 'options' are hints for the codec as name / value pairs, unknown ones are ignored.
 */
class GUCEF_VIDEO_PUBLIC_CPP CIVideoCodec : public CORE::CICodec
{
    public:

    typedef CORE::CIOAccess::IOAccessPtr    TIOAccessPtr;
    typedef GUCEF::vector< AUDIO::CAudioFormat > TAudioFormatVector;

    static const CString VideoCodecFamilyName;

    CIVideoCodec( void );

    CIVideoCodec( const CIVideoCodec& src );

    CIVideoCodec& operator=( const CIVideoCodec& src );

    virtual ~CIVideoCodec() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Starts decoding the given data. The source reads from the input while it plays, so the input must stay
     *  valid and not be used by anyone else for as long as the source exists.
     */
    bool OpenSource( CORE::CIOAccess& encodedInput, TVideoSourcePtr& source, const CORE::CValueList& options = CORE::CValueList() );

    /** Same, but the source keeps the input alive itself */
    bool OpenSource( const TIOAccessPtr& encodedInput, TVideoSourcePtr& source, const CORE::CValueList& options = CORE::CValueList() );

    /**
     *  Reads what the data says about itself without decoding the frames: "width", "height", "pixelFormat",
     *  "frameRate", "frames" and "durationInMs" when known, "audioTracks", per track "audioTrack<n>.sampleRate",
     *  "audioTrack<n>.channels" and "audioTrack<n>.sampleType", and format specific tags.
     */
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput, CORE::CValueList& metaData ) = 0;

    /** True when the codec can encode: OpenSink() works. The default is false */
    virtual bool CanEncode( void ) const;

    /**
     *  Starts encoding a video of the given size and frame rate (the pixel format of 'format' is not used: the
     *  sink takes pictures of any format) with the given audio tracks into the output. The output must be
     *  writable and seekable, and stay valid and not be used by anyone else for as long as the sink exists.
     *  Options: "quality" 0..100, "speed" 0..10, "threads", "keyFrameInterval", "audioBitrate"; codecs ignore
     *  what they do not know.
     */
    bool OpenSink( CORE::CIOAccess& encodedOutput, const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, TVideoSinkPtr& sink, const CORE::CValueList& options = CORE::CValueList() );

    /** Same, but the sink keeps the output alive itself */
    bool OpenSink( const TIOAccessPtr& encodedOutput, const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, TVideoSinkPtr& sink, const CORE::CValueList& options = CORE::CValueList() );

    /** Generic codec interface: not used by video codecs, a video has no plain byte form. Use OpenSource() */
    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE;

    /** Generic codec interface: not used by video codecs, they only decode */
    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  What a codec implements to open a source. 'owner' is set when the source has to keep the input alive,
     *  'input' is the access to read from in both cases.
     */
    virtual bool OpenSourceImpl( CORE::CIOAccess* input, const TIOAccessPtr& owner, const CORE::CValueList& options, TVideoSourcePtr& source ) = 0;

    /** What an encoding codec implements to open a sink, as OpenSourceImpl() for the output. The default fails */
    virtual bool OpenSinkImpl( CORE::CIOAccess* output, const TIOAccessPtr& owner, const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, const CORE::CValueList& options, TVideoSinkPtr& sink );
};

typedef CORE::CTSharedPtr< CIVideoCodec, MT::CMutex > TVideoCodecPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CIVIDEOCODEC_H ? */
