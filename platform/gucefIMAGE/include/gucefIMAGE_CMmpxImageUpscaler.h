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

#ifndef GUCEF_IMAGE_CMMPXIMAGEUPSCALER_H
#define GUCEF_IMAGE_CMMPXIMAGEUPSCALER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  Image upscaler "mmpx": MMPX pixel art magnification by Morgan McGuire and Mara Gagiu
 *  (https://casual-effects.com/research/McGuire2021PixelArt/, MIT license) on the CPU. It doubles a picture,
 *  rounding corners and filling in slopes of lines while keeping sharp right angles; the result only has
 *  colours of the picture. 4 times is doubling twice.
 */
class GUCEF_IMAGE_EXPORT_CPP CMmpxImageUpscaler : public CImageUpscaler
{
    public:

    CMmpxImageUpscaler( void );

    virtual ~CMmpxImageUpscaler();

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TFactorVector GetFactors( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Upscale( const CPixelMap& source, UInt32 factor, TPixelMapPtr& result, CIUpscaleProgress* progress ) GUCEF_VIRTUAL_OVERRIDE;

    /** Doubles an RGBA picture with 8 bits per channel; progress goes from 'progressFrom' to 'progressTo' */
    static bool Double( const CPixelMap& source, TPixelMapPtr& result, CIUpscaleProgress* progress, Float32 progressFrom, Float32 progressTo );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CMMPXIMAGEUPSCALER_H ? */
