/*
 *  AssetWorkbenchLib: what a thread of a pool is asked to prepare for an open, and what came of it
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

#include "AssetWorkbenchLib_CAssetLoadTaskData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetLoadTaskData::CAssetLoadTaskData( void )
    : CORE::CICloneable()
    , job()
    , activity( GUCEF_NULL )
    , activityId( 0 )
    , prepared( false )
    , cancelled( false )
    , problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLoadTaskData::CAssetLoadTaskData( const CAssetLoadTaskData& src )
    : CORE::CICloneable( src )
    , job( src.job )
    , activity( src.activity )
    , activityId( src.activityId )
    , prepared( src.prepared )
    , cancelled( src.cancelled )
    , problem( src.problem )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLoadTaskData::~CAssetLoadTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLoadTaskData&
CAssetLoadTaskData::operator=( const CAssetLoadTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        job = src.job;
        activity = src.activity;
        activityId = src.activityId;
        prepared = src.prepared;
        cancelled = src.cancelled;
        problem = src.problem;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLoadTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLoadTaskData( *this );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
