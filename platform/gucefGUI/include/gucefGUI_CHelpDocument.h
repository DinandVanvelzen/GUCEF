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

#ifndef GUCEF_GUI_CHELPDOCUMENT_H
#define GUCEF_GUI_CHELPDOCUMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

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

class CIHelpSource;

/**
 *  One help topic: its stable id, the title, the group the topic index shows it under, the brief (one to three sentences,
 *  what a tooltip shows), the version and the date it was last reviewed, the topics to see also, and the content as a list
 *  of simple blocks (headings, paragraphs, bullets, definitions, table rows and notes). The text of a block keeps the
 *  inline markup of CHelpMarkupParser (bold and links); ToPlainText() gives what a reader sees.
 */
class GUCEF_GUI_PUBLIC_CPP CHelpDocument
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    enum EBlockKind
    {
        BLOCK_HEADING = 0   ,
        BLOCK_PARAGRAPH     ,
        BLOCK_BULLET        ,
        BLOCK_DEFINITION    ,   /**< a term and its explanation */
        BLOCK_TABLE_ROW     ,   /**< the cells of one row of a small table */
        BLOCK_NOTE              /**< a remark set apart from the text */
    };

    /** One block of the content */
    struct GUCEF_GUI_PUBLIC_CPP SBlock
    {
        EBlockKind kind;
        UInt32 level;           /**< the level of a heading, 1 to 3; 0 for the other kinds */
        CString term;           /**< the term of a definition */
        CString text;           /**< the text with its inline markup; empty for a table row */
        TStringVector cells;    /**< the cells of a table row, with their inline markup */

        SBlock( void );
        SBlock( EBlockKind blockKind, const CString& blockText, UInt32 blockLevel = 0 );
    };
    typedef struct SBlock TBlock;
    typedef GUCEF::vector< TBlock > TBlockVector;

    CString id;
    CString title;
    CString group;              /**< the heading the topic index lists the topic under; empty: the topics without a group */
    CString brief;
    CString version;
    CString reviewed;           /**< when the content was last reviewed, as the author wrote it */
    TStringVector seeAlso;      /**< topic ids */
    TBlockVector blocks;

    CHelpDocument( void );

    void Clear( void );

    /** Whether the document has an id and a title, the least a source can serve */
    bool IsValid( void ) const;

    /** The topics the document links to: the see also list first, then the inline links of the blocks in their order, each once */
    void CollectLinks( TStringVector& topicIds ) const;

    /**
     *  The content as the text a reader sees: headings on a line of their own with a blank line before them, paragraphs and
     *  notes followed by a blank line, bullets indented with "- ", definitions as "term: text", the cells of a table row
     *  separated by " | ". Bold markup is removed; a link shows its label, else the title of its topic in 'source', else its id
     */
    CString ToPlainText( const CIHelpSource* source ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CHELPDOCUMENT_H ? */
