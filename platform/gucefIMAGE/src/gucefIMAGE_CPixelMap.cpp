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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h> /* <- for memset() */
#include <assert.h>
#include <math.h>

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_LOGGINGMACROS_H
#include "gucefCORE_LoggingMacros.h"
#define GUCEF_CORE_LOGGINGMACROS_H
#endif /* GUCEF_CORE_LOGGINGMACROS_H ? */

#ifndef GUCEF_CORE_CMFILEACCESS_H
#include "CMFileAccess.h"
#define GUCEF_CORE_CMFILEACCESS_H
#endif /* GUCEF_CORE_CMFILEACCESS_H ? */

#include "gucefIMAGE_CPixelMap.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPixelMap::CPixelMap( const TImageMipMapLevel& mipmapLevel )
    : m_pixelComponentDataType( (TBuildinDataType) mipmapLevel.mipLevelInfo.pixelComponentDataType ) ,
      m_pixelStorageFormat( (TPixelStorageFormat) mipmapLevel.mipLevelInfo.pixelStorageFormat )      ,
      m_heightInPixels( mipmapLevel.mipLevelInfo.frameHeight )                                       ,
      m_widthInPixels( mipmapLevel.mipLevelInfo.frameWidth )                                         ,
      m_isNormalizedAsPercentage( false )                                                            ,
      m_pixelMapData( NULL )
{GUCEF_TRACE;

    m_pixelMapData = new UInt8[ mipmapLevel.pixelDataSizeInBytes ];
    memcpy( m_pixelMapData, mipmapLevel.pixelData, mipmapLevel.pixelDataSizeInBytes );
}

/*------------------------------------------------------------------------*/

CPixelMap::CPixelMap( const void* pixelMapData                      , 
                      const UInt32 widthInPixels                    ,
                      const UInt32 heightInPixels                   ,
                      const TPixelStorageFormat pixelStorageFormat  ,
                      const TBuildinDataType pixelComponentDataType )
    : m_pixelComponentDataType( pixelComponentDataType ) ,
      m_pixelStorageFormat( pixelStorageFormat )         ,
      m_heightInPixels( 0 )                              ,
      m_widthInPixels( 0 )                               ,
      m_isNormalizedAsPercentage( false )                ,
      m_pixelMapData( NULL )
{GUCEF_TRACE;

    Assign( pixelMapData           ,
            widthInPixels          ,
            heightInPixels         ,
            pixelStorageFormat     ,
            pixelComponentDataType );
}

/*--------------------------------------------------------------------------*/

CPixelMap::CPixelMap( const CPixelMap& src )
    : m_pixelComponentDataType( src.m_pixelComponentDataType ) ,
      m_pixelStorageFormat( src.m_pixelStorageFormat )         ,
      m_heightInPixels( 0 )                                    ,
      m_widthInPixels( 0 )                                     ,
      m_isNormalizedAsPercentage( src.m_isNormalizedAsPercentage ) ,
      m_pixelMapData( NULL )
{GUCEF_TRACE;

    Assign( src.m_pixelMapData           ,
            src.m_widthInPixels          ,
            src.m_heightInPixels         ,
            src.m_pixelStorageFormat     ,
            src.m_pixelComponentDataType );    
}

/*--------------------------------------------------------------------------*/

CPixelMap::~CPixelMap()
{GUCEF_TRACE;

    Clear();
}

/*--------------------------------------------------------------------------*/

