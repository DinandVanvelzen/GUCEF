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

#ifndef GUCEF_IMAGE_CIMAGEUPSCALER_H
#define GUCEF_IMAGE_CIMAGEUPSCALER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

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

/** Told how far an upscale is; it can stop the work */
class GUCEF_IMAGE_EXPORT_CPP CIUpscaleProgress
{
    public:

    virtual ~CIUpscaleProgress();

    /** 'fraction' is 0 .. 1; returning false stops the upscale */
    virtual bool OnUpscaleProgress( Float32 fraction ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Makes a picture a whole number of times larger with more detail than repeating its pixels: a scaler for
 *  pixel art, a neural network, ... Upscalers come from plugins (and gucefIMAGE has "mmpx" built in) and are
 *  known by name to the CImageUpscalerRegistry of CImageGlobal.
 */
class GUCEF_IMAGE_EXPORT_CPP CImageUpscaler
{
    public:

    typedef GUCEF::vector< UInt32 > TFactorVector;

    CImageUpscaler( void );

    virtual ~CImageUpscaler();

    /** The name it registers under, like "mmpx" */
    virtual CORE::CString GetName( void ) const = 0;

    /** The factors it upscales by, smallest first */
    virtual TFactorVector GetFactors( void ) const = 0;

    /**
     *  Upscales an RGBA picture (other formats are converted) 'factor' times
     *
     *  @param progress told how far the work is, may stop it; NULL for none
     *  @return false when the factor is not one of GetFactors(), the picture cannot be read or the work was stopped
     */
    virtual bool Upscale( const CPixelMap& source, UInt32 factor, TPixelMapPtr& result, CIUpscaleProgress* progress ) = 0;

    /**
     *  Upscale() on a thread of a thread pool, for work that takes long such as a neural network. The upscaler is
     *  looked up by its name in the registry of CImageGlobal when the task runs, so it has to be registered. The
     *  source is shared, not copied: it must not change until the task is done. RequestCancellation() on the
     *  future stops the work; once the future is ready GetAsyncResult() has the result.
     *
     *  @param threadPoolName the thread pool to run on, the default pool when empty
     */
    CORE::CFutureResult UpscaleAsync( const TPixelMapPtr& source, UInt32 factor, const CORE::CString& threadPoolName = CORE::CString::Empty ) const;

    /** The result of an UpscaleAsync() whose future is ready; false when it failed or was cancelled */
    static bool GetAsyncResult( const CORE::CFutureResult& future, TPixelMapPtr& result );

    /** The smallest factor of at least 'wanted', the largest factor when none is that large, 0 when it has none */
    UInt32 GetFactorOfAtLeast( UInt32 wanted ) const;

    private:

    CImageUpscaler( const CImageUpscaler& src );
    CImageUpscaler& operator=( const CImageUpscaler& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGEUPSCALER_H ? */
