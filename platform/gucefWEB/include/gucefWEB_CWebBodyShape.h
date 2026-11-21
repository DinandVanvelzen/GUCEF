/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CWEBBODYSHAPE_H
#define GUCEF_WEB_CWEBBODYSHAPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The structure of a request or response body without its data: which fields it has and of what type, one line per
 *  field ("items[].price: number"), which fields tell how an endpoint pages and which hold URLs. That is what is needed
 *  to learn how to use an endpoint, while the values are the data of the user and do not belong in a report.
 *
 *  Values are left out unless sample values are asked for (private use only). Even then the value of a field whose
 *  name is secret, as the given name redactor says (CORE::CRedactor::IsSensitiveName()), is never shown. Without sample
 *  values a URL field is described by its template (scheme://host/path-template), never by its value.
 */
class GUCEF_WEB_PUBLIC_CPP CWebBodyShape
{
    public:

    CORE::CString::StringVector lines;              /**< one line per field: path: type [= sample] */
    CORE::CString::StringVector paginationHints;    /**< fields that look like they are about paging: path [= sample] */
    CORE::CString::StringVector urlFields;          /**< fields holding a URL or an absolute path: path = sample, or path -> template */
    bool truncated;                                 /**< the body had more fields than the line limit allowed */
    UInt32 mergedCount;                             /**< the number of response shapes merged into this one, see Merge(); 0 for the shape of one body */

    CWebBodyShape( void );

    void Clear( void );

    bool IsEmpty( void ) const;

    /**
     *  Describes a JSON document (as parsed into a data node)
     *  @param withSampleValues     show the values of paging and URL fields, private use only
     *  @param secretNames          says which field names are secret; GUCEF_NULL: only the built-in word rule of CORE::CRedactor
     */
    static void Describe( const CORE::CDataNode& root                     ,
                          CWebBodyShape& shape                            ,
                          UInt32 maxDepth = 5                             ,
                          UInt32 maxLines = 150                           ,
                          bool withSampleValues = false                   ,
                          const CORE::CRedactor* secretNames = GUCEF_NULL );

    /** Describes an application/x-www-form-urlencoded body: one line per field name with the type its value looks like */
    static void DescribeFormUrlEncoded( const CORE::CString& body                       ,
                                        CWebBodyShape& shape                            ,
                                        UInt32 maxLines = 150                           ,
                                        bool withSampleValues = false                   ,
                                        const CORE::CRedactor* secretNames = GUCEF_NULL );

    /** Describes a multipart/form-data body: one line per part with its field name and kind (text, file or csrf-like) and its size in bytes, never a value */
    static void DescribeMultipart( const CORE::CString& body                        ,
                                   CWebBodyShape& shape                             ,
                                   UInt32 maxLines = 150                            ,
                                   const CORE::CRedactor* secretNames = GUCEF_NULL  );

    /** Is this field name one of an anti forgery token: csrf, xsrf, authenticity token, nonce, request verification token */
    static bool IsCsrfLikeName( const CORE::CString& name );

    /**
     *  Merges the shape of one more response of the same endpoint into this one, which then describes the union of
     *  what was seen: every path with the types it had, in the order of first sight. mergedCount counts the responses.
     *  Afterwards lines holds the union, as RenderMergedLines() makes it: array sizes are dropped, a field that was
     *  not in every response that could have had it is marked "(optional)", a field that had more than one type shows
     *  them ("null | string"), an array that was never anything but empty is marked "(always empty so far)".
     *  @param maxMergedLines   the union keeps at most this many fields; truncated is set when it would have had more
     */
    void Merge( const CWebBodyShape& single, UInt32 maxMergedLines = 600 );

    /**
     *  Would merging this shape add something: a path or a type of a path that the merged shape does not have yet, or
     *  an array that was empty before and has elements now. A merged shape with no responses merged takes everything.
     */
    bool AddsToMerged( const CWebBodyShape& merged, UInt32 maxMergedLines = 600 ) const;

    /** The lines of the union, see Merge() */
    void RenderMergedLines( CORE::CString::StringVector& out ) const;

    /** The keys through which APIs tell how paging works: page, offset, limit, next, cursor, total... */
    static bool IsPaginationKey( const CORE::CString& key );

    /** object, array, string, integer, number, boolean, null, binary */
    static CORE::CString GetTypeName( const CORE::CVariant& value );

    /** integer, number, boolean or string: what a text value of a form or query looks like */
    static CORE::CString GuessTextType( const CORE::CString& text );

    private:

    struct SWalkContext;
    typedef GUCEF::map< CORE::CString, UInt32 >             TKeyCountMap;
    typedef GUCEF::map< CORE::CString, CORE::CString >      TKeyTextMap;

    /** The keys of a shape ("path: type", an array that has elements adds "path: array[+]") and the text of the line each key stands for */
    static void ToKeys( const CWebBodyShape& shape, CORE::CString::StringVector& keys, TKeyTextMap& texts );

    TKeyCountMap m_keyCounts;                       /**< Merge(): per key the number of merged responses that had it */
    CORE::CString::StringVector m_keyOrder;         /**< Merge(): the keys in the order they were first seen */
    TKeyTextMap m_keyTexts;                         /**< Merge(): the text of the first line of each key */

    static void Walk( const CORE::CDataNode& node, const CORE::CString& path, UInt32 depth, const SWalkContext& context, CWebBodyShape& shape );
    static void AddLine( const CORE::CString& line, UInt32 maxLines, CWebBodyShape& shape );
    static void AddField( const CORE::CString& fieldPath, const CORE::CString& name, const CORE::CString& typeName,
                          const CORE::CString& value, const SWalkContext& context, CWebBodyShape& shape,
                          const CORE::CString& suffix = CORE::CString::Empty );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBBODYSHAPE_H ? */
