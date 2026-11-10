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

#ifndef GUCEF_COM_CIMAPRESPONSE_H
#define GUCEF_COM_CIMAPRESPONSE_H

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

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#ifndef GUCEF_COM_CIMAPENVELOPE_H
#include "gucefCOM_CImapEnvelope.h"
#define GUCEF_COM_CIMAPENVELOPE_H
#endif /* GUCEF_COM_CIMAPENVELOPE_H ? */

#ifndef GUCEF_COM_CIMAPBODYSTRUCTURE_H
#include "gucefCOM_CImapBodyStructure.h"
#define GUCEF_COM_CIMAPBODYSTRUCTURE_H
#endif /* GUCEF_COM_CIMAPBODYSTRUCTURE_H ? */

#ifndef GUCEF_COM_CIMAPFETCHSECTION_H
#include "gucefCOM_CImapFetchSection.h"
#define GUCEF_COM_CIMAPFETCHSECTION_H
#endif /* GUCEF_COM_CIMAPFETCHSECTION_H ? */

#ifndef GUCEF_COM_CIMAPMAILBOXNAME_H
#include "gucefCOM_CImapMailboxName.h"
#define GUCEF_COM_CIMAPMAILBOXNAME_H
#endif /* GUCEF_COM_CIMAPMAILBOXNAME_H ? */

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
 *  One response of an IMAP server, parsed: a tagged completion ("A0001 OK done"), untagged data or status
 *  ("* 3 EXISTS", "* OK [UIDVALIDITY 7] ok") or a continuation request ("+ idling").
 *
 *  The bytes that are parsed hold the first line of the response and, for a response with literals, the
 *  literals and the lines after them as the server sent them: the text {n}, a CR LF, n bytes, and on.
 */
class GUCEF_COM_PUBLIC_CPP CImapResponse
{
    public:

    enum EKind
    {
        KIND_NONE = 0,
        KIND_TAGGED,
        KIND_UNTAGGED,
        KIND_CONTINUATION
    };

    EKind kind;
    CORE::CString tag;                  /**< tagged responses */
    bool isStatus;                      /**< OK, NO, BAD, PREAUTH or BYE (tagged: OK, NO or BAD) */
    CORE::CString status;               /**< upper case */
    CORE::CString code;                 /**< the response code in brackets, upper case: UIDVALIDITY, CAPABILITY, READ-ONLY, ... */
    CORE::CString codeArguments;        /**< what follows the name in the brackets */
    CORE::CString text;                 /**< the human readable text of a status response, of a continuation the text after the plus */
    bool hasNumber;                     /**< untagged data of the form "* 3 EXISTS" */
    UInt32 number;
    CORE::CString keyword;              /**< upper case: EXISTS, FETCH, LIST, SEARCH, CAPABILITY ... */
    CImapValue data;                    /**< the values after the keyword, as a list. Not for status responses */

    CImapResponse( void );

    /**
     *  Reads a response. @return false (with 'error', a short phrase) when the bytes are not a response of IMAP or a
     *  limit is exceeded: the nesting of lists (32), the number of values ('maxValues')
     */
    static bool Parse( const void* bytes, UInt32 size, UInt32 maxValues, CImapResponse& response, CORE::CString& error );

    /** Whether a line ends with {n} or {n+}: a literal of n bytes follows it. 'nonSynchronizing' for the plus form */
    static bool EndsWithLiteral( const CORE::CString& line, UInt32& literalSize, bool& nonSynchronizing );
};

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPRESPONSE_H ? */
