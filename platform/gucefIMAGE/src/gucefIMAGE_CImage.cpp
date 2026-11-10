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

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_IMAGE_IMAGETAGS_H
#include "gucefIMAGE_c_image_tags.h"
#define GUCEF_IMAGE_IMAGETAGS_H
#endif /* GUCEF_IMAGE_IMAGETAGS_H ? */

#ifndef GUCEF_IMAGE_CIMGCODEC_H
#include "gucefIMAGE_CIMGCodec.h"
#define GUCEF_IMAGE_CIMGCODEC_H
#endif /* GUCEF_IMAGE_CIMGCODEC_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#include "gucefIMAGE_CImage.h"       /* Header for this class */

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

GUCEF_IMPLEMENT_MSGEXCEPTION( CImage, EInvalidIndex );

/*--------------------------------------------------------------------------*/

CImage::CImage( void )
    : m_frameList()
    , m_frameDurationsInMs()
    , m_loopCount( 0 )
    , m_metaData()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CImage::CImage( const TFrameList& frameList )
    : m_frameList( frameList )
    , m_frameDurationsInMs( frameList.size(), 0 )
    , m_loopCount( 0 )
    , m_metaData()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CImage::CImage( const TMipMapList& mipmapList )
    : m_frameList()
    , m_frameDurationsInMs()
    , m_loopCount( 0 )
    , m_metaData()
{GUCEF_TRACE;

    m_frameList.push_back( mipmapList );
    m_frameDurationsInMs.push_back( 0 );
}

/*--------------------------------------------------------------------------*/

CImage::CImage( const TPixelMapPtr& pixelMapPtr )
    : m_frameList()
    , m_frameDurationsInMs()
    , m_loopCount( 0 )
    , m_metaData()
{GUCEF_TRACE;

    TMipMapList mipmapList;
    mipmapList.push_back( pixelMapPtr );
    m_frameList.push_back( mipmapList );
    m_frameDurationsInMs.push_back( 0 );
}

/*--------------------------------------------------------------------------*/

void
CImage::Assign( const TFrameList& frameList )
{GUCEF_TRACE;

    // First we clean up our toys
    Clear();

    // Assign the image data
    m_frameList = frameList;
    m_frameDurationsInMs.assign( frameList.size(), 0 );
}

/*--------------------------------------------------------------------------*/

void
CImage::Assign( const TMipMapList& mipmapList )
{GUCEF_TRACE;

    // First we clean up our toys
    Clear();

    // Assign the image data
    m_frameList.push_back( mipmapList );
    m_frameDurationsInMs.push_back( 0 );
}

/*--------------------------------------------------------------------------*/

void
CImage::Assign( const TPixelMapPtr& pixelMapPtr )
{GUCEF_TRACE;

    // First we clean up our toys
    Clear();

    // Assign the image data
    TMipMapList mipmapList;
    mipmapList.push_back( pixelMapPtr );
    m_frameList.push_back( mipmapList );
    m_frameDurationsInMs.push_back( 0 );
}

/*--------------------------------------------------------------------------*/

void
CImage::Assign( const TImage& cStyleImage )
{GUCEF_TRACE;

    // First we clean up our toys
    Clear();

    // Assign the image data
    UInt32 frameCount = cStyleImage.imageInfo.nrOfFramesInImage;
    if ( cStyleImage.imageInfo.version >= 2 )
        m_loopCount = cStyleImage.imageInfo.loopCount;
    for ( UInt32 i=0; i<frameCount; ++i )
    {
        const TImageFrame* frame = &cStyleImage.frames[ i ];

        UInt32 mipMapCount = frame->frameInfo.nrOfMipmapLevels;
        TMipMapList mipmapList;
        mipmapList.reserve( mipMapCount );

        for ( UInt32 n=0; n<mipMapCount; ++n )
        {
            const TImageMipMapLevel* mipMapLevel = &frame->mipmapLevel[ n ];
            TPixelMapPtr pixelMap( new CPixelMap( *mipMapLevel ) );
            mipmapList.push_back( pixelMap );
        }

        m_frameList.push_back( mipmapList );
        m_frameDurationsInMs.push_back( frame->frameInfo.version >= 2 ? frame->frameInfo.frameDurationInMs : 0 );
    }
}

