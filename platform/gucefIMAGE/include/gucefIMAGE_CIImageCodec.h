/*
 *  gucefIMAGE: GUCEF module providing image utilities
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_IMAGE_CIIMAGECODEC_H
#define GUCEF_IMAGE_CIIMAGECODEC_H

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

#ifndef GUCEFIMAGE_MACROS_H
#include "gucefIMAGE_macros.h"       /* module macro's */
#define GUCEFIMAGE_MACROS_H
#endif /* GUCEFIMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace CORE { class CIOAccess; } }

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

class CImage;

/*-------------------------------------------------------------------------*/

/**
 *  Interface class providing additional functionality expanding upon
 *  the basic codec functionality
 */
class GUCEF_IMAGE_EXPORT_CPP CIImageCodec : public CORE::CICodec
{
    public:

    CIImageCodec( void );

    CIImageCodec( const CIImageCodec& src );

    virtual ~CIImageCodec();

    CIImageCodec& operator=( const CIImageCodec& src );

    /** What a codec can hold of an image, see GetImageCapabilities() */
    enum EImageCapability
    {
        IMAGECAPABILITY_MULTIPLE_FRAMES = 0x01,     /**< more than one frame: an animation or a set of pictures */
        IMAGECAPABILITY_MIP_LEVELS      = 0x02      /**< the mipmap levels of a frame */
    };

    /**
     *  What the format of the codec can hold of an image beyond one picture: IMAGECAPABILITY_* combined.
     *  The default: nothing, one picture per file
     */
    virtual UInt32 GetImageCapabilities( void ) const;

    virtual bool Encode( const CImage& inputImage       ,
                         CORE::CIOAccess& encodedOutput ) = 0;

    /**
     *  Encodes with parameters of the writer (a quality, a compression codec). Parameters the codec does not know are
     *  ignored. The default ignores all of them and runs the version without parameters.
     */
    virtual bool Encode( const CImage& inputImage       ,
                         CORE::CIOAccess& encodedOutput ,
                         const CORE::CValueList& params );

    virtual bool Decode( CORE::CIOAccess& encodedInput ,
                         CImage& outputImage           ) = 0;

    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput ,
                                 CORE::CValueList& metaData    ) = 0;

};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIImageCodec, MT::CMutex > TImageCodecPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGECODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 20-07-2005 :
        - Dinand: Added this class

-----------------------------------------------------------------------------*/
