/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#include "gucefVIDEO_CIVideoCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CIVideoCodec::VideoCodecFamilyName = "VideoCodec";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIVideoCodec::CIVideoCodec( void )
    : CORE::CICodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIVideoCodec::CIVideoCodec( const CIVideoCodec& src )
    : CORE::CICodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIVideoCodec&
CIVideoCodec::operator=( const CIVideoCodec& src )
{GUCEF_TRACE;

    CORE::CICodec::operator=( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CIVideoCodec::~CIVideoCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::OpenSource( CORE::CIOAccess& encodedInput, TVideoSourcePtr& source, const CORE::CValueList& options )
{GUCEF_TRACE;

    return OpenSourceImpl( &encodedInput, TIOAccessPtr(), options, source );
}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::OpenSource( const TIOAccessPtr& encodedInput, TVideoSourcePtr& source, const CORE::CValueList& options )
{GUCEF_TRACE;

    if ( encodedInput.IsNULL() )
        return false;

    TIOAccessPtr input( encodedInput );
    return OpenSourceImpl( input.GetPointerAlways(), input, options, source );
}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::CanEncode( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::OpenSink( CORE::CIOAccess& encodedOutput, const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, TVideoSinkPtr& sink, const CORE::CValueList& options )
{GUCEF_TRACE;

    return OpenSinkImpl( &encodedOutput, TIOAccessPtr(), format, audioTrackFormats, options, sink );
}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::OpenSink( const TIOAccessPtr& encodedOutput, const CVideoFormat& format, const TAudioFormatVector& audioTrackFormats, TVideoSinkPtr& sink, const CORE::CValueList& options )
{GUCEF_TRACE;

    if ( encodedOutput.IsNULL() )
        return false;

    TIOAccessPtr output( encodedOutput );
    return OpenSinkImpl( output.GetPointerAlways(), output, format, audioTrackFormats, options, sink );
}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::OpenSinkImpl( CORE::CIOAccess*, const TIOAccessPtr&, const CVideoFormat&, const TAudioFormatVector&, const CORE::CValueList&, TVideoSinkPtr& )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::Decode( CORE::CIOAccess&, CORE::CIOAccess& )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIVideoCodec::Encode( CORE::CIOAccess&, CORE::CIOAccess& )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CString
CIVideoCodec::GetFamilyName( void ) const
{GUCEF_TRACE;

    return VideoCodecFamilyName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
