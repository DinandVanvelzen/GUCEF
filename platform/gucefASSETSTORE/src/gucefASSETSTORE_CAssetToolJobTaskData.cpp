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

#include "gucefASSETSTORE_CAssetToolJobTaskData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetToolJobTaskData::TaskTypePrefix = "GUCEF::ASSETSTORE::AssetToolJob.";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetToolJobTaskData::CAssetToolJobTaskData( const CORE::CString& taskTypeName )
    : CORE::CIDataNodeSerializableTaskData()
    , kind()
    , commandId()
    , request( GUCEF_DATATYPE_OBJECT )
    , m_taskTypeName( taskTypeName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskData::CAssetToolJobTaskData( const CAssetToolJobTaskData& src )
    : CORE::CIDataNodeSerializableTaskData( src )
    , kind( src.kind )
    , commandId( src.commandId )
    , request( src.request )
    , m_taskTypeName( src.m_taskTypeName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetToolJobTaskData::~CAssetToolJobTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetToolJobTaskData::SetTaskTypeName( const CORE::CString& taskTypeName )
{GUCEF_TRACE;

    m_taskTypeName = taskTypeName;
}

/*-------------------------------------------------------------------------*/

void
CAssetToolJobTaskData::ToPayload( CORE::CDataNode& payload ) const
{GUCEF_TRACE;

    payload.Clear();
    payload.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings settings;
    Serialize( payload, settings );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetToolJobTaskData::GetTaskTypeName( void ) const
{GUCEF_TRACE;

    return m_taskTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetToolJobTaskData::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolJobTaskData::GetOnlyUseExistingThreadPool( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolJobTaskData::GetTaskCanBeQueued( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetToolJobTaskData::GetTaskId( void ) const
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolJobTaskData::Serialize( CORE::CDataNode& domRootNode                        ,
                                  const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "kind", kind );
    domRootNode.SetAttribute( "commandId", commandId );
    CORE::CDataNode* requestNode = domRootNode.AddChild( "request", GUCEF_DATATYPE_OBJECT );
    requestNode->Copy( request );
    requestNode->SetName( "request" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetToolJobTaskData::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    kind = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true );
    commandId = domRootNode.GetAttributeValueOrChildValueByName( "commandId" ).AsString( CORE::CString::Empty, true );
    const CORE::CDataNode* requestNode = domRootNode.FindChild( "request" );
    if ( GUCEF_NULL != requestNode )
        request.Copy( *requestNode );
    return !kind.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetToolJobTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetToolJobTaskData( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetToolJobTaskData::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetToolJobTaskData::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetToolJobTaskData";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetToolJobTaskData::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetToolJobTaskData";
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
