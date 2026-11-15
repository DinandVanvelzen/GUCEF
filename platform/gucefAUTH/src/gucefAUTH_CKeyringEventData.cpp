/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#include "gucefAUTH_CKeyringEventData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKeyringEventData::CKeyringEventData( void )
    : CORE::CICloneable()
    , result( KEYRING_OK )
    , lockReason( KEYRINGLOCK_REQUESTED )
    , failedAttempts( 0 )
    , retryWaitInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyringEventData::CKeyringEventData( const CKeyringEventData& src )
    : CORE::CICloneable( src )
    , result( src.result )
    , lockReason( src.lockReason )
    , failedAttempts( src.failedAttempts )
    , retryWaitInMs( src.retryWaitInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyringEventData::~CKeyringEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKeyringEventData&
CKeyringEventData::operator=( const CKeyringEventData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        result = src.result;
        lockReason = src.lockReason;
        failedAttempts = src.failedAttempts;
        retryWaitInMs = src.retryWaitInMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CKeyringEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKeyringEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CKeyringEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CKeyringEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::AUTH::CKeyringEventData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
