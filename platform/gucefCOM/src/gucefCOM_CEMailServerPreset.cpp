/*
 *  gucefCOM: GUCEF module providing communication implementations
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

/*
 *  gucefCOM: GUCEF module providing communication implementations
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



#include "gucefCOM_CEMailServerPreset.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailServerPreset::CEMailServerPreset( void )
    : host()
    , port( 0 )
    , security( MAILTLS_IMPLICIT )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEMailServerPreset::IsSet( void ) const
{GUCEF_TRACE;

    return !host.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

UInt16
CEMailServerPreset::GetEffectivePort( EMailProtocol protocol ) const
{GUCEF_TRACE;

    if ( 0 != port )
        return port;
    const bool implicit = MAILTLS_IMPLICIT == security;
    switch ( protocol )
    {
        case MAILPROTOCOL_IMAP: return implicit ? 993 : 143;
        case MAILPROTOCOL_POP3: return implicit ? 995 : 110;
        default: return implicit ? 465 : ( MAILTLS_NONE == security ? 25 : 587 );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
