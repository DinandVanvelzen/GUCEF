/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_WEB_CHTMLTEXTEXTRACTOR_H
#include "gucefWEB_CHtmlTextExtractor.h"
#define GUCEF_WEB_CHTMLTEXTEXTRACTOR_H
#endif /* GUCEF_WEB_CHTMLTEXTEXTRACTOR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestHtmlTextExtractor.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef WEB::CHtmlTextExtractor TExtractor;

/** The plain text, logged when it is not what is expected so that a failure shows what came out */
bool
Gives( const CORE::CString& html, const CORE::CString& expected, bool inlineAddresses = true )
{
    const CORE::CString text = TExtractor::ToPlainText( html, GUCEF_NULL, inlineAddresses );
    if ( text == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TestHtmlTextExtractor: for \"" << html << "\" expected \"" << expected << "\" got \"" << text << "\"" );
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestBlocksAndText( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "HtmlText: tags are dropped, paragraphs and headings are separated by an empty line, line breaks start a line, white space collapses, pre keeps it" )
    try
    {
        ASSERT_TRUE( Gives( "plain text", "plain text" ) );
        ASSERT_TRUE( Gives( "<span>Hello</span> <b>bold</b> <i><u>world</u></i>", "Hello bold world" ) );
        ASSERT_TRUE( Gives( "<p>One</p><p>Two</p>", "One\n\nTwo" ) );
        ASSERT_TRUE( Gives( "<h2>Title</h2>Text after", "Title\n\nText after" ) );
        ASSERT_TRUE( Gives( "line one<br>line two<br/>line three<BR />", "line one\nline two\nline three" ) );
        ASSERT_TRUE( Gives( "a<br><br>b", "a\n\nb" ) );
        ASSERT_TRUE( Gives( "  lots   of\n\n spaces\t here  ", "lots of spaces here" ) );
        ASSERT_TRUE( Gives( "<p>  padded  </p>", "padded" ) );
        ASSERT_TRUE( Gives( "<p></p><p></p><div><p>deep <span>in <b>nested</b></span> tags</p></div><p></p>", "deep in nested tags" ) );
        ASSERT_TRUE( Gives( "<pre>keep   this\n  as is</pre>after", "keep   this\n  as is\n\nafter" ) );
        ASSERT_TRUE( Gives( "", "" ) );
        ASSERT_TRUE( Gives( "<p></p>", "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestListsAndTables( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "HtmlText: the items of a list are lines with a dash, nested lists are indented, the cells of a table row are separated" )
    try
    {
        ASSERT_TRUE( Gives( "Includes:<ul><li>Trees</li><li>Rocks</li></ul>Done", "Includes:\n\n- Trees\n- Rocks\n\nDone" ) );
        ASSERT_TRUE( Gives( "<ol><li>First<ul><li>Inner</li></ul></li><li>Second</li></ol>", "- First\n  - Inner\n- Second" ) );
        ASSERT_TRUE( Gives( "<table><tr><th>Name</th><th>Size</th></tr><tr><td>Tree</td><td>51M</td></tr></table>", "Name | Size\nTree | 51M" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestCharacterReferences( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "HtmlText: named and numeric character references are decoded, unknown ones stay as written, numbers that are no character become U+FFFD" )
    try
    {
        ASSERT_TRUE( Gives( "Tom &amp; Jerry &lt;3 &quot;quoted&quot; &apos;single&apos;", "Tom & Jerry <3 \"quoted\" 'single'" ) );
        ASSERT_TRUE( Gives( "a&nbsp;b", "a b" ) );
        ASSERT_TRUE( Gives( "&copy; 2020 &#169; &#xA9; &#XA9;", "\xC2\xA9 2020 \xC2\xA9 \xC2\xA9 \xC2\xA9" ) );
        ASSERT_TRUE( Gives( "wait&hellip; &mdash; done", "wait\xE2\x80\xA6 \xE2\x80\x94 done" ) );
        ASSERT_TRUE( Gives( "&unknown; &amp &", "&unknown; &amp &" ) );
        ASSERT_TRUE( Gives( "&#0;&#xD800;&#x110000;", "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD" ) );
        ASSERT_TRUE( Gives( "soft&shy;hyphen", "softhyphen" ) );
        ASSERT_TRUE( Gives( "caf\xC3\xA9 <b>na\xC3\xAFve</b>", "caf\xC3\xA9 na\xC3\xAFve" ) );
        ASSERT_TRUE( "a < b & c" == TExtractor::DecodeCharacterReferences( "a &lt; b &amp; c" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestLinks( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "HtmlText: a link keeps its text with its address after it, the links are listed, any valid URI is kept, relative ones are resolved against the base" )
    try
    {
        // the shape of the description of an item of a store
        const CORE::CString description =
            "<span>This bundle includes the following packs:</span><p></p><p><a target=\"_blank\" href=\"https://www.example.com/listings/1234\" "
            "rel=\"noopener noreferrer nofollow\"><span><u>Realistic Low Poly Tree Pack 51M</u></span></a></p><p><a href='https://www.example.com/listings/5678'>"
            "Rock &amp; Cliff Pack</a></p>";
        TExtractor::TLinkVector links;
        const CORE::CString text = TExtractor::ToPlainText( description, &links );
        ASSERT_TRUE( "This bundle includes the following packs:\n\nRealistic Low Poly Tree Pack 51M (https://www.example.com/listings/1234)\n\n"
                     "Rock & Cliff Pack (https://www.example.com/listings/5678)" == text );
        ASSERT_TRUE( 2 == links.size() );
        if ( 2 == links.size() )
        {
            ASSERT_TRUE( "Realistic Low Poly Tree Pack 51M" == links[ 0 ].text && "https://www.example.com/listings/1234" == links[ 0 ].address );
            ASSERT_TRUE( "Rock & Cliff Pack" == links[ 1 ].text && "https://www.example.com/listings/5678" == links[ 1 ].address );
        }

        // without the addresses in the text, they are in the list only
        const CORE::CString bare = TExtractor::ToPlainText( description, &links, false );
        ASSERT_TRUE( "This bundle includes the following packs:\n\nRealistic Low Poly Tree Pack 51M\n\nRock & Cliff Pack" == bare && 2 == links.size() );

        // a link whose text is its address, a mail address, an address with a reference in it
        ASSERT_TRUE( Gives( "<a href=\"https://example.com/a\">https://example.com/a</a>", "https://example.com/a" ) );
        ASSERT_TRUE( Gives( "<a href=\"mailto:help@example.com\">Write us</a>", "Write us (mailto:help@example.com)" ) );
        ASSERT_TRUE( Gives( "<a href=\"HTTPS://Example.com\">Site</a>", "Site (https://Example.com/)" ) );
        ASSERT_TRUE( Gives( "<a href=\"https://example.com/?a=1&amp;b=2\">Query</a>", "Query (https://example.com/?a=1&b=2)" ) );
        // the scheme is not judged: any valid URI is text with its address, nothing is opened or run
        ASSERT_TRUE( Gives( "<a href=\"javascript:alert(1)\">Click</a>", "Click (javascript:alert(1))" ) );
        ASSERT_TRUE( Gives( "<a href=\"tel:+15551234\">Call</a>", "Call (tel:+15551234)" ) );
        // what is no valid URI leaves the text of the link: a relative reference without a base, no address at all
        ASSERT_TRUE( Gives( "<a href=\"/relative/path\">Relative</a>", "Relative" ) );
        ASSERT_TRUE( Gives( "<a href=\"page.html\">Page</a>", "Page" ) );
        ASSERT_TRUE( Gives( "<a>No address</a>", "No address" ) );
        ASSERT_TRUE( Gives( "<a href=\"https://example.com/x\"></a>", "https://example.com/x" ) );
        TExtractor::ToPlainText( "<a href=\"page.html\">x</a>", &links );
        ASSERT_TRUE( links.empty() );

        // relative links with the address of the page as the base: resolved as CUri resolves references
        const CORE::CUri page( "https://www.example.com/listings/1234" );
        TExtractor::ToPlainText( "<a href=\"/listings/5678\">Other</a> <a href=\"../about\">About</a> <a href=\"#top\">Top</a>", &links, true, &page );
        ASSERT_TRUE( 3 == links.size() );
        if ( 3 == links.size() )
        {
            ASSERT_TRUE( "https://www.example.com/listings/5678" == links[ 0 ].address );
            ASSERT_TRUE( "https://www.example.com/about" == links[ 1 ].address );
            ASSERT_TRUE( "https://www.example.com/listings/1234#top" == links[ 2 ].address );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestLeftOutAndMalformed( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "HtmlText: scripts, styles and comments are left out with their content, malformed markup gives text and never stops the conversion" )
    try
    {
        ASSERT_TRUE( Gives( "before<script type=\"text/javascript\">alert('<p>x</p>');</script>after", "beforeafter" ) );
        ASSERT_TRUE( Gives( "a <SCRIPT>document.write(1)</SCRIPT > b", "a b" ) );
        ASSERT_TRUE( Gives( "<style>p { color: red; }</style><p>styled</p>", "styled" ) );
        ASSERT_TRUE( Gives( "a<!-- a comment <p>with tags</p> -->b", "ab" ) );
        ASSERT_TRUE( Gives( "<!DOCTYPE html><html><head><title>T</title></head><body><p>Body</p></body></html>", "Body" ) );
        ASSERT_TRUE( Gives( "x<![CDATA[raw <text>]]>y", "xraw <text>y" ) );
        // a "<" that starts no tag is text
        ASSERT_TRUE( Gives( "1 < 2 and 3 <4", "1 < 2 and 3 <4" ) );
        // closing tags without an opening one, attributes with odd quoting, a tag that is not closed
        ASSERT_TRUE( Gives( "</p></div>text</span>", "text" ) );
        ASSERT_TRUE( Gives( "<p class=x title='a > b'>quoted</p>", "quoted" ) );
        ASSERT_TRUE( Gives( "kept <p class=\"never closed", "kept" ) );
        ASSERT_TRUE( Gives( "<script>never closed", "" ) );
        ASSERT_TRUE( Gives( "<!-- never closed", "" ) );
        ASSERT_TRUE( Gives( "<a href=\"https://example.com/x\">open link", "open link (https://example.com/x)" ) );
        ASSERT_TRUE( Gives( "<averyveryveryverylongtagname>text</averyveryveryverylongtagname>", "text" ) );
        // control characters are no text
        ASSERT_TRUE( Gives( "a\x01" "b&#7;c", "abc" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRecognition( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "HtmlText: text with tags or character references is recognised as HTML, plain text (with < and & in it) is not and is given back unchanged" )
    try
    {
        ASSERT_TRUE( TExtractor::LooksLikeHtml( "<span>x</span>" ) );
        ASSERT_TRUE( TExtractor::LooksLikeHtml( "line<br/>line" ) );
        ASSERT_TRUE( TExtractor::LooksLikeHtml( "a <p class=\"x\">b" ) );
        ASSERT_TRUE( TExtractor::LooksLikeHtml( "Tom &amp; Jerry" ) );
        ASSERT_TRUE( TExtractor::LooksLikeHtml( "It&#39;s" ) );
        ASSERT_TRUE( TExtractor::LooksLikeHtml( "x <!-- note --> y" ) );
        ASSERT_FALSE( TExtractor::LooksLikeHtml( "A plain description." ) );
        ASSERT_FALSE( TExtractor::LooksLikeHtml( "1 < 2 > 0 & more" ) );
        ASSERT_FALSE( TExtractor::LooksLikeHtml( "Fish & Chips; tasty" ) );
        ASSERT_FALSE( TExtractor::LooksLikeHtml( "" ) );
        const CORE::CString plain = "Keep  these   spaces\nand lines & signs < >";
        ASSERT_TRUE( plain == TExtractor::ToPlainTextIfHtml( plain ) );
        ASSERT_TRUE( "a & b" == TExtractor::ToPlainTextIfHtml( "<b>a</b> &amp; b" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformHtmlTextExtractorTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefWEB.HtmlText" );

    TestBlocksAndText();
    TestListsAndTables();
    TestCharacterReferences();
    TestLinks();
    TestLeftOutAndMalformed();
    TestRecognition();
}

/*-------------------------------------------------------------------------*/
