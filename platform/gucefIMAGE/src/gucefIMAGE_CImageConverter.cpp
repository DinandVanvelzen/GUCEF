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
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#include "gucefIMAGE_CImageCodecCapabilities.h"
#define GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#endif /* GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H ? */

#include "gucefIMAGE_CImageConverter.h"

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

namespace {

typedef GUCEF::vector< UInt32 > TFrameIndexVector;

/*-------------------------------------------------------------------------*/

/** Reads the image of the input, and its meta-data when the format has it */
bool
LoadSource( CORE::CIOAccess& input, const CORE::CString& fromType, CImage& image, CImageConversionResult& result )
{GUCEF_TRACE;

    CImageCodecCapabilities::SImageCodecInfo info;
    if ( !CImageCodecCapabilities::TryGetCodec( fromType, info ) )
    {
        result.SetFailed( "there is no image codec for the format \"" + fromType + "\"" );
        return false;
    }
    if ( !info.canDecode )
    {
        result.SetFailed( "the format \"" + fromType + "\" can be written but not read" );
        return false;
    }

    const UInt64 start = input.Tell();
    if ( !image.Load( input, fromType, true, false ) || !image.HasFrames() )
    {
        result.SetFailed( "the input could not be read as \"" + fromType + "\"" );
        return false;
    }

    // the meta-data is a bonus: a codec that cannot give it does not fail the conversion
    if ( 0 == input.Setpos( start ) )
    {
        CImage withMetaData;
        if ( withMetaData.Load( input, fromType, false, true ) )
            image.GetMetaData() = withMetaData.GetMetaData();
    }
    result.SetSourceFrameCount( image.GetFrameCount() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The frames to convert, false with the reason in the result when the settings name a frame the image does not have */
bool
SelectFrames( const CImage& image, const CImageConversionSettings& settings, TFrameIndexVector& indices, CImageConversionResult& result )
{GUCEF_TRACE;

    indices.clear();
    if ( settings.IsAllFrames() )
    {
        for ( UInt32 f=0; f<image.GetFrameCount(); ++f )
            indices.push_back( f );
        return true;
    }
    if ( settings.GetFrameIndex() >= image.GetFrameCount() )
    {
        result.SetFailed( "frame " + CORE::UInt32ToString( settings.GetFrameIndex() ) + " was asked for, the image has " +
                          CORE::UInt32ToString( image.GetFrameCount() ) + " frames" );
        return false;
    }
    indices.push_back( settings.GetFrameIndex() );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The image with only the given frames (and their first mipmap level only when asked), sharing the pixel maps of the source */
void
BuildImage( const CImage& source, const TFrameIndexVector& indices, size_t from, size_t count, bool firstLevelOnly, CImage& target )
{GUCEF_TRACE;

    target.Clear();
    for ( size_t i=from; i<from+count; ++i )
    {
        CImage::TMipMapList levels = source.GetFrame( indices[ i ] );
        if ( firstLevelOnly && levels.size() > 1 )
            levels.resize( 1 );
        target.AddFrame( levels, source.GetFrameDurationInMs( indices[ i ] ) );
    }
    target.SetLoopCount( source.GetLoopCount() );
    target.GetMetaData() = source.GetMetaData();
}

/*-------------------------------------------------------------------------*/

/** Converts every pixel map of the image to the format, false when one cannot be converted */
bool
ConvertImageFormat( const CImage& source, TPixelStorageFormat format, TBuildinDataType type, CImage& target )
{GUCEF_TRACE;

    target.Clear();
    for ( UInt32 f=0; f<source.GetFrameCount(); ++f )
    {
        const CImage::TMipMapList& levels = source.GetFrame( f );
        CImage::TMipMapList converted;
        for ( size_t l=0; l<levels.size(); ++l )
        {
            TPixelMapPtr pixels = levels[ l ];
            if ( pixels->GetPixelStorageFormat() != format || pixels->GetPixelComponentDataType() != type )
            {
                TPixelMapPtr result;
                if ( !pixels->ConvertFormatTo( format, type, result ) || result.IsNULL() )
                    return false;
                pixels = result;
            }
            converted.push_back( pixels );
        }
        target.AddFrame( converted, source.GetFrameDurationInMs( f ) );
    }
    target.SetLoopCount( source.GetLoopCount() );
    target.GetMetaData() = source.GetMetaData();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IsLossyConversion( const CImage& source, TPixelStorageFormat format, TBuildinDataType type, CORE::CString& reason )
{GUCEF_TRACE;

    bool narrowed = false;
    bool alphaDropped = false;
    for ( UInt32 f=0; f<source.GetFrameCount(); ++f )
    {
        const CImage::TMipMapList& levels = source.GetFrame( f );
        for ( size_t l=0; l<levels.size(); ++l )
        {
            const TPixelMapPtr& pixels = levels[ l ];
            if ( CPixelMap::GetPixelChannelSize( pixels->GetPixelComponentDataType() ) > CPixelMap::GetPixelChannelSize( type ) ||
                 ( pixels->GetPixelComponentDataType() != type && ( DATATYPE_LE_FLOAT32 == pixels->GetPixelComponentDataType() ||
                                                                    DATATYPE_LE_FLOAT64 == pixels->GetPixelComponentDataType() ) ) )
                narrowed = true;
            if ( pixels->GetHasAlpha() && !CPixelMap::GetHasAlpha( format ) )
                alphaDropped = true;
        }
    }
    if ( narrowed )
        reason = "the channels are reduced to a narrower type";
    if ( alphaDropped )
        reason += ( reason.IsNULLOrEmpty() ? "" : ", " ) + CORE::CString( "the alpha channel is dropped" );
    return narrowed || alphaDropped;
}

/*-------------------------------------------------------------------------*/

/** Writes the image with the writer of the format: the image codec registry first, the codec registry second, as CImage::Save() does */
bool
EncodeWithCodec( CImage& image, const CORE::CString& toType, const CORE::CValueList& params, CORE::CDynamicBuffer& encoded )
{GUCEF_TRACE;

    encoded.Clear();
    CORE::CDynamicBufferAccess access( &encoded, false );
    bool success = false;
    CImageCodecRegistry::TImageCodecPtr imageCodec;
    if ( CImageGlobal::Instance()->GetImageCodecRegistry().TryLookup( toType, imageCodec, false ) && !imageCodec.IsNULL() )
        success = imageCodec->Encode( image, access, params );
    else
        success = image.Save( access, toType );
    return success && encoded.GetDataSize() > 0;
}

/*-------------------------------------------------------------------------*/

/**
 *  Writes the image to a buffer as a file of the format. When the writer does not take the image as it is, RGBA8 and then
 *  RGB8 are tried and what that changes is noted in the result
 */
bool
EncodeNegotiated( CImage& image                              ,
                  const CORE::CString& toType                ,
                  const CImageConversionSettings& settings   ,
                  CORE::CDynamicBuffer& encoded              ,
                  CImageConversionResult& result             )
{GUCEF_TRACE;

    if ( EncodeWithCodec( image, toType, settings.GetEncoderParams(), encoded ) )
        return true;

    // what is tried when the writer does not take the image as it is: 8 bit pictures first, floating point ones for the formats that hold those
    struct SCandidate { TPixelStorageFormat format; TBuildinDataType type; const char* name; };
    const SCandidate candidates[] =
    {
        { PSF_RGBA, DATATYPE_UINT8, "RGBA 8 bit" }, { PSF_RGB, DATATYPE_UINT8, "RGB 8 bit" },
        { PSF_RGBA, DATATYPE_LE_FLOAT32, "RGBA floating point" }, { PSF_RGB, DATATYPE_LE_FLOAT32, "RGB floating point" }
    };
    CORE::CString refusedForLoss;
    for ( size_t c=0; c<sizeof( candidates ) / sizeof( candidates[ 0 ] ); ++c )
    {
        CORE::CString reason;
        const bool lossy = IsLossyConversion( image, candidates[ c ].format, candidates[ c ].type, reason );
        if ( lossy && !settings.GetAllowLossyConversion() )
        {
            refusedForLoss = reason;
            continue;
        }

        CImage converted;
        if ( !ConvertImageFormat( image, candidates[ c ].format, candidates[ c ].type, converted ) )
            continue;
        if ( EncodeWithCodec( converted, toType, settings.GetEncoderParams(), encoded ) )
        {
            CORE::CString note = "the pixel format was converted to " + CORE::CString( candidates[ c ].name ) + " because the format \"" + toType + "\" cannot hold the image as it is";
            if ( lossy )
                note += ": " + reason;
            result.AddNote( note, lossy );
            return true;
        }
    }

    if ( !refusedForLoss.IsNULLOrEmpty() )
        result.SetFailed( "the format \"" + toType + "\" cannot hold the image without losing information (" + refusedForLoss + ")" );
    else
        result.SetFailed( "the format \"" + toType + "\" could not write the image" );
    return false;
}

/*-------------------------------------------------------------------------*/

/** Everything both conversions share until the image is encoded: the target, the source, the frames */
bool
Prepare( CORE::CIOAccess& input                    ,
         const CORE::CString& fromType             ,
         const CORE::CString& toType               ,
         const CImageConversionSettings& settings  ,
         CImageConversionResult& result            ,
         CImageCodecCapabilities::SImageCodecInfo& target ,
         CImage& source                            ,
         TFrameIndexVector& indices                )
{GUCEF_TRACE;

    result.Clear();
    if ( !CImageCodecCapabilities::TryGetCodec( toType, target ) )
    {
        result.SetFailed( "there is no image codec for the format \"" + toType + "\"" );
        return false;
    }
    if ( !target.canEncode )
    {
        result.SetFailed( "the format \"" + toType + "\" can be read but not written" );
        return false;
    }
    return LoadSource( input, fromType, source, result ) && SelectFrames( source, settings, indices, result );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageConverter::CImageConverter( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConverter::CImageConverter( const CImageConverter& src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConverter::~CImageConverter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConverter&
CImageConverter::operator=( const CImageConverter& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CImageConverter::Convert( CORE::CIOAccess& input                     ,
                          const CORE::CString& fromType              ,
                          CORE::CIOAccess& output                    ,
                          const CORE::CString& toType                ,
                          const CImageConversionSettings& settings   ,
                          CImageConversionResult& result             ) const
{GUCEF_TRACE;

    CImageCodecCapabilities::SImageCodecInfo target;
    CImage source;
    TFrameIndexVector indices;
    if ( !Prepare( input, fromType, toType, settings, result, target, source, indices ) )
        return false;

    if ( indices.size() > 1 && !target.multipleFrames )
    {
        result.SetFailed( "the format \"" + toType + "\" holds one picture and the image has " + CORE::UInt32ToString( (UInt32) indices.size() ) +
                          " frames: choose a frame or ask for the frames as separate files" );
        return false;
    }

    CImage selected;
    BuildImage( source, indices, 0, indices.size(), !target.mipLevels, selected );
    CORE::CDynamicBuffer encoded;
    if ( !EncodeNegotiated( selected, toType, settings, encoded, result ) )
        return false;

    if ( 1 != output.Write( encoded.GetConstBufferPtr(), encoded.GetDataSize(), 1 ) )
    {
        result.SetFailed( "the converted image could not be written to the output" );
        return false;
    }
    result.SetWrittenFrameCount( (UInt32) indices.size() );
    result.SetSuccess();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImageConverter::ConvertToFrames( CORE::CIOAccess& input                     ,
                                  const CORE::CString& fromType              ,
                                  const CORE::CString& toType                ,
                                  const CImageConversionSettings& settings   ,
                                  CImageConversionResult& result             ,
                                  TEncodedFrameVector& frames                ) const
{GUCEF_TRACE;

    frames.clear();
    CImageCodecCapabilities::SImageCodecInfo target;
    CImage source;
    TFrameIndexVector indices;
    if ( !Prepare( input, fromType, toType, settings, result, target, source, indices ) )
        return false;

    TEncodedFrameVector encodedFrames;
    for ( size_t i=0; i<indices.size(); ++i )
    {
        CImage single;
        BuildImage( source, indices, i, 1, !target.mipLevels, single );
        SEncodedFrame frame;
        frame.frameIndex = indices[ i ];
        if ( !EncodeNegotiated( single, toType, settings, frame.data, result ) )
            return false;
        encodedFrames.push_back( frame );
    }

    frames = encodedFrames;
    result.SetWrittenFrameCount( (UInt32) frames.size() );
    result.SetSuccess();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
