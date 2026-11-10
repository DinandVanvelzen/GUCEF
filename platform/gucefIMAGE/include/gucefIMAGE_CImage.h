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

#ifndef GUCEF_IMAGE_CIMAGE_H
#define GUCEF_IMAGE_CIMAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CGEOLOCATION_H
#include "gucefCORE_CGeoLocation.h"
#define GUCEF_CORE_CGEOLOCATION_H
#endif /* GUCEF_CORE_CGEOLOCATION_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEFIMAGE_MACROS_H
#include "gucefIMAGE_macros.h"       /* module macro's */
#define GUCEFIMAGE_MACROS_H
#endif /* GUCEFIMAGE_MACROS_H ? */

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
 *  Image class that defines an image as a series of frames with each frame beeing
 *  capable of having multiple mipmapping levels. Pixel maps can be shared between
 *  image objects if desired.
 */
class GUCEF_IMAGE_EXPORT_CPP CImage
{
    public :

    typedef std::vector< TPixelMapPtr >    TMipMapList;
    typedef TMipMapList                    TFrame;
    typedef std::vector< TMipMapList >     TFrameList;
        
    public:    

    CImage( const TFrameList& frameList );
    
    CImage( const TMipMapList& mipmapList );
    
    explicit CImage( const TPixelMapPtr& pixelMapPtr );

    /**
     *      Creates an empty image object
     */
    CImage( void );     

    ~CImage();

    void Assign( const TImage& cStyleImage );
    
    void Assign( const TFrameList& frameList );
    
    void Assign( const TMipMapList& mipmapList );
    
    void Assign( const TPixelMapPtr& pixelMapPtr );
    
    bool Load( const CORE::CString& filePath );

    bool Load( const CORE::CString& filePath ,
               const CORE::CString& dataType ,
               bool loadImageData = true     ,
               bool loadMetaData = true      );

    bool Load( CORE::CIOAccess& data         ,
               const CORE::CString& dataType ,
               bool loadImageData = true     ,
               bool loadMetaData = true      );

    bool Save( const CORE::CString& filePath );

    bool Save( const CORE::CString& filePath ,
               const CORE::CString& dataType );

    bool Save( CORE::CIOAccess& data         ,
               const CORE::CString& dataType );
    
    UInt32 GetFrameCount( void ) const;

    UInt32 GetTotalPixelStorageSize( void ) const;

    /**
     *  Get the number of mipmap levels for the given frame
     *
     *  @return mipmap levels per frame.
     *  @throw EInvalidIndex is thrown if an invalid index is given
     */
    UInt32 GetMipmapLevels( const UInt32 frameIndex = 0 ) const;
        
    /**
     *  @throw EInvalidIndex is thrown if an invalid index is given
     */
    TPixelMapPtr GetPixelMap( const UInt32 frameIndex = 0  ,
                              const UInt32 mipMapLevel = 0 ) const;

    bool TryGetPixelMap( const UInt32 frameIndex  ,
                         const UInt32 mipMapLevel ,
                         TPixelMapPtr& pixelMap   ) const;

    void AddFrame( TPixelMapPtr& imageFrame               ,
                   const UInt32 frameDurationInMs = 0     );

    void AddFrame( TMipMapList& imageFrame                ,
                   const UInt32 frameDurationInMs = 0     );
    
    void SetFrame( TPixelMapPtr& imageFrame    ,
                   const UInt32 frameIndex = 0 );

    void SetFrame( TMipMapList& imageFrame     ,
                   const UInt32 frameIndex = 0 );
    
    TMipMapList& GetFrame( const UInt32 frameIndex = 0 );
    
    const TMipMapList& GetFrame( const UInt32 frameIndex = 0 ) const;
    
    bool HasFrames( void ) const;

    /**
     *  Animation support: an image with multiple frames can be an animation (GIF, FLIC, etc).
     *  Every frame can have a duration. A duration of 0 means no duration was specified.
     *  The frame data itself is always the complete (composited) frame.
     *
     *  @throw EInvalidIndex is thrown if an invalid index is given
     */
    UInt32 GetFrameDurationInMs( const UInt32 frameIndex = 0 ) const;

    /**
     *  @throw EInvalidIndex is thrown if an invalid index is given
     */
    void SetFrameDurationInMs( const UInt32 frameIndex      ,
                               const UInt32 durationInMs    );

    /**
     *  Sum of the durations of all frames
     */
    UInt32 GetTotalDurationInMs( void ) const;

    /**
     *  An image is considered an animation if it has more than 1 frame and at least one
     *  frame has a duration.
     */
    bool IsAnimation( void ) const;

    /**
     *  Number of times the frame sequence of an animation should be played, 0 means loop forever
     */
    UInt32 GetLoopCount( void ) const;

    void SetLoopCount( const UInt32 loopCount );

    /**
     *      Unloads image data if needed and resets values to there defaults
     */
    void Clear( void );

    CORE::CValueList& GetMetaData( void );

    const CORE::CValueList& GetMetaData( void ) const;

    /**
     *  Helper member function to easily interpret the meta data and extract the geo location
     */
    bool TryGetGeoLocationFromMetaData( CORE::CGeoLocation& geoLoc ) const; 

    /**
     *  Helper member function to easily interpret the meta data and extract the image creation datetime
     */
    bool TryGetOriginalImageCreationDtFromMetaData( CORE::CDateTime& imageCreationDt, bool& isTimezoneAware ) const;

    /**
     *  Helper member function to easily interpret the meta data and extract the image last modified datetime
     */
    bool TryGetImageLastModifiedDtFromMetaData( CORE::CDateTime& imageLastModifiedDt, bool& isTimezoneAware ) const;

    TImage* CreateCStyleAccess( void ) const;

    void FreeCStyleAccess( TImage* cStyleImage ) const;
    
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_IMAGE_EXPORT_CPP, EInvalidIndex );

    private :
    CImage( const CImage& src ); /* @TODO: not implemented yet */
    CImage& operator=( const CImage& src ); /* @TODO: not implemented yet */

    private:        

    TFrameList m_frameList;
    std::vector< UInt32 > m_frameDurationsInMs;  /**< always has the same number of entries as m_frameList */
    UInt32 m_loopCount;
    CORE::CValueList m_metaData;
};

/*--------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CImage, MT::CMutex >    TImagePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace IMAGE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGE_H ? */
