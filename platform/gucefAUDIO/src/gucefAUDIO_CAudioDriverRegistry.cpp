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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include <algorithm>

#include "gucefAUDIO_CAudioDriverRegistry.h"

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

CAudioDriverRegistry::CAudioDriverRegistry( void )
    : CORE::CTGlobalONRegistry< CAudioDriver, MT::CMutex >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioDriverRegistry::~CAudioDriverRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioDriverRegistry::TryFindDriver( const CString& driverName, TAudioDriverPtr& driver ) const
{GUCEF_TRACE;

    return TryLookup( driverName.Lowercase(), driver );
}

/*-------------------------------------------------------------------------*/

namespace {

struct SDriverPriorityLess
{
    bool operator()( const CAudioDriverRegistry::TAudioDriverPtr& a, const CAudioDriverRegistry::TAudioDriverPtr& b ) const
    {
        return a->GetPriority() > b->GetPriority();
    }
};

}

void
CAudioDriverRegistry::GetDriversByPriority( GUCEF::vector< TAudioDriverPtr >& drivers ) const
{GUCEF_TRACE;

    drivers.clear();
    TRegisteredObjPtrVector all;
    GetRegisteredObjs( all );
    for ( size_t i=0; i<all.size(); ++i )
        if ( !all[ i ].IsNULL() )
            drivers.push_back( all[ i ] );
    std::stable_sort( drivers.begin(), drivers.end(), SDriverPriorityLess() );
}

/*-------------------------------------------------------------------------*/

bool
CAudioDriverRegistry::OpenDevice( const CString& driverName, const CAudioDeviceParams& params, IAudioDeviceCallback* callback, TAudioDevicePtr& device, CString& usedDriverName ) const
{GUCEF_TRACE;

    usedDriverName.Clear();
    if ( !driverName.IsNULLOrEmpty() )
    {
        TAudioDriverPtr driver;
        if ( !TryFindDriver( driverName, driver ) || driver.IsNULL() )
            return false;
        if ( !driver->OpenDevice( params, callback, device ) )
            return false;
        usedDriverName = driver->GetName();
        return true;
    }

    GUCEF::vector< TAudioDriverPtr > drivers;
    GetDriversByPriority( drivers );
    for ( size_t i=0; i<drivers.size(); ++i )
    {
        if ( drivers[ i ]->OpenDevice( params, callback, device ) )
        {
            usedDriverName = drivers[ i ]->GetName();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
