/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CDrnUtilsGridAccessRules.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnUtilsGridAccessRules::CDrnUtilsGridAccessRules( void )
    : m_roles()
    , m_patterns()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsGridAccessRules::SetRoleMap( const CDrnUtilsRoleMap& roles )
{GUCEF_TRACE;

    m_roles = roles;
}

/*-------------------------------------------------------------------------*/

void
CDrnUtilsGridAccessRules::AddGridPattern( const CORE::CString& gridNamePattern, const CORE::CString& genericRole )
{GUCEF_TRACE;

    m_patterns[ gridNamePattern ] = genericRole;
}

/*-------------------------------------------------------------------------*/

/** Glob match: '*' matches any run of characters (also none) */
static bool
MatchesPattern( const char* pattern, const char* text )
{
    while ( '\0' != *pattern )
    {
        if ( '*' == *pattern )
        {
            ++pattern;
            if ( '\0' == *pattern )
                return true;
            for ( ; '\0' != *text; ++text )
                if ( MatchesPattern( pattern, text ) )
                    return true;
            return false;
        }
        if ( *pattern != *text )
            return false;
        ++pattern;
        ++text;
    }
    return '\0' == *text;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsGridAccessRules::IsGoverned( const CORE::CString& gridName ) const
{GUCEF_TRACE;

    TPatternMap::const_iterator i = m_patterns.begin();
    while ( i != m_patterns.end() )
    {
        if ( MatchesPattern( (*i).first.C_String(), gridName.C_String() ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsGridAccessRules::MaySubscribe( const CORE::CString::StringSet& memberRoles, const CORE::CString& gridName ) const
{GUCEF_TRACE;

    // every pattern that matches must be satisfied: the most restrictive one decides
    bool governed = false;
    TPatternMap::const_iterator i = m_patterns.begin();
    while ( i != m_patterns.end() )
    {
        if ( MatchesPattern( (*i).first.C_String(), gridName.C_String() ) )
        {
            governed = true;
            if ( !m_roles.HasRole( memberRoles, (*i).second ) )
                return false;
        }
        ++i;
    }
    return governed;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsGridAccessRules::MayWrite( const CORE::CString::StringSet& memberRoles, const CORE::CString& gridName ) const
{GUCEF_TRACE;

    // the grids are written by their publisher only; members never write them (and nothing else is decided here)
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
