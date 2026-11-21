/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include "gucefDRN_CDRNResourceIoTaskData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNResourceIoTaskData::CDRNResourceIoTaskData( void )
    : CORE::CICloneable()
    , job()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceIoTaskData::CDRNResourceIoTaskData( const CIDRNResourceIoJobPtr& theJob )
    : CORE::CICloneable()
    , job( theJob )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceIoTaskData::CDRNResourceIoTaskData( const CDRNResourceIoTaskData& src )
    : CORE::CICloneable( src )
    , job( src.job )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNResourceIoTaskData::~CDRNResourceIoTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDRNResourceIoTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDRNResourceIoTaskData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
