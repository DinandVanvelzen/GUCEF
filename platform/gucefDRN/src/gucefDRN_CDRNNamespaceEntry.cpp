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

#include <string.h>

#include "gucefDRN_CDRNNamespaceEntry.h"

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

CDRNNamespaceEntry::CDRNNamespaceEntry( void )
    : name()
    , kind( DRN_NS_KIND_FILE )
    , size( 0 )
    , modifiedUtcMs( 0 )
    , flags( 0 )
    , revision()
    , hasSha256( false )
    , status( DRN_RESSTATUS_OK )
{GUCEF_TRACE;

    memset( sha256, 0, sizeof( sha256 ) );
}

/*-------------------------------------------------------------------------*/

void
CDRNNamespaceEntry::Clear( void )
{GUCEF_TRACE;

    name.Clear();
    kind = DRN_NS_KIND_FILE;
    size = 0;
    modifiedUtcMs = 0;
    flags = 0;
    revision.Clear();
    hasSha256 = false;
    memset( sha256, 0, sizeof( sha256 ) );
    status = DRN_RESSTATUS_OK;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceEntry::IsDirectory( void ) const
{GUCEF_TRACE;

    return DRN_NS_KIND_DIRECTORY == kind;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNamespaceEntry::IsFile( void ) const
{GUCEF_TRACE;

    return DRN_NS_KIND_FILE == kind;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
