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

#ifndef GUCEF_IMAGE_CIMAGECOMPRESSOR_H
#define GUCEF_IMAGE_CIMAGECOMPRESSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_IMAGE_CIMAGEUPSCALER_H
#include "gucefIMAGE_CImageUpscaler.h"
#define GUCEF_IMAGE_CIMAGEUPSCALER_H
#endif /* GUCEF_IMAGE_CIMAGEUPSCALER_H ? */

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
 *  Compresses a picture to a GPU block format. Compressors come from plugins and are known by name to the
 *  CImageCompressorRegistry of CImageGlobal. Progress and cancellation use CIUpscaleProgress.
 */
class GUCEF_IMAGE_EXPORT_CPP CImageCompressor
{
    public:

    typedef GUCEF::vector< CORE::CString > TFormatVector;

    CImageCompressor( void );

    virtual ~CImageCompressor();

    /** The name it registers under */
    virtual CORE::CString GetName( void ) const = 0;

    /**
     *  Block format names: "BC1", "BC3", "BC4", "BC5", "BC6H", "BC7", "ETC2_RGB", "ETC2_RGBA", "EAC_R11",
     *  "EAC_RG11", "ASTC_4x4" ... "ASTC_12x12"
     */
    virtual TFormatVector GetFormats( void ) const = 0;

    /**
     *  Compresses one level
     *
     *  @param quality 0 .. 1
     *  @param blocks receives the blocks in row order
     *  @param progress told how far the work is, may stop it; NULL for none
     *  @return false when the format is not one of GetFormats(), the picture cannot be read or the work was stopped
     */
    virtual bool Compress( const CPixelMap& level           ,
                           const CORE::CString& format      ,
                           Float32 quality                  ,
                           TColourSpace colourSpace         ,
                           CORE::CDynamicBuffer& blocks     ,
                           CIUpscaleProgress* progress      ) = 0;

    private:

    CImageCompressor( const CImageCompressor& src );
    CImageCompressor& operator=( const CImageCompressor& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGECOMPRESSOR_H ? */
