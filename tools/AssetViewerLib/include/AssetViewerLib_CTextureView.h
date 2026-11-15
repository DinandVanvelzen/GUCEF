/*
 *  AssetViewerLib: the texture view: a picture of any format Ogre or gucefIMAGE reads, as an image for the image view and the facts for the inspector
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

#ifndef ASSETVIEWERLIB_CTEXTUREVIEW_H
#define ASSETVIEWERLIB_CTEXTUREVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <OgreImage.h>

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A picture from the VFS for the image view. The file is decoded by the image codecs of Ogre (DDS, KTX, PVR, ETC and what
 *  its image plugins and OgreCodec_GucefIMAGE add), or by the gucefIMAGE codecs when Ogre has none for it. The result is an
 *  IMAGE::CImage of 8 bit RGBA pixel maps with every mip level of every face the file has (cube faces, and the slices of
 *  a 3D texture as frames), and the facts of the file: format, size, mip levels, faces and the memory it takes.
 *
 *  Block compressed data of the formats BC1 to BC5 is decoded with CBlockDecoder; other compressed formats (BC6H, BC7, ETC,
 *  ASTC, PVRTC) give the facts but no picture (Displayable() is false and Note() says why).
 */
class ASSETVIEWERLIB_PUBLIC_CPP CTextureView
{
    public:

    struct SInfo
    {
        CORE::CString path;
        CORE::CString formatName;       /**< the pixel format as the codec has it ("PF_R8G8B8A8", "PF_DXT1") */
        UInt32 width;
        UInt32 height;
        UInt32 depth;
        UInt32 mipLevels;
        UInt32 faces;                   /**< 6 for a cube map */
        UInt64 memoryBytes;             /**< what the data of the file takes, all levels and faces */
        bool isCompressed;
        bool hasAlpha;
        bool isCube;
        bool displayable;
        CORE::CString note;
        SInfo( void );
    };

    CTextureView( void );

    ~CTextureView();

    /** Reads and decodes a picture of the VFS; false with a reason in Note() when that is not possible */
    bool Load( const CORE::CString& vfsPath );

    /** Takes the picture of an Ogre image (the same conversion as Load() makes); used by the thumbnails and the tests */
    bool SetImage( const Ogre::Image& image, const CORE::CString& name );

    void Unload( void );

    bool IsLoaded( void ) const;

    /** The picture for CImageViewForm, which does not own it; NULL when nothing is displayable */
    IMAGE::CImage* GetImage( void );

    const SInfo& GetInfo( void ) const;

    /** The facts as the tree of the property inspector, group Texture */
    void WriteInspectorData( CORE::CDataNode& root ) const;

    /** The first level of the first face as a pixel map, NULL when there is none */
    IMAGE::TPixelMapPtr GetFirstPixelMap( void ) const;

    private:

    CTextureView( const CTextureView& src );
    CTextureView& operator=( const CTextureView& src );

    bool Convert( const Ogre::Image& image );

    private:

    SInfo m_info;
    IMAGE::CImage m_image;
    bool m_loaded;
    bool m_hasImage;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CTEXTUREVIEW_H ? */
