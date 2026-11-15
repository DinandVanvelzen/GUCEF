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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#include "AssetViewerLib_CAssetTypeResolver.h"
#define ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#endif /* ASSETVIEWERLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETVIEWERLIB_CBLOCKDECODER_H
#include "AssetViewerLib_CBlockDecoder.h"
#define ASSETVIEWERLIB_CBLOCKDECODER_H
#endif /* ASSETVIEWERLIB_CBLOCKDECODER_H ? */

#ifndef ASSETVIEWERLIB_CMESHINSPECTOR_H
#include "AssetViewerLib_CMeshInspector.h"
#define ASSETVIEWERLIB_CMESHINSPECTOR_H
#endif /* ASSETVIEWERLIB_CMESHINSPECTOR_H ? */

#include "AssetViewerLib_CTextureView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTextureView::SInfo::SInfo( void )
    : path()
    , formatName()
    , width( 0 )
    , height( 0 )
    , depth( 1 )
    , mipLevels( 0 )
    , faces( 1 )
    , memoryBytes( 0 )
    , isCompressed( false )
    , hasAlpha( false )
    , isCube( false )
    , displayable( false )
    , note()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTextureView::CTextureView( void )
    : m_info()
    , m_image()
    , m_loaded( false )
    , m_hasImage( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTextureView::~CTextureView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTextureView::Unload( void )
{GUCEF_TRACE;

    m_image.Assign( IMAGE::CImage::TFrameList() );
    m_info = SInfo();
    m_loaded = false;
    m_hasImage = false;
}

/*-------------------------------------------------------------------------*/

bool
CTextureView::IsLoaded( void ) const
{GUCEF_TRACE;

    return m_loaded;
}

/*-------------------------------------------------------------------------*/

IMAGE::CImage*
CTextureView::GetImage( void )
{GUCEF_TRACE;

    return m_hasImage ? &m_image : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CTextureView::SInfo&
CTextureView::GetInfo( void ) const
{GUCEF_TRACE;

    return m_info;
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
CTextureView::GetFirstPixelMap( void ) const
{GUCEF_TRACE;

    if ( !m_hasImage || 0 == m_image.GetFrameCount() || 0 == m_image.GetMipmapLevels( 0 ) )
        return IMAGE::TPixelMapPtr();
    return m_image.GetPixelMap( 0, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CTextureView::Load( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    Unload();
    m_info.path = vfsPath;

    CORE::CDynamicBuffer data;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( data, vfsPath, "rb" ) || 0 == data.GetDataSize() )
    {
        m_info.note = "The file cannot be read";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CTextureView: " << vfsPath << " cannot be read" );
        return false;
    }

    const CORE::CString extension = CAssetTypeResolver::GetExtension( vfsPath );
    try
    {
        Ogre::DataStreamPtr stream( OGRE_NEW Ogre::MemoryDataStream( data.GetBufferPtr(), data.GetDataSize(), false, true ) );
        Ogre::Image image;
        image.load( stream, extension.STL_String() );
        m_info.path = vfsPath;
        return Convert( image );
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CTextureView: Ogre has no decoder for " << vfsPath << " (" << e.getDescription() << "), trying the image codecs of gucefIMAGE" );
    }

    // the codecs of gucefIMAGE
    CORE::CDynamicBufferAccess access( data );
    if ( m_image.Load( access, extension ) && m_image.GetFrameCount() > 0 )
    {
        IMAGE::TPixelMapPtr first = m_image.GetPixelMap( 0, 0 );
        m_info.width = first->GetWidthInPixels();
        m_info.height = first->GetHeightInPixels();
        m_info.mipLevels = m_image.GetMipmapLevels( 0 );
        m_info.faces = m_image.GetFrameCount();
        m_info.formatName = "gucefIMAGE";
        m_info.memoryBytes = m_image.GetTotalPixelStorageSize();
        m_info.hasAlpha = IMAGE::PSF_RGBA == first->GetPixelStorageFormat() || IMAGE::PSF_BGRA == first->GetPixelStorageFormat();
        m_info.displayable = true;
        m_hasImage = true;
        m_loaded = true;
        return true;
    }
    m_info.note = "No codec reads this file";
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CTextureView: No image codec of Ogre or gucefIMAGE reads " << vfsPath );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTextureView::SetImage( const Ogre::Image& image, const CORE::CString& name )
{GUCEF_TRACE;

    Unload();
    m_info.path = name;
    return Convert( image );
}

/*-------------------------------------------------------------------------*/

bool
CTextureView::Convert( const Ogre::Image& image )
{GUCEF_TRACE;

    const Ogre::PixelFormat format = image.getFormat();
    m_info.formatName = Ogre::PixelUtil::getFormatName( format );
    m_info.width = image.getWidth();
    m_info.height = image.getHeight();
    m_info.depth = image.getDepth();
    m_info.mipLevels = image.getNumMipmaps() + 1;
    m_info.faces = image.getNumFaces();
    m_info.isCube = image.hasFlag( Ogre::IF_CUBEMAP );
    m_info.memoryBytes = image.getSize();
    m_info.isCompressed = Ogre::PixelUtil::isCompressed( format );
    m_info.hasAlpha = Ogre::PixelUtil::hasAlpha( format );
    m_loaded = true;

    if ( m_info.isCompressed && !CBlockDecoder::CanDecode( format ) )
    {
        m_info.note = "The format " + m_info.formatName + " is not decoded for display";
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CTextureView: " << m_info.path << ": " << m_info.note );
        return true;
    }

    try
    {
        IMAGE::CImage::TFrameList frames;
        const bool slices = !m_info.isCube && m_info.depth > 1;
        const UInt32 frameCount = slices ? m_info.depth : m_info.faces;
        for ( UInt32 f=0; f<frameCount; ++f )
        {
            IMAGE::CImage::TMipMapList levels;
            const UInt32 levelCount = slices ? 1 : m_info.mipLevels;
            for ( UInt32 m=0; m<levelCount; ++m )
            {
                Ogre::PixelBox source = image.getPixelBox( slices ? 0 : f, m );
                if ( slices )
                    source = source.getSubVolume( Ogre::Box( 0, 0, f, source.getWidth(), source.getHeight(), f+1 ) );
                const UInt32 width = (UInt32) source.getWidth();
                const UInt32 height = (UInt32) source.getHeight();
                IMAGE::TPixelMapPtr map( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
                if ( m_info.isCompressed )
                {
                    if ( !CBlockDecoder::Decode( format, source.data, width, height, static_cast< UInt8* >( map->GetDataPtr() ) ) )
                        return true;
                }
                else
                {
                    Ogre::PixelBox target( width, height, 1, Ogre::PF_BYTE_RGBA, map->GetDataPtr() );
                    Ogre::PixelUtil::bulkPixelConversion( source, target );
                }
                levels.push_back( map );
            }
            frames.push_back( levels );
        }
        m_image.Assign( frames );
        m_hasImage = true;
        m_info.displayable = true;
        return true;
    }
    catch ( const Ogre::Exception& e )
    {
        m_info.note = "The pixels cannot be converted: " + CORE::CString( e.getDescription() );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CTextureView: " << m_info.path << ": " << m_info.note );
        return true;
    }
}

/*-------------------------------------------------------------------------*/

void
CTextureView::WriteInspectorData( CORE::CDataNode& root ) const
{GUCEF_TRACE;

    root.SetName( "Texture" );
    root.SetAttribute( "Name", m_info.path );
    CORE::CDataNode* image = root.AddChild( "Image" );
    image->SetAttribute( "Format", m_info.formatName );
    image->SetAttribute( "Width", CORE::UInt32ToString( m_info.width ) );
    image->SetAttribute( "Height", CORE::UInt32ToString( m_info.height ) );
    if ( m_info.depth > 1 )
        image->SetAttribute( "Depth", CORE::UInt32ToString( m_info.depth ) );
    image->SetAttribute( "MipLevels", CORE::UInt32ToString( m_info.mipLevels ) );
    image->SetAttribute( "Faces", CORE::UInt32ToString( m_info.faces ) );
    image->SetAttribute( "Memory", CMeshInspector::FormatByteCount( m_info.memoryBytes ) );
    image->SetAttribute( "Compressed", m_info.isCompressed ? "yes" : "no" );
    image->SetAttribute( "Alpha", m_info.hasAlpha ? "yes" : "no" );
    image->SetAttribute( "Cube", m_info.isCube ? "yes" : "no" );
    if ( !m_info.note.IsNULLOrEmpty() )
        image->SetAttribute( "Note", m_info.note );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
