/*
 *  guidriverCEGUI: glue module for the CEGUI GUI backend
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H 
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#ifndef GUCEF_GUIDRIVERCEGUI_MACROS_H
#include "guidriverCEGUI_macros.h" 
#define GUCEF_GUIDRIVERCEGUI_MACROS_H
#endif /* GUCEF_GUIDRIVERCEGUI_MACROS_H ? */

#include "guidriverCEGUI_ImageCodecAdapter.h" 

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

ImageCodecAdapter::ImageCodecAdapter( VfsResourceProvider* vfsResourceProvider )
    : ::CEGUI::ImageCodec( "gucefIMAGE" ) ,
      m_codecName( "gucefIMAGE" )         ,
      m_vfsResourceProvider( vfsResourceProvider )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ImageCodecAdapter::~ImageCodecAdapter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CEGUI::String& 
ImageCodecAdapter::getIdentifierString() const
{GUCEF_TRACE;

    return m_codecName;
}

/*-------------------------------------------------------------------------*/

const CEGUI::String&
ImageCodecAdapter::getSupportedFormat() const
{GUCEF_TRACE;

    IMAGE::CImageCodecRegistry& codecRegistry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
    
    CORE::CString::StringVector codecList;
    codecRegistry.GetRegisteredObjNames( codecList );
    
    static CEGUI::String formatStr;

    formatStr.clear();
    CORE::CString::StringVector::iterator i = codecList.begin();
    while ( i != codecList.end() )
    {
        if ( !formatStr.empty() )
            formatStr += " ";
        formatStr += (*i).STL_String();
        ++i;
    }
    return formatStr;
}

/*-------------------------------------------------------------------------*/

bool
ImageCodecAdapter::TryConvertPixelFormat( IMAGE::TPixelStorageFormat pixelFormat , 
                                          CORE::UInt32 channelSize               , 
                                          CEGUI::Texture::PixelFormat& outFormat )
{GUCEF_TRACE;

    if ( 1 == channelSize )
    {
        switch ( pixelFormat )
        {
            case IMAGE::PSF_RGB : 
            { 
                outFormat = CEGUI::Texture::PF_RGB;
                return true;
            }
            case IMAGE::PSF_RGBA : 
            {
                outFormat = CEGUI::Texture::PF_RGBA;
                return true;
            }
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CEGUI::Texture* 
ImageCodecAdapter::load( const CEGUI::RawDataContainer& data , 
                         CEGUI::Texture* result              )
{GUCEF_TRACE;

    if ( NULL == result ) 
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: Unable to load texture since no texture object was provided" );
        return NULL;
    }

    CORE::CString imgType = CORE::ExtractFileExtention( result->getName() );
    if ( imgType.IsNULLOrEmpty() )
    {
        if ( NULL != m_vfsResourceProvider )
        {
            VfsResourceProvider::DataContainerInfoPtr containerInfo = m_vfsResourceProvider->GetInfoOnLoadedContainer( data );
            if ( !containerInfo.IsNULL() )
            {
                imgType = CORE::ExtractFileExtention( containerInfo->requestFilename );
            }
        }
    }

    if ( imgType.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: Unable to load texture since no image type could be derived from the the texture name" );
        return NULL;
    }

    IMAGE::CImageCodecRegistry& codecRegistry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
    
    IMAGE::CImageCodecRegistry::TImageCodecPtr codec;
    if ( !codecRegistry.TryLookup( imgType, codec ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: Unable to load texture since no image codec could be found for type: " + imgType );
        return NULL;
    }

    CORE::CDynamicBufferAccess bufferAccess;
    bufferAccess.LinkTo( data.getDataPtr(), (CORE::UInt32) data.getSize() );

    IMAGE::CImage image;
    if ( !codec->Decode( bufferAccess, image ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: Unable to load texture since the image codec decoding failed for type: " + imgType );
        return NULL;
    }

    IMAGE::TPixelMapPtr pixelMap;
    if ( !image.TryGetPixelMap( 0, 0, pixelMap ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: Unable to get the pixel map from the image" );
        return NULL;
    }
    
    // CEGUI does not support single channel formats
    if ( pixelMap->GetNumberOfChannelsPerPixel() < 3 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: CEGUI cannot handle images with less than 3 channels per pixels" );
        return NULL;        
    }
    
    // CEGUI only supports a subset of formats, convert as needed
    IMAGE::TPixelStorageFormat sourceFormat = pixelMap->GetPixelStorageFormat();
    switch ( sourceFormat )
    {
        case IMAGE::PSF_BGR:
        {
            IMAGE::TPixelMapPtr newPixelMap;
            if ( !pixelMap->ConvertFormatTo( IMAGE::PSF_RGB, DATATYPE_UINT8, newPixelMap ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: CEGUI cannot handle BGR images and we were unable to convert" );
                return NULL;
            }
            if ( newPixelMap )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: CEGUI cannot handle BGR images, we successfully converted the image" );
                pixelMap = newPixelMap;
            }
            break;
        } 
        case IMAGE::PSF_BGRA:
        {
            IMAGE::TPixelMapPtr newPixelMap;
            if ( !pixelMap->ConvertFormatTo( IMAGE::PSF_RGBA, DATATYPE_UINT8, newPixelMap ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: CEGUI cannot handle BGRA images and we were unable to convert" );
                return NULL;
            }
            if ( newPixelMap )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: CEGUI cannot handle BGRA images, we sucecssfully converted the image" );
                pixelMap = newPixelMap;
            }
            break;
        } 
    }

    // CEGUI only supports a subset of channel sizes, convert as needed
    // If we get here then the pixels are stored in a supported format either due to conversion or they were already
    // However the channelsize can still be wrong in the latter case
    CORE::UInt32 channelSize = pixelMap->GetPixelChannelSize();
    if ( channelSize > 1 )
    {
        IMAGE::TPixelStorageFormat sourceFormat = pixelMap->GetPixelStorageFormat();
        IMAGE::TPixelMapPtr newPixelMap;
        if ( !pixelMap->ConvertFormatTo( sourceFormat, DATATYPE_UINT8, newPixelMap ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: CEGUI cannot handle images with multi-byte channels and we were unable to convert" );
            return NULL;
        }
        
        if ( newPixelMap )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: CEGUI cannot handle images with multi-byte channels, we sucecssfully converted the image to single byte" );
            pixelMap = newPixelMap;
        }
        channelSize = pixelMap->GetPixelChannelSize();
    }
    
    // Now that we have the image in a format we know CEGUI can handle,.. convert the type enum
    CEGUI::Texture::PixelFormat ceguiPixelFormat;
    if ( !TryConvertPixelFormat( pixelMap->GetPixelStorageFormat(), channelSize, ceguiPixelFormat ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: Failed to map pixel storage format to a CEGUI supported type" );
        return NULL;        
    }
        
    CEGUI::Sizef imageSizeInPixels( (float) pixelMap->GetWidthInPixels(), (float) pixelMap->GetHeightInPixels() );
    result->loadFromMemory( pixelMap->GetDataPtr(), imageSizeInPixels, ceguiPixelFormat );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodecAdapter:load: Successfully loaded image into texture obj" );
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
