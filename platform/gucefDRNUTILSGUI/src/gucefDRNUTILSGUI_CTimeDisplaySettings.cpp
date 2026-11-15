/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRNUTILSGUI_CTimeDisplaySettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CTimeDisplaySettings::ClassTypeName = "GUCEF::DRNUTILSGUI::CTimeDisplaySettings";
const char* CTimeDisplaySettings::LocalZoneName = "local";
const char* CTimeDisplaySettings::UtcZoneName = "utc";
const char* CTimeDisplaySettings::ConfigSectionName = "TimeDisplay";

/*-------------------------------------------------------------------------*/

CTimeDisplaySettings::CTimeDisplaySettings( void )
    : CORE::CIConfigurable()
    , m_displayTimeZone( "local" )
    , m_showZoneSuffix( true )
    , m_dateTimeFormat( "%Y-%m-%d %H:%M:%S" )
    , m_rules( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTimeDisplaySettings::~CTimeDisplaySettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplaySettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = config.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;
    section->SetAttribute( "displayTimeZone", m_displayTimeZone );
    section->SetAttribute( "showZoneSuffix", m_showZoneSuffix );
    section->SetAttribute( "dateTimeFormat", m_dateTimeFormat );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplaySettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &config;
    if ( config.GetName() != ConfigSectionName )
        section = config.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    const CORE::CString zone = section->GetAttributeValueOrChildValueByName( "displayTimeZone" ).AsString( CORE::CString::Empty, true );
    if ( !zone.IsNULLOrEmpty() )
        m_displayTimeZone = zone;
    m_showZoneSuffix = section->GetAttributeValueOrChildValueByName( "showZoneSuffix" ).AsBool( m_showZoneSuffix, true );
    const CORE::CString format = section->GetAttributeValueOrChildValueByName( "dateTimeFormat" ).AsString( CORE::CString::Empty, false );
    if ( !format.IsNULLOrEmpty() )
        m_dateTimeFormat = format;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTimeDisplaySettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CTimeDisplaySettings::SetDisplayTimeZone( const CORE::CString& displayTimeZone )
{GUCEF_TRACE;

    m_displayTimeZone = displayTimeZone;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTimeDisplaySettings::GetDisplayTimeZone( void ) const
{GUCEF_TRACE;

    return m_displayTimeZone;
}

/*-------------------------------------------------------------------------*/

void
CTimeDisplaySettings::SetShowZoneSuffix( bool showZoneSuffix )
{GUCEF_TRACE;

    m_showZoneSuffix = showZoneSuffix;
}

/*-------------------------------------------------------------------------*/

bool
CTimeDisplaySettings::GetShowZoneSuffix( void ) const
{GUCEF_TRACE;

    return m_showZoneSuffix;
}

/*-------------------------------------------------------------------------*/

void
CTimeDisplaySettings::SetDateTimeFormat( const CORE::CString& dateTimeFormat )
{GUCEF_TRACE;

    m_dateTimeFormat = dateTimeFormat;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTimeDisplaySettings::GetDateTimeFormat( void ) const
{GUCEF_TRACE;

    return m_dateTimeFormat;
}

/*-------------------------------------------------------------------------*/

void
CTimeDisplaySettings::SetTimeZoneRules( const CORE::CTimeZoneRules* rules )
{GUCEF_TRACE;

    m_rules = rules;
}

/*-------------------------------------------------------------------------*/

const CORE::CTimeZoneRules*
CTimeDisplaySettings::GetTimeZoneRules( void ) const
{GUCEF_TRACE;

    return m_rules;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
