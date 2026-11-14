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

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"          /* abstract base for IO Access */
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#include "gucefIMAGE_CPixel.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPixel::CPixel( void )
    : m_pixelStorageFormat( PSF_UNKNOWN )              ,
      m_pixelComponentDataType( DATATYPE_UNKNOWN ) ,  
      m_pixelData( GUCEF_NULL )                              ,
      m_ownsData( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPixel::CPixel( const void* pixelData                         , 
                const TPixelStorageFormat pixelStorageFormat  ,
                const TBuildinDataType pixelComponentDataType ,
                bool referenceData /* = false */              )
    : m_pixelStorageFormat( pixelStorageFormat )         ,
      m_pixelComponentDataType( pixelComponentDataType ) ,  
      m_pixelData( GUCEF_NULL )                                ,
      m_ownsData( false )
{GUCEF_TRACE;

    if ( referenceData )
    {
        m_pixelData = (UInt8*) pixelData;
        m_ownsData = false;
    }
    else
    {
        UInt32 pixelSize = GetSizeOfPixelInBytes();
        m_pixelData = new UInt8[ pixelSize ];
        memcpy( m_pixelData, pixelData, pixelSize );
        m_ownsData = true;
    }
}

/*-------------------------------------------------------------------------*/

CPixel::CPixel( const CPixel& src )
    : m_pixelStorageFormat( src.m_pixelStorageFormat )         ,
      m_pixelComponentDataType( src.m_pixelComponentDataType ) ,  
      m_pixelData( GUCEF_NULL )                                      ,
      m_ownsData( false )
{GUCEF_TRACE;

    if ( !src.m_ownsData )
    {
        m_pixelData = src.m_pixelData;
        m_ownsData = false;
    }
    else
    {
        UInt32 pixelSize = GetSizeOfPixelInBytes();
        m_pixelData = new UInt8[ pixelSize ];
        memcpy( m_pixelData, src.m_pixelData, pixelSize );
        m_ownsData = true;
    }
}

/*-------------------------------------------------------------------------*/
    
CPixel::~CPixel()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CPixel::GetSizeOfPixelInBytes( void ) const
{GUCEF_TRACE;

    return CPixelMap::GetExpectedPixelMapSize( 1, 1, m_pixelStorageFormat, m_pixelComponentDataType );
}

/*-------------------------------------------------------------------------*/

UInt32 
CPixel::GetNumberOfChannels( void ) const
{GUCEF_TRACE;

    return CPixelMap::GetChannelCountForFormat(  m_pixelStorageFormat );
}

/*-------------------------------------------------------------------------*/
    
TPixelStorageFormat 
CPixel::GetPixelStorageFormat( void ) const
{GUCEF_TRACE;

    return m_pixelStorageFormat;
}

/*-------------------------------------------------------------------------*/
    
void* 
CPixel::GetDataPtr( void )
{GUCEF_TRACE;

    return m_pixelData;
}

/*-------------------------------------------------------------------------*/
    
const void* 
CPixel::GetDataPtr( void ) const
{GUCEF_TRACE;

    return m_pixelData;
}

/*-------------------------------------------------------------------------*/

void* 
CPixel::RelinquishPixelDataOwnership( void )
{GUCEF_TRACE;

    void* pixelData = m_pixelData;
    m_pixelData = GUCEF_NULL;
    return pixelData;
}

/*-------------------------------------------------------------------------*/

TBuildinDataType 
CPixel::GetPixelComponentDataType( void ) const
{GUCEF_TRACE;

    return m_pixelComponentDataType;
}

/*-------------------------------------------------------------------------*/

bool 
CPixel::HasAlpha( void ) const
{GUCEF_TRACE;

    return CPixelMap::GetHasAlpha( m_pixelStorageFormat );
}

/*-------------------------------------------------------------------------*/
    
void 
CPixel::Clear( void )
{GUCEF_TRACE;

    if ( m_ownsData )
    {
        delete []m_pixelData;
    }
    m_pixelData = GUCEF_NULL;
    m_ownsData = false;
}

/*-------------------------------------------------------------------------*/                 

bool 
CPixel::CopyTo( CORE::CIOAccess& resource )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_pixelData )
    {
        return 1 == resource.Write( m_pixelData, GetSizeOfPixelInBytes(), 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/
    
CPixel& 
CPixel::operator=( const CPixel& other )
{GUCEF_TRACE;

    if ( &other != this )
    {
        Clear();

        UInt32 pixelSize = other.GetSizeOfPixelInBytes();
        m_pixelData = new UInt8[ pixelSize ];
        memcpy( m_pixelData, other.m_pixelData, pixelSize );
        m_ownsData = true;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool 
CPixel::operator==( const CPixel& other ) const
{GUCEF_TRACE;

    if ( m_pixelStorageFormat != other.m_pixelStorageFormat ) return false;
    if ( m_pixelComponentDataType != other.m_pixelComponentDataType ) return false;

    UInt32 ourPixelSize = GetSizeOfPixelInBytes();
    UInt32 otherPixelSize = other.GetSizeOfPixelInBytes();
    if ( ourPixelSize != otherPixelSize ) return false;

    return 0 == memcmp( m_pixelData, other.m_pixelData, ourPixelSize );
}

/*-------------------------------------------------------------------------*/

bool 
CPixel::operator<( const CPixel& other ) const
{GUCEF_TRACE;

    if ( m_pixelStorageFormat < other.m_pixelStorageFormat ) return true;
    if ( m_pixelComponentDataType < other.m_pixelComponentDataType ) return true;

    UInt32 ourPixelSize = GetSizeOfPixelInBytes();
    UInt32 otherPixelSize = other.GetSizeOfPixelInBytes();
    if ( ourPixelSize < otherPixelSize ) return true;

    return 0 < memcmp( m_pixelData, other.m_pixelData, ourPixelSize );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace IMAGE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

