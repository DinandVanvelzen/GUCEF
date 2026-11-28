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

#include "gucefIMAGE_CIImageCodec.h"

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

CIImageCodec::CIImageCodec( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIImageCodec::CIImageCodec( const CIImageCodec& src )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CIImageCodec::~CIImageCodec()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

UInt32
CIImageCodec::GetImageCapabilities( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CIImageCodec::Encode( const CImage& inputImage       ,
                     CORE::CIOAccess& encodedOutput ,
                     const CORE::CValueList& params )
{GUCEF_TRACE;

    return Encode( inputImage, encodedOutput );
}

/*-------------------------------------------------------------------------*/

CIImageCodec&
CIImageCodec::operator=( const CIImageCodec& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
