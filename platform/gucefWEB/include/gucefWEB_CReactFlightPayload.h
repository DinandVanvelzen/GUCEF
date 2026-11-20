/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CREACTFLIGHTPAYLOAD_H
#define GUCEF_WEB_CREACTFLIGHTPAYLOAD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CREACTFLIGHTROW_H
#include "gucefWEB_CReactFlightRow.h"
#define GUCEF_WEB_CREACTFLIGHTROW_H
#endif /* GUCEF_WEB_CREACTFLIGHTROW_H ? */

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
 *  Reads the data of a React Server Components payload ("flight" data), the form in which a Next.js site (app router)
 *  sends its pages to the browser:
 *
 *  - a page as HTML carries the payload in script elements, as JavaScript string literals in calls
 *    self.__next_f.push([1,"..."]); the strings in order, joined, are the payload (ExtractFromHtml()). Chunks of other
 *    types (0 the start, 2 form state, 3 binary as base64) are not part of the text and are counted as skipped;
 *  - a request of the page with the header "RSC: 1", and the answer of a server action (content type
 *    text/x-component), is the payload itself.
 *
 *  The payload is a list of rows "<hex id>:<tag><payload>": a row without a tag is a model (JSON: an element tree
 *  ["$", type, key, props], objects, lists); tag "T" is a text whose length in bytes is given in hexadecimal before a
 *  comma ("T1a,<26 bytes>") and that has no line end of its own; the binary types of newer React are also length
 *  prefixed; every other row ends at a line end. Strings in the models refer to other rows: "$<id>", "$L<id>" (lazy),
 *  "$@<id>" (promise), "$F<id>" and "$h<id>" (server references), "$Q<id>" and "$W<id>" (map and set), optionally with a
 *  path "$<id>:props:children"; "$$..." is a string that starts with "$"; other "$..." strings are special values
 *  ("$undefined", "$Sreact.suspense", "$D<date>", ...).
 *
 *  The JSON of the rows is parsed with the "json" DStore codec (dstorepluginJSONPARSER); without it the rows keep their
 *  text only. The class only reads: it never runs script and never asks anything over the network.
 */
class GUCEF_WEB_PUBLIC_CPP CReactFlightPayload
{
    public:

    typedef GUCEF::vector< const CORE::CDataNode* > TConstNodeVector;
    typedef std::pair< CORE::CString, CORE::CString > TStringPair;
    typedef GUCEF::vector< TStringPair > TStringPairVector;

    CReactFlightPayload( void );
    ~CReactFlightPayload();

    void Clear( void );

    /**
     *  The payload of a Next.js page: the strings of self.__next_f.push([1,"..."]) in order, joined. False when the page
     *  has none. 'chunkCount' gets the number of type 1 strings taken, 'skippedCount' the number of other chunks.
     */
    static bool ExtractFromHtml( const CORE::CString& html, CORE::CString& flightText, UInt32* chunkCount = GUCEF_NULL, UInt32* skippedCount = GUCEF_NULL );

    /**
     *  Decodes the JavaScript (or JSON) string literal that starts with its quote (" or ') at 'start' into UTF-8;
     *  'end' gets the index just after the closing quote. False when there is no quote at 'start' or no closing quote.
     */
    static bool DecodeStringLiteral( const CORE::CString& text, UInt32 start, CORE::CString& value, UInt32& end );

    /** Whether a text starts as a payload does: hexadecimal digits and a colon */
    static bool LooksLikePayload( const CORE::CString& text );

    /** Reads the rows of a payload; false when there is none */
    bool Parse( const CORE::CString& payloadText );

    /** ExtractFromHtml() and Parse() */
    bool ParseHtml( const CORE::CString& html );

    /** A page or a payload, whichever the text is: HTML when it starts with '<' after blanks, else a payload */
    bool ParseAny( const CORE::CString& text );

    const TReactFlightRowVector& GetRows( void ) const;

    /** The row with the id, GUCEF_NULL when there is none */
    const CReactFlightRow* FindRow( const CORE::CString& id ) const;

    /** The number of rows whose payload was parsed as JSON */
    UInt32 GetJsonRowCount( void ) const;

    /** The number of chunks of the page that were not text (binary, form state) and lines that were not rows */
    UInt32 GetSkippedCount( void ) const;

    /** Whether a string of the payload refers to a row ("$1", "$L1a", "$@2", "$1:props:children"); the id in rowId */
    static bool IsRowReference( const CORE::CString& value, CORE::CString& rowId );

    /** The row a string of the payload refers to, GUCEF_NULL when it is no reference or the row is not there */
    const CReactFlightRow* ResolveReference( const CORE::CString& value ) const;

    /** A string of the payload as text: "$$x" is "$x", a reference to a text row is its text, other "$..." strings are empty */
    CORE::CString ValueAsText( const CORE::CString& value ) const;

    /** Every object node of every row that was parsed, depth first, the rows in order */
    void CollectObjects( TConstNodeVector& objects ) const;

    /** Every object node of a tree, depth first, the node itself first when it is one */
    static void CollectObjects( const CORE::CDataNode& node, TConstNodeVector& objects );

    /**
     *  The links of the element trees: for every object with a string member "href" (the props of an anchor or of a
     *  link component), the href and the text of its children (GetElementText()), in the order they are found
     */
    void CollectLinks( TStringPairVector& links ) const;

    /** The visible text of the children of a props object (its member "children"), strings joined with single blanks */
    CORE::CString GetElementText( const CORE::CDataNode& props ) const;

    private:

    void AppendText( const CORE::CDataNode& node, CORE::CString& text, UInt32 depth ) const;
    void AppendValueText( const CORE::CString& value, CORE::CString& text, UInt32 depth ) const;

    TReactFlightRowVector m_rows;
    UInt32 m_skipped;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CREACTFLIGHTPAYLOAD_H ? */
