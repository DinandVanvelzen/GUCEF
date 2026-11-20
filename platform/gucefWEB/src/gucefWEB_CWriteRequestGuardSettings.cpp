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

#ifndef GUCEF_WEB_CWRITEREQUESTRULE_H
#include "gucefWEB_CWriteRequestRule.h"
#define GUCEF_WEB_CWRITEREQUESTRULE_H
#endif /* GUCEF_WEB_CWRITEREQUESTRULE_H ? */

#include "gucefWEB_CWriteRequestGuardSettings.h"

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

CWriteRequestGuardSettings::CWriteRequestGuardSettings( void )
    : CORE::CIConfigurable()
    , allow()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWriteRequestGuardSettings::CWriteRequestGuardSettings( const CWriteRequestGuardSettings& src )
    : CORE::CIConfigurable( src )
    , allow( src.allow )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWriteRequestGuardSettings::~CWriteRequestGuardSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWriteRequestGuardSettings&
CWriteRequestGuardSettings::operator=( const CWriteRequestGuardSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        allow = src.allow;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuardSettings::AreRulesValid( CORE::CString* invalidRule, CORE::CString* problem ) const
{GUCEF_TRACE;

    CWriteRequestRule rule;
    for ( size_t i=0; i<allow.size(); ++i )
    {
        if ( !CWriteRequestRule::Parse( allow[ i ], rule, problem ) )
        {
            if ( GUCEF_NULL != invalidRule )
                *invalidRule = allow[ i ];
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuardSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetValuesOfChildByName( "allow", allow );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWriteRequestGuardSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CWriteRequestGuardSettings loaded( *this );
    CORE::CString::StringVector list;
    if ( cfg.GetStringListByName( "allow", list ) )
        loaded.allow = list;

    CORE::CString invalid;
    CORE::CString problem;
    if ( !loaded.AreRulesValid( &invalid, &problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "WriteRequestGuardSettings: The rule \"" << invalid << "\" is not a write request rule: " << problem );
        return false;
    }
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWriteRequestGuardSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CWriteRequestGuardSettings";
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
