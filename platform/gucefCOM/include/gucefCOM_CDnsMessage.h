/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_CDNSMESSAGE_H
#define GUCEF_COM_CDNSMESSAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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
 *  A DNS message (RFC 1035) as mDNS and DNS-SD (RFC 6762, RFC 6763) use it: the header, the questions and the
 *  records, with name compression. The data of A, PTR, SRV and TXT records is decoded, other records keep their
 *  raw data.
 *
 *  Names are dotted strings without the final dot, for example "_gucef-game._udp.local". A label that contains a
 *  dot or a backslash, as DNS-SD instance names can, has them escaped as "\." and "\\".
 */
class GUCEF_COM_PUBLIC_CPP CDnsMessage
{
    public:

    enum ERecordType
    {
        RECORDTYPE_A    = 1   ,
        RECORDTYPE_PTR  = 12  ,
        RECORDTYPE_TXT  = 16  ,
        RECORDTYPE_AAAA = 28  ,
        RECORDTYPE_SRV  = 33  ,
        RECORDTYPE_NSEC = 47  ,
        RECORDTYPE_ANY  = 255
    };

    static const UInt16 ClassIn;                /**< 1 */
    static const UInt16 FlagResponse;           /**< QR */
    static const UInt16 FlagAuthoritative;      /**< AA */
    static const UInt16 FlagTruncated;          /**< TC */

    class GUCEF_COM_PUBLIC_CPP CQuestion
    {
        public:

        CORE::CString name;
        UInt16 type;
        UInt16 recordClass;
        bool unicastResponse;       /**< the top bit of the class in mDNS: the asker wants a unicast answer */

        CQuestion( void );
        CQuestion( const CORE::CString& questionName, UInt16 questionType );
    };
    typedef GUCEF::vector< CQuestion > TQuestionVector;

    class GUCEF_COM_PUBLIC_CPP CResourceRecord
    {
        public:

        CORE::CString name;
        UInt16 type;
        UInt16 recordClass;
        bool cacheFlush;            /**< the top bit of the class in mDNS: this record replaces what is cached for the name */
        UInt32 ttlInSecs;

        CORE::CString target;                   /**< PTR: the name pointed to, SRV: the host */
        UInt16 priority;                        /**< SRV */
        UInt16 weight;                          /**< SRV */
        UInt16 port;                            /**< SRV, host byte order */
        UInt32 ipv4AddressInNetworkByteOrder;   /**< A */
        CORE::CString::StringVector texts;      /**< TXT: the character strings */
        CORE::CDynamicBuffer rawData;           /**< the data of record types that are not decoded */

        CResourceRecord( void );

        static CResourceRecord MakePtr( const CORE::CString& name, const CORE::CString& target, UInt32 ttlInSecs );
        static CResourceRecord MakeSrv( const CORE::CString& name, const CORE::CString& host, UInt16 port, UInt32 ttlInSecs );
        static CResourceRecord MakeTxt( const CORE::CString& name, const CORE::CString::StringVector& texts, UInt32 ttlInSecs );
        static CResourceRecord MakeA( const CORE::CString& name, UInt32 ipv4AddressInNetworkByteOrder, UInt32 ttlInSecs );
    };
    typedef GUCEF::vector< CResourceRecord > TResourceRecordVector;

    UInt16 id;
    UInt16 flags;
    TQuestionVector questions;
    TResourceRecordVector answers;
    TResourceRecordVector authorities;
    TResourceRecordVector additionals;

    CDnsMessage( void );

    bool IsResponse( void ) const;

    void Clear( void );

    /**
     *  Writes the message in wire format, replacing the content of the buffer
     */
    bool Serialize( CORE::CDynamicBuffer& buffer ) const;

    /**
     *  Reads a message in wire format
     *
     *  @return false if the data is not a valid DNS message
     */
    bool Deserialize( const void* data, UInt32 dataSize );

    /** Compares names the DNS way: without regard to the case of ASCII letters */
    static bool NamesAreEqual( const CORE::CString& a, const CORE::CString& b );

    /** Makes a name of a single label that may contain dots, and a domain: "My Game.1" + "_x._udp.local" */
    static CORE::CString MakeName( const CORE::CString& label, const CORE::CString& domain );

    /** Splits a name in its first label, unescaped, and the rest */
    static bool SplitFirstLabel( const CORE::CString& name, CORE::CString& label, CORE::CString& rest );

    /** Turns key/value pairs into TXT strings "key=value" */
    static void AttributesToTexts( const CORE::CValueList& attributes, CORE::CString::StringVector& texts );

    /** Turns TXT strings "key=value" into key/value pairs, a lone "key" gets an empty value */
    static void TextsToAttributes( const CORE::CString::StringVector& texts, CORE::CValueList& attributes );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CDNSMESSAGE_H ? */
