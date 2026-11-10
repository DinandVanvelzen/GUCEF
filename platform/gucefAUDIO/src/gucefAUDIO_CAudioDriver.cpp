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

#include "gucefAUDIO_CAudioDriver.h"

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

CAudioDriver::CAudioDriver( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDriver::~CAudioDriver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CAudioDriver::GetDescription( void ) const
{GUCEF_TRACE;

    return GetName();
}

/*-------------------------------------------------------------------------*/

Int32
CAudioDriver::GetPriority( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CAudioDriver::GetDefaultDevice( TAudioDirection direction, CAudioDeviceInfo& device )
{GUCEF_TRACE;

    TAudioDeviceInfoVector devices;
    if ( !EnumerateDevices( direction, devices ) || devices.empty() )
        return false;

    for ( size_t i=0; i<devices.size(); ++i )
        if ( devices[ i ].IsDefault() )
        {
            device = devices[ i ];
            return true;
        }
    device = devices[ 0 ];
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
