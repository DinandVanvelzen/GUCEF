/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CGRAMPSTEXT_H
#define ANCESTRYBACKUP_CGRAMPSTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <string>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Text handling for writing XML and for turning what the Ancestry export has in its text into what a person would read */
class CGrampsText
{
    public:

    /**
     *  Makes text safe for XML: & < > " ' are escaped, characters XML 1.0 does not allow are dropped and
     *  bytes which are not valid UTF-8 become U+FFFD. Suitable for both attribute values and element content.
     *  @param inAttribute  line breaks and tabs have to be escaped in an attribute value or they are read as spaces, in element content they are kept as they are
     */
    static std::string XmlEscape( const std::string& text, bool inAttribute = true );

    /**
     *  Ancestry puts HTML escapes in the text of its export (&amp; &lt; &quot; &#39; &nbsp;, also &#13;).
     *  Turns the ones it knows back into the character. Everything else, for example a lone & is left alone.
     */
    static std::string DecodeHtmlEntities( const std::string& text );

    /**
     *  Notes can hold the HTML of the site: <p> and <br> become line breaks, other tags are dropped, entities are decoded.
     *  Text without any tag is only decoded.
     */
    static std::string HtmlToPlainText( const std::string& text );

    static std::string Trim( const std::string& text );

    /** Lower case for the ASCII letters only */
    static std::string ToLowerAscii( const std::string& text );

    /**
     *  A stable 128 bit digest of the text as 32 hex digits. Not cryptographic, it exists to give the same object the
     *  same handle every time the tree is converted.
     */
    static std::string HashHex( const std::string& text );

    /** Characters which are fine in an id of Gramps: anything else becomes '_' */
    static std::string MakeId( const std::string& text );

    /** Days since 1970-01-01 of a date in the proleptic Gregorian calendar */
    static Int64 DaysSinceEpoch( Int32 year, UInt32 month, UInt32 day );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CGRAMPSTEXT_H ? */
