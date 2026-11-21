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

#ifndef GUCEF_WEB_CHTMLTEXTEXTRACTOR_H
#define GUCEF_WEB_CHTMLTEXTEXTRACTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

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
 *  Makes readable plain text of a fragment of HTML, such as the description of an item that a web site gives: for showing it
 *  and searching it where no HTML is rendered.
 *
 *  - The tags are dropped. The block structure stays: paragraphs, headings, list blocks, tables and the like are separated by
 *    an empty line, a line break by a new line, an item of a list starts a line with "- " (indented two spaces per level of
 *    nesting), the cells of a table row are separated by " | ". Runs of white space outside "pre" collapse to one space.
 *  - Character references are decoded: the named ones of common text (&amp; &lt; &gt; &quot; &apos; &nbsp; dashes, quotes,
 *    the ellipsis, the bullet, the common symbols) and the numeric ones (&#169; &#xA9;). A reference that is not known stays
 *    as it is written; a numeric one that is no character becomes U+FFFD.
 *  - What is no text is left out with its content: script, style, template, noscript, iframe, object, svg, head and
 *    comments. Nothing is executed or fetched: this is a text transformation only.
 *  - A link ("a" with "href") keeps its text; its address is written after it as "text (address)" (unless the text is the
 *    address) and is given in the list of links when asked. The address is a CORE::CUri: parsed, resolved against the base
 *    address when one is given, and rendered again; any valid URI is kept, whatever its scheme. A reference that is no valid
 *    URI (a relative one without a base) leaves the text of the link only. The extractor never opens or fetches a link:
 *    whether one may be opened is for the code that acts on it to decide.
 *  - Malformed markup does not stop it: a "<" that starts no tag is text, a tag that is not closed ends the input, a closing
 *    tag without an opening tag is ignored.
 *
 *  The input is UTF-8; the output is UTF-8 with LF line ends and no white space at its ends.
 */
class GUCEF_WEB_PUBLIC_CPP CHtmlTextExtractor
{
    public:

    /** A link of the text: the text that was shown for it and its address */
    struct GUCEF_WEB_PUBLIC_CPP SLink
    {
        CORE::CString text;
        CORE::CString address;

        SLink( void );
    };
    typedef struct SLink TLink;
    typedef GUCEF::vector< TLink > TLinkVector;

    /**
     *  The plain text of the HTML. 'links' (may be NULL) gets the links in the order of the text; with 'inlineAddresses' the
     *  address of a link is written after its text in parentheses as well. 'baseAddress' (may be NULL) is the address of the
     *  page the HTML is from, against which relative links are resolved
     */
    static CORE::CString ToPlainText( const CORE::CString& html                     ,
                                      TLinkVector* links = GUCEF_NULL               ,
                                      bool inlineAddresses = true                   ,
                                      const CORE::CUri* baseAddress = GUCEF_NULL    );

    /** As ToPlainText() for text that may or may not be HTML: text that does not look like HTML (LooksLikeHtml()) is given back as it is */
    static CORE::CString ToPlainTextIfHtml( const CORE::CString& text                ,
                                            TLinkVector* links = GUCEF_NULL          ,
                                            bool inlineAddresses = true              ,
                                            const CORE::CUri* baseAddress = GUCEF_NULL );

    /** Whether the text has markup: a tag ("<p>", "</span>", "<br/>", "<!--") or a character reference ("&amp;", "&#39;") */
    static bool LooksLikeHtml( const CORE::CString& text );

    /** Decodes the character references of a text (not the tags) */
    static CORE::CString DecodeCharacterReferences( const CORE::CString& text );

    private:

    CHtmlTextExtractor( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTMLTEXTEXTRACTOR_H ? */
