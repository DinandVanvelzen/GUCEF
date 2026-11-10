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

#ifndef GUCEF_COM_CIMAPMAILBOXNAME_H
#define GUCEF_COM_CIMAPMAILBOXNAME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAPVALUE_H
#include "gucefCOM_CImapValue.h"
#define GUCEF_COM_CIMAPVALUE_H
#endif /* GUCEF_COM_CIMAPVALUE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The names of mailboxes on the wire: modified UTF-7 (RFC 3501 section 5.1.3) */
class GUCEF_COM_PUBLIC_CPP CImapMailboxName
{
    public:

    /** UTF-8 to the form for the wire. @return false for a name with a control character */
    static bool Encode( const CORE::CString& utf8Name, CORE::CString& wireName );

    /** The form of the wire to UTF-8. @return false for a name that is not valid modified UTF-7 */
    static bool Decode( const CORE::CString& wireName, CORE::CString& utf8Name );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPMAILBOXNAME_H ? */
