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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNDataStreamSettings.h"

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

const CORE::CString CDRNDataStreamSettings::ClassTypeName = "GUCEF::DRN::CDRNDataStreamSettings";

/*-------------------------------------------------------------------------*/

CDRNDataStreamSettings::CDRNDataStreamSettings( void )
    : CORE::CIConfigurable()
    , name()
    , backlogLimit( 256 )
    , retainedHistory( 0 )
    , priority( 1.0f )
    , notify( DRN_NOTIFY_PER_PACKET )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataStreamSettings::CDRNDataStreamSettings( const CDRNDataStreamSettings& src )
    : CORE::CIConfigurable( src )
    , name( src.name )
    , backlogLimit( src.backlogLimit )
    , retainedHistory( src.retainedHistory )
    , priority( src.priority )
    , notify( src.notify )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataStreamSettings::~CDRNDataStreamSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNDataStreamSettings&
CDRNDataStreamSettings::operator=( const CDRNDataStreamSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        name = src.name;
        backlogLimit = src.backlogLimit;
        retainedHistory = src.retainedHistory;
        priority = src.priority;
        notify = src.notify;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStreamSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "name", name ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "backlogLimit", backlogLimit ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "retainedHistory", retainedHistory ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "priority", priority ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "notify", CDRNDataGridSettings::NotifyModeToString( notify ) ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDataStreamSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    name = cfg.GetAttributeValueOrChildValueByName( "name" ).AsString( name, true );
    backlogLimit = cfg.GetAttributeValueOrChildValueByName( "backlogLimit" ).AsUInt32( backlogLimit, true );
    retainedHistory = cfg.GetAttributeValueOrChildValueByName( "retainedHistory" ).AsUInt32( retainedHistory, true );
    priority = cfg.GetAttributeValueOrChildValueByName( "priority" ).AsFloat32( priority, true );
    notify = CDRNDataGridSettings::StringToNotifyMode( cfg.GetAttributeValueOrChildValueByName( "notify" ).AsString( CDRNDataGridSettings::NotifyModeToString( notify ), true ), notify );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDataStreamSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
