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

#ifndef GUCEF_GUI_CLAYOUTLINTBUDGET_H
#define GUCEF_GUI_CLAYOUTLINTBUDGET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_ELAYOUTLINTRULE_H
#include "gucefGUI_ELayoutLintRule.h"
#define GUCEF_GUI_ELAYOUTLINTRULE_H
#endif /* GUCEF_GUI_ELAYOUTLINTRULE_H ? */

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
 *  The findings a form is still allowed: per scope (a form, a screen, a test) and rule, the most findings a lint run over
 *  a matrix may give. It lets a code base adopt the lint while it has defects: the budget records what is there today,
 *  a test fails when a count goes above it, and the budget is lowered as defects are fixed, never raised. A scope that is
 *  not listed has a budget of zero for every rule, so new forms start clean.
 *
 *  The text form has one entry per line, "scope rule count" (rule as an id or a name), '#' starts a comment:
 *
 *      # the transfer queue before its rows are laid out from the font
 *      TransferQueue L04 12
 *      TransferQueue L07 30
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutLintBudget
{
    public:

    CLayoutLintBudget( void );

    /** Adds the entries of the text; false when a line cannot be read (the problems say which), the others are taken */
    bool LoadFromText( const CString& text, CString& problems );

    void SetLimit( const CString& scope, ELayoutLintRule rule, UInt32 limit );

    /** The limit of the rule for the scope, 0 when it is not listed */
    UInt32 GetLimit( const CString& scope, ELayoutLintRule rule ) const;

    /**
     *  Compares the counts per rule (LAYOUTLINTRULE_COUNT entries) of a scope with its limits. Returns false when a count
     *  is above its limit, with a line per such rule in 'problems'; a count below its limit gets a line in 'improvements'
     *  (the budget can be lowered)
     */
    bool Check( const CString& scope, const UInt32* counts, CString& problems, CString& improvements ) const;

    /** The counts as budget lines for the scope, the rules with a count only: what a budget of today's state reads */
    static CString FormatCounts( const CString& scope, const UInt32* counts );

    private:

    typedef GUCEF::map< CString, UInt32 > TLimitMap;    /**< "scope rule-id" to the limit */

    static CString KeyOf( const CString& scope, ELayoutLintRule rule );

    TLimitMap m_limits;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTLINTBUDGET_H ? */