/*--------------------------------------------------------------------------*/

TImage*
CImage::CreateCStyleAccess( void ) const
{GUCEF_TRACE;

    TImage* cStyleImage = new TImage;
    cStyleImage->version = GUCEF_IMAGE_TIMAGE_VERSION;
    cStyleImage->imageInfo.nrOfFramesInImage = (UInt32) m_frameList.size();
    cStyleImage->imageInfo.version = GUCEF_IMAGE_TIMAGEINFO_VERSION;
    cStyleImage->imageInfo.loopCount = m_loopCount;
    cStyleImage->frames = new TImageFrame[ m_frameList.size() ];

    TFrameList::const_iterator n = m_frameList.begin();
    for ( UInt32 i=0; i<m_frameList.size(); ++i )
    {
        TImageFrame* frame = &cStyleImage->frames[ i ];
        const TMipMapList& cppFrame = (*n);

        frame->version = GUCEF_IMAGE_TIMAGEFRAME_VERSION;
        frame->frameInfo.version = GUCEF_IMAGE_TIMAGEFRAMEINFO_VERSION;
        frame->frameInfo.nrOfMipmapLevels = (UInt32) cppFrame.size();
        frame->frameInfo.frameDurationInMs = m_frameDurationsInMs[ i ];
        frame->mipmapLevel = new TImageMipMapLevel[ frame->frameInfo.nrOfMipmapLevels ];

        TMipMapList::const_iterator m = cppFrame.begin();
        for ( UInt32 p=0; p<cppFrame.size(); ++p )
        {
            TImageMipMapLevel* mipmapLevel = &frame->mipmapLevel[ p ];
            TPixelMapPtr pixelMap = (*m);

            mipmapLevel->version = GUCEF_IMAGE_TIMAGEMIPMAPLEVEL_VERSION;
            mipmapLevel->mipLevelInfo.version = GUCEF_IMAGE_TIMAGEMIPMAPLEVELINFO_VERSION;
            mipmapLevel->mipLevelInfo.pixelComponentDataType = (Int32) pixelMap->GetPixelComponentDataType();
            mipmapLevel->mipLevelInfo.pixelStorageFormat = (Int32) pixelMap->GetPixelStorageFormat();
            mipmapLevel->mipLevelInfo.frameWidth = pixelMap->GetWidthInPixels();
            mipmapLevel->mipLevelInfo.frameHeight = pixelMap->GetHeightInPixels();
            mipmapLevel->pixelDataSizeInBytes = pixelMap->GetTotalSizeInBytes();
            mipmapLevel->pixelData = pixelMap->GetDataPtr();

            ++m;
        }
        ++n;
    }


    return cStyleImage;
}

/*--------------------------------------------------------------------------*/

void
CImage::FreeCStyleAccess( TImage* cStyleImage ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<cStyleImage->imageInfo.nrOfFramesInImage; ++i )
    {
        TImageFrame* frame = &cStyleImage->frames[ i ];
        delete[] frame->mipmapLevel;
    }
    delete[] cStyleImage->frames;
    delete cStyleImage;
}

/*--------------------------------------------------------------------------*/

void
CImage::AddFrame( TMipMapList& imageFrame         ,
                  const UInt32 frameDurationInMs  )
{GUCEF_TRACE;

    m_frameList.push_back( imageFrame );
    m_frameDurationsInMs.push_back( frameDurationInMs );
}

/*--------------------------------------------------------------------------*/

void
CImage::AddFrame( TPixelMapPtr& imageFrame        ,
                  const UInt32 frameDurationInMs  )
{GUCEF_TRACE;

    TMipMapList mipMapList;
    mipMapList.push_back( imageFrame );
    m_frameList.push_back( mipMapList );
    m_frameDurationsInMs.push_back( frameDurationInMs );
}

/*--------------------------------------------------------------------------*/

void
CImage::SetFrame( TPixelMapPtr& imageFrame          ,
                  const UInt32 frameIndex /* = 0 */ )
{GUCEF_TRACE;

    TMipMapList mipMapList;
    mipMapList.push_back( imageFrame );
    SetFrame( mipMapList, frameIndex );
}

