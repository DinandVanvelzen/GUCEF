/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#include "gucefWEB_CSiteDiscoverySettings.h"
#define GUCEF_WEB_CSITEDISCOVERYSETTINGS_H
#endif /* GUCEF_WEB_CSITEDISCOVERYSETTINGS_H ? */

#ifndef GUCEF_WEB_CNAVIGATIONRULE_H
#include "gucefWEB_CNavigationRule.h"
#define GUCEF_WEB_CNAVIGATIONRULE_H
#endif /* GUCEF_WEB_CNAVIGATIONRULE_H ? */

#include "gucefWEB_CNavigationGuardSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNavigationGuardSettings::CNavigationGuardSettings( void )
    : CORE::CIConfigurable()
    , allow()
    , deny()
    , guardSubframes( true )
    , refuseNeverRecordUnlessAllowed( true )
    , neverRecordHosts( CSiteDiscoverySettings::GetDefaultNeverRecordHosts() )
    , neverRecordPathPatterns( CSiteDiscoverySettings::GetDefaultNeverRecordPathPatterns() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNavigationGuardSettings::CNavigationGuardSettings( const CNavigationGuardSettings& src )
    : CORE::CIConfigurable( src )
    , allow( src.allow )
    , deny( src.deny )
    , guardSubframes( src.guardSubframes )
    , refuseNeverRecordUnlessAllowed( src.refuseNeverRecordUnlessAllowed )
    , neverRecordHosts( src.neverRecordHosts )
    , neverRecordPathPatterns( src.neverRecordPathPatterns )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNavigationGuardSettings::~CNavigationGuardSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CNavigationGuardSettings&
CNavigationGuardSettings::operator=( const CNavigationGuardSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        allow = src.allow;
        deny = src.deny;
        guardSubframes = src.guardSubframes;
        refuseNeverRecordUnlessAllowed = src.refuseNeverRecordUnlessAllowed;
        neverRecordHosts = src.neverRecordHosts;
        neverRecordPathPatterns = src.neverRecordPathPatterns;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuardSettings::AreRulesValid( CORE::CString* invalidRule ) const
{GUCEF_TRACE;

    CNavigationRule rule;
    for ( size_t i=0; i<allow.size(); ++i )
    {
        if ( !CNavigationRule::Parse( allow[ i ], rule ) )
        {
            if ( GUCEF_NULL != invalidRule )
                *invalidRule = allow[ i ];
            return false;
        }
    }
    for ( size_t i=0; i<deny.size(); ++i )
    {
        if ( !CNavigationRule::Parse( deny[ i ], rule ) )
        {
            if ( GUCEF_NULL != invalidRule )
                *invalidRule = deny[ i ];
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuardSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetValuesOfChildByName( "allow", allow );
    cfg.SetValuesOfChildByName( "deny", deny );
    cfg.SetAttribute( "guardSubframes", guardSubframes );
    cfg.SetAttribute( "refuseNeverRecordUnlessAllowed", refuseNeverRecordUnlessAllowed );
    cfg.SetValuesOfChildByName( "neverRecordHosts", neverRecordHosts );
    cfg.SetValuesOfChildByName( "neverRecordPathPatterns", neverRecordPathPatterns );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNavigationGuardSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CNavigationGuardSettings loaded( *this );
    CORE::CString::StringVector list;
    if ( cfg.GetStringListByName( "allow", list ) )
        loaded.allow = list;
    if ( cfg.GetStringListByName( "deny", list ) )
        loaded.deny = list;
    loaded.guardSubframes = cfg.GetAttributeValueOrChildValueByName( "guardSubframes" ).AsBool( guardSubframes, true );
    loaded.refuseNeverRecordUnlessAllowed = cfg.GetAttributeValueOrChildValueByName( "refuseNeverRecordUnlessAllowed" ).AsBool( refuseNeverRecordUnlessAllowed, true );
    if ( cfg.GetStringListByName( "neverRecordHosts", list ) )
        loaded.neverRecordHosts = list;
    if ( cfg.GetStringListByName( "neverRecordPathPatterns", list ) )
        loaded.neverRecordPathPatterns = list;

    CORE::CString invalid;
    if ( !loaded.AreRulesValid( &invalid ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "NavigationGuardSettings: The rule \"" << invalid << "\" is not a host or host/path rule" );
        return false;
    }
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CNavigationGuardSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CNavigationGuardSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