CPixelMap&
CPixelMap::operator=( const CPixelMap& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Assign( src.m_pixelMapData           ,
                src.m_widthInPixels          ,
                src.m_heightInPixels         ,
                src.m_pixelStorageFormat     ,
                src.m_pixelComponentDataType );
    }
    
    return *this;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::IsNormalizedAsPercentage( void ) const
{GUCEF_TRACE;
    
    return m_isNormalizedAsPercentage;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::SetIsNormalizedAsPercentage( bool isNormalizedAsPercentage )
{GUCEF_TRACE;
    
    if ( m_pixelComponentDataType == DATATYPE_LE_FLOAT32 ||
         m_pixelComponentDataType == DATATYPE_LE_FLOAT64  )
    {
        m_isNormalizedAsPercentage = isNormalizedAsPercentage;
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::NormalizeAsPercentage( void )
{GUCEF_TRACE;

    if ( m_isNormalizedAsPercentage )
        return true;
    
    // First we convert the pixel component format to Float64
    // This will give us the precision we want for normalization except when int64 was used
    // but that is such an edge case we dont worry about it right now
    if ( ConvertFormatTo( DATATYPE_LE_FLOAT64 ) )
    {
        Float64* pixelMapData = (Float64*) m_pixelMapData;
        Float64 percentage = std::numeric_limits< Float64 >::max() / 100.0;
        UInt64 valueCount = GetTotalNumberOfChannelComponentValues();

        for ( UInt64 i=0; i<valueCount; ++i )
        {
            pixelMapData[ i ] /= percentage;
        }

        m_isNormalizedAsPercentage = true;
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::NormalizeAsPercentage( TPixelMapPtr& resultImage ) const
{GUCEF_TRACE;

    TPixelMapPtr conversionImage( new CPixelMap( *this ) );
    if ( conversionImage->NormalizeAsPercentage() )
    {
        resultImage = conversionImage;
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

template < typename T >
bool
CPixelMap::ApplyContrastImp( Float32 adjustmentPercentage )
{GUCEF_TRACE;
   
    // adjustment of 100% is max of T
    // adjustment of -100% is negative max of T

    if ( adjustmentPercentage > 100.0 )
        adjustmentPercentage = 100.0;
    else
    if ( adjustmentPercentage < -100.0 )
        adjustmentPercentage = -100.0;

    T theMax = std::numeric_limits< T >::max();
    Float64 adjustedMax = (Float64) theMax;
    bool tIsFloat = m_pixelComponentDataType == DATATYPE_LE_FLOAT32 || m_pixelComponentDataType == DATATYPE_LE_FLOAT64;
    if ( tIsFloat && m_isNormalizedAsPercentage )
    {
        adjustedMax = 1.0;
    }
    Float64 adjustmentPerc = adjustmentPercentage;

    // color correction factor should be in range 0-1 for a reduction in contrast with -100% being 0
    // 1-2 is an increase with 2 being 100%
    Float64 c = 0.0;
    if ( adjustmentPerc < 0 )
    {
        c = 1.0 - ( adjustmentPerc / -100.0 );       
    }
    else
    {
        c = 1.0 + ( adjustmentPerc / 100.0 );
    }
    
    Float64 halfMax = 0.5 * theMax;
    T* pixelMapData = (T*) m_pixelMapData;
    UInt64 vMax = GetTotalNumberOfChannelComponentValues();
    for ( UInt64 i=0; i<vMax; ++i )
    {
        T value = pixelMapData[ i ];
        Float64 newValue = ( c * ( value - halfMax ) ) + halfMax;
        if ( !tIsFloat )
            newValue = round( newValue );
        if ( newValue > adjustedMax )
            newValue = adjustedMax;
        pixelMapData[ i ] = (T) newValue;
    }

    return true;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyContrast( Float32 adjustmentPercentage )
{GUCEF_TRACE;

    switch ( m_pixelComponentDataType )
    {
        case DATATYPE_LE_FLOAT32:
        {
            return ApplyContrastImp< CORE::Float32 >( adjustmentPercentage );
        }
        case DATATYPE_LE_FLOAT64:
        {
            return ApplyContrastImp< CORE::Float64 >( adjustmentPercentage );
        }
        case DATATYPE_UINT8:
        {
            return ApplyContrastImp< CORE::UInt8 >( adjustmentPercentage );
        }
        case DATATYPE_INT8:
        {
            return ApplyContrastImp< CORE::Int8 >( adjustmentPercentage );
        }
        case DATATYPE_LE_UINT16:
        {
            return ApplyContrastImp< CORE::UInt16 >( adjustmentPercentage );
        }
        case DATATYPE_LE_INT16:
        {
            return ApplyContrastImp< CORE::Int16 >( adjustmentPercentage );
        }
        case DATATYPE_LE_UINT32:
        {
            return ApplyContrastImp< CORE::UInt32 >( adjustmentPercentage );
        }
        case DATATYPE_LE_INT32:
        {
            return ApplyContrastImp< CORE::Int32 >( adjustmentPercentage );
        }
        case DATATYPE_LE_UINT64:
        {
            return ApplyContrastImp< CORE::UInt64 >( adjustmentPercentage );
        }
        case DATATYPE_LE_INT64:
        {
            return ApplyContrastImp< CORE::Int64 >( adjustmentPercentage );
        }
        default:
        {
            // this should not happen
            return false;
        }
    }     
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyContrast( Float32 adjustmentPercentage, TPixelMapPtr& resultImage ) const
{GUCEF_TRACE;

    TPixelMapPtr conversionImage( new CPixelMap( *this ) );    
    if ( conversionImage->ApplyContrast( adjustmentPercentage ) )
    {
        resultImage = conversionImage;
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyBrightness( Float32 adjustmentPercentage, TPixelMapPtr& resultImage ) const
{
    TPixelMapPtr conversionImage( new CPixelMap( *this ) );    
    if ( conversionImage->ApplyBrightness( adjustmentPercentage ) )
    {
        resultImage = conversionImage;
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyBrightness( Float32 adjustmentPercentage )
{GUCEF_TRACE;


    switch ( m_pixelComponentDataType )
    {
        case DATATYPE_LE_FLOAT32:
        {
            return ApplyBrightnessImp< CORE::Float32 >( adjustmentPercentage );
        }
        case DATATYPE_LE_FLOAT64:
        {
            return ApplyBrightnessImp< CORE::Float64 >( adjustmentPercentage );
        }
        case DATATYPE_UINT8:
        {
            return ApplyBrightnessImp< CORE::UInt8 >( adjustmentPercentage );
        }
        case DATATYPE_INT8:
        {
            return ApplyBrightnessImp< CORE::Int8 >( adjustmentPercentage );
        }
        case DATATYPE_LE_UINT16:
        {
            return ApplyBrightnessImp< CORE::UInt16 >( adjustmentPercentage );
        }
        case DATATYPE_LE_INT16:
        {
            return ApplyBrightnessImp< CORE::Int16 >( adjustmentPercentage );
        }
        case DATATYPE_LE_UINT32:
        {
            return ApplyBrightnessImp< CORE::UInt32 >( adjustmentPercentage );
        }
        case DATATYPE_LE_INT32:
        {
            return ApplyBrightnessImp< CORE::Int32 >( adjustmentPercentage );
        }
        case DATATYPE_LE_UINT64:
        {
            return ApplyBrightnessImp< CORE::UInt64 >( adjustmentPercentage );
        }
        case DATATYPE_LE_INT64:
        {
            return ApplyBrightnessImp< CORE::Int64 >( adjustmentPercentage );
        }
        default:
        {
            // this should not happen
            return false;
        }
    }
}

/*--------------------------------------------------------------------------*/

template < typename T >
bool
CPixelMap::ApplyBrightnessImp( Float32 adjustmentPercentage )
{GUCEF_TRACE;

    //if ( adjustmentPercentage > 100.0 )
    //    adjustmentPercentage = 100.0;
    //else
    //if ( adjustmentPercentage < -100.0 )
    //    adjustmentPercentage = -100.0;

    Float64 adjustmentFactor = adjustmentPercentage / 100;
    if ( adjustmentPercentage > 0 )
    {
        adjustmentFactor += 1; // Increase in brightness 1.x factor
    }
    else
    {
        adjustmentFactor *= -1; // Decrease in brightness 0.x factor
    }                 
    
    T theMax = std::numeric_limits< T >::max();
    Float64 adjustedMax = (Float64) theMax;
    bool tIsFloat = m_pixelComponentDataType == DATATYPE_LE_FLOAT32 || m_pixelComponentDataType == DATATYPE_LE_FLOAT64;
    if ( tIsFloat && m_isNormalizedAsPercentage )
    {
        adjustedMax = 1.0;
    }

    //Float64 brightnessAdjustment = adjustmentPercentage * ( adjustedMax / 100.0 );

    T* pixelMapData = (T*) m_pixelMapData;
    UInt64 vMax = GetTotalNumberOfChannelComponentValues();

    for ( UInt64 i=0; i<vMax; ++i )
    {
        T value = pixelMapData[ i ];
        
        //Float64 newValue = value + brightnessAdjustment;
        Float64 newValue = adjustmentFactor * value;

        if ( !tIsFloat )
            newValue = round( newValue );
        if ( newValue > adjustedMax )
            newValue = adjustedMax;
        else
        if ( newValue < 0.0 )
            newValue = 0.0;

        pixelMapData[ i ] = (T) newValue;
    }

    return true;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyGamma( Float32 gamma, TPixelMapPtr& resultImage ) const
{
    // Allocate a duplicate pixelmap
    resultImage = TPixelMapPtr( new CPixelMap( *this ) );
    
    // Apply the gamma to the duplicate
    return resultImage->ApplyGamma( gamma );
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyGamma( Float32 gamma )
{GUCEF_TRACE;

    UInt32 pixelCount = GetPixelCount();
    UInt32 valueCount = pixelCount * GetNumberOfChannelsPerPixel();

    Float32 gammaPow = 1 / gamma;
    switch ( m_pixelComponentDataType )
    {
        case DATATYPE_LE_FLOAT32:
        {
            for ( UInt32 n=0; n<valueCount; ++n )
            {
                Float32* value = (Float32*) ( m_pixelMapData + ( sizeof(Float32) * n ) );
                *value = std::numeric_limits< Float32 >::max() * (Float32) pow( *value / std::numeric_limits< Float32 >::max(), gammaPow );
            }
            break;
        }                            
        case DATATYPE_LE_FLOAT64:
        {
            for (UInt32 n = 0; n < valueCount; ++n)
            {
                Float64* value = (Float64*)(m_pixelMapData + (sizeof(Float64) * n));
                *value = std::numeric_limits< Float64 >::max() * pow( *value / std::numeric_limits< Float64 >::max(), gammaPow );
            }
            break;
        }
        case DATATYPE_UINT8:
        {
            for ( UInt32 n=0; n<valueCount; ++n )
            {
                UInt8* value = (UInt8*) ( m_pixelMapData + ( sizeof(UInt8) * n ) );
                *value = (UInt8) ( std::numeric_limits< UInt8 >::max() * pow( *value / std::numeric_limits< UInt8 >::max(), gammaPow ) );
            }
            break;        
        }
        case DATATYPE_INT8:
        {
            for ( UInt32 n=0; n<valueCount; ++n )
            {
                Int8* value = (Int8*) ( m_pixelMapData + ( sizeof(Int8) * n ) );
                *value = (Int8) ( std::numeric_limits< Int8 >::max() * pow( *value / std::numeric_limits< Int8 >::max(), gammaPow ) );
            }
            break;        
        }
        case DATATYPE_LE_UINT16:
        {
            for ( UInt32 n=0; n<valueCount; ++n )
            {
                UInt16* value = (UInt16*) ( m_pixelMapData + ( sizeof(UInt16) * n ) );
                *value = (UInt16) ( std::numeric_limits< UInt16 >::max() * pow( *value / std::numeric_limits< UInt16 >::max(), gammaPow ) );
            }
            break;        
        }
        case DATATYPE_LE_INT16:
        {
            for ( UInt32 n=0; n<valueCount; ++n )
            {
                Int16* value = (Int16*) ( m_pixelMapData + ( sizeof(Int16) * n ) );
                *value = (Int16) ( std::numeric_limits< Int16 >::max() * pow( *value / std::numeric_limits< Int16 >::max(), gammaPow ) );
            }
            break;        
        }
        case DATATYPE_LE_UINT32:
        {
            for (UInt32 n = 0; n < valueCount; ++n)
            {
                UInt32* value = (UInt32*)(m_pixelMapData + (sizeof(UInt32) * n));
                *value = (UInt32) ( std::numeric_limits< UInt32 >::max() * pow( *value / std::numeric_limits< UInt32 >::max(), gammaPow ) );
            }
            break;
        }
        case DATATYPE_LE_INT32:
        {
            for (UInt32 n = 0; n < valueCount; ++n)
            {
                Int32* value = (Int32*)(m_pixelMapData + (sizeof(Int32) * n));
                *value = (Int32) ( std::numeric_limits< Int32 >::max() * pow( *value / std::numeric_limits< Int32 >::max(), gammaPow ) );
            }
            break;
        }
        case DATATYPE_LE_UINT64:
        {
            for (UInt32 n = 0; n < valueCount; ++n)
            {
                UInt64* value = (UInt64*)(m_pixelMapData + (sizeof(UInt64) * n));
                *value = (UInt64) ( std::numeric_limits< UInt64 >::max() * pow( *value / std::numeric_limits< UInt64 >::max(), gammaPow ) );
            }
            break;
        }
        case DATATYPE_LE_INT64:
        {
            for (UInt32 n = 0; n < valueCount; ++n)
            {
                Int64* value = (Int64*)(m_pixelMapData + (sizeof(Int64) * n));
                *value = (Int64) ( std::numeric_limits< Int64 >::max() * pow( *value / std::numeric_limits< Int64 >::max(), gammaPow ) );
            }
            break;
        }
    }

    return true;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyPalette( TPixelMapPtr palette, TPixelMapPtr& resultImage ) const
{GUCEF_TRACE;

    if ( m_pixelStorageFormat == PSF_PALETTE_INDICES )
    {
        // Allocate a new blank pixelmap with the right dimensions
        resultImage = TPixelMapPtr( new CPixelMap( NULL, m_widthInPixels, m_heightInPixels, palette->GetPixelStorageFormat(), palette->GetPixelComponentDataType() ) );
        
        UInt32 totalPaletteBytes = palette->GetTotalSizeInBytes();
        UInt32 maxPaletteIndex = palette->GetPixelCount()-1;
        UInt32 totalNewImageBytes = resultImage->GetTotalSizeInBytes();
        UInt32 pixelByteSize = palette->GetSizeOfPixelInBytes();        
        for ( UInt32 i=0; i<m_heightInPixels; ++i )
        {
            for ( UInt32 n=0; n<m_widthInPixels; ++n )
            {
                UInt32 pixelIndex = (i*m_widthInPixels)+n;

                UInt32 paletteIndex = 0;
                switch ( m_pixelComponentDataType )
                {
                    case DATATYPE_LE_FLOAT32:
                    {
                        Float32 index = *(Float32*) ( m_pixelMapData + ( sizeof(Float32) * pixelIndex ) );
                        paletteIndex = (UInt32) index;
                        break;
                    }                            
                    case DATATYPE_LE_FLOAT64:
                    {
                        Float64 index = *(Float64*) ( m_pixelMapData + ( sizeof(Float64) * pixelIndex ) );
                        paletteIndex = (UInt32) index;
                        break;
                    }
                    case DATATYPE_UINT8:
                    {
                        paletteIndex = *(UInt8*) ( m_pixelMapData + ( sizeof(UInt8) * pixelIndex ) );
                        break;
                    }
                    case DATATYPE_INT8:
                    {
                        paletteIndex = *(Int8*) ( m_pixelMapData + ( sizeof(Int8) * pixelIndex ) );
                        break;
                    }
                    case DATATYPE_LE_UINT16:
                    {
                        paletteIndex = *(UInt16*) ( m_pixelMapData + ( sizeof(UInt16) * pixelIndex ) );
                        break;
                    }
                    case DATATYPE_LE_INT16:
                    {
                        paletteIndex = *(UInt16*) ( m_pixelMapData + ( sizeof(UInt16) * pixelIndex ) );
                        break;
                    }
                    case DATATYPE_LE_UINT32:
                    {
                        paletteIndex = *(UInt32*) ( m_pixelMapData + ( sizeof(UInt32) * pixelIndex ) );
                        break;
                    }
                    case DATATYPE_LE_INT32:
                    {
                        paletteIndex = *(UInt32*) ( m_pixelMapData + ( sizeof(UInt32) * pixelIndex ) );
                        break;
                    }
                    case DATATYPE_LE_UINT64:
                    {
                        UInt64 index = *(UInt64*) ( m_pixelMapData + ( sizeof(UInt64) * pixelIndex ) );
                        paletteIndex = (UInt32) index;
                        break;
                    }
                    case DATATYPE_LE_INT64:
                    {
                        Int64 index = *(Int64*) ( m_pixelMapData + ( sizeof(Int64) * pixelIndex ) );
                        paletteIndex = (UInt32) index;
                        break;
                    }
                }

                if ( paletteIndex > maxPaletteIndex )
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Image:ApplyPalette: Found a palette index which is out of bounds: " 
                        + CORE::UInt32ToString( paletteIndex ) + " with a max of " + CORE::UInt32ToString( maxPaletteIndex ) );
                    paletteIndex = maxPaletteIndex;
                }
                                                   // paletteIndex++;
                UInt32 srcPixelOffset = paletteIndex*pixelByteSize;
                UInt32 dstPixelOffset = pixelIndex*pixelByteSize;

                // Make sure we cannot cause memory corruption by copying beyond the buffers
                if ( srcPixelOffset+pixelByteSize <= totalPaletteBytes && dstPixelOffset+pixelByteSize <= totalNewImageBytes )
                {
                    memcpy( resultImage->m_pixelMapData+dstPixelOffset, palette->m_pixelMapData+srcPixelOffset, pixelByteSize );
                }
                else
                {
                    // this should not happen if a valid palette is used for this palettized image
                    // the index is out of the range of the palette
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Image:ApplyPalette: Aborted pixel asssignment since the operation would exceed the buffer size" );
                    //return false;
                }
            }
        }        
        
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyPalette( TPixelMapPtr palette )
{GUCEF_TRACE;
    
    TPixelMapPtr resultImage; 
    if ( ApplyPalette( palette, resultImage ) )
    {
        Clear();
        
        // for efficiency we just steal the data
        m_widthInPixels = resultImage->m_widthInPixels;
        m_heightInPixels = resultImage->m_heightInPixels;
        m_pixelStorageFormat = resultImage->m_pixelStorageFormat;
        m_pixelComponentDataType = resultImage->m_pixelComponentDataType;
        m_pixelMapData = resultImage->m_pixelMapData;
        resultImage->m_pixelMapData = NULL;
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

void
CPixelMap::Assign( const void* pixelMapData                      , 
                   const UInt32 widthInPixels                    ,
                   const UInt32 heightInPixels                   ,
                   const TPixelStorageFormat pixelStorageFormat  ,
                   const TBuildinDataType pixelComponentDataType )
{GUCEF_TRACE;

    // First we clean up whatever may already be allocated
    Clear();
    
    // Assign the pixel map properties
    m_pixelComponentDataType = pixelComponentDataType;
    m_pixelStorageFormat = pixelStorageFormat;
    m_widthInPixels = widthInPixels;
    m_heightInPixels = heightInPixels;

    UInt32 totalSize = GetTotalSizeInBytes();
    m_pixelMapData = new UInt8[ totalSize ];
    
    // If no source data is given we will generate our own pixel map
    // The user can fill in the actual data at a later time
    if ( NULL == pixelMapData )
    {
        memset( m_pixelMapData, 0, totalSize );
    }
    else
    {
        memcpy( m_pixelMapData, pixelMapData, totalSize );
    }
}

/*--------------------------------------------------------------------------*/
    
UInt32
CPixelMap::GetWidthInBytes( void ) const
{GUCEF_TRACE;
    
    return GetSizeOfPixelInBytes() * GetWidthInPixels();
}

/*--------------------------------------------------------------------------*/
    
UInt32
CPixelMap::GetHeightInBytes( void ) const
{GUCEF_TRACE;

    return GetSizeOfPixelInBytes() * GetHeightInPixels();
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetWidthInPixels( void ) const
{GUCEF_TRACE;

    return m_widthInPixels;
}
    
/*--------------------------------------------------------------------------*/
    
UInt32
CPixelMap::GetHeightInPixels( void ) const
{GUCEF_TRACE;

    return m_heightInPixels;
}

/*--------------------------------------------------------------------------*/
    
TPixelStorageFormat
CPixelMap::GetPixelStorageFormat( void ) const
{GUCEF_TRACE;

    return m_pixelStorageFormat;
}

/*--------------------------------------------------------------------------*/
    
void*
CPixelMap::GetDataPtr( const UInt32 pixelOffset /* = 0 */ )
{GUCEF_TRACE;

    return m_pixelMapData + ( GetSizeOfPixelInBytes() * pixelOffset );
}

/*--------------------------------------------------------------------------*/
    
const void*
CPixelMap::GetDataPtr( const UInt32 pixelOffset /* = 0 */ ) const
{GUCEF_TRACE;

    return m_pixelMapData + ( GetSizeOfPixelInBytes() * pixelOffset );
}

/*--------------------------------------------------------------------------*/

TBuildinDataType
CPixelMap::GetPixelComponentDataType( void ) const
{GUCEF_TRACE;

    return m_pixelComponentDataType;
}

/*--------------------------------------------------------------------------*/    

void*
CPixelMap::GetDataAtScanLine( const UInt32 scanLineIndex )
{GUCEF_TRACE;
    
    return m_pixelMapData + ( scanLineIndex * GetWidthInBytes() );
}

/*--------------------------------------------------------------------------*/

const void*
CPixelMap::GetDataAtScanLine( const UInt32 scanLineIndex ) const
{GUCEF_TRACE;

    return m_pixelMapData + ( scanLineIndex * GetWidthInBytes() );
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetPixelChannelSize( void ) const
{GUCEF_TRACE;

    return GetPixelChannelSize( m_pixelComponentDataType );  
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetNumberOfChannelsPerPixel( void ) const
{GUCEF_TRACE;
    
    return GetChannelCountForFormat( m_pixelStorageFormat );
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetSizeOfPixelInBytes( void ) const
{GUCEF_TRACE;

    return GetNumberOfChannelsPerPixel() * GetPixelChannelSize();
}

/*--------------------------------------------------------------------------*/

UInt64
CPixelMap::GetTotalNumberOfChannelComponentValues( void ) const
{GUCEF_TRACE;

    return GetNumberOfChannelsPerPixel() * GetPixelCount();    
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetPixelCount( void ) const
{GUCEF_TRACE;

    return m_widthInPixels * m_heightInPixels;
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetTotalSizeInBytes( void ) const
{GUCEF_TRACE;
    
    return GetPixelCount() * GetSizeOfPixelInBytes();
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::FlipVertical( void )
{GUCEF_TRACE;

    assert( 0 ); // @TODO makeme
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::FlipHorizontal( void )
{GUCEF_TRACE;

    UInt32 scanLineSize = GetWidthInBytes(); 
    void* scanLineBuffer = new UInt8[ scanLineSize ]; 

    UInt32 centerLine = m_heightInPixels / 2;
    for ( UInt32 i=0; i<centerLine; ++i )
    {
        void* frontScanLine = GetDataAtScanLine( i );
        void* backScanLine =  GetDataAtScanLine( (m_heightInPixels-1)-i );
        
        // swap the rows
        memcpy( scanLineBuffer, frontScanLine, scanLineSize );
        memcpy( frontScanLine, backScanLine, scanLineSize );
        memcpy( backScanLine, scanLineBuffer, scanLineSize );
    }
    
    delete []scanLineBuffer;

    return true;
}
    
/*--------------------------------------------------------------------------*/

bool
CPixelMap::ApplyMaskColor( Float32 r ,
                           Float32 g ,
                           Float32 b )
{GUCEF_TRACE;

//    ConvertPixelStorageFormatTo( PSF_RGBA );
    
    assert( 0 ); // @TODO makeme
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::GetHasAlpha( void ) const
{GUCEF_TRACE;

    return GetHasAlpha( m_pixelStorageFormat );    
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::GetHasAlpha( const TPixelStorageFormat pixelStorageFormat )
{GUCEF_TRACE;

    return ( pixelStorageFormat == PSF_RGBA ) || 
           ( pixelStorageFormat == PSF_BGRA ) ||
           ( pixelStorageFormat == PSF_SINGLE_CHANNEL_ALPHA ) ||
           ( pixelStorageFormat == PSF_LUMINANCE_ALPHA );
}
   
/*--------------------------------------------------------------------------*/

void
CPixelMap::Clear( void )
{GUCEF_TRACE;

    m_widthInPixels = 0;
    m_heightInPixels = 0;

    delete []m_pixelMapData;    
    m_pixelMapData = NULL;
}

/*--------------------------------------------------------------------------*/

void*
CPixelMap::RelinquishPixelDataOwnership( void )
{GUCEF_TRACE;

    m_widthInPixels = 0;
    m_heightInPixels = 0;

    void* pixelData = m_pixelMapData;    
    m_pixelMapData = NULL;
    return pixelData;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CPixelMap::ConvertFormatToImp( T* pixelMapData                               ,
                               const TPixelStorageFormat pixelStorageFormat  , 
                               const TBuildinDataType pixelComponentDataType ,
                               TPixelMapPtr& newMap                          )
{
    // create new pixelmap with enough space
    newMap = TPixelMapPtr( new CPixelMap( NULL, m_widthInPixels, m_heightInPixels, pixelStorageFormat, pixelComponentDataType ) );
    
    UInt32 pixelCount = GetPixelCount();
    UInt32 channelCount = GetNumberOfChannelsPerPixel();

    switch ( m_pixelStorageFormat )
    {   
        case PSF_BGR:
        {
            switch ( pixelStorageFormat )
            {
                case PSF_RGB:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_RGBA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ], 1 }; // fully opaque

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_BGRA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 0 ], orgPixelData[ 1 ], orgPixelData[ 2 ], 1 };  // fully opaque

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_RED:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 2 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_GREEN:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 1 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_BLUE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }         
                case PSF_SINGLE_CHANNEL_STD_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (standard for certain colour spaces): (0.2126*R + 0.7152*G + 0.0722*B) 
                        CORE::Float64 luminance = ( 0.0722f * orgPixelData[ 0 ] ) + ( 0.7152f * orgPixelData[ 1 ] ) + ( 0.2126f * orgPixelData[ 2 ] );                      
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P1_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 1): (0.299*R + 0.587*G + 0.114*B)
                        // Note that both of these emphasize the physiological aspects: the human eyeball is most sensitive to green light, less to red and least to blue.                       
                        CORE::Float64 luminance = ( 0.114f * orgPixelData[ 0 ] ) + ( 0.587f * orgPixelData[ 1 ] ) + ( 0.299f * orgPixelData[ 2 ] );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P2_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 2): sqrt( 0.299*R^2 + 0.587*G^2 + 0.114*B^2 )                      
                        // Note that both of these emphasize the physiological aspects: the human eyeball is most sensitive to green light, less to red and least to blue.
                        CORE::Float64 luminance = sqrt( ( 0.114f * pow( orgPixelData[ 0 ], 2 ) ) + ( 0.587f * pow( orgPixelData[ 1 ], 2 ) ) + ( 0.299f * pow( orgPixelData[ 2 ], 2 ) ) );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
            }
            break;
        }
        case PSF_BGRA:
        {
            switch ( pixelStorageFormat )
            {
                case PSF_RGB:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_RGBA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ], orgPixelData[ 3 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_BGR:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 0 ], orgPixelData[ 1 ], orgPixelData[ 2 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_RED:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 2 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_GREEN:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 1 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_BLUE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }         
                case PSF_SINGLE_CHANNEL_ALPHA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 3 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_STD_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (standard for certain colour spaces): (0.2126*R + 0.7152*G + 0.0722*B)                        
                        CORE::Float64 luminance = ( 0.0722f * orgPixelData[ 0 ] ) + ( 0.7152f * orgPixelData[ 1 ] ) + ( 0.2126f * orgPixelData[ 2 ] );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P1_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 1): (0.299*R + 0.587*G + 0.114*B)                       
                        CORE::Float64 luminance = ( 0.114f * orgPixelData[ 0 ] ) + ( 0.587f * orgPixelData[ 1 ] ) + ( 0.299f * orgPixelData[ 2 ] );
                        T pixelData[ 1 ] { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P2_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 1): sqrt( 0.299*R^2 + 0.587*G^2 + 0.114*B^2 )                      
                        CORE::Float64 luminance = sqrt( ( 0.114f * pow( orgPixelData[ 0 ], 2 ) ) + ( 0.587f * pow( orgPixelData[ 1 ], 2 ) ) + ( 0.299f * pow( orgPixelData[ 2 ], 2 ) ) );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
            }
            break;
        }
        case PSF_RGB:
        {
            switch ( pixelStorageFormat )
            {
                case PSF_BGR:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_BGRA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ], 0 }; // fully opaque

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_RGBA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 0 ], orgPixelData[ 1 ], orgPixelData[ 2 ], 0 };  // fully opaque

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_HSV:
                {
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);

                        // find the lowest value component
                        T min = orgPixelData[ 0 ] < orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                        min = min < orgPixelData[ 2 ] ? min : orgPixelData[ 2 ];
                                                 
                        // find the highest value component, this is the V in HSV
                        T max = orgPixelData[ 0 ] > orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                        max = max > orgPixelData[ 2 ] ? max : orgPixelData[ 2 ];

                        T s=0; T h=0; T v=max;
                        T delta = max - min;
                        if ( delta < 0.00001 )
                        {
                            s = 0;
                            h = 0; // undefined, maybe nan?
                        }
                        else
                        {
                            if ( max > 0.0 ) 
                            {   
                                // NOTE: if Max is == 0, this divide would cause a crash
                                s = (delta / max);
                            }
                        }

                        // Assign new pixel value and convert data type from T to its type
                        T pixelData[ 3 ] = { h, s, v };
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_HSVA:
                {
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);

                        // find the lowest value component
                        T min = orgPixelData[ 0 ] < orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                        min = min < orgPixelData[ 2 ] ? min : orgPixelData[ 2 ];
                                                 
                        // find the highest value component, this is the V in HSV
                        T max = orgPixelData[ 0 ] > orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                        max = max > orgPixelData[ 2 ] ? max : orgPixelData[ 2 ];

                        T s=0; T h=0; T v=max;
                        T delta = max - min;
                        if ( delta < 0.00001 )
                        {
                            s = 0;
                            h = 0; // undefined, maybe nan?
                        }
                        else
                        {
                            if ( max > 0.0 ) 
                            {   
                                // NOTE: if Max is == 0, this divide would cause a crash
                                s = (delta / max);
                            }
                        }

                        // Assign new pixel value and convert data type from T to its type
                        T pixelData[ 4 ] = { h, s, v, std::numeric_limits< T >::max() };
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_RED:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_GREEN:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 1 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_BLUE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 2 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }         
                case PSF_SINGLE_CHANNEL_STD_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (standard for certain colour spaces): (0.2126*R + 0.7152*G + 0.0722*B)                        
                        CORE::Float64 luminance = ( 0.0722f * orgPixelData[ 2 ] ) + ( 0.7152f * orgPixelData[ 1 ] ) + ( 0.2126f * orgPixelData[ 0 ] );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P1_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 1): (0.299*R + 0.587*G + 0.114*B)                       
                        CORE::Float64 luminance = ( 0.114f * orgPixelData[ 2 ] ) + ( 0.587f * orgPixelData[ 1 ] ) + ( 0.299f * orgPixelData[ 0 ] );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P2_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 1): sqrt( 0.299*R^2 + 0.587*G^2 + 0.114*B^2 )                      
                        CORE::Float64 luminance = sqrt( ( 0.114f * pow( orgPixelData[ 2 ], 2 ) ) + ( 0.587f * pow( orgPixelData[ 1 ], 2 ) ) + ( 0.299f * pow( orgPixelData[ 0 ], 2 ) ) );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
            }
            break;
        }
        case PSF_RGBA:
        {
            switch ( pixelStorageFormat )
            {
                case PSF_BGR:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_BGRA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 2 ], orgPixelData[ 1 ], orgPixelData[ 0 ], orgPixelData[ 3 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_RGB:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 0 ], orgPixelData[ 1 ], orgPixelData[ 2 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                //case PSF_HSV:
                //{
                //    for ( UInt32 i=0; i<pixelCount; ++i )
                //    {                            
                //        T* orgPixelData = pixelMapData+(i*channelCount);

                //        // find the lowest value component
                //        T min = orgPixelData[ 0 ] < orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                //        min = min < orgPixelData[ 2 ] ? min : orgPixelData[ 2 ];
                //                                 
                //        // find the highest value component, this is the V in HSV
                //        T max = orgPixelData[ 0 ] > orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                //        max = max > orgPixelData[ 2 ] ? max : orgPixelData[ 2 ];

                //        T s=0, T h=0; T v=max;
                //        T delta = max - min;
                //        if ( delta < 0.00001 )
                //        {
                //            s = 0;
                //            h = 0; // undefined, maybe nan?
                //        }
                //        else
                //        {
                //            if ( max > 0.0 ) 
                //            {   
                //                // NOTE: if Max is == 0, this divide would cause a crash
                //                s = (delta / max);
                //            }
                //        }

                //        // Assign new pixel value and convert data type from T to its type
                //        T pixelData[ 3 ] = { h, s, v };
                //        newMap->AssignToPixel< T >( i, pixelData );
                //    }
                //    return true;
                //}
                //case PSF_HSVA:
                //{
                //    for ( UInt32 i=0; i<pixelCount; ++i )
                //    {                            
                //        T* orgPixelData = pixelMapData+(i*channelCount);

                //        // find the lowest value component
                //        T min = orgPixelData[ 0 ] < orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                //        min = min < orgPixelData[ 2 ] ? min : orgPixelData[ 2 ];
                //                                 
                //        // find the highest value component, this is the V in HSV
                //        T max = orgPixelData[ 0 ] > orgPixelData[ 1 ] ? orgPixelData[ 0 ] : orgPixelData[ 1 ];
                //        max = max > orgPixelData[ 2 ] ? max : orgPixelData[ 2 ];

                //        T s=0, T h=0; T v=max;
                //        T delta = max - min;
                //        if ( delta < 0.00001 )
                //        {
                //            s = 0;
                //            h = 0; // undefined, maybe nan?
                //        }
                //        else
                //        {
                //            if ( max > 0.0 ) 
                //            {   
                //                // NOTE: if Max is == 0, this divide would cause a crash
                //                s = (delta / max);
                //            }
                //        }

                //        // Assign new pixel value and convert data type from T to its type
                //        T pixelData[ 4 ] = { h, s, v, orgPixelData[ 3 ] };
                //        newMap->AssignToPixel< T >( i, pixelData );
                //    }
                //    return true;
                //}
                case PSF_SINGLE_CHANNEL_RED:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_GREEN:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 1 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_BLUE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 2 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }         
                case PSF_SINGLE_CHANNEL_ALPHA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 3 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_STD_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (standard for certain colour spaces): (0.2126*R + 0.7152*G + 0.0722*B)
                        CORE::Float64 luminance = ( 0.0722f * orgPixelData[ 2 ] ) + ( 0.7152f * orgPixelData[ 1 ] ) + ( 0.2126f * orgPixelData[ 0 ] );                        
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P1_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 1): (0.299*R + 0.587*G + 0.114*B)                       
                        CORE::Float64 luminance = ( 0.114f * orgPixelData[ 2 ] ) + ( 0.587f * orgPixelData[ 1 ] ) + ( 0.299f * orgPixelData[ 0 ] );
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_P2_LUMINANCE:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);
                        
                        // Luminance (perceived option 1): sqrt( 0.299*R^2 + 0.587*G^2 + 0.114*B^2 )                      
                        CORE::Float64 luminance = sqrt( ( 0.114f * pow( orgPixelData[ 2 ], 2 ) ) + ( 0.587f * pow( orgPixelData[ 1 ], 2 ) ) + ( 0.299f * pow( orgPixelData[ 0 ], 2 ) ) ); 
                        T pixelData[ 1 ] = { (T) round( luminance ) };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
            }
            break;
        }
        //case PSF_HSV:
        //{
        //    switch ( pixelStorageFormat )
        //    {
        //        case PSF_RGB:
        //        {
        //            for ( UInt32 i=0; i<pixelCount; ++i )
        //            {                            
        //                T* orgPixelData = pixelMapData+(i*channelCount);
        //                T h = orgPixelData[ 0 ];  // angle in degrees
        //                T s = orgPixelData[ 1 ];  // percent
        //                T v = orgPixelData[ 2 ];  // percent

        //                if ( s <= 0.0 ) 
        //                {    
        //                    // special case, just shuts up warnings
        //                    // Assign new pixel value and convert data type from T to its type
        //                    T pixelData[ 3 ] = { v, v, v };
        //                    newMap->AssignToPixel< T >( i, pixelData );
        //                }
        //                else
        //                {
        //                    Float64 hh = h;
        //                    if ( hh >= 360.0 ) 
        //                        hh = 0.0;
        //                    hh /= 60.0;
        //                    Int32 order = (Int32) hh;
        //                    Float64 ff = hh - i;
        //                    Float64 p = v * ( 1.0 - s );
        //                    Float64 q = v * ( 1.0 - ( s * ff ) );
        //                    Float64 t = v * ( 1.0 - ( s * ( 1.0 - ff) ) );

        //                    switch( order )
        //                    {
        //                        case 0:
        //                        {
        //                            Float64 percPixelData[ 3 ] = { v, t, p };
        //                            T pixelData[ 3 ] = { v * std::max, t, p };
        //                            newMap->AssignToPixel< T >( i, pixelData );
        //                            break;
        //                        }
        //                        case 1:
        //                        {
        //                            T pixelData[ 3 ] = { q, v, p };
        //                            newMap->AssignToPixel< T >( i, pixelData );
        //                            break;
        //                        }
        //                        case 2:
        //                        {
        //                            T pixelData[ 3 ] = { p, v, t };
        //                            newMap->AssignToPixel< T >( i, pixelData );
        //                            break;
        //                        }
        //                        case 3:
        //                        {
        //                            T pixelData[ 3 ] = { p, q, v };
        //                            newMap->AssignToPixel< T >( i, pixelData );
        //                            break;
        //                        }
        //                        case 4:
        //                        {
        //                            T pixelData[ 3 ] = { t, p, v };
        //                            newMap->AssignToPixel< T >( i, pixelData );
        //                            break;
        //                        }
        //                        case 5:
        //                        default:
        //                        {
        //                            T pixelData[ 3 ] = { v, p, q };
        //                            newMap->AssignToPixel< T >( i, pixelData );
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //            return true;
        //        }
        //    }
        //    break;
        //}
        case PSF_SINGLE_CHANNEL_GRAYSCALE:
        {
            switch ( pixelStorageFormat )
            {
                case PSF_BGR:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 0 ], orgPixelData[ 0 ], orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_BGRA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 0 ], orgPixelData[ 0 ], orgPixelData[ 0 ], 0 };  // fully opaque

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_RGB:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 0 ], orgPixelData[ 0 ], orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_RED:
                case PSF_SINGLE_CHANNEL_GREEN:
                case PSF_SINGLE_CHANNEL_BLUE:
                case PSF_SINGLE_CHANNEL_ALPHA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
            }
            break;
        }
        case PSF_PALETTE_INDICES:
        {
            switch ( pixelStorageFormat )
            {
                case PSF_BGR:
                case PSF_RGB:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 3 ] = { orgPixelData[ 0 ], orgPixelData[ 0 ], orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_RGBA:
                case PSF_BGRA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 4 ] = { orgPixelData[ 0 ], orgPixelData[ 0 ], orgPixelData[ 0 ], 0 };  // fully opaque

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
                case PSF_SINGLE_CHANNEL_RED:
                case PSF_SINGLE_CHANNEL_GREEN:
                case PSF_SINGLE_CHANNEL_BLUE:
                case PSF_SINGLE_CHANNEL_ALPHA:
                {
                    // copy segments and alter/supplement
                    for ( UInt32 i=0; i<pixelCount; ++i )
                    {                            
                        T* orgPixelData = pixelMapData+(i*channelCount);                        
                        T pixelData[ 1 ] = { orgPixelData[ 0 ] };

                        // Assign new pixel value and convert data type from T to its type
                        newMap->AssignToPixel< T >( i, pixelData );
                    }
                    return true;
                }
            }
            break;
        }
    }   

    newMap.Unlink();
    return false;
}

/*--------------------------------------------------------------------------*/

bool 
CPixelMap::ConvertFormatTo( const TBuildinDataType pixelComponentDataType , 
                            TPixelMapPtr& newMap                          )
{GUCEF_TRACE;

    return ConvertFormatTo( m_pixelStorageFormat, pixelComponentDataType, newMap );
}

/*--------------------------------------------------------------------------*/

bool 
CPixelMap::ConvertFormatTo( const TBuildinDataType pixelComponentDataType )
{GUCEF_TRACE;

    return ConvertFormatTo( m_pixelStorageFormat, pixelComponentDataType );
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ConvertFormatTo( const TPixelStorageFormat pixelStorageFormat  , 
                            TPixelMapPtr& newMap                          )
{GUCEF_TRACE;

    return ConvertFormatTo( pixelStorageFormat, m_pixelComponentDataType, newMap );
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ConvertFormatTo( const TPixelStorageFormat pixelStorageFormat )
{GUCEF_TRACE;

    return ConvertFormatTo( pixelStorageFormat, m_pixelComponentDataType );
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ConvertFormatTo( const TPixelStorageFormat pixelStorageFormat  , 
                            const TBuildinDataType pixelComponentDataType )
{GUCEF_TRACE;

    TPixelMapPtr resultImage; 
    if ( ConvertFormatTo( pixelStorageFormat, pixelComponentDataType, resultImage ) )
    {
        Clear();
        
        // for efficiency we just steal the data
        m_widthInPixels = resultImage->m_widthInPixels;
        m_heightInPixels = resultImage->m_heightInPixels;
        m_pixelStorageFormat = resultImage->m_pixelStorageFormat;
        m_pixelComponentDataType = resultImage->m_pixelComponentDataType;
        m_pixelMapData = resultImage->m_pixelMapData;
        resultImage->m_pixelMapData = NULL;
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::ConvertFormatTo( const TPixelStorageFormat pixelStorageFormat  , 
                            const TBuildinDataType pixelComponentDataType ,
                            TPixelMapPtr& newMap                          )
{GUCEF_TRACE;

    // Only do conversions if needed
    if ( m_pixelStorageFormat != pixelStorageFormat || m_pixelComponentDataType != pixelComponentDataType )
    {
        switch ( m_pixelComponentDataType )
        {
            case DATATYPE_LE_FLOAT32:
            {
                CORE::Float32* pixelMapData = (CORE::Float32*) m_pixelMapData;
                return ConvertFormatToImp< CORE::Float32 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_LE_FLOAT64:
            {
                CORE::Float64* pixelMapData = (CORE::Float64*) m_pixelMapData;
                return ConvertFormatToImp< CORE::Float64 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_UINT8:
            {
                CORE::UInt8* pixelMapData = (CORE::UInt8*) m_pixelMapData;
                return ConvertFormatToImp< CORE::UInt8 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_INT8:
            {
                CORE::Int8* pixelMapData = (CORE::Int8*) m_pixelMapData;
                return ConvertFormatToImp< CORE::Int8 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_LE_UINT16:
            {
                CORE::UInt16* pixelMapData = (CORE::UInt16*) m_pixelMapData;
                return ConvertFormatToImp< CORE::UInt16 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_LE_INT16:
            {
                CORE::Int16* pixelMapData = (CORE::Int16*) m_pixelMapData;
                return ConvertFormatToImp< CORE::Int16 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_LE_UINT32:
            {
                CORE::UInt32* pixelMapData = (CORE::UInt32*) m_pixelMapData;
                return ConvertFormatToImp< CORE::UInt32 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_LE_INT32:
            {
                CORE::Int32* pixelMapData = (CORE::Int32*) m_pixelMapData;
                return ConvertFormatToImp< CORE::Int32 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_LE_UINT64:
            {
                CORE::UInt64* pixelMapData = (CORE::UInt64*) m_pixelMapData;
                return ConvertFormatToImp< CORE::UInt64 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            case DATATYPE_LE_INT64:
            {
                CORE::Int64* pixelMapData = (CORE::Int64*) m_pixelMapData;
                return ConvertFormatToImp< CORE::Int64 >( pixelMapData, pixelStorageFormat, pixelComponentDataType, newMap );
            }
            default:
            {
                // this should not happen
                return false;
            }
        }
    }
    else
    {
        // no change needed
        newMap.Unlink();
        return true;
    }        
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetChannelCountForFormat( const TPixelStorageFormat pixelStorageFormat )
{GUCEF_TRACE;

    switch ( pixelStorageFormat )
    {
        case PSF_BGR :
        case PSF_RGB :
        case PSF_HSV :
        case PSF_HSL :
        {
            return 3;
        }
        case PSF_BGRA :
        case PSF_RGBA :
        case PSF_HSVA :
        case PSF_HSLA :
        {
            return 4;
        }
        case PSF_SINGLE_CHANNEL_GRAYSCALE :
        case PSF_SINGLE_CHANNEL_RED :
        case PSF_SINGLE_CHANNEL_GREEN :
        case PSF_SINGLE_CHANNEL_BLUE :
        case PSF_SINGLE_CHANNEL_ALPHA :
        case PSF_SINGLE_CHANNEL_STD_LUMINANCE :
        case PSF_SINGLE_CHANNEL_P1_LUMINANCE :
        case PSF_SINGLE_CHANNEL_P2_LUMINANCE :
        case PSF_LUMINANCE_ALPHA :
        case PSF_PALETTE_INDICES :
        {
            return 1;
        }
        case PSF_COUNT :
        default :
        {
            return 0;
        }
    }
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetPixelChannelSize( const TBuildinDataType pixelComponentDataType )
{GUCEF_TRACE;

    switch ( pixelComponentDataType )
    {
        case DATATYPE_INT8 :
        case DATATYPE_UINT8 :
        {
            return 1;
        }
        case DATATYPE_LE_INT16 :
        case DATATYPE_LE_UINT16 :
        case DATATYPE_BE_INT16 :
        case DATATYPE_BE_UINT16 :
        {
            return 2;
        }        
        case DATATYPE_LE_INT32 :
        case DATATYPE_LE_UINT32 :
        case DATATYPE_LE_FLOAT32 :
        case DATATYPE_BE_INT32 :
        case DATATYPE_BE_UINT32 :
        case DATATYPE_BE_FLOAT32 :
        {
            return 4;
        }
        case DATATYPE_LE_INT64 :
        case DATATYPE_LE_UINT64 :
        case DATATYPE_LE_FLOAT64 :
        case DATATYPE_BE_INT64 :
        case DATATYPE_BE_UINT64 :
        case DATATYPE_BE_FLOAT64 :
        {
            return 8;
        }
        default :
        {
            return 0;
        }        
    }            
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetPixelSize( const TPixelStorageFormat pixelStorageFormat  ,
                         const TBuildinDataType pixelComponentDataType )
{GUCEF_TRACE;
    
    return GetChannelCountForFormat( pixelStorageFormat ) * GetPixelChannelSize( pixelComponentDataType );
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::GetExpectedPixelMapSize( const UInt32 widthInPixels                    ,
                                    const UInt32 heightInPixels                   ,
                                    const TPixelStorageFormat pixelStorageFormat  ,
                                    const TBuildinDataType pixelComponentDataType )
{GUCEF_TRACE;

    return widthInPixels * heightInPixels * GetPixelSize( pixelStorageFormat, pixelComponentDataType );
}

/*--------------------------------------------------------------------------*/

bool
CPixelMap::CopyTo( CORE::CIOAccess& resource )
{GUCEF_TRACE;

    UInt32 dataSize = GetTotalSizeInBytes();
    CORE::CMFileAccess memoryFile( m_pixelMapData, dataSize );
    return resource.Write( memoryFile ) == dataSize;
}

/*--------------------------------------------------------------------------*/

bool 
CPixelMap::GetColorCounters( TColorCounters& colorCountMap ) const
{GUCEF_TRACE;

    UInt32 pixelCount = GetPixelCount();
    for ( UInt32 i=0; i<pixelCount; ++i )
    {
        CPixel pixel( GetDataPtr( i ), m_pixelStorageFormat, m_pixelComponentDataType, true );
        ++( colorCountMap[ pixel ] );
    }
    return true;
}

/*--------------------------------------------------------------------------*/

UInt32
CPixelMap::DetermineActualColorCount( void ) const
{GUCEF_TRACE;

    TColorCounters colorCounters;
    if ( GetColorCounters( colorCounters ) )
    {
        return (UInt32) colorCounters.size();
    }
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace IMAGE */
} /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
