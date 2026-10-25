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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefAUDIO_CAudioDevice.h"

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

CAudioDevice::CAudioDevice( const CAudioDeviceInfo& info, IAudioDeviceCallback* callback )
    : m_info( info )
    , m_callback( callback )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDevice::~CAudioDevice()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAudioDeviceInfo&
CAudioDevice::GetDeviceInfo( void ) const
{GUCEF_TRACE;

    return m_info;
}

/*-------------------------------------------------------------------------*/

TAudioDirection
CAudioDevice::GetDirection( void ) const
{GUCEF_TRACE;

    return m_info.GetDirection();
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioDevice::GetPeriodSizeInFrames( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioDevice::GetLatencyInFrames( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

CNullAudioDevice*
CAudioDevice::AsNullDevice( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAudioPushDevice*
CAudioDevice::AsPushDevice( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

IAudioDeviceCallback*
CAudioDevice::GetCallback( void ) const
{GUCEF_TRACE;

    return m_callback;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
