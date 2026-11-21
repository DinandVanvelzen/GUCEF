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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H
#include "gucefASSETSTORE_CAssetLibrarySyncScheduleTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCSCHEDULETASKCONSUMER_H ? */

#include "gucefASSETSTORE_CAssetLibrarySyncScheduleTaskData.h"

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

CAssetLibrarySyncScheduleTaskData::CAssetLibrarySyncScheduleTaskData( void )
    : CORE::CIDataNodeSerializableTaskData()
    , serviceKey()
    , storeId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncScheduleTaskData::CAssetLibrarySyncScheduleTaskData( const CAssetLibrarySyncScheduleTaskData& src )
    : CORE::CIDataNodeSerializableTaskData( src )
    , serviceKey( src.serviceKey )
    , storeId( src.storeId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetLibrarySyncScheduleTaskData::~CAssetLibrarySyncScheduleTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetLibrarySyncScheduleTaskData::ToPayload( CORE::CDataNode& payload ) const
{GUCEF_TRACE;

    payload.Clear();
    payload.SetName( "payload" );
    payload.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings settings;
    Serialize( payload, settings );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncScheduleTaskData::GetTaskTypeName( void ) const
{GUCEF_TRACE;

    return CAssetLibrarySyncScheduleTaskConsumer::TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncScheduleTaskData::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncScheduleTaskData::GetOnlyUseExistingThreadPool( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncScheduleTaskData::GetTaskCanBeQueued( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetLibrarySyncScheduleTaskData::GetTaskId( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncScheduleTaskData::Serialize( CORE::CDataNode& domRootNode                        ,
                                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "serviceKey", serviceKey );
    success = domRootNode.SetAttribute( "storeId", storeId ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetLibrarySyncScheduleTaskData::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    serviceKey = domRootNode.GetAttributeValueOrChildValueByName( "serviceKey" ).AsString( CORE::CString::Empty, true );
    storeId = domRootNode.GetAttributeValueOrChildValueByName( "storeId" ).AsString( CORE::CString::Empty, true );
    return !serviceKey.IsNULLOrEmpty() && !storeId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetLibrarySyncScheduleTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetLibrarySyncScheduleTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLibrarySyncScheduleTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetLibrarySyncScheduleTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetLibrarySyncScheduleTaskData";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetLibrarySyncScheduleTaskData::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetLibrarySyncScheduleTaskData";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