/*--------------------------------------------------------------------------*/

void
CImage::SetFrame( TMipMapList& imageFrame           ,
                  const UInt32 frameIndex /* = 0 */ )
{GUCEF_TRACE;

    if ( frameIndex < m_frameList.size() )
    {
        m_frameList[ frameIndex ] = imageFrame;
    }
    else
    if ( frameIndex == m_frameList.size() )
    {
        m_frameList.push_back( imageFrame );
        m_frameDurationsInMs.push_back( 0 );
    }
    else
    {
        GUCEF_EMSGTHROW( EInvalidIndex, "CImage::SetFrame(): Invalid frame index" );
    }
}

/*--------------------------------------------------------------------------*/

CImage::~CImage()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CImage::GetFrameCount( void ) const
{GUCEF_TRACE;

    return static_cast< UInt32 >( m_frameList.size() );
}

/*-------------------------------------------------------------------------*/

UInt32
CImage::GetMipmapLevels( const UInt32 frameIndex /* = 0 */ ) const
{GUCEF_TRACE;

    if ( m_frameList.size() > frameIndex )
    {
        return static_cast< UInt32 >( m_frameList[ frameIndex ].size() );
    }

    GUCEF_EMSGTHROW( EInvalidIndex, "CImage::GetMipmapLevels(): Invalid frame index" );
}

/*-------------------------------------------------------------------------*/

