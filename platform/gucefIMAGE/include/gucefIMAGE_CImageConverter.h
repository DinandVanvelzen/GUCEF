/*
 *  gucefIMAGE: GUCEF module providing image utilities
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
#ifndef GUCEF_IMAGE_CIMAGECONVERTER_H
#define GUCEF_IMAGE_CIMAGECONVERTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H
#include "gucefIMAGE_CImageConversionSettings.h"
#define GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H
#endif /* GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H ? */

#ifndef GUCEF_IMAGE_CIMAGECONVERSIONRESULT_H
#include "gucefIMAGE_CImageConversionResult.h"
#define GUCEF_IMAGE_CIMAGECONVERSIONRESULT_H
#endif /* GUCEF_IMAGE_CIMAGECONVERSIONRESULT_H ? */

#ifndef GUCEFIMAGE_MACROS_H
#include "gucefIMAGE_macros.h"       /* module macro's */
#define GUCEFIMAGE_MACROS_H
#endif /* GUCEFIMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Converts an image from any image format that can be read to any image format that can be written, through the
 *  codecs that are loaded (CImageCodecCapabilities says which). The work is on streams, so files, the VFS and memory
 *  are all possible sources and destinations. Formats are named by type: the file extension.
 *
 *  What the converter takes care of:
 *   - the frames: all, or one; an image of more than one frame to a format that holds one picture is refused by
 *     Convert() and handed out frame by frame by ConvertToFrames()
 *   - the mipmap levels: only kept when the target format holds them
 *   - the pixel format: the image is offered to the writer as it is, and when the writer cannot take it as RGBA8 and then
 *     as RGB8. What is changed and what that loses is told in the result. Nothing is written when the conversion fails
 *   - the meta-data of the image is carried to formats that keep it
 *
 *  The converter has no state: a conversion only uses its arguments, conversions may run on several threads.
 */
class GUCEF_IMAGE_EXPORT_CPP CImageConverter
{
    public:

    /** One frame of an image as a file of the target format */
    struct SEncodedFrame
    {
        UInt32 frameIndex;              /**< the frame in the image that was read */
        CORE::CDynamicBuffer data;      /**< the frame as a file of the target format */

        SEncodedFrame( void ) : frameIndex( 0 ), data() {}
    };
    typedef GUCEF::vector< SEncodedFrame > TEncodedFrameVector;

    CImageConverter( void );

    CImageConverter( const CImageConverter& src );

    ~CImageConverter();

    CImageConverter& operator=( const CImageConverter& src );

    /**
     *  Reads the image of the input from its current position and writes it as a single file of the target format
     *  to the output. Nothing is written to the output unless it all worked: see the result for the reason when not.
     */
    bool Convert( CORE::CIOAccess& input                     ,
                  const CORE::CString& fromType              ,
                  CORE::CIOAccess& output                    ,
                  const CORE::CString& toType                ,
                  const CImageConversionSettings& settings   ,
                  CImageConversionResult& result             ) const;

    /**
     *  As Convert(), with one file of the target format per frame in 'frames', for the formats that hold one picture
     *  and for animations that are to become pictures. Works with every target format
     */
    bool ConvertToFrames( CORE::CIOAccess& input                     ,
                          const CORE::CString& fromType              ,
                          const CORE::CString& toType                ,
                          const CImageConversionSettings& settings   ,
                          CImageConversionResult& result             ,
                          TEncodedFrameVector& frames                ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGECONVERTER_H ? */
