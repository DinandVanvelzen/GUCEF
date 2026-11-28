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
#ifndef GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H
#define GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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

/** What CImageConverter is asked to do besides changing the format */
class GUCEF_IMAGE_EXPORT_CPP CImageConversionSettings
{
    public:

    CImageConversionSettings( void );

    CImageConversionSettings( const CImageConversionSettings& src );

    ~CImageConversionSettings();

    CImageConversionSettings& operator=( const CImageConversionSettings& src );

    /** Converts only this frame of the image. The default is all frames */
    void SetFrameIndex( UInt32 frameIndex );

    void SetAllFrames( void );

    bool IsAllFrames( void ) const;

    UInt32 GetFrameIndex( void ) const;

    /**
     *  Whether the converter may change the pixel format to one the target format can hold when it cannot hold the
     *  one of the image, even when that loses information (a 16 bit channel to 8 bit, an alpha channel dropped).
     *  The loss is always reported in the result. The default is true
     */
    void SetAllowLossyConversion( bool allowLossyConversion );

    bool GetAllowLossyConversion( void ) const;

    /** Parameters for the writer of the target format, such as the compression of the native format. Writers ignore what they do not know */
    CORE::CValueList& GetEncoderParams( void );

    const CORE::CValueList& GetEncoderParams( void ) const;

    private:

    bool m_allFrames;
    UInt32 m_frameIndex;
    bool m_allowLossyConversion;
    CORE::CValueList m_encoderParams;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGECONVERSIONSETTINGS_H ? */
