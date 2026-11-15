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

#ifndef GUCEF_GUI_CLAYOUTLINTSWEEP_H
#define GUCEF_GUI_CLAYOUTLINTSWEEP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CLAYOUTLINT_H
#include "gucefGUI_CLayoutLint.h"
#define GUCEF_GUI_CLAYOUTLINT_H
#endif /* GUCEF_GUI_CLAYOUTLINT_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINTMATRIX_H
#include "gucefGUI_CLayoutLintMatrix.h"
#define GUCEF_GUI_CLAYOUTLINTMATRIX_H
#endif /* GUCEF_GUI_CLAYOUTLINTMATRIX_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Runs the layout lint over every combination of a matrix (CLayoutLintMatrix) on what a context shows and adds up what it
 *  finds: the warnings and errors per rule (what a CLayoutLintBudget is checked against), the totals per severity and a line
 *  per combination. The context is restored afterwards.
 *
 *  Two hooks for what the sweep cannot know: ApplyTextScale() registers the fonts of the application again at a larger size
 *  (the default cannot and skips those combinations), Settle() lets the application and its forms catch up after a change of
 *  size (the default draws three frames of 16 ms; an application with a loop of its own pulses that instead).
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutLintSweep
{
    public:

    CLayoutLintSweep( void );

    virtual ~CLayoutLintSweep();

    /** Every combination of the matrix: apply, settle, lint, add up; then the matrix restores the context. Returns the errors found */
    UInt32 Run( CGUIContext& context, CLayoutLintMatrix& matrix, const CLayoutLintOptions& options );

    /** Warnings and errors per rule, summed over the combinations (LAYOUTLINTRULE_COUNT entries) */
    const UInt32* GetRuleCounts( void ) const;

    UInt32 GetCount( ELayoutLintSeverity severity ) const;

    /** A line per combination: "1600x900 pseudo: 3 errors, 1 warnings, 2 info" */
    const CString& GetSummary( void ) const;

    /** What the run found for a combination ('index' of the matrix); empty for one that was skipped */
    const TLayoutLintFindingVector& GetFindings( UInt32 index ) const;

    protected:

    /** Registers the fonts again at their size times 'scale' (and back at 1); false: the combination is skipped */
    virtual bool ApplyTextScale( CGUIContext& context, Float32 scale );

    /** After a combination is applied: what the forms do on their next frames happens here */
    virtual void Settle( CGUIContext& context );

    private:

    CLayoutLintSweep( const CLayoutLintSweep& src );
    CLayoutLintSweep& operator=( const CLayoutLintSweep& src );

    typedef GUCEF::vector< TLayoutLintFindingVector > TFindingsVector;

    UInt32 m_ruleCounts[ LAYOUTLINTRULE_COUNT ];
    UInt32 m_counts[ LAYOUTLINTSEVERITY_COUNT ];
    CString m_summary;
    TFindingsVector m_findings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTLINTSWEEP_H ? */
