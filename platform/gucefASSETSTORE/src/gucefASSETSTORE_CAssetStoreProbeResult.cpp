/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include "gucefASSETSTORE_CAssetStoreProbeResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreProbeResult::SCheck::SCheck( void )
    : name()
    , outcome()
    , passed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreProbeResult::SCheck::SCheck( const CORE::CString& checkName, const CORE::CString& checkOutcome, bool checkPassed )
    : name( checkName )
    , outcome( checkOutcome )
    , passed( checkPassed )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreProbeResult::CAssetStoreProbeResult( void )
    : CORE::CICloneable()
    , checks()
    , summary()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreProbeResult::~CAssetStoreProbeResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreProbeResult::Clear( void )
{GUCEF_TRACE;

    checks.clear();
    summary.Clear();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreProbeResult::AddCheck( const CORE::CString& name, const CORE::CString& outcome, bool passed )
{GUCEF_TRACE;

    checks.push_back( TCheck( name, outcome, passed ) );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreProbeResult::IsAllPassed( void ) const
{GUCEF_TRACE;

    return !checks.empty() && GetPassedCount() == (UInt32) checks.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreProbeResult::GetPassedCount( void ) const
{GUCEF_TRACE;

    UInt32 passed = 0;
    for ( size_t i=0; i<checks.size(); ++i )
        if ( checks[ i ].passed )
            ++passed;
    return passed;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreProbeResult::GetLines( CORE::CString::StringVector& lines ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<checks.size(); ++i )
        lines.push_back( ( checks[ i ].passed ? CORE::CString( "ok: " ) : CORE::CString( "failed: " ) ) + checks[ i ].name + ": " + checks[ i ].outcome );
    if ( !summary.IsNULLOrEmpty() )
        lines.push_back( summary );
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreProbeResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreProbeResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreProbeResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreProbeResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreProbeResult";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
