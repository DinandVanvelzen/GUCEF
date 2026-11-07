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

#ifndef GUCEF_COM_CMAILSETUPVALIDATION_H
#define GUCEF_COM_CMAILSETUPVALIDATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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

/**
 *  Checks of what a person types when a mailbox is set up: the address, the user name, the server names and ports, the folder and the name
 *  the other players see. All input is hostile: every check is bounded before any text is copied, accepts nothing but what a mail server
 *  name or address can be (plain ASCII, no control characters, no spaces), and a failed check gives a sentence for the person that
 *  never repeats the input. Header injection is excluded by construction: a carriage return, a line feed and every other control
 *  character is refused everywhere. Addresses with non ASCII characters need SMTPUTF8 and are not accepted here.
 */
class GUCEF_COM_PUBLIC_CPP CMailSetupValidation
{
    public:

    static const UInt32 MaxAddressLength = 254;         /**< RFC 5321 path limit */
    static const UInt32 MaxLocalPartLength = 64;
    static const UInt32 MaxDomainLength = 253;
    static const UInt32 MaxLabelLength = 63;
    static const UInt32 MaxUserNameLength = 256;
    static const UInt32 MaxFolderNameLength = 255;
    static const UInt32 MaxDisplayNameLength = 100;

    /** Splits an address at its one '@' into the local part and the domain, both as written; false when it is not a plausible address (see CheckAddress()) */
    static bool SplitAddress( const CORE::CString& address, CORE::CString& localPart, CORE::CString& domain );

    /** The domain of an address in lower case, empty when the address is not valid. Plus addressing (name+tag@example.org) is a valid local part */
    static CORE::CString GetDomain( const CORE::CString& address );

    /** Whether the text is a host name of dot separated labels (letters, digits and hyphens, not starting or ending with a hyphen) or an IPv4 address; also "localhost" */
    static bool IsValidHostName( const CORE::CString& host );

    /** The checks below: true when the text is fine, else false and 'problem' is a sentence for the person */
    static bool CheckAddress( const CORE::CString& address, CORE::CString& problem );
    static bool CheckHost( const CORE::CString& host, CORE::CString& problem );
    static bool CheckUserName( const CORE::CString& userName, CORE::CString& problem );           /**< empty is fine: the address is used */
    static bool CheckFolderName( const CORE::CString& folder, CORE::CString& problem );           /**< empty is fine where a folder is optional */
    static bool CheckDisplayName( const CORE::CString& name, CORE::CString& problem );            /**< empty is fine; any text without control characters */

    /** A port as typed: empty is fine and means the usual port (port becomes 0), else a whole number from 1 to 65535 */
    static bool CheckPortText( const CORE::CString& text, UInt16& port, CORE::CString& problem );

    /** Whether the text has a control character (below space, DEL) or is longer than 'maxLength' bytes */
    static bool IsPlainText( const CORE::CString& text, UInt32 maxLength );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILSETUPVALIDATION_H ? */