void
CImage::Clear( void )
{
    // Because we are using shared pointers all we have to do is clear the list
    // this will cause things to be de-allocated as needed
    m_frameList.clear();
    m_frameDurationsInMs.clear();
    m_loopCount = 0;

    m_metaData.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CImage::TryGetPixelMap( const UInt32 frameIndex /* = 0 */  ,
                        const UInt32 mipMapLevel /* = 0 */ ,
                        TPixelMapPtr& pixelMap             ) const
{GUCEF_TRACE;

    if ( m_frameList.size() > frameIndex )
    {
        const TMipMapList& mipmapList = m_frameList[ frameIndex ];
        if ( mipmapList.size() > mipMapLevel )
        {
            pixelMap = mipmapList[ mipMapLevel ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TPixelMapPtr
CImage::GetPixelMap( const UInt32 frameIndex /* = 0 */  ,
                     const UInt32 mipMapLevel /* = 0 */ ) const
{GUCEF_TRACE;

    if ( !m_frameList.empty() && frameIndex < m_frameList.size() )
    {
        const TMipMapList& mipmapList = m_frameList[ frameIndex ];
        if ( !mipmapList.empty() && mipMapLevel < mipmapList.size() )
        {
            return mipmapList[ mipMapLevel ];
        }
    }

    GUCEF_EMSGTHROW( EInvalidIndex, "CImage::GetFrame(): Invalid frame or mipmap index" );
}

/*-------------------------------------------------------------------------*/

CImage::TMipMapList&
CImage::GetFrame( const UInt32 frameIndex /* = 0 */ )
{GUCEF_TRACE;

    if ( !m_frameList.empty() && frameIndex < m_frameList.size() )
    {
        return m_frameList[ frameIndex ];
    }

    GUCEF_EMSGTHROW( EInvalidIndex, "CImage::GetFrame(): Invalid frame or mipmap index" );
}

/*-------------------------------------------------------------------------*/

const CImage::TMipMapList&
CImage::GetFrame( const UInt32 frameIndex /* = 0 */ ) const
{GUCEF_TRACE;

    if ( !m_frameList.empty() && frameIndex < m_frameList.size() )
    {
        return m_frameList[ frameIndex ];
    }

    GUCEF_EMSGTHROW( EInvalidIndex, "CImage::GetFrame(): Invalid frame or mipmap index" );
}

/*-------------------------------------------------------------------------*/

UInt32
CImage::GetFrameDurationInMs( const UInt32 frameIndex /* = 0 */ ) const
{GUCEF_TRACE;

    if ( frameIndex < m_frameDurationsInMs.size() )
        return m_frameDurationsInMs[ frameIndex ];

    GUCEF_EMSGTHROW( EInvalidIndex, "CImage::GetFrameDurationInMs(): Invalid frame index" );
}

/*-------------------------------------------------------------------------*/

void
CImage::SetFrameDurationInMs( const UInt32 frameIndex   ,
                              const UInt32 durationInMs )
{GUCEF_TRACE;

    if ( frameIndex < m_frameDurationsInMs.size() )
    {
        m_frameDurationsInMs[ frameIndex ] = durationInMs;
        return;
    }

    GUCEF_EMSGTHROW( EInvalidIndex, "CImage::SetFrameDurationInMs(): Invalid frame index" );
}

/*-------------------------------------------------------------------------*/

UInt32
CImage::GetTotalDurationInMs( void ) const
{GUCEF_TRACE;

    UInt32 totalDurationInMs = 0;
    std::vector< UInt32 >::const_iterator i = m_frameDurationsInMs.begin();
    while ( i != m_frameDurationsInMs.end() )
    {
        totalDurationInMs += (*i);
        ++i;
    }
    return totalDurationInMs;
}

/*-------------------------------------------------------------------------*/

bool
CImage::IsAnimation( void ) const
{GUCEF_TRACE;

    return m_frameList.size() > 1 && GetTotalDurationInMs() > 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CImage::GetLoopCount( void ) const
{GUCEF_TRACE;

    return m_loopCount;
}

/*-------------------------------------------------------------------------*/

void
CImage::SetLoopCount( const UInt32 loopCount )
{GUCEF_TRACE;

    m_loopCount = loopCount;
}

/*-------------------------------------------------------------------------*/

bool
CImage::HasFrames( void ) const
{GUCEF_TRACE;

    return !m_frameList.empty();
}

/*-------------------------------------------------------------------------*/

UInt32
CImage::GetTotalPixelStorageSize( void ) const
{GUCEF_TRACE;

    UInt32 totalBytes = 0;
    TFrameList::const_iterator i = m_frameList.begin();
    while ( i != m_frameList.end() )
    {
        const TMipMapList& mipmapList = (*i);
        TMipMapList::const_iterator n = mipmapList.begin();
        while ( n != mipmapList.end() )
        {
            totalBytes += (*n)->GetTotalSizeInBytes();
            ++n;
        }
        ++i;
    }

    return totalBytes;
}

/*-------------------------------------------------------------------------*/

bool
CImage::Load( CORE::CIOAccess& data         ,
              const CORE::CString& dataType ,
              bool loadImageData            ,
              bool loadMetaData             )
{GUCEF_TRACE;

    CImageCodecRegistry::TImageCodecPtr imageCodec;
    if ( CImageGlobal::Instance()->GetImageCodecRegistry().TryLookup( dataType, imageCodec, false ) )
    {
        bool success = true;
        if ( loadImageData )
        {
            // We have found a codec we can use, now try to load the data
            success = success && imageCodec->Decode( data  ,
                                                     *this );
        }
        if ( loadMetaData )
        {
            // We have found a codec we can use, now try to load the meta-data
            success = success && imageCodec->DecodeMetaData( data       ,
                                                             m_metaData );

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Image:Load: The following image meta-data is available: " + ToString( m_metaData ) );
        }
        return success;
    }

    // We have not found a codec with the extended image interface
    // because meta data is requested we will not try to load the data
    if ( loadMetaData )
        return false;

    if ( loadImageData )
    {
        // Since no codec was found with the extended image interface we will see if we can use a codec with the basic interface
        CORE::CCodecRegistry::TCodecFamilyRegistryPtr codecRegistry;
        if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryLookup( "ImageCodec", codecRegistry, true ) )
        {
            CORE::CCodecRegistry::TICodecPtr codec;
            if ( codecRegistry->TryLookup( dataType, codec, false ) )
            {
                // We have found a codec we can use, now try to load the data
                CIMGCodec codecUtil( codec );
                return codecUtil.Decode( data  ,
                                         *this );
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImage::Load( const CORE::CString& filePath ,
              const CORE::CString& dataType ,
              bool loadImageData            ,
              bool loadMetaData             )
{GUCEF_TRACE;

    CORE::CFileAccess fileAccess;
    if ( fileAccess.Open( filePath ) )
    {
        return Load( fileAccess, dataType, loadImageData, loadMetaData );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImage::Load( const CORE::CString& filePath )
{GUCEF_TRACE;

    CORE::CString dataType = CORE::ExtractFileExtention( filePath );
    return Load( filePath, dataType );
}

/*-------------------------------------------------------------------------*/

bool
CImage::Save( CORE::CIOAccess& data         ,
              const CORE::CString& dataType )
{GUCEF_TRACE;

    CImageCodecRegistry::TImageCodecPtr imageCodec;
    if ( CImageGlobal::Instance()->GetImageCodecRegistry().TryLookup( dataType, imageCodec, false ) )
    {
        // We have found a codec we can use, now try to save the data
        return imageCodec->Encode( *this ,
                                   data  );
    }

    // Since no codec was found with the extended image interface we will see if we can use a codec with the basic interface
    CORE::CCodecRegistry::TCodecFamilyRegistryPtr codecRegistry;
    if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryLookup( "ImageCodec", codecRegistry, true ) )
    {
        CORE::CCodecRegistry::TICodecPtr codec;
        if ( codecRegistry->TryLookup( dataType, codec, false ) )
        {
            // We have found a codec we can use, now try to save the data
            CIMGCodec codecUtil( codec );
            return codecUtil.Encode( *this ,
                                     data  );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImage::Save( const CORE::CString& filePath ,
              const CORE::CString& dataType )
{GUCEF_TRACE;

    CORE::CFileAccess fileAccess;
    if ( fileAccess.Open( filePath, "wb" ) )
    {
        return Save( fileAccess, dataType );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CImage::Save( const CORE::CString& filePath )
{GUCEF_TRACE;

    CORE::CString dataType = CORE::ExtractFileExtention( filePath );
    return Save( filePath, dataType );
}

/*-------------------------------------------------------------------------*/

CORE::CValueList& 
CImage::GetMetaData( void )
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList& 
CImage::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

bool 
CImage::TryGetGeoLocationFromMetaData( CORE::CGeoLocation& geoLoc ) const
{GUCEF_TRACE;

    // Try to get the GPS location from the meta-data
    // latitude and longitude are required at minimum
    CORE::CVariant::VariantVector latitude;
    CORE::CVariant latitudeRef;
    CORE::CVariant::VariantVector longitude;
    CORE::CVariant longitudeRef;
    CORE::CVariant::VariantVector altitude;

    // The EXIF standard specifies that the latitude and longitude are stored as vectors of 3 fractions
    // the first fraction is the degrees, the second the minutes and the third the seconds
    if ( m_metaData.TryGetValueVector( GUCEF_IMAGE_TAG_EXIF_GPS_LATITUDE, latitude ) &&
         m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_GPS_LATITUDE_REF, latitudeRef ) &&
         m_metaData.TryGetValueVector( GUCEF_IMAGE_TAG_EXIF_GPS_LONGITUDE, longitude ) &&
         m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_GPS_LONGITUDE_REF, longitudeRef ) )
    {
        if ( latitude.size() < 3 || longitude.size() < 3 )
            return false;      

        Int32 latDegrees = latitude[ 0 ].AsInt32();
        Int32 latMinutes = latitude[ 1 ].AsInt32();
        Float32 latSeconds = latitude[ 2 ].AsFloat32();

        Int32 lonDegrees = longitude[ 0 ].AsInt32();
        Int32 lonMinutes = longitude[ 1 ].AsInt32();
        Float32 lonSeconds = longitude[ 2 ].AsFloat32();

        // Note that negative longitudes represent the western hemisphere
        // Possible values are 'W' for west and 'E' for east
        char longitudeRefValue = longitudeRef.AsChar();       
        if ( longitudeRefValue == 'W' )
            lonDegrees = -lonDegrees;

        // Note that negative latitudes represent the southern hemisphere
        // Possible values are 'S' for south and 'N' for north
        char latitudeRefValue = latitudeRef.AsChar();    
        if ( latitudeRefValue == 'S' )
            latDegrees = -latDegrees;

        // altitude is an optional value, we will use it if its available
        CORE::CVariant altitude;
        CORE::CVariant altitudeRef;
        if ( m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_GPS_ALTITUDE, altitude )        &&
             m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_GPS_ALTITUDE_REF, altitudeRef ) )
        {
            Float64 altitudeInMeters = altitude.AsFloat64();
            
            // Note that negative altitude represent below sea level
            // A ref value of 0 means sea level
            char altitudeRefValue = altitudeRef.AsChar();        
            if ( altitudeRefValue != 0 )
                altitudeInMeters = -altitudeInMeters;

            geoLoc = CORE::CGeoLocation( latDegrees, latMinutes, latSeconds,
                                         lonDegrees, lonMinutes, lonSeconds,
                                         altitudeInMeters );
            return true;
        }
        else
        {
            geoLoc = CORE::CGeoLocation( latDegrees, latMinutes, latSeconds ,
                                         lonDegrees, lonMinutes, lonSeconds );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CImage::TryGetOriginalImageCreationDtFromMetaData( CORE::CDateTime& imageCreationDt, bool& isTimezoneAware ) const
{GUCEF_TRACE;

    // Try to get the image creation datetime from the meta-data
    // The EXIF standard specifies that DATETIME_ORIGINAL is stored as a string in ISO 8601 format
    CORE::CVariant imageCreation;
    if ( m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME_ORIGINAL, imageCreation ) )
    {
        imageCreationDt = imageCreation.AsDateTime();

        // Timezone information was added to the EXIF standard in version 2.31
        // Source: CIPA's Exif standard 2.31 from 2016, semantically page 49, digitally page 54.
        // Some vendors may have included it before then but it is not guaranteed to be present

        CORE::CVariant utcOffset;
        if ( m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME_OFFSETTIME_ORIGINAL, utcOffset ) )
        {
            CORE::CTime timeSegment;
            if ( timeSegment.FromIso8601TimeString( utcOffset.AsString() ) )
            {
                Int16 minutesOffset = ( timeSegment.GetHours() * 60 * 60 );
                minutesOffset += timeSegment.GetMinutes();
                minutesOffset += (Int16)( timeSegment.GetSeconds() / 60 );                
                imageCreationDt.SetTimeZoneUTCOffsetInMins( minutesOffset );

                isTimezoneAware = true;
                return true;
            }           
        }

        // the image sadly has no timezone relevant information
        // You could still try to derive it from the GPS data if available
        isTimezoneAware = false; 
        return true;
    }    
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CImage::TryGetImageLastModifiedDtFromMetaData( CORE::CDateTime& imageLastModifiedDt, bool& isTimezoneAware ) const
{GUCEF_TRACE;

    // Try to get the image creation datetime from the meta-data
    // The EXIF standard specifies that DATETIME_ORIGINAL is stored as a string in ISO 8601 format
    CORE::CVariant imageLastModified;
    if ( m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_IFD_DATETIME, imageLastModified ) )
    {
        imageLastModifiedDt = imageLastModified.AsDateTime();        

        // Timezone information was added to the EXIF standard in version 2.31
        // Source: CIPA's Exif standard 2.31 from 2016, semantically page 49, digitally page 54.
        // Some vendors may have included it before then but it is not guaranteed to be present

        CORE::CVariant utcOffset;
        if ( m_metaData.TryGetValue( GUCEF_IMAGE_TAG_EXIF_IFD_TIMEZONEOFFSET, utcOffset ) )
        {
            CORE::CTime timeSegment;
            if ( timeSegment.FromIso8601TimeString( utcOffset.AsString() ) )
            {
                Int16 minutesOffset = ( timeSegment.GetHours() * 60 * 60 );
                minutesOffset += timeSegment.GetMinutes();
                minutesOffset += (Int16)( timeSegment.GetSeconds() / 60 );                
                imageLastModifiedDt.SetTimeZoneUTCOffsetInMins( minutesOffset );

                isTimezoneAware = true;
                return true;
            }           
        }

        // the image sadly has no timezone relevant information
        // You could still try to derive it from the GPS data if available
        isTimezoneAware = false;
        return true;
    }    
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace IMAGE */
} /* namespace GUCEF */

/*--------------------------------------------------------------------------*/




