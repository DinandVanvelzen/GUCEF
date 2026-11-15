/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_CLayoutLintBudget.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLayoutLintBudget::CLayoutLintBudget( void )
    : m_limits()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CLayoutLintBudget::KeyOf( const CString& scope, ELayoutLintRule rule )
{GUCEF_TRACE;

    return scope + " " + LayoutLintRuleToId( rule );
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintBudget::LoadFromText( const CString& text, CString& problems )
{GUCEF_TRACE;

    bool allRead = true;
    CString::StringVector lines = text.ReplaceChar( '\r', '\n' ).ParseElements( '\n', false );
    for ( UInt32 i=0; i<(UInt32) lines.size(); ++i )
    {
        const CString line = lines[ i ].ReplaceChar( '\t', ' ' ).Trim( true ).Trim( false );
        if ( line.IsNULLOrEmpty() || '#' == line[ 0 ] )
            continue;
        CString::StringVector parts = line.ParseElements( ' ', false );
        ELayoutLintRule rule = LAYOUTLINTRULE_COUNT;
        if ( 3 != parts.size() || !StringToLayoutLintRule( parts[ 1 ], rule ) || parts[ 2 ].IsNULLOrEmpty() )
        {
            problems += "Cannot read the budget line \"" + line + "\"\n";
            allRead = false;
            continue;
        }
        SetLimit( parts[ 0 ], rule, CORE::StringToUInt32( parts[ 2 ] ) );
    }
    return allRead;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintBudget::SetLimit( const CString& scope, ELayoutLintRule rule, UInt32 limit )
{GUCEF_TRACE;

    m_limits[ KeyOf( scope, rule ) ] = limit;
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutLintBudget::GetLimit( const CString& scope, ELayoutLintRule rule ) const
{GUCEF_TRACE;

    TLimitMap::const_iterator i = m_limits.find( KeyOf( scope, rule ) );
    return i == m_limits.end() ? 0 : (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintBudget::Check( const CString& scope, const UInt32* counts, CString& problems, CString& improvements ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == counts )
        return true;
    bool within = true;
    for ( UInt32 r=0; r<(UInt32) LAYOUTLINTRULE_COUNT; ++r )
    {
        const ELayoutLintRule rule = (ELayoutLintRule) r;
        const UInt32 limit = GetLimit( scope, rule );
        if ( counts[ r ] > limit )
        {
            problems += scope + " " + LayoutLintRuleToId( rule ) + " " + LayoutLintRuleToString( rule ) + ": " + CORE::UInt32ToString( counts[ r ] ) +
                        " findings, the budget allows " + CORE::UInt32ToString( limit ) + "\n";
            within = false;
        }
        else
        if ( counts[ r ] < limit )
        {
            improvements += scope + " " + LayoutLintRuleToId( rule ) + " " + LayoutLintRuleToString( rule ) + ": " + CORE::UInt32ToString( counts[ r ] ) +
                            " findings, below the budget of " + CORE::UInt32ToString( limit ) + ": the budget can be lowered\n";
        }
    }
    return within;
}

/*-------------------------------------------------------------------------*/

CString
CLayoutLintBudget::FormatCounts( const CString& scope, const UInt32* counts )
{GUCEF_TRACE;

    CString text;
    if ( GUCEF_NULL == counts )
        return text;
    for ( UInt32 r=0; r<(UInt32) LAYOUTLINTRULE_COUNT; ++r )
        if ( counts[ r ] > 0 )
            text += scope + " " + LayoutLintRuleToId( (ELayoutLintRule) r ) + " " + CORE::UInt32ToString( counts[ r ] ) + "\n";
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
