/*
 *  OgreCodec_GucefIMAGE: Ogre plugin that decodes and encodes the image formats of gucefIMAGE for Ogre
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

#include <string.h>
#include <sstream>

#include "OgrePixelFormat.h"

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "OgreCodec_GucefIMAGE_CGucefImageConverter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Ogre::PixelFormat
CGucefImageConverter::ToOgrePixelFormat( IMAGE::TPixelStorageFormat storageFormat ,
                                         TBuildinDataType componentDataType       )
{GUCEF_TRACE;

    #if OGRE_ENDIAN == OGRE_ENDIAN_LITTLE
    const TBuildinDataType uint16Type = DATATYPE_LE_UINT16;
    const TBuildinDataType float32Type = DATATYPE_LE_FLOAT32;
    #else
    const TBuildinDataType uint16Type = DATATYPE_BE_UINT16;
    const TBuildinDataType float32Type = DATATYPE_BE_FLOAT32;
    #endif

    if ( DATATYPE_UINT8 == componentDataType )
    {
        switch ( storageFormat )
        {
            case IMAGE::PSF_RGB :                           return Ogre::PF_BYTE_RGB;
            case IMAGE::PSF_RGBA :                          return Ogre::PF_BYTE_RGBA;
            case IMAGE::PSF_BGR :                           return Ogre::PF_BYTE_BGR;
            case IMAGE::PSF_BGRA :                          return Ogre::PF_BYTE_BGRA;
            case IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE :
            case IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE :  return Ogre::PF_L8;
            case IMAGE::PSF_SINGLE_CHANNEL_RED :            return Ogre::PF_R8;
            case IMAGE::PSF_SINGLE_CHANNEL_ALPHA :          return Ogre::PF_A8;
            case IMAGE::PSF_LUMINANCE_ALPHA :               return Ogre::PF_BYTE_LA;
            default :                                       return Ogre::PF_UNKNOWN;
        }
    }
    if ( uint16Type == componentDataType )
    {
        switch ( storageFormat )
        {
            case IMAGE::PSF_RGB :                           return Ogre::PF_SHORT_RGB;
            case IMAGE::PSF_RGBA :                          return Ogre::PF_SHORT_RGBA;
            case IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE :
            case IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE :  return Ogre::PF_L16;
            default :                                       return Ogre::PF_UNKNOWN;
        }
    }
    if ( float32Type == componentDataType )
    {
        switch ( storageFormat )
        {
            case IMAGE::PSF_RGB :                           return Ogre::PF_FLOAT32_RGB;
            case IMAGE::PSF_RGBA :                          return Ogre::PF_FLOAT32_RGBA;
            case IMAGE::PSF_SINGLE_CHANNEL_RED :
            case IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE :
            case IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE :  return Ogre::PF_FLOAT32_R;
            default :                                       return Ogre::PF_UNKNOWN;
        }
    }
    return Ogre::PF_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
CGucefImageConverter::FromOgrePixelFormat( Ogre::PixelFormat format                  ,
                                           IMAGE::TPixelStorageFormat& storageFormat ,
                                           TBuildinDataType& componentDataType       )
{GUCEF_TRACE;

    componentDataType = DATATYPE_UINT8;
    switch ( format )
    {
        case Ogre::PF_BYTE_RGB :    { storageFormat = IMAGE::PSF_RGB; return true; }
        case Ogre::PF_BYTE_RGBA :   { storageFormat = IMAGE::PSF_RGBA; return true; }
        case Ogre::PF_BYTE_BGR :    { storageFormat = IMAGE::PSF_BGR; return true; }
        case Ogre::PF_BYTE_BGRA :   { storageFormat = IMAGE::PSF_BGRA; return true; }
        case Ogre::PF_L8 :          { storageFormat = IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE; return true; }
        case Ogre::PF_R8 :          { storageFormat = IMAGE::PSF_SINGLE_CHANNEL_RED; return true; }
        case Ogre::PF_A8 :          { storageFormat = IMAGE::PSF_SINGLE_CHANNEL_ALPHA; return true; }
        case Ogre::PF_BYTE_LA :     { storageFormat = IMAGE::PSF_LUMINANCE_ALPHA; return true; }
        default :                   { return false; }
    }
}

/*-------------------------------------------------------------------------*/

