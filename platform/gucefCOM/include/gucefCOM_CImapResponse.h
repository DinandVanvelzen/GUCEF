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

#ifndef GUCEF_COM_CMAILADDRESS_H
#include "gucefCOM_CMailAddress.h"
#define GUCEF_COM_CMAILADDRESS_H
#endif /* GUCEF_COM_CMAILADDRESS_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

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

/** The ENVELOPE of a message (RFC 3501 section 7.4.2) */
class GUCEF_COM_PUBLIC_CPP CImapEnvelope
{
    public:

    CORE::CString date;                         /**< as in the header */
    CORE::CString subject;                      /**< decoded */
    CMailAddress::TMailAddressVector from;
    CMailAddress::TMailAddressVector sender;
    CMailAddress::TMailAddressVector replyTo;
    CMailAddress::TMailAddressVector to;
    CMailAddress::TMailAddressVector cc;
    CMailAddress::TMailAddressVector bcc;
    CORE::CString inReplyTo;
    CORE::CString messageId;

    CImapEnvelope( void );

    static bool FromValue( const CImapValue& value, CImapEnvelope& envelope );
};

/*-------------------------------------------------------------------------*/

/** The BODYSTRUCTURE of a message (RFC 3501 section 7.4.2): the parts a message has without having to fetch it */
class GUCEF_COM_PUBLIC_CPP CImapBodyStructure
{
    public:

    bool isMultipart;
    CORE::CString type;                         /**< lower case: text, image, application, multipart */
    CORE::CString subtype;                      /**< lower case */
    CMimeCodec::TParamVector params;
    CORE::CString contentId;
    CORE::CString description;
    CORE::CString encoding;                     /**< lower case */
    UInt32 size;                                /**< octets, not for multiparts */
    UInt32 lines;                               /**< text parts */
    CORE::CString disposition;                  /**< lower case: attachment or inline */
    CMimeCodec::TParamVector dispositionParams;
    GUCEF::vector< CImapBodyStructure* > children;  /**< owned: the parts of a multipart */

    CImapBodyStructure( void );
    CImapBodyStructure( const CImapBodyStructure& src );
    ~CImapBodyStructure();
    CImapBodyStructure& operator=( const CImapBodyStructure& src );

    /** type/subtype */
    CORE::CString GetContentType( void ) const;

    /** The file name of the part, from the disposition or the type parameters, empty when it has none */
    CORE::CString GetFileName( void ) const;

    void Clear( void );

    static bool FromValue( const CImapValue& value, CImapBodyStructure& structure );

    private:

    void CopyFrom( const CImapBodyStructure& src );
};

/*-------------------------------------------------------------------------*/

/** A part or section of a message that was fetched with BODY[...] */
class GUCEF_COM_PUBLIC_CPP CImapFetchSection
{
    public:

    CORE::CString name;                 /**< what was in the brackets: "", "HEADER", "TEXT", "1", "1.2", "HEADER.FIELDS (FROM TO)" */
    bool hasOrigin;                     /**< a partial fetch <origin> */
    UInt32 origin;
    bool isNil;                         /**< the server said NIL: there is nothing */
    CORE::CDynamicBuffer data;

    CImapFetchSection( void );
};

/*-------------------------------------------------------------------------*/

/** What a FETCH response said about one message */
class GUCEF_COM_PUBLIC_CPP CImapFetchedItem
{
    public:

    UInt32 sequence;
    bool hasUid;
    UInt32 uid;
    bool hasFlags;
    CORE::CString::StringVector flags;
    bool hasInternalDate;
    CORE::CString internalDate;         /**< as the server wrote it, 02-Nov-2020 10:11:12 +0000 */
    bool hasSize;
    UInt32 size;                        /**< RFC822.SIZE */
    bool hasEnvelope;
    CImapEnvelope envelope;
    bool hasBodyStructure;
    CImapBodyStructure bodyStructure;
    GUCEF::vector< CImapFetchSection > sections;

    CImapFetchedItem( void );

    /** The section of the name ("" for the whole message), GUCEF_NULL when it was not fetched */
    const CImapFetchSection* FindSection( const CORE::CString& name ) const;

    /** Whether the flag is set, \Seen for example, without regard for case */
    bool HasFlag( const CORE::CString& flag ) const;

    /** Reads the items of an untagged FETCH response. @return false when the response is not a valid FETCH */
    static bool FromResponse( const CImapResponse& response, CImapFetchedItem& item );
};

/*-------------------------------------------------------------------------*/

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

#endif /* GUCEF_COM_CIMAPRESPONSE_H ? */
