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

#include "gucefIMAGE_CImageConversionSettings.h"

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

CImageConversionSettings::CImageConversionSettings( void )
    : m_allFrames( true )             ,
      m_frameIndex( 0 )               ,
      m_allowLossyConversion( true )  ,
      m_encoderParams()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConversionSettings::CImageConversionSettings( const CImageConversionSettings& src )
    : m_allFrames( src.m_allFrames )                        ,
      m_frameIndex( src.m_frameIndex )                      ,
      m_allowLossyConversion( src.m_allowLossyConversion )  ,
      m_encoderParams( src.m_encoderParams )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConversionSettings::~CImageConversionSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConversionSettings&
CImageConversionSettings::operator=( const CImageConversionSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_allFrames = src.m_allFrames;
        m_frameIndex = src.m_frameIndex;
        m_allowLossyConversion = src.m_allowLossyConversion;
        m_encoderParams = src.m_encoderParams;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionSettings::SetFrameIndex( UInt32 frameIndex )
{GUCEF_TRACE;

    m_allFrames = false;
    m_frameIndex = frameIndex;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionSettings::SetAllFrames( void )
{GUCEF_TRACE;

    m_allFrames = true;
    m_frameIndex = 0;
}

/*-------------------------------------------------------------------------*/

bool
CImageConversionSettings::IsAllFrames( void ) const
{GUCEF_TRACE;

    return m_allFrames;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageConversionSettings::GetFrameIndex( void ) const
{GUCEF_TRACE;

    return m_frameIndex;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionSettings::SetAllowLossyConversion( bool allowLossyConversion )
{GUCEF_TRACE;

    m_allowLossyConversion = allowLossyConversion;
}

/*-------------------------------------------------------------------------*/

bool
CImageConversionSettings::GetAllowLossyConversion( void ) const
{GUCEF_TRACE;

    return m_allowLossyConversion;
}

/*-------------------------------------------------------------------------*/

CORE::CValueList&
CImageConversionSettings::GetEncoderParams( void )
{GUCEF_TRACE;

    return m_encoderParams;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
CImageConversionSettings::GetEncoderParams( void ) const
{GUCEF_TRACE;

    return m_encoderParams;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
