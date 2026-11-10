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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include "gucefIMAGE_CIMGCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/* make sure no structure packing is enabled, this allows us to use the structures directly */
#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif /* WIN32 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_IMPLEMENT_MSGEXCEPTION( CIMGCodec, EInvalidCodec );

/*-------------------------------------------------------------------------*/

CIMGCodec::CIMGCodec( const CCodecPtr& codecPtr )
    : m_codecPtr( codecPtr )
{GUCEF_TRACE;

    if ( m_codecPtr->GetFamilyName() != "ImageCodec" )
    {
        m_codecPtr.Unlink();
        GUCEF_EMSGTHROW( EInvalidCodec, "CIMGCodec::CIMGCodec(): Invalid codec type given" );
    }
}

/*-------------------------------------------------------------------------*/
    
CIMGCodec::CIMGCodec( const CIMGCodec& src )
    : m_codecPtr( src.m_codecPtr )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/    

CIMGCodec::~CIMGCodec()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/
    
CIMGCodec&
CIMGCodec::operator=( const CIMGCodec& src )
{GUCEF_TRACE;

    if ( &src != this )
    {        
        m_codecPtr = src.m_codecPtr;
    }
    
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CIMGCodec::Encode( CORE::CIOAccess& source ,
                   CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    return m_codecPtr->Encode( source ,
                               dest   );
}

/*-------------------------------------------------------------------------*/

bool
CIMGCodec::Decode( CORE::CIOAccess& source ,
                   CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    return m_codecPtr->Decode( source ,
                               dest   );
}

/*-------------------------------------------------------------------------*/

bool
CIMGCodec::Encode( const CImage& inputImage       ,
                   CORE::CIOAccess& encodedOutput )
{GUCEF_TRACE;

    if ( inputImage.HasFrames() )
    {    
        CORE::CDynamicBuffer inputBuffer( 102400, true );
        CORE::CDynamicBufferAccess bufferAccess( &inputBuffer, false );
        
        // Fill our header
        TImageInfo imageInfo;
        imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
        imageInfo.nrOfFramesInImage = inputImage.GetFrameCount();
        imageInfo.loopCount = inputImage.GetLoopCount();

        // write header
        bufferAccess.Write( &imageInfo, sizeof( TImageInfo ), 1 );
        
        // Now we add each frame's info + mipmap info
        TImageFrameInfo frameInfo;
        frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
        TImageMipMapLevelInfo mipMapInfo;
        mipMapInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;
        const CImage::TMipMapList* mipMapList = NULL;
        const TPixelMapPtr* pixelMap = NULL;
        for ( UInt32 frameNr=0; frameNr<imageInfo.nrOfFramesInImage; ++frameNr )
        {   
            mipMapList = &inputImage.GetFrame( frameNr );
            frameInfo.nrOfMipmapLevels = static_cast< UInt32 >( mipMapList->size() );
            frameInfo.frameDurationInMs = inputImage.GetFrameDurationInMs( frameNr );
            bufferAccess.Write( &frameInfo, sizeof( TImageFrameInfo ), 1 );
            
            // Now we add the info for each mipmap level
            for ( UInt32 mipLvl=0; mipLvl<frameInfo.nrOfMipmapLevels; ++mipLvl )
            {
                pixelMap = &(*mipMapList)[ mipLvl ];
                mipMapInfo.frameWidth = (*pixelMap)->GetWidthInPixels();
                mipMapInfo.frameHeight = (*pixelMap)->GetHeightInPixels();
                mipMapInfo.pixelStorageFormat = (*pixelMap)->GetPixelStorageFormat();
                mipMapInfo.pixelComponentDataType = (*pixelMap)->GetPixelComponentDataType();
                
                bufferAccess.Write( &mipMapInfo, sizeof( TImageMipMapLevelInfo ), 1 );
            }
        }
        
        // Now it is time to add the pixel data itself
        for ( UInt32 frameNr=0; frameNr<imageInfo.nrOfFramesInImage; ++frameNr )
        {
            mipMapList = &inputImage.GetFrame( frameNr );
            for ( UInt32 mipLvl=0; mipLvl<mipMapList->size(); ++mipLvl )
            {
                pixelMap = &(*mipMapList)[ mipLvl ];
                bufferAccess.Write( (*pixelMap)->GetDataPtr(), (*pixelMap)->GetTotalSizeInBytes(), 1 );
            }
        }
        
        // We have now merged all the image object data into a single buffer using the decoded 'ImageCodec' format
        // It is time to perform the actual Encode(), which reads the buffer from its start
        bufferAccess.Setpos( 0 );
        return Encode( bufferAccess  ,
                       encodedOutput );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIMGCodec::Decode( CORE::CIOAccess& encodedInput ,
                   CImage& outputImage           )
{GUCEF_TRACE;

    CORE::CDynamicBuffer outputBuffer( 102400, true );
    CORE::CDynamicBufferAccess bufferAccess( &outputBuffer, false );
    
    if ( Decode( encodedInput  ,
                 bufferAccess  ) )
    {
        bufferAccess.Setpos( 0 );
        UInt32 bytesRead = 0;
        bufferAccess.Setpos( 0 );
        
        // Fill our header from the buffer
        TImageInfo imageInfo;
        bytesRead = bufferAccess.Read( &imageInfo, sizeof( TImageInfo ), 1 );
        if ( bytesRead != 1 ) return false;   // Read() returns the number of elements read, not bytes
        
        // Check the header version for compatibility
        if ( imageInfo.version != GUCEF_IMAGE_TIMAGEINFO_VERSION ) return false;
        
        // Check if we are finished already
        if ( imageInfo.nrOfFramesInImage == 0 ) return true;
        
        // Process the image frames
        TImageFrame* imageFrameInfo = new TImageFrame[ imageInfo.nrOfFramesInImage ]; 
        for ( UInt32 i=0; i<imageInfo.nrOfFramesInImage; ++i )
        {            
            // Read the frame info
            TImageFrameInfo* currentFrame = &imageFrameInfo[ i ].frameInfo;
            bytesRead = bufferAccess.Read( currentFrame, sizeof( TImageFrameInfo ), 1 );
            
            // Sanity check 
            if ( ( bytesRead != 1 )                                                 ||
                 ( currentFrame->version != GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION )  )
            {
                delete []imageFrameInfo;
                return false;
            }
            
            imageFrameInfo[ i ].mipmapLevel = new TImageMipMapLevel[ currentFrame->nrOfMipmapLevels ];
            for ( UInt32 n=0; n<currentFrame->nrOfMipmapLevels; ++n )
            {
                // Read the frame's mipmap info
                TImageMipMapLevelInfo* mipMapLevel = &imageFrameInfo[ i ].mipmapLevel[ n ].mipLevelInfo;
                bytesRead = bufferAccess.Read( mipMapLevel, sizeof( TImageMipMapLevelInfo ), 1 );
                
                // Sanity check
                if ( ( bytesRead != 1 )                                                ||
                     ( mipMapLevel->version != GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION )  )
                {
                    for ( UInt32 m=0; m<n; ++m ) delete []imageFrameInfo[ m ].mipmapLevel;
                    delete []imageFrameInfo;
                    return false;
                }
            }
        }

        // Now we can read all the pixel data
        CImage::TFrameList frameList;
        CImage::TMipMapList mipMapList;
        std::vector< UInt32 > frameDurations;
        UInt64 bufferOffset = bufferAccess.Tell();        
        char* pixelBuffer = static_cast< char* >( outputBuffer.GetBufferPtr() ) + bufferOffset;
        for ( UInt32 i=0; i<imageInfo.nrOfFramesInImage; ++i )
        {
            TImageFrameInfo* currentFrame = &imageFrameInfo[ i ].frameInfo;
            for ( UInt32 n=0; n<currentFrame->nrOfMipmapLevels; ++n )
            {
                // Create a pixelmap using the information we obtained + out pixelBuffer pointer
                TImageMipMapLevelInfo* mipMapLevel = &imageFrameInfo[ i ].mipmapLevel[ n ].mipLevelInfo;
                
                // Obtain the expected pixel map size in bytes
                TPixelStorageFormat pixelStorageFormat = (TPixelStorageFormat) mipMapLevel->pixelStorageFormat;
                TBuildinDataType pixelComponentDataType = (TBuildinDataType) mipMapLevel->pixelComponentDataType;
                UInt32 pixelMapSize = CPixelMap::GetExpectedPixelMapSize( mipMapLevel->frameWidth  ,
                                                                          mipMapLevel->frameHeight ,
                                                                          pixelStorageFormat       ,
                                                                          pixelComponentDataType   );
                
                // Check to make sure the buffer is large enough for the data we expect
                if ( pixelMapSize <= outputBuffer.GetDataSize() - bufferOffset )
                {
                    CPixelMap* pixelMap = new CPixelMap( pixelBuffer              ,
                                                         mipMapLevel->frameWidth  ,
                                                         mipMapLevel->frameHeight ,
                                                         pixelStorageFormat       ,
                                                         pixelComponentDataType   );                    
                    
                    // Add this mipmap level to the frame
                    mipMapList.push_back( TPixelMapPtr( pixelMap ) );
                    
                    if ( i+1 < imageInfo.nrOfFramesInImage )
                    {
                        // Jump to the next image component in the buffer
                        UInt32 pixelBlockSize = ( mipMapLevel->frameWidth * mipMapLevel->frameHeight ) * pixelMap->GetSizeOfPixelInBytes();
                        bufferOffset += pixelBlockSize;
                        if ( bufferOffset < outputBuffer.GetDataSize() )
                        {
                            pixelBuffer += pixelBlockSize;
                        }
                        else
                        {
                            // oh oh,.. we ran out of pixels in our buffer even though our image info
                            // suggests we should have more pixels
                            for ( UInt32 m=n; m<currentFrame->nrOfMipmapLevels; ++m ) delete []imageFrameInfo[ m ].mipmapLevel;
                            delete []imageFrameInfo;
                            return false;
                        }
                    }
                }
                else
                {
                    // oh oh,.. we ran out of pixels in our buffer even though our image info
                    // suggests we should have more pixels
                    for ( UInt32 m=n; m<currentFrame->nrOfMipmapLevels; ++m ) delete []imageFrameInfo[ m ].mipmapLevel;
                    delete []imageFrameInfo;
                    return false;
                }
            }
            
            // Add the frame with all it's mipmap levels to the frame list
            frameList.push_back( mipMapList );
            frameDurations.push_back( currentFrame->frameDurationInMs );
            mipMapList.clear();
            
            // Clean up our info storage
            delete []imageFrameInfo[ i ].mipmapLevel;
        }
                
        // Clean up our info storage
        delete []imageFrameInfo;
        
        // Assign the parsed data to the output image object
        outputImage.Assign( frameList );
        outputImage.SetLoopCount( imageInfo.loopCount );
        for ( UInt32 i=0; i<frameDurations.size(); ++i )
            outputImage.SetFrameDurationInMs( i, frameDurations[ i ] );
        return true;
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIMGCodec::GetFamilyName( void ) const
{GUCEF_TRACE;
    
    return m_codecPtr->GetFamilyName();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIMGCodec::GetType( void ) const
{GUCEF_TRACE;

    return m_codecPtr->GetType();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/* restore structure packing */
#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(pop)
#else
#pragma pack()
#endif /* WIN32 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/