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

#ifndef GUCEF_IMAGE_CPNGIMAGECODEC_H
#define GUCEF_IMAGE_CPNGIMAGECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_IMAGE_CIIMAGECODEC_H
#include "gucefIMAGE_CIImageCodec.h"
#define GUCEF_IMAGE_CIIMAGECODEC_H
#endif /* GUCEF_IMAGE_CIIMAGECODEC_H ? */

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

/**
 *  Built-in PNG encoder, image codec "png" of the codec family "ImageCodec". It needs no image library.
 *
 *  Writes non-interlaced PNG with 8 bits per channel and filter type 0 on every line, from pixel maps with
 *  8 bit channels: RGBA and BGRA as colour type 6, RGB and BGR as colour type 2, single channel greyscale as
 *  colour type 0. Other formats are refused with a reason, not converted. The first line of the pixel map is
 *  the top line of the picture.
 *
 *  The image data is compressed with the "zlib" codec of the family CORE::CoreCodecTypes::CompressionCodec in
 *  the codec registry when there is one (codecspluginZLIB), else it is written as stored deflate blocks, which
 *  every PNG reader takes.
 *
 *  The module registers an instance in the codec registry of gucefCORE with CCodecRegistry::SoftwareCodecPriority,
 *  so a "png" codec of an image plugin takes its place while that plugin is loaded. It is not registered in the
 *  image codec registry. Decoding is not supported: Decode answers false.
 *
 *  The codec has no state: its members may be called from several threads at once.
 */
class GUCEF_IMAGE_EXPORT_CPP CPngImageCodec : public CIImageCodec
{
    public:

    static const CString CodecFamilyName;           /**< "ImageCodec" */
    static const CString CodecTypeName;             /**< "png" */
    static const CString CompressionCodecTypeName;  /**< "zlib": the codec of family CORE::CoreCodecTypes::CompressionCodec that is used */

    CPngImageCodec( void );

    CPngImageCodec( const CPngImageCodec& src );

    virtual ~CPngImageCodec() GUCEF_VIRTUAL_OVERRIDE;

    CPngImageCodec& operator=( const CPngImageCodec& src );

    /**
     *  The PNG of the pixel map in 'png', replacing its content.
     *  False with the reason in 'problem' when the pixel map cannot be written; 'png' is empty then
     */
    static bool EncodePixelMap( const CPixelMap& pixels     ,
                                CORE::CDynamicBuffer& png   ,
                                CString& problem            );

    /** The same as EncodePixelMap(), written to a file, replacing it */
    static bool WritePixelMapToFile( const CPixelMap& pixels   ,
                                     const CString& path       ,
                                     CString& problem          );

    /** The CRC-32 of PNG chunks (ISO 3309), over 'size' bytes, continuing from 'crc' (0 to start) */
    static UInt32 Crc32( const UInt8* data      ,
                         UInt32 size            ,
                         UInt32 crc = 0         );

    /** Whether the pixel map can be written as PNG, false with the reason in 'problem' */
    static bool IsPixelMapSupported( const CPixelMap& pixels ,
                                     CString& problem        );

    /** Writes the first mipmap level of the first frame of the image */
    virtual bool Encode( const CImage& inputImage       ,
                         CORE::CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: false */
    virtual bool Decode( CORE::CIOAccess& encodedInput ,
                         CImage& outputImage           ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: false */
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput ,
                                 CORE::CValueList& metaData    ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Encodes the serialized image that CIMGCodec makes (TImageInfo, the frame and mipmap level infos, the pixel data)
     *  from the current position of 'source': the first mipmap level of the first frame
     */
    virtual bool Encode( CORE::CIOAccess& source ,
                         CORE::CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: false */
    virtual bool Decode( CORE::CIOAccess& source ,
                         CORE::CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    /** As Encode( source, dest ), the codec takes no parameters; the outcome is recorded for GetLastStatus() */
    virtual bool Encode( CORE::CIOAccess& source          ,
                         CORE::CIOAccess& dest            ,
                         const CORE::CValueList& params   ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: false with GUCEF_CODEC_STATUS_UNSUPPORTED */
    virtual bool Decode( CORE::CIOAccess& source          ,
                         CORE::CIOAccess& dest            ,
                         const CORE::CValueList& params   ) GUCEF_VIRTUAL_OVERRIDE;

    /** GUCEF_CODEC_CAPABILITY_ENCODE */
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Encodes the serialized image of CIMGCodec, false with the reason in 'problem' */
    static bool EncodeSerializedImage( CORE::CIOAccess& source ,
                                       CORE::CIOAccess& dest   ,
                                       CString& problem        );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CPNGIMAGECODEC_H ? */
