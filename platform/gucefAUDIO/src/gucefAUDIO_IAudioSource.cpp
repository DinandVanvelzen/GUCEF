/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#include "gucefAUDIO_IAudioSource.h"
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

IAudioSource::~IAudioSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
IAudioSource::IsRealtimeSafe( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
IAudioSource::IsSeekable( void ) const
{
    // no trace macro: this runs where the audio is mixed

    return false;
}

/*-------------------------------------------------------------------------*/

bool
IAudioSource::GetMetaData( CORE::CValueList& ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
IAudioSource::SeekToFrame( UInt64 )
{
    // no trace macro: this runs where the audio is mixed

    return false;
}

/*-------------------------------------------------------------------------*/

UInt64
IAudioSource::GetLengthInFrames( void ) const
{GUCEF_TRACE;

    return GUCEF_AUDIO_LENGTH_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

bool
IAudioSource::Rewind( void )
{GUCEF_TRACE;

    return SeekToFrame( 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
