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

#include "gucefGUI_ELayoutLintRule.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SRuleInfo
{
    const char* id;
    const char* name;
    ELayoutLintSeverity severity;
    bool complete;
};

/** In the order of the enumeration */
const SRuleInfo g_rules[ LAYOUTLINTRULE_COUNT ] =
{
    { "L01", "Overlap",             LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L02", "OutsideParent",       LAYOUTLINTSEVERITY_WARNING, true  },
    { "L03", "OutsideScreen",       LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L04", "TextClipped",         LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L05", "TextElidedNoTooltip", LAYOUTLINTSEVERITY_WARNING, true  },
    { "L06", "BelowMinimum",        LAYOUTLINTSEVERITY_WARNING, false },
    { "L07", "SmallTarget",         LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L08", "ZeroSize",            LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L09", "ViewportTooSmall",    LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L10", "TightSpacing",        LAYOUTLINTSEVERITY_WARNING, true  },
    { "L11", "EstimatedText",       LAYOUTLINTSEVERITY_WARNING, true  },
    { "L12", "NumberElided",        LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L13", "TabUnreachable",      LAYOUTLINTSEVERITY_ERROR,   false },
    { "L14", "ContentUnreachable",  LAYOUTLINTSEVERITY_ERROR,   true  },
    { "L15", "LowContrast",         LAYOUTLINTSEVERITY_WARNING, true  },
    { "L16", "EmptyViewport",       LAYOUTLINTSEVERITY_INFO,    true  },
    { "L17", "AbsoluteInFlow",      LAYOUTLINTSEVERITY_INFO,    true  }
};

/** The ids and names as strings, made on first use */
struct SRuleStrings
{
    CString ids[ LAYOUTLINTRULE_COUNT ];
    CString names[ LAYOUTLINTRULE_COUNT ];

    SRuleStrings( void )
    {
        for ( UInt32 i=0; i<(UInt32) LAYOUTLINTRULE_COUNT; ++i )
        {
            ids[ i ] = g_rules[ i ].id;
            names[ i ] = g_rules[ i ].name;
        }
    }
};

const SRuleStrings&
GetRuleStrings( void )
{
    static const SRuleStrings strings;
    return strings;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
LayoutLintRuleToString( ELayoutLintRule rule )
{GUCEF_TRACE;

    if ( (UInt32) rule >= (UInt32) LAYOUTLINTRULE_COUNT )
        return CString::Empty;
    return GetRuleStrings().names[ rule ];
}

/*-------------------------------------------------------------------------*/

const CString&
LayoutLintRuleToId( ELayoutLintRule rule )
{GUCEF_TRACE;

    if ( (UInt32) rule >= (UInt32) LAYOUTLINTRULE_COUNT )
        return CString::Empty;
    return GetRuleStrings().ids[ rule ];
}

/*-------------------------------------------------------------------------*/

bool
StringToLayoutLintRule( const CString& nameOrId, ELayoutLintRule& rule )
{GUCEF_TRACE;

    const SRuleStrings& strings = GetRuleStrings();
    const CString trimmed = nameOrId.Trim( true ).Trim( false );
    for ( UInt32 i=0; i<(UInt32) LAYOUTLINTRULE_COUNT; ++i )
    {
        if ( strings.ids[ i ].Equals( trimmed, false ) || strings.names[ i ].Equals( trimmed, false ) )
        {
            rule = (ELayoutLintRule) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

ELayoutLintSeverity
GetLayoutLintRuleDefaultSeverity( ELayoutLintRule rule )
{GUCEF_TRACE;

    if ( (UInt32) rule >= (UInt32) LAYOUTLINTRULE_COUNT )
        return LAYOUTLINTSEVERITY_INFO;
    return g_rules[ rule ].severity;
}

/*-------------------------------------------------------------------------*/

bool
IsLayoutLintRuleComplete( ELayoutLintRule rule )
{GUCEF_TRACE;

    if ( (UInt32) rule >= (UInt32) LAYOUTLINTRULE_COUNT )
        return false;
    return g_rules[ rule ].complete;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
