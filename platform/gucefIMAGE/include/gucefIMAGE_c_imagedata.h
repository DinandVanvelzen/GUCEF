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

#ifndef GUCEF_IMAGE_IMAGEDATA_H
#define GUCEF_IMAGE_IMAGEDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_ETYPES_H
#include "gucefIMAGE_ETypes.h"      /* simple types used */
#define GUCEF_IMAGE_ETYPES_H
#endif /* GUCEF_IMAGE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace IMAGE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The following are a number of values that you should always assign to the
 *  matching field of the struct mentioned. This will allow checking of the
 *  linked type definition versus the runtime passed object
 */
#define GUCEF_IMAGE_TIMAGE_VERSION                  1
#define GUCEF_IMAGE_TIMAGEINFO_VERSION              2
#define GUCEF_IMAGE_TIMAGEFRAME_VERSION             1
#define GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION         2
#define GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION       1
#define GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION   1

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Image Storage formats
 */
enum EPixelStorageFormat
{
    PSF_UNKNOWN = 0                  ,

    PSF_RGB = 1                      ,
    PSF_RGBA                         ,
    PSF_BGR                          ,
    PSF_BGRA                         , 
    PSF_HSV                          , /* aka HSB */
    PSF_HSVA                         , /* aka HSBA */
    PSF_HSL                          ,
    PSF_HSLA                         ,
    PSF_SINGLE_CHANNEL_GRAYSCALE     ,
    PSF_SINGLE_CHANNEL_RED           ,
    PSF_SINGLE_CHANNEL_GREEN         ,
    PSF_SINGLE_CHANNEL_BLUE          ,
    PSF_SINGLE_CHANNEL_ALPHA         ,
    PSF_SINGLE_CHANNEL_STD_LUMINANCE ,
    PSF_SINGLE_CHANNEL_P1_LUMINANCE  ,
    PSF_SINGLE_CHANNEL_P2_LUMINANCE  ,
    PSF_LUMINANCE_ALPHA              ,
    PSF_PALETTE_INDICES              ,
    PSF_PERCEIVED_BRIGHTNESS         ,

    PSF_COUNT
};
typedef enum EPixelStorageFormat TPixelStorageFormat;

/*-------------------------------------------------------------------------*/

/**
 *      structure for image information
 */
struct SImageInfo
{
    UInt32 version;                 /**< version of the TImageInfo definition */
    UInt32 nrOfFramesInImage;       /**< total number of frames in the image */
    UInt32 loopCount;               /**< animations only: nr of times the frame sequence is played, 0 = loop forever. Added in version 2 */
};

typedef struct SImageInfo TImageInfo;

/*-------------------------------------------------------------------------*/

/**
 *      structure for image frame information
 */
struct SImageFrameInfo
{
    UInt32 version;                 /**< version of the TImageInfo definition */
    UInt32 nrOfMipmapLevels;        /**< total number of mimmap levels in the image */
    UInt32 frameDurationInMs;       /**< animations only: how long the frame is shown in milliseconds, 0 = unspecified. Added in version 2 */
};

typedef struct SImageFrameInfo TImageFrameInfo;

/*-------------------------------------------------------------------------*/

/**
 *      structure for image frame mipmap level information
 */
struct SImageMipMapLevelInfo
{
    UInt32 version;                    /**< version of the TImageInfo definition */
    Int32 pixelComponentDataType;      /**< data type of each value of idata */
    UInt32 frameWidth;                 /**< Width of an image frame in pixels, mipmap level 0 */
    UInt32 frameHeight;                /**< Height of an image frame in pixels, mipmap level 0 */
    Int32 pixelStorageFormat;          /**< format in which the pixel components are stored */
};

typedef struct SImageMipMapLevelInfo TImageMipMapLevelInfo;

/*-------------------------------------------------------------------------*/

/**
 *      Storage structure for an image frame's mipmap level
 */
struct SImageMipMapLevel
{
    UInt32 version;                     /**< version of the TImageInfo definition */
    TImageMipMapLevelInfo mipLevelInfo; /**< information about the mipmap level and it's data */
    void* pixelData;                    /**< array with the actual pixel data */
    UInt32 pixelDataSizeInBytes;        /**< size of the pixel data buffer in bytes */
};

typedef struct SImageMipMapLevel TImageMipMapLevel;

/*-------------------------------------------------------------------------*/

/**
 *      Storage structure for an image frame
 */
struct SImageFrame
{
    UInt32 version;                  /**< version of the TImageFrame definition */
    TImageFrameInfo frameInfo;       /**< information about the frame */
    TImageMipMapLevel* mipmapLevel;  /**< array with mipmap level info + data */
};

typedef struct SImageFrame TImageFrame;

/*-------------------------------------------------------------------------*/

/**
 *      Storage structure for an image
 */
struct SImage
{
    UInt32 version;             /**< version of the TImage definition */
    TImageInfo imageInfo;       /**< basic image information */
    TImageFrame* frames;        /**< array of image frames */
};

typedef struct SImage TImage;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace IMAGE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_IMAGEDATA_H ? */
