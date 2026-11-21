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

#include "gucefDRN_CIDRNResourceProvider.h"

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

CIDRNResourceProvider::CIDRNResourceProvider( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNResourceProvider::~CIDRNResourceProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIDRNResourceProvider::GetHashTree( const CORE::CString& /* name */, const CDRNResourceInfo& /* resolved */, CDRNResourceHashTree& /* tree */ )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CIDRNResourceProvider::GetNamespaceCapabilities( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CIDRNResourceProvider::GetNamespaceFlags( void ) const
{GUCEF_TRACE;

    return DRN_NS_SHAREFLAG_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

void
CIDRNResourceProvider::GetNamespaceLimits( UInt32& maxStatPaths, UInt32& maxPageEntries ) const
{GUCEF_TRACE;

    maxStatPaths = 0;
    maxPageEntries = 0;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CIDRNResourceProvider::Stat( const CORE::CString& /* name */, CDRNNamespaceEntry& /* entry */ )
{GUCEF_TRACE;

    return DRN_RESSTATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

TDRNResourceStatus
CIDRNResourceProvider::OpenEnumerator( const CORE::CString& /* dirName */, CIDRNNamespaceEnumeratorPtr& /* enumerator */ )
{GUCEF_TRACE;

    return DRN_RESSTATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
