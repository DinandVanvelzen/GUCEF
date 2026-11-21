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

#include "gucefDRN_CIDRNResourceSink.h"

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

CIDRNResourceSink::CIDRNResourceSink( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNResourceSink::~CIDRNResourceSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIDRNResourceSink::PeekPartialState( CDRNResourcePartialState& /* state */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CIDRNResourceSink::Commit( const CDRNResourcePartialState& /* state */ )
{GUCEF_TRACE;

    return DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CIDRNResourceSink::ReadBack( UInt64 /* offset */, UInt32 /* size */, void* /* destination */ )
{GUCEF_TRACE;

    return DRN_RESSTATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

bool
CIDRNResourceSink::IsInOrder( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIDRNResourceSink::IsResumable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
