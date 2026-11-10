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

#ifndef GUCEF_IMAGE_CPIXEL_H
#define GUCEF_IMAGE_CPIXEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_IMAGE_MACROS_H
#include "gucefIMAGE_macros.h"       /* module macro's */
#define GUCEF_IMAGE_MACROS_H
#endif /* GUCEF_IMAGE_MACROS_H ? */

#ifndef GUCEF_IMAGE_IMAGEDATA_H
#include "gucefIMAGE_imagedata.h"    /* C-style shared header for image related types */
#define GUCEF_IMAGE_IMAGEDATA_H
#endif /* GUCEF_IMAGE_IMAGEDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

// Forward declarations of classes used
namespace GUCEF { namespace CORE  { class CIOAccess; } }

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
 *  Helper class to deal with a single pixel
 */
class GUCEF_IMAGE_EXPORT_CPP CPixel
{
    public:

    typedef CORE::CTSharedPtr< CPixel, MT::CMutex > TPixelPtr;
    
    CPixel( void );
    
    CPixel( const void* pixelMapData                      , 
            const TPixelStorageFormat pixelStorageFormat  ,
            const TBuildinDataType pixelComponentDataType ,
            bool referenceData = false                    );

    CPixel( const CPixel& src );
    
    virtual ~CPixel();

    UInt32 GetSizeOfPixelInBytes( void ) const;

    UInt32 GetNumberOfChannels( void ) const;   
    
    TPixelStorageFormat GetPixelStorageFormat( void ) const;
    
    void* GetDataPtr( void );
    
    const void* GetDataPtr( void ) const;

    void* RelinquishPixelDataOwnership( void );

    TBuildinDataType GetPixelComponentDataType( void ) const;

    /**
     *      Check if the pixel data has an alpha channel
     *
     *      @return Wheter or not the image data has an alpha channel.
     */
    bool HasAlpha( void ) const;
    
    /**
     *      Unloads pixel data if needed and resets values to there defaults
     */
    void Clear( void );
                 
    /**
     *  Copy the pixel map data to the resource given.
     *  No meta data is added, just the pixel itself
     *
     *  @param resource the resource to which the pixel map will be written
     */
    bool CopyTo( CORE::CIOAccess& resource );
    
    CPixel& operator=( const CPixel& other );

    bool operator==( const CPixel& other ) const;
    
    bool operator<( const CPixel& other ) const;

    private:
    
    TPixelStorageFormat m_pixelStorageFormat;
    TBuildinDataType m_pixelComponentDataType;
    
    UInt8* m_pixelData;
    bool m_ownsData; 
};

/*-------------------------------------------------------------------------*/

typedef CPixel::TPixelPtr TPixelPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace IMAGE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-11-2006 :
        - Dinand: designed and implemented CPixel class

---------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CPIXEL_H ? */
