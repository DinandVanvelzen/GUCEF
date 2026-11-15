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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CHELPMARKUPPARSER_H
#include "gucefGUI_CHelpMarkupParser.h"
#define GUCEF_GUI_CHELPMARKUPPARSER_H
#endif /* GUCEF_GUI_CHELPMARKUPPARSER_H ? */

#include "gucefGUI_CHelpDocument.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

void
AddOnce( CHelpDocument::TStringVector& list, const CString& item )
{
    for ( UInt32 i=0; i<(UInt32)list.size(); ++i )
        if ( list[ i ] == item )
            return;
    list.push_back( item );
}

bool
IsListBlock( CHelpDocument::EBlockKind kind )
{
    return CHelpDocument::BLOCK_BULLET == kind || CHelpDocument::BLOCK_DEFINITION == kind || CHelpDocument::BLOCK_TABLE_ROW == kind;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHelpDocument::SBlock::SBlock( void )
    : kind( BLOCK_PARAGRAPH )
    , level( 0 )
    , term()
    , text()
    , cells()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHelpDocument::SBlock::SBlock( EBlockKind blockKind, const CString& blockText, UInt32 blockLevel )
    : kind( blockKind )
    , level( blockLevel )
    , term()
    , text( blockText )
    , cells()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHelpDocument::CHelpDocument( void )
    : id()
    , title()
    , group()
    , brief()
    , version()
    , reviewed()
    , seeAlso()
    , blocks()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CHelpDocument::Clear( void )
{GUCEF_TRACE;

    id.Clear();
    title.Clear();
    group.Clear();
    brief.Clear();
    version.Clear();
    reviewed.Clear();
    seeAlso.clear();
    blocks.clear();
}

/*-------------------------------------------------------------------------*/

bool
CHelpDocument::IsValid( void ) const
{GUCEF_TRACE;

    return !id.IsNULLOrEmpty() && !title.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

void
CHelpDocument::CollectLinks( TStringVector& topicIds ) const
{GUCEF_TRACE;

    topicIds.clear();
    for ( UInt32 i=0; i<(UInt32)seeAlso.size(); ++i )
        AddOnce( topicIds, seeAlso[ i ] );
    TStringVector inline_;
    for ( UInt32 b=0; b<(UInt32)blocks.size(); ++b )
    {
        const TBlock& block = blocks[ b ];
        CHelpMarkupParser::CollectLinks( block.term, inline_ );
        CHelpMarkupParser::CollectLinks( block.text, inline_ );
        for ( UInt32 c=0; c<(UInt32)block.cells.size(); ++c )
            CHelpMarkupParser::CollectLinks( block.cells[ c ], inline_ );
    }
    for ( UInt32 i=0; i<(UInt32)inline_.size(); ++i )
        if ( inline_[ i ] != id )
            AddOnce( topicIds, inline_[ i ] );
}

/*-------------------------------------------------------------------------*/

CString
CHelpDocument::ToPlainText( const CIHelpSource* source ) const
{GUCEF_TRACE;

    CString out;
    bool inList = false;            /**< the last block was a bullet, a definition or a table row */
    bool blankLineEnded = true;     /**< the text so far ends with a blank line, or is empty */
    for ( UInt32 b=0; b<(UInt32)blocks.size(); ++b )
    {
        const TBlock& block = blocks[ b ];
        const bool listBlock = IsListBlock( block.kind );
        if ( inList && !listBlock )
        {
            // a list ends with a blank line
            out += "\n";
            blankLineEnded = true;
        }
        inList = listBlock;
        switch ( block.kind )
        {
            case BLOCK_HEADING:
            {
                if ( !blankLineEnded )
                    out += "\n";
                out += CHelpMarkupParser::ToPlainText( block.text, source ) + "\n";
                blankLineEnded = false;
                break;
            }
            case BLOCK_BULLET:
            {
                out += "- " + CHelpMarkupParser::ToPlainText( block.text, source ) + "\n";
                blankLineEnded = false;
                break;
            }
            case BLOCK_DEFINITION:
            {
                out += CHelpMarkupParser::ToPlainText( block.term, source ) + ": " + CHelpMarkupParser::ToPlainText( block.text, source ) + "\n";
                blankLineEnded = false;
                break;
            }
            case BLOCK_TABLE_ROW:
            {
                CString row;
                for ( UInt32 c=0; c<(UInt32)block.cells.size(); ++c )
                    row += ( 0 == c ? CString::Empty : CString( " | " ) ) + CHelpMarkupParser::ToPlainText( block.cells[ c ], source );
                out += row + "\n";
                blankLineEnded = false;
                break;
            }
            case BLOCK_NOTE:
            {
                out += "Note: " + CHelpMarkupParser::ToPlainText( block.text, source ) + "\n\n";
                blankLineEnded = true;
                break;
            }
            case BLOCK_PARAGRAPH:
            default:
            {
                out += CHelpMarkupParser::ToPlainText( block.text, source ) + "\n\n";
                blankLineEnded = true;
                break;
            }
        }
    }
    // no line ends after the last line
    UInt32 length = out.Length();
    while ( length > 0 && '\n' == out[ length - 1 ] )
        --length;
    return out.SubstrFromRange( 0, length );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