bool
CGucefImageConverter::ToOgreImage( const IMAGE::CImage& source ,
                                   Ogre::Image& target         ,
                                   Ogre::String& problem       )
{GUCEF_TRACE;

    const UInt32 frameCount = source.GetFrameCount();
    if ( 0 == frameCount )
    {
        problem = "the decoded image has no frames";
        return false;
    }

    // Ogre has one face or six, six frames of an animation are not the six faces of a cube
    const bool isCubeMap = 6 == frameCount && !source.IsAnimation();
    const UInt32 faceCount = isCubeMap ? 6 : 1;
    if ( frameCount > 1 && !isCubeMap )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CGucefImageConverter: image has " << frameCount << " frames, the first one is used" );
    }

    IMAGE::TPixelMapPtr base;
    if ( !source.TryGetPixelMap( 0, 0, base ) || base.IsNULL() )
    {
        problem = "the decoded image has no pixels";
        return false;
    }
    if ( IMAGE::PSF_PALETTE_INDICES == base->GetPixelStorageFormat() )
    {
        problem = "the decoded image holds palette indices, the palette is not part of the pixel map";
        return false;
    }
    const UInt32 width = base->GetWidthInPixels();
    const UInt32 height = base->GetHeightInPixels();
    if ( 0 == width || 0 == height )
    {
        problem = "the decoded image is empty";
        return false;
    }

    // the format of the first level is the format of all, formats Ogre has none for become 8 bit RGBA
    IMAGE::TPixelStorageFormat storageFormat = base->GetPixelStorageFormat();
    TBuildinDataType dataType = base->GetPixelComponentDataType();
    Ogre::PixelFormat ogreFormat = ToOgrePixelFormat( storageFormat, dataType );
    if ( Ogre::PF_UNKNOWN == ogreFormat )
    {
        storageFormat = IMAGE::PSF_RGBA;
        dataType = DATATYPE_UINT8;
        ogreFormat = Ogre::PF_BYTE_RGBA;
    }

    // the levels every face has, as far as they are the halved size of the one before
    UInt32 levelCount = source.GetMipmapLevels( 0 );
    for ( UInt32 face=1; face<faceCount; ++face )
    {
        const UInt32 faceLevels = source.GetMipmapLevels( face );
        if ( faceLevels < levelCount )
            levelCount = faceLevels;
    }
    if ( 0 == levelCount )
        levelCount = 1;

    UInt32 validLevels = 1;
    for ( UInt32 level=1; level<levelCount; ++level )
    {
        const UInt32 expectedWidth = ( width >> level ) > 0 ? ( width >> level ) : 1;
        const UInt32 expectedHeight = ( height >> level ) > 0 ? ( height >> level ) : 1;
        bool levelIsValid = true;
        for ( UInt32 face=0; face<faceCount && levelIsValid; ++face )
        {
            IMAGE::TPixelMapPtr levelMap;
            levelIsValid = source.TryGetPixelMap( face, level, levelMap ) && !levelMap.IsNULL() &&
                           levelMap->GetWidthInPixels() == expectedWidth && levelMap->GetHeightInPixels() == expectedHeight;
        }
        if ( !levelIsValid )
            break;
        validLevels = level + 1;
    }

    // every face and level in the one format
    GUCEF::vector< IMAGE::TPixelMapPtr > maps;
    maps.reserve( faceCount * validLevels );
    for ( UInt32 face=0; face<faceCount; ++face )
    {
        for ( UInt32 level=0; level<validLevels; ++level )
        {
            IMAGE::TPixelMapPtr map;
            if ( !source.TryGetPixelMap( face, level, map ) || map.IsNULL() )
            {
                problem = "a level of the decoded image has no pixels";
                return false;
            }
            if ( map->GetPixelStorageFormat() != storageFormat || map->GetPixelComponentDataType() != dataType )
            {
                IMAGE::TPixelMapPtr converted;
                if ( !map->ConvertFormatTo( storageFormat, dataType, converted ) || converted.IsNULL() )
                {
                    problem = "a level of the decoded image could not be converted to the pixel format of its first level";
                    return false;
                }
                map = converted;
            }
            maps.push_back( map );
        }
    }

    const size_t pixelSize = Ogre::PixelUtil::getNumElemBytes( ogreFormat );
    target.create( ogreFormat, width, height, 1, faceCount, validLevels - 1 );

    // Ogre stores a face with its levels one after the other and then the next face: the box of every
    // face and level knows where its pixels go
    for ( UInt32 face=0; face<faceCount; ++face )
    {
        for ( UInt32 level=0; level<validLevels; ++level )
        {
            const IMAGE::TPixelMapPtr& map = maps[ (size_t) face * validLevels + level ];
            const Ogre::PixelBox box = target.getPixelBox( face, level );
            if ( box.getWidth() != map->GetWidthInPixels() || box.getHeight() != map->GetHeightInPixels() ||
                 map->GetSizeOfPixelInBytes() != pixelSize )
            {
                problem = "a level of the decoded image does not have the size of the Ogre image";
                return false;
            }

            const size_t rowBytes = (size_t) box.getWidth() * pixelSize;
            const size_t pitchBytes = (size_t) box.rowPitch * pixelSize;
            Ogre::uchar* destination = static_cast< Ogre::uchar* >( box.data );
            for ( UInt32 row=0; row<map->GetHeightInPixels(); ++row )
            {
                memcpy( destination + row * pitchBytes, map->GetDataAtScanLine( row ), rowBytes );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefImageConverter::FromOgreImage( const Ogre::Image& source ,
                                     IMAGE::CImage& target     ,
                                     Ogre::String& problem     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == source.getData() || 0 == source.getWidth() || 0 == source.getHeight() )
    {
        problem = "the Ogre image has no pixels";
        return false;
    }

    Ogre::PixelBox box = source.getPixelBox( 0, 0 );
    Ogre::Image converted;
    IMAGE::TPixelStorageFormat storageFormat = IMAGE::PSF_UNKNOWN;
    TBuildinDataType dataType = DATATYPE_UINT8;
    if ( !FromOgrePixelFormat( box.format, storageFormat, dataType ) )
    {
        // formats of gucefIMAGE are the byte formats: everything else that can be read is written as 8 bit RGBA
        if ( Ogre::PixelUtil::isCompressed( box.format ) || !Ogre::PixelUtil::isAccessible( box.format ) )
        {
            problem = "the pixel format of the Ogre image cannot be converted";
            return false;
        }
        converted.create( Ogre::PF_BYTE_RGBA, box.getWidth(), box.getHeight() );
        Ogre::PixelBox destination = converted.getPixelBox();
        Ogre::PixelUtil::bulkPixelConversion( box, destination );
        box = converted.getPixelBox();
        storageFormat = IMAGE::PSF_RGBA;
        dataType = DATATYPE_UINT8;
    }

    // the rows of the image go together without padding, the pixel map keeps them that way
    const size_t pixelSize = Ogre::PixelUtil::getNumElemBytes( box.format );
    const size_t rowBytes = (size_t) box.getWidth() * pixelSize;
    GUCEF::vector< Ogre::uchar > packed;
    packed.resize( rowBytes * box.getHeight() );
    const Ogre::uchar* sourceData = static_cast< const Ogre::uchar* >( box.data );
    for ( size_t row=0; row<box.getHeight(); ++row )
    {
        memcpy( &packed[ row * rowBytes ], sourceData + row * (size_t) box.rowPitch * pixelSize, rowBytes );
    }

    IMAGE::TPixelMapPtr map( GUCEF_NEW IMAGE::CPixelMap( &packed[ 0 ], box.getWidth(), box.getHeight(), storageFormat, dataType ) );
    target.Assign( map );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
