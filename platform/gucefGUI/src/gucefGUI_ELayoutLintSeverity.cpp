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

#include "gucefGUI_ELayoutLintSeverity.h"

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

/** The names, made on first use so the order of the static initializers of the modules never matters */
struct SSeverityNames
{
    CString names[ LAYOUTLINTSEVERITY_COUNT ];

    SSeverityNames( void )
    {
        names[ LAYOUTLINTSEVERITY_INFO ] = "Info";
        names[ LAYOUTLINTSEVERITY_WARNING ] = "Warning";
        names[ LAYOUTLINTSEVERITY_ERROR ] = "Error";
    }
};

const SSeverityNames&
GetSeverityNames( void )
{
    static const SSeverityNames names;
    return names;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
LayoutLintSeverityToString( ELayoutLintSeverity severity )
{GUCEF_TRACE;

    if ( (UInt32) severity >= (UInt32) LAYOUTLINTSEVERITY_COUNT )
        return CString::Empty;
    return GetSeverityNames().names[ severity ];
}

/*-------------------------------------------------------------------------*/

bool
StringToLayoutLintSeverity( const CString& name, ELayoutLintSeverity& severity )
{GUCEF_TRACE;

    const SSeverityNames& names = GetSeverityNames();
    for ( UInt32 i=0; i<(UInt32) LAYOUTLINTSEVERITY_COUNT; ++i )
    {
        if ( names.names[ i ].Equals( name, false ) )
        {
            severity = (ELayoutLintSeverity) i;
            return true;
        }
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
