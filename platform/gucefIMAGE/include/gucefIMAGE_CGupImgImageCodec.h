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

#ifndef GUCEF_IMAGE_CGUPIMGIMAGECODEC_H
#define GUCEF_IMAGE_CGUPIMGIMAGECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CIIMAGECODEC_H
#include "gucefIMAGE_CIImageCodec.h"
#define GUCEF_IMAGE_CIIMAGECODEC_H
#endif /* GUCEF_IMAGE_CIIMAGECODEC_H ? */

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
 *  The native image file format of the framework, extension "gupimg". It stores what a CImage holds, whatever it is:
 *  frames with their durations, the loop count, mipmap levels, any pixel storage format and component data type,
 *  and the meta-data of the image.
 *
 *  Layout, all integers little-endian:
 *
 *    header (32 bytes, or the header size it states)
 *      8 bytes   signature  47 55 50 49 4D 47 1A 0A  ("GUPIMG", ^Z, LF)
 *      u16       format version, 1
 *      u16       header size in bytes, 32. A larger value skips the extra bytes: later versions append fields
 *      u32       flags: bit 0 = the level CRC-32 fields are filled, bit 1 = the rows run bottom to top.
 *                Other bits are refused: they ask for something this version does not know
 *      u32       frame count
 *      u32       loop count (0 = forever)
 *      u32       size of the table section in bytes
 *      u32       size of the meta-data section in bytes
 *    table section
 *      per frame:  u32 duration in milliseconds, u32 number of mipmap levels
 *      then per level, frame by frame, level 0 first:
 *        u32 width, u32 height, i32 pixel storage format (EPixelStorageFormat), i32 component data type (GUCEF_DATATYPE_*),
 *        u32 level flags (bit 0 = the float components are normalized as percentage), u32 size of the pixels in bytes,
 *        u32 size as stored, u64 offset of the stored bytes in the data section, u32 CRC-32 of the stored bytes,
 *        u16 length of the compression codec name, that many bytes of name
 *    meta-data section (when its size is not 0)
 *      u32 number of keys, per key: u32 length, key bytes, u32 number of values, per value: u32 length, value bytes (text)
 *    data section
 *      the stored bytes of every level, in the order of the table, back to back
 *
 *  Compression is per level and by name: the name is that of a codec of the CORE::CoreCodecTypes::CompressionCodec family of
 *  the codec registry ("zlib" is the default), an empty name means the bytes are stored as they are. The writer is free to
 *  store a level uncompressed when compressing does not make it smaller. Any codec registered under a name can be used
 *  by a file, the format does not know the names. A reader that does not have the codec a level names refuses the file
 *  and says which codec it misses.
 *
 *  The pixels are the pixel map as it is: the first row is the top row, unless flag bit 1 says otherwise, in which case
 *  a reader turns the rows over.
 *
 *  Parameters of the writer (CIImageCodec::Encode with parameters):
 *    "compression"   name of the compression codec, "none" for none. Without the parameter "zlib" is used when it is
 *                    there and the levels are stored as they are when it is not
 *    "checksums"     boolean, default true: a CRC-32 per level that readers verify
 *
 *  The codec has no state: its members may be called from several threads at once.
 */
class GUCEF_IMAGE_EXPORT_CPP CGupImgImageCodec : public CIImageCodec
{
    public:

    static const CString CodecFamilyName;               /**< "ImageCodec" */
    static const CString CodecTypeName;                 /**< "gupimg" */
    static const CString DefaultCompressionCodecName;   /**< "zlib" */
    static const CString CompressionParamName;          /**< "compression" */
    static const CString ChecksumsParamName;            /**< "checksums" */
    static const CString NoCompressionName;             /**< "none" */

    /** The version of the format this codec writes and the newest it reads */
    static const UInt16 FormatVersion;

    CGupImgImageCodec( void );

    CGupImgImageCodec( const CGupImgImageCodec& src );

    virtual ~CGupImgImageCodec() GUCEF_VIRTUAL_OVERRIDE;

    CGupImgImageCodec& operator=( const CGupImgImageCodec& src );

    /** Writes all frames and levels of the image with the default settings */
    virtual bool Encode( const CImage& inputImage       ,
                         CORE::CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE;

    /** Writes all frames and levels of the image, see the parameters of the writer */
    virtual bool Encode( const CImage& inputImage       ,
                         CORE::CIOAccess& encodedOutput ,
                         const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Reads all frames and levels from the current position of the input into the image, which is only touched
     *  when all of it could be read. The meta-data of the image is not set, see DecodeMetaData()
     */
    virtual bool Decode( CORE::CIOAccess& encodedInput ,
                         CImage& outputImage           ) GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the header, the tables and the meta-data of the file: no pixels are read or decompressed */
    virtual bool DecodeMetaData( CORE::CIOAccess& encodedInput ,
                                 CORE::CValueList& metaData    ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: the codec works on images, false */
    virtual bool Encode( CORE::CIOAccess& source ,
                         CORE::CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    /** Not supported: the codec works on images, false */
    virtual bool Decode( CORE::CIOAccess& source ,
                         CORE::CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    /** GUCEF_CODEC_CAPABILITY_ENCODE and GUCEF_CODEC_CAPABILITY_DECODE */
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** IMAGECAPABILITY_MULTIPLE_FRAMES and IMAGECAPABILITY_MIP_LEVELS */
    virtual UInt32 GetImageCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CGUPIMGIMAGECODEC_H ? */
