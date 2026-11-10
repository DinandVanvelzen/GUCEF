/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOMCORE_CIDatabaseClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CIDatabaseClient::DatabaseOpenedEvent = "GUCEF::COMCORE::CIDatabaseClient::DatabaseOpenedEvent";
const CORE::CEvent CIDatabaseClient::DatabaseClosedEvent = "GUCEF::COMCORE::CIDatabaseClient::DatabaseClosedEvent";
const CORE::CEvent CIDatabaseClient::DatabaseErrorEvent = "GUCEF::COMCORE::CIDatabaseClient::DatabaseErrorEvent";
const CORE::CEvent CIDatabaseClient::TableCreatedEvent = "GUCEF::COMCORE::CIDatabaseClient::TableCreatedEvent";
const CORE::CEvent CIDatabaseClient::TableDroppedEvent = "GUCEF::COMCORE::CIDatabaseClient::TableDroppedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CIDatabaseClient::RegisterEvents( void )
{GUCEF_TRACE;

    DatabaseOpenedEvent.Initialize();
    DatabaseClosedEvent.Initialize();
    DatabaseErrorEvent.Initialize();
    TableCreatedEvent.Initialize();
    TableDroppedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIDatabaseClient::CIDatabaseClient( void )
    : CORE::CTSGNotifier()
    , CORE::CIConfigurable()
    , m_settings()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIDatabaseClient::~CIDatabaseClient()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CDatabaseClientSettings&
CIDatabaseClient::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CIDatabaseClient::SetSettings( const CDatabaseClientSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

bool
CIDatabaseClient::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.Serialize( config, CORE::CDataNodeSerializableSettings() );
}

/*-------------------------------------------------------------------------*/

bool
CIDatabaseClient::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return m_settings.Deserialize( config, CORE::CDataNodeSerializableSettings() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIDatabaseClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CIDatabaseClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
