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

#include "gucefGUI_CLayoutLintOptions.h"

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

CLayoutLintOptions::CLayoutLintOptions( void )
    : failSeverity( LAYOUTLINTSEVERITY_ERROR )
    , strict( false )
    , root( GUCEF_NULL )
    , layoutFirst( true )
    , minHitTarget( DefaultMinHitTarget )
    , minRows( DefaultMinRows )
    , relatedSpacing( DefaultRelatedSpacing )
    , maxAnchoredInteractive( DefaultMaxAnchoredInteractive )
    , tolerance( DefaultTolerance )
    , minContrast( 4.5f )
    , minContrastLargeText( 3.0f )
    , largeTextLineHeight( 32 )
    , m_suppressions()
{GUCEF_TRACE;

    EnableAllRules( true );
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintOptions::EnableAllRules( bool enabled )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) LAYOUTLINTRULE_COUNT; ++i )
        m_enabled[ i ] = enabled;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintOptions::EnableRule( ELayoutLintRule rule, bool enabled )
{GUCEF_TRACE;

    if ( (UInt32) rule < (UInt32) LAYOUTLINTRULE_COUNT )
        m_enabled[ rule ] = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintOptions::IsRuleEnabled( ELayoutLintRule rule ) const
{GUCEF_TRACE;

    return (UInt32) rule < (UInt32) LAYOUTLINTRULE_COUNT && m_enabled[ rule ];
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintOptions::AddSuppression( ELayoutLintRule rule, const CString& pathPattern )
{GUCEF_TRACE;

    SSuppression suppression;
    suppression.rule = rule;
    if ( (UInt32) rule >= (UInt32) LAYOUTLINTRULE_COUNT || !suppression.pattern.SetPattern( pathPattern, true ) )
        return false;
    m_suppressions.push_back( suppression );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintOptions::ClearSuppressions( void )
{GUCEF_TRACE;

    m_suppressions.clear();
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintOptions::IsSuppressed( ELayoutLintRule rule, const CString& widgetPath ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) m_suppressions.size(); ++i )
    {
        if ( m_suppressions[ i ].rule == rule && m_suppressions[ i ].pattern.IsMatch( widgetPath ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
