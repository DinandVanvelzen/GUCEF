/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOMCORE_CIDiscoveryEntry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CIDiscoveryEntry::CIDiscoveryEntry( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDiscoveryEntry::CIDiscoveryEntry( const CIDiscoveryEntry& src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDiscoveryEntry::~CIDiscoveryEntry()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CIDiscoveryEntry& 
CIDiscoveryEntry::operator=( const CIDiscoveryEntry& src )
{GUCEF_TRACE;

    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIDiscoveryEntry::MakeEntryKey( const CORE::CString& serviceType, const CORE::CString& instanceId )
{GUCEF_TRACE;

    CORE::CString key( serviceType );
    key += '|';
    key += instanceId;
    return key;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIDiscoveryEntry::GetEntryKey( void ) const
{GUCEF_TRACE;

    CORE::CString instanceId;
    if ( !GetApplicationInstanceId( instanceId ) || instanceId.IsNULLOrEmpty() )
        instanceId = GetInstanceName();
    return MakeEntryKey( GetServiceType(), instanceId );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
