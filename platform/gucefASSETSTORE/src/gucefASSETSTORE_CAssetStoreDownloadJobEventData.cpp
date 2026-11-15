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

#include "gucefASSETSTORE_CAssetStoreDownloadJobEventData.h"

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

CAssetStoreDownloadJobEventData::CAssetStoreDownloadJobEventData( void )
    : CORE::CICloneable()
    , job()
    , bytesPerSecond( 0 )
    , secondsLeft( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadJobEventData::CAssetStoreDownloadJobEventData( const CAssetStoreDownloadJob& jobState )
    : CORE::CICloneable()
    , job( jobState )
    , bytesPerSecond( 0 )
    , secondsLeft( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadJobEventData::~CAssetStoreDownloadJobEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreDownloadJobEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreDownloadJobEventData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreDownloadJobEventData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreDownloadJobEventData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreDownloadJobEventData";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
