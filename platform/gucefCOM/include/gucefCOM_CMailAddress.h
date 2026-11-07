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

#ifndef GUCEF_COM_CMAILADDRESS_H
#define GUCEF_COM_CMAILADDRESS_H

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
 *  One mailbox: an address (local part and domain) and an optional display name, as in
 *  'Jane Doe <jane@example.com>'. The display name is UTF-8 text and the local part is stored unquoted; quoting
 *  and RFC 2047 encoding happen when the address is written to a header.
 *
 *  Addresses end up in protocol commands and headers, so everything here refuses control characters, white
 *  space, angle brackets and the like where they could change the meaning of a line: IsValid() is the check
 *  that the SMTP client and the composer apply before an address is used.
 */
class GUCEF_COM_PUBLIC_CPP CMailAddress
{
    public:

    typedef GUCEF::vector< CMailAddress > TMailAddressVector;

    CORE::CString displayName;      /**< UTF-8, may be empty */
    CORE::CString localPart;        /**< unquoted */
    CORE::CString domain;           /**< a host name or a domain literal in brackets */

    CMailAddress( void );

    /** Takes an address without display name ('jane@example.com'). The members stay empty when the text is not an address */
    explicit CMailAddress( const CORE::CString& addrSpec );

    CMailAddress( const CORE::CString& displayName, const CORE::CString& addrSpec );

    CMailAddress( const CMailAddress& src );

    ~CMailAddress();

    CMailAddress& operator=( const CMailAddress& src );

    bool IsEmpty( void ) const;

    /** The address as local@domain, with the local part quoted when it has to be. Empty when there is no address */
    CORE::CString GetAddress( void ) const;

    /** Splits 'local@domain' at the last @ that is not inside quotes. @return false when there is none or a side is empty */
    bool SetAddress( const CORE::CString& addrSpec );

    /**
     *  Whether the address is acceptable for an envelope and a header: dot-atom or quoted local part of at most 64
     *  bytes, a domain of host name labels or a bracketed literal, 254 bytes in all. Non ASCII bytes are only
     *  accepted with 'allowUtf8' (RFC 6531). Control characters, spaces outside quotes and angle brackets never are.
     */
    bool IsValid( bool allowUtf8 = false ) const;

    /** Whether the address (not the name) has bytes above 0x7F */
    bool IsInternational( void ) const;

    /** The same address: the local part exactly, the domain without regard for case; the display name is ignored */
    bool IsSameAddress( const CMailAddress& other ) const;

    /**
     *  The address as a header wants it: 'Name <local@domain>' or 'local@domain' without a name. A name with specials
     *  is quoted, a name with non ASCII text or control characters is encoded as RFC 2047 words.
     *  @return false (and 'text' empty) when IsValid() is false
     */
    bool Format( CORE::CString& text, bool allowUtf8 = false ) const;

    /** A single mailbox from header text; encoded words in the name are decoded, comments are dropped */
    static bool Parse( const CORE::CString& headerText, CMailAddress& address );

    /**
     *  The mailboxes of an address list header. Groups ('team: a@x, b@y;') are flattened to their members, an
     *  element that holds no usable address is skipped. At most 'maxEntries' are returned.
     *  @return the number of mailboxes added
     */
    static UInt32 ParseList( const CORE::CString& headerText, TMailAddressVector& addresses, UInt32 maxEntries = 500 );

    /**
     *  The list written as a header value, comma separated. Fails (false) when one of the addresses is not valid
     *  or the list is empty.
     */
    static bool FormatList( const TMailAddressVector& addresses, CORE::CString& text, bool allowUtf8 = false );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILADDRESS_H ? */
