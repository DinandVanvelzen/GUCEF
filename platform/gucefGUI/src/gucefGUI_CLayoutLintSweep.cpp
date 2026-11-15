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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CLayoutLintSweep.h"

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

CLayoutLintSweep::CLayoutLintSweep( void )
    : m_summary()
    , m_findings()
{GUCEF_TRACE;

    for ( UInt32 r=0; r<(UInt32) LAYOUTLINTRULE_COUNT; ++r )
        m_ruleCounts[ r ] = 0;
    for ( UInt32 s=0; s<(UInt32) LAYOUTLINTSEVERITY_COUNT; ++s )
        m_counts[ s ] = 0;
}

/*-------------------------------------------------------------------------*/

CLayoutLintSweep::~CLayoutLintSweep()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintSweep::ApplyTextScale( CGUIContext& context, Float32 scale )
{GUCEF_TRACE;

    return 1.0f == scale;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintSweep::Settle( CGUIContext& context )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<3; ++i )
        context.RenderFrame( 16 );
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutLintSweep::Run( CGUIContext& context, CLayoutLintMatrix& matrix, const CLayoutLintOptions& options )
{GUCEF_TRACE;

    for ( UInt32 r=0; r<(UInt32) LAYOUTLINTRULE_COUNT; ++r )
        m_ruleCounts[ r ] = 0;
    for ( UInt32 s=0; s<(UInt32) LAYOUTLINTSEVERITY_COUNT; ++s )
        m_counts[ s ] = 0;
    m_summary.Clear();
    m_findings.clear();
    m_findings.resize( matrix.GetCombinationCount() );

    CLayoutLint lint;
    for ( UInt32 c=0; c<matrix.GetCombinationCount(); ++c )
    {
        const CLayoutLintMatrix::TCombination& combination = matrix.GetCombination( c );
        matrix.ApplyCombination( c, context );
        const bool scaled = 1.0f != combination.textScale;
        if ( scaled && !ApplyTextScale( context, combination.textScale ) )
        {
            m_summary += matrix.DescribeCombination( c ) + ": skipped, the fonts cannot be scaled\n";
            continue;
        }
        Settle( context );

        TLayoutLintFindingVector& findings = m_findings[ c ];
        lint.Run( context, options, findings );
        CLayoutLint::CountByRule( findings, LAYOUTLINTSEVERITY_WARNING, m_ruleCounts );
        UInt32 counts[ LAYOUTLINTSEVERITY_COUNT ] = { 0, 0, 0 };
        for ( UInt32 i=0; i<(UInt32) findings.size(); ++i )
            if ( (UInt32) findings[ i ].severity < (UInt32) LAYOUTLINTSEVERITY_COUNT )
                ++counts[ findings[ i ].severity ];
        for ( UInt32 s=0; s<(UInt32) LAYOUTLINTSEVERITY_COUNT; ++s )
            m_counts[ s ] += counts[ s ];
        m_summary += matrix.DescribeCombination( c ) + ": " + CORE::UInt32ToString( counts[ LAYOUTLINTSEVERITY_ERROR ] ) + " errors, " +
                     CORE::UInt32ToString( counts[ LAYOUTLINTSEVERITY_WARNING ] ) + " warnings, " + CORE::UInt32ToString( counts[ LAYOUTLINTSEVERITY_INFO ] ) + " info\n";

        if ( scaled )
            ApplyTextScale( context, 1.0f );
    }
    matrix.Restore( context );
    Settle( context );
    return m_counts[ LAYOUTLINTSEVERITY_ERROR ];
}

/*-------------------------------------------------------------------------*/

const UInt32*
CLayoutLintSweep::GetRuleCounts( void ) const
{GUCEF_TRACE;

    return m_ruleCounts;
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutLintSweep::GetCount( ELayoutLintSeverity severity ) const
{GUCEF_TRACE;

    if ( (UInt32) severity >= (UInt32) LAYOUTLINTSEVERITY_COUNT )
        return 0;
    return m_counts[ severity ];
}

/*-------------------------------------------------------------------------*/

const CString&
CLayoutLintSweep::GetSummary( void ) const
{GUCEF_TRACE;

    return m_summary;
}

/*-------------------------------------------------------------------------*/

const TLayoutLintFindingVector&
CLayoutLintSweep::GetFindings( UInt32 index ) const
{GUCEF_TRACE;

    static const TLayoutLintFindingVector none;
    if ( index >= (UInt32) m_findings.size() )
        return none;
    return m_findings[ index ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
