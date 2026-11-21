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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefASSETSTORE_CAssetPackIndexReport.h"

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

CAssetPackIndexReport::CAssetPackIndexReport( void )
    : CORE::CICloneable()
    , status( ASSETSTORE_STATUS_OK )
    , message()
    , packsSeen( 0 )
    , packsLoaded( 0 )
    , packsUnchanged( 0 )
    , packsBuilt( 0 )
    , packsDerived( 0 )
    , packsSkipped( 0 )
    , packsFailed( 0 )
    , documentsWithoutPack( 0 )
    , problems()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPackIndexReport::~CAssetPackIndexReport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackIndexReport::ToSummaryText( void ) const
{GUCEF_TRACE;

    return CORE::ToString( packsSeen ) + " pack(s): " + CORE::ToString( packsLoaded ) + " loaded, " + CORE::ToString( packsUnchanged ) + " unchanged, " +
           CORE::ToString( packsBuilt ) + " built, " + CORE::ToString( packsDerived ) + " derived, " + CORE::ToString( packsSkipped ) + " skipped, " +
           CORE::ToString( packsFailed ) + " failed, " + CORE::ToString( documentsWithoutPack ) + " document(s) without a pack";
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetPackIndexReport::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetPackIndexReport( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPackIndexReport::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetPackIndexReport::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetPackIndexReport";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
