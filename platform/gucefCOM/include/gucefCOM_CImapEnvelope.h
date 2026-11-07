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

#ifndef GUCEF_COM_CIMAPENVELOPE_H
#define GUCEF_COM_CIMAPENVELOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CIMAPVALUE_H
#include "gucefCOM_CImapValue.h"
#define GUCEF_COM_CIMAPVALUE_H
#endif /* GUCEF_COM_CIMAPVALUE_H ? */

#ifndef GUCEF_COM_CEMAILADDRESS_H
#include "gucefCOM_CEMailAddress.h"
#define GUCEF_COM_CEMAILADDRESS_H
#endif /* GUCEF_COM_CEMAILADDRESS_H ? */

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

/** The ENVELOPE of a message (RFC 3501 section 7.4.2) */
class GUCEF_COM_PUBLIC_CPP CImapEnvelope
{
    public:

    CORE::CString date;                         /**< as in the header */
    CORE::CString subject;                      /**< decoded */
    CEMailAddress::TEMailAddressVector from;
    CEMailAddress::TEMailAddressVector sender;
    CEMailAddress::TEMailAddressVector replyTo;
    CEMailAddress::TEMailAddressVector to;
    CEMailAddress::TEMailAddressVector cc;
    CEMailAddress::TEMailAddressVector bcc;
    CORE::CString inReplyTo;
    CORE::CString messageId;

    CImapEnvelope( void );

    static bool FromValue( const CImapValue& value, CImapEnvelope& envelope );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPENVELOPE_H ? */
