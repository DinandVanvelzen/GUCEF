/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CHELPMARKUPPARSER_H
#define GUCEF_GUI_CHELPMARKUPPARSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CHELPDOCUMENT_H
#include "gucefGUI_CHelpDocument.h"
#define GUCEF_GUI_CHELPDOCUMENT_H
#endif /* GUCEF_GUI_CHELPDOCUMENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The markup-light format of help documents (UTF-8 text):
 *
 *      @id net.yield                       header fields first: id and title are required; group, brief, seealso (ids
 *      @title Net yield                    separated by commas), version and reviewed are optional
 *      @seealso net.income, tax.niit
 *
 *      # Heading                           '#', '##', '###': a heading of level 1 to 3
 *      A paragraph: lines that follow each other are joined with a space; a blank line ends it.
 *      - A bullet ('- ' or '* '); a following line indented by a blank continues it
 *      Term :: what the term means         a definition
 *      | cell | cell |                     a row of a small table
 *      > A note                            a remark set apart
 *
 *  Inline in any text: **bold**, [[topic.id]] and [[topic.id|label]] links. A marker that is not closed is plain text.
 *
 *  Brief tables (ParseBriefTable()) hold short texts by id, one per line: "id | topic | text"; the topic (the document a
 *  reader can open from the brief) may be empty; lines starting with '#' and blank lines are skipped.
 *
 *  The parser works on the UTF-8 bytes: every marker is ASCII, so it never splits a code point.
 */
class GUCEF_GUI_PUBLIC_CPP CHelpMarkupParser
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    enum ESpanKind
    {
        SPAN_TEXT = 0   ,
        SPAN_BOLD       ,
        SPAN_LINK
    };

    /** A run of inline text */
    struct GUCEF_GUI_PUBLIC_CPP SSpan
    {
        ESpanKind kind;
        CString text;       /**< the text; the label of a link (empty when the link has none) */
        CString target;     /**< the topic id of a link */

        SSpan( void );
        SSpan( ESpanKind spanKind, const CString& spanText, const CString& spanTarget = CString::Empty );
    };
    typedef struct SSpan TSpan;
    typedef GUCEF::vector< TSpan > TSpanVector;

    /** One entry of a brief table */
    struct GUCEF_GUI_PUBLIC_CPP SBriefEntry
    {
        CString id;
        CString topic;
        CString text;
    };
    typedef struct SBriefEntry TBriefEntry;
    typedef GUCEF::vector< TBriefEntry > TBriefEntryVector;

    /** Ids of topics: 1 to MaxTopicIdLength characters of a-z, 0-9, '.', '_' and '-', the first a letter or a digit */
    enum ELimits
    {
        MaxTopicIdLength = 96
    };

    static bool IsValidTopicId( const CString& id );

    /**
     *  Reads a whole document: the header fields, then the blocks. False with the reason when the id or the title is missing
     *  or the id is not valid; the document is cleared first
     */
    static bool ParseDocument( const CString& text, CHelpDocument& document, CString& error );

    /** The blocks of a body without header fields, appended to 'blocks' */
    static void ParseBlocks( const CString& body, CHelpDocument::TBlockVector& blocks );

    /** The inline runs of a text, appended to 'spans'; neighbouring plain runs are one span */
    static void ParseInline( const CString& text, TSpanVector& spans );

    /** The text without inline markup: bold markers removed, a link as its label, else the title of its topic in 'source', else its id */
    static CString ToPlainText( const CString& text, const CIHelpSource* source );

    /** The link targets of a text in their order, appended to 'targets' (repeats included) */
    static void CollectLinks( const CString& text, TStringVector& targets );

    /**
     *  Reads a brief table; the entries are appended. Lines that are not of the form "id | topic | text" with a valid id, a
     *  topic that is empty or valid, and a text, are skipped and each described in 'problems' with its line number. The
     *  number of entries read
     */
    static UInt32 ParseBriefTable( const CString& text, TBriefEntryVector& entries, TStringVector& problems );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CHELPMARKUPPARSER_H ? */
