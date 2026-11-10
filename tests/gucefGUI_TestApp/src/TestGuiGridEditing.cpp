/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#include "gucefCORE_CDataTableBatchBuilder.h"
#include "gucefCORE_CDataTableBufferPool.h"
#include "gucefCORE_CMemoryDataTable.h"

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestGridSupport.h"
#include "gucefGUI_TestGridEditSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GridTest;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

typedef GUI::CGridCellEditor TEditor;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiGridEditingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.GridEditing" );

    GUCEF_TESTFW_TESTCASE( "GridEditing: The text of a cell is read as a value of the type of its column, and the value is written back as the same text" )
    try
    {
        CORE::CVariant value;
        CORE::CString error;
        GUI::CTableViewColumn price( "Price", CORE::DATATABLE_TYPE_FLOAT64, 0, false );
        price.prefix = "$";
        price.thousandsSeparator = true;
        price.decimals = 2;
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "$1,234.50", price, GUCEF_NULL, value, error ) && 1234.5 == value.AsFloat64() && value.IsFloat() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "-$3.25", price, GUCEF_NULL, value, error ) && -3.25 == value.AsFloat64() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "$-3.25", price, GUCEF_NULL, value, error ) && -3.25 == value.AsFloat64() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( " 1e3 ", price, GUCEF_NULL, value, error ) && 1000.0 == value.AsFloat64() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "1.2.3", price, GUCEF_NULL, value, error ) && "Not a number" == error );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "abc", price, GUCEF_NULL, value, error ) && !GUI::CTableCellParser::Parse( "1e", price, GUCEF_NULL, value, error ) );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "1,", price, GUCEF_NULL, value, error ) && !GUI::CTableCellParser::Parse( "--1", price, GUCEF_NULL, value, error ) );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "", price, GUCEF_NULL, value, error ) && value.IsNULL() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "   ", price, GUCEF_NULL, value, error ) && value.IsNULL() );

        GUI::CTableViewColumn whole( "Count", CORE::DATATABLE_TYPE_INT32, 0, false );
        whole.suffix = " pcs";
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "12 pcs", whole, GUCEF_NULL, value, error ) && 12 == value.AsInt32() && GUCEF_DATATYPE_INT32 == value.GetTypeId() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "-2147483648", whole, GUCEF_NULL, value, error ) && -2147483647 - 1 == value.AsInt32() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "2147483648", whole, GUCEF_NULL, value, error ) && "Out of range" == error );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "12.0", whole, GUCEF_NULL, value, error ) && 12 == value.AsInt32() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "12.5", whole, GUCEF_NULL, value, error ) && "Not a whole number" == error );
        GUI::CTableViewColumn byte( "Byte", CORE::DATATABLE_TYPE_UINT8, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "255", byte, GUCEF_NULL, value, error ) && 255 == value.AsUInt32() && GUCEF_DATATYPE_UINT8 == value.GetTypeId() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "256", byte, GUCEF_NULL, value, error ) && !GUI::CTableCellParser::Parse( "-1", byte, GUCEF_NULL, value, error ) && "Must not be negative" == error );
        GUI::CTableViewColumn big( "Big", CORE::DATATABLE_TYPE_UINT64, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "18446744073709551615", big, GUCEF_NULL, value, error ) && 0xFFFFFFFFFFFFFFFFULL == value.AsUInt64() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "18446744073709551616", big, GUCEF_NULL, value, error ) );
        GUI::CTableViewColumn signed64( "S", CORE::DATATABLE_TYPE_INT64, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "-9223372036854775808", signed64, GUCEF_NULL, value, error ) && ( (Int64) -9223372036854775807LL - 1 ) == value.AsInt64() );

        GUI::CTableViewColumn amount( "Amount", CORE::DATATABLE_TYPE_DECIMAL, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "12.50", amount, GUCEF_NULL, value, error ) && value.IsDecimal() && "12.50" == value.AsDecimal().ToString() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "12,5x", amount, GUCEF_NULL, value, error ) );

        GUI::CTableViewColumn flag( "Flag", CORE::DATATABLE_TYPE_BOOL, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "True", flag, GUCEF_NULL, value, error ) && value.AsBool() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "no", flag, GUCEF_NULL, value, error ) && !value.AsBool() && value.IsBoolean() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "1", flag, GUCEF_NULL, value, error ) && value.AsBool() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "maybe", flag, GUCEF_NULL, value, error ) && "Not true or false" == error );

        GUI::CTableViewColumn day( "Day", CORE::DATATABLE_TYPE_DATE, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "2021-02-03", day, GUCEF_NULL, value, error ) && "2021-02-03" == GUI::CTableCellFormatter::FormatText( value, day ) );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "2021-13-03", day, GUCEF_NULL, value, error ) && !GUI::CTableCellParser::Parse( "2021-02-30x", day, GUCEF_NULL, value, error ) );
        day.dateFormat = "MM/dd/yyyy";
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "2/3/2021", day, GUCEF_NULL, value, error ) || GUI::CTableCellParser::Parse( "02/03/2021", day, GUCEF_NULL, value, error ) );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "02/03/2021", day, GUCEF_NULL, value, error ) && "02/03/2021" == GUI::CTableCellFormatter::FormatText( value, day ) );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "2021-02-03", day, GUCEF_NULL, value, error ) );     // ISO is read too
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "03.02.2021", day, GUCEF_NULL, value, error ) && error.HasSubstr( "MM/dd/yyyy" ) >= 0 );

        GUI::CTableViewColumn stamp( "Stamp", CORE::DATATABLE_TYPE_TIMESTAMP, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "2023-11-14 22:13:20", stamp, GUCEF_NULL, value, error ) && 1700000000000LL == value.AsInt64() );
        CORE::CVariant micro = CORE::CDataTableValues::Timestamp( 5, CORE::DATATABLE_TIMEUNIT_MICROSECONDS );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "2023-11-14 22:13:20", stamp, &micro, value, error ) && 1700000000000000LL == value.AsInt64() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "2023-11-14T22:13:20.5Z", stamp, GUCEF_NULL, value, error ) && 1700000000500LL == value.AsInt64() );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "2023-11-14 25:13:20", stamp, GUCEF_NULL, value, error ) );

        GUI::CTableViewColumn text( "Text", CORE::DATATABLE_TYPE_UTF8, 0, false );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "  keep  ", text, GUCEF_NULL, value, error ) && "  keep  " == value.AsString() );
        ASSERT_TRUE( GUI::CTableCellParser::Parse( "", text, GUCEF_NULL, value, error ) && value.IsString() && value.AsString().IsNULLOrEmpty() );
        GUI::CTableViewColumn binary( "Bin", CORE::DATATABLE_TYPE_BINARY, 0, false );
        ASSERT_TRUE( !GUI::CTableCellParser::Parse( "x", binary, GUCEF_NULL, value, error ) && !GUI::CTableCellParser::IsEditableType( binary ) && GUI::CTableCellParser::IsEditableType( text ) );

        // the empty value of a cleared cell, and the text an editor starts with
        ASSERT_TRUE( GUI::CTableCellParser::EmptyValue( text ).IsString() && GUI::CTableCellParser::EmptyValue( price ).IsNULL() );
        ASSERT_TRUE( "1234.5" == GUI::CTableCellParser::FormatEditText( CORE::CVariant( (Float64) 1234.5 ), price ) );
        ASSERT_TRUE( GUI::CTableCellParser::FormatEditText( GUI::CTableCellParser::EmptyValue( price ), price ).IsNULLOrEmpty() );
        ASSERT_TRUE( "12" == GUI::CTableCellParser::FormatEditText( CORE::CVariant( (Int32) 12 ), whole ) );
        ASSERT_TRUE( "true" == GUI::CTableCellParser::FormatEditText( CORE::CVariant( true ), flag ) );
        ASSERT_TRUE( "yyyy-MM-dd" == GUI::CTableCellParser::GetTypeHint( GUI::CTableViewColumn( "d", CORE::DATATABLE_TYPE_DATE ) ) && "number" == GUI::CTableCellParser::GetTypeHint( price ) );

        // what is shown can be typed back: the same value
        CORE::CVariant shown = CORE::CVariant( (Float64) 12.25 );
        CORE::CVariant back;
        ASSERT_TRUE( GUI::CTableCellParser::Parse( GUI::CTableCellFormatter::FormatText( shown, price ), price, GUCEF_NULL, back, error ) && 12.25 == back.AsFloat64() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: Tab separated text quotes the cells that need it and reads what a spreadsheet writes" )
    try
    {
        ASSERT_TRUE( "plain" == GUI::CTabularText::QuoteCell( "plain" ) && "\"a\tb\"" == GUI::CTabularText::QuoteCell( "a\tb" ) );
        ASSERT_TRUE( "\"say \"\"hi\"\"\"" == GUI::CTabularText::QuoteCell( "say \"hi\"" ) && "\"l1\nl2\"" == GUI::CTabularText::QuoteCell( "l1\nl2" ) && "\"l1\rl2\"" == GUI::CTabularText::QuoteCell( "l1\rl2" ) );
        ASSERT_TRUE( GUI::CTabularText::QuoteCell( "" ).IsNULLOrEmpty() && "caf\xC3\xA9" == GUI::CTabularText::QuoteCell( "caf\xC3\xA9" ) );

        GUI::CTabularText::TTable table;
        GUI::CTabularText::TRow first, second;
        first.push_back( "a" ); first.push_back( "b\tc" ); first.push_back( "" );
        second.push_back( "x\ny" ); second.push_back( "\"q\"" );
        table.push_back( first );
        table.push_back( second );
        const CORE::CString text = GUI::CTabularText::Format( table );
        ASSERT_TRUE( "a\t\"b\tc\"\t\n\"x\ny\"\t\"\"\"q\"\"\"" == text );
        GUI::CTabularText::TTable parsed;
        ASSERT_TRUE( GUI::CTabularText::Parse( text, parsed ) && 2 == parsed.size() && 3 == parsed[ 0 ].size() && 2 == parsed[ 1 ].size() );
        ASSERT_TRUE( parsed[ 0 ] == first && parsed[ 1 ] == second );

        // line ends of other systems, the line end at the end, empty cells, a tab at the end
        ASSERT_TRUE( GUI::CTabularText::Parse( "a\tb\r\nc\td\r\n", parsed ) && 2 == parsed.size() && "d" == parsed[ 1 ][ 1 ] );
        ASSERT_TRUE( GUI::CTabularText::Parse( "a\rb\r", parsed ) && 2 == parsed.size() && "b" == parsed[ 1 ][ 0 ] );
        ASSERT_TRUE( GUI::CTabularText::Parse( "a\tb\n", parsed ) && 1 == parsed.size() && 2 == parsed[ 0 ].size() );
        ASSERT_TRUE( GUI::CTabularText::Parse( "a\t", parsed ) && 1 == parsed.size() && 2 == parsed[ 0 ].size() && parsed[ 0 ][ 1 ].IsNULLOrEmpty() );
        ASSERT_TRUE( GUI::CTabularText::Parse( "\t\tz", parsed ) && 1 == parsed.size() && 3 == parsed[ 0 ].size() && "z" == parsed[ 0 ][ 2 ] );
        ASSERT_TRUE( GUI::CTabularText::Parse( "a\n\nb", parsed ) && 3 == parsed.size() && parsed[ 1 ].size() == 1 && parsed[ 1 ][ 0 ].IsNULLOrEmpty() );
        ASSERT_TRUE( GUI::CTabularText::Parse( "", parsed ) && parsed.empty() );
        ASSERT_TRUE( GUI::CTabularText::Parse( "caf\xC3\xA9\t\xE2\x82\xAC", parsed ) && "\xE2\x82\xAC" == parsed[ 0 ][ 1 ] );

        // a quoted cell may hold anything; a quote in the middle of a plain cell is a quote
        ASSERT_TRUE( GUI::CTabularText::Parse( "\"a\r\nb\"\t\"c\"\"d\"", parsed ) && 1 == parsed.size() && "a\r\nb" == parsed[ 0 ][ 0 ] && "c\"d" == parsed[ 0 ][ 1 ] );
        ASSERT_TRUE( GUI::CTabularText::Parse( "5\" pipe\tx", parsed ) && "5\" pipe" == parsed[ 0 ][ 0 ] );
        ASSERT_TRUE( !GUI::CTabularText::Parse( "\"never closed\tx", parsed ) && 1 == parsed.size() );

        // a last row of one empty cell is kept by a line end of its own
        GUI::CTabularText::TTable endingEmpty;
        GUI::CTabularText::TRow oneCell, noText;
        oneCell.push_back( "a" );
        noText.push_back( "" );
        endingEmpty.push_back( oneCell );
        endingEmpty.push_back( noText );
        ASSERT_TRUE( GUI::CTabularText::EndsInEmptyCell( endingEmpty ) && !GUI::CTabularText::EndsInEmptyCell( table ) && "a\n\n" == GUI::CTabularText::Format( endingEmpty ) );
        ASSERT_TRUE( GUI::CTabularText::Parse( GUI::CTabularText::Format( endingEmpty ), parsed ) && 2 == parsed.size() && parsed[ 1 ][ 0 ].IsNULLOrEmpty() );
        endingEmpty.erase( endingEmpty.begin() );
        ASSERT_TRUE( "\n" == GUI::CTabularText::Format( endingEmpty ) && GUI::CTabularText::Parse( "\n", parsed ) && 1 == parsed.size() );

        // appending rows to text that is being built
        CORE::CString built;
        GUCEF::vector< CORE::CString > lines;
        lines.push_back( GUI::CTabularText::FormatRow( first ) );
        lines.push_back( GUI::CTabularText::FormatRow( second ) );
        built = GUI::CTabularText::JoinLines( lines );
        GUCEF::vector< CORE::CString > startsEmpty;
        startsEmpty.push_back( GUI::CTabularText::FormatRow( noText ) );
        startsEmpty.push_back( GUI::CTabularText::FormatRow( oneCell ) );
        ASSERT_TRUE( "\na" == GUI::CTabularText::JoinLines( startsEmpty ) && GUI::CTabularText::JoinLines( GUCEF::vector< CORE::CString >() ).IsNULLOrEmpty() );
        ASSERT_TRUE( built == text );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: The history of a model undoes and redoes whole steps, a group is one step, and a new edit ends the redo" )
    try
    {
        GUI::CTableEditHistory history;
        ASSERT_TRUE( !history.CanUndo() && !history.CanRedo() );
        GUI::CTableEditHistory::TEditVector edits;
        ASSERT_TRUE( !history.TakeUndo( edits ) && edits.empty() );

        history.Record( CORE::CVariant( (UInt32) 1 ), 2, CORE::CVariant( (Int32) 10 ), CORE::CVariant( (Int32) 11 ) );
        history.BeginGroup();
        history.Record( CORE::CVariant( (UInt32) 2 ), 2, CORE::CVariant( (Int32) 20 ), CORE::CVariant( (Int32) 21 ) );
        history.BeginGroup();
        history.Record( CORE::CVariant( (UInt32) 3 ), 3, CORE::CVariant( (Int32) 30 ), CORE::CVariant( (Int32) 31 ) );
        history.EndGroup();
        ASSERT_TRUE( !history.CanUndo() );                              // a step that is still being made is not undone
        history.EndGroup();
        ASSERT_TRUE( 2 == history.GetUndoStepCount() && history.CanUndo() && !history.CanRedo() );

        ASSERT_TRUE( history.TakeUndo( edits ) && 2 == edits.size() );
        ASSERT_TRUE( 3 == edits[ 0 ].rowKey.AsUInt32() && 30 == edits[ 0 ].oldValue.AsInt32() && 2 == edits[ 1 ].rowKey.AsUInt32() );     // newest first
        ASSERT_TRUE( history.CanRedo() && 1 == history.GetUndoStepCount() && 1 == history.GetRedoStepCount() );
        ASSERT_TRUE( history.TakeRedo( edits ) && 2 == edits.size() && 2 == edits[ 0 ].rowKey.AsUInt32() && 31 == edits[ 1 ].newValue.AsInt32() );   // in the order they were made
        ASSERT_TRUE( history.TakeUndo( edits ) && history.TakeUndo( edits ) && 1 == edits.size() && 10 == edits[ 0 ].oldValue.AsInt32() && !history.CanUndo() && 2 == history.GetRedoStepCount() );
        history.Record( CORE::CVariant( (UInt32) 9 ), 0, CORE::CVariant(), CORE::CVariant( (Int32) 1 ) );
        ASSERT_TRUE( !history.CanRedo() && 1 == history.GetUndoStepCount() );

        history.SetMaxSteps( 3 );
        for ( UInt32 i=0; i<10; ++i )
            history.Record( CORE::CVariant( i ), 0, CORE::CVariant( (Int32) i ), CORE::CVariant( (Int32) i + 1 ) );
        ASSERT_TRUE( 3 == history.GetUndoStepCount() && 3 == history.GetMaxSteps() );
        ASSERT_TRUE( history.TakeUndo( edits ) && 9 == edits[ 0 ].rowKey.AsUInt32() );
        history.SetMaxSteps( 0 );
        ASSERT_TRUE( 1 == history.GetMaxSteps() && 1 >= history.GetUndoStepCount() );
        history.Clear();
        ASSERT_TRUE( !history.CanUndo() && !history.CanRedo() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: The editor of a cell takes text by the kind of the column, moves its caret, selects, and picks from a list" )
    try
    {
        TEditor editor;
        ASSERT_TRUE( !editor.IsOpen() && TEditor::KIND_NONE == editor.GetKind() && !editor.InsertText( "x" ) );
        editor.Open( TEditor::KIND_TEXT, 3, 4, "caf\xC3\xA9", false );
        ASSERT_TRUE( editor.IsOpen() && 3 == editor.GetRow() && 4 == editor.GetColumn() && 4 == editor.GetCaret() && !editor.HasSelection() && !editor.IsModified() );
        ASSERT_TRUE( editor.InsertText( "s\ttab\n" ) && "caf\xC3\xA9stab" == editor.GetText() && 8 == editor.GetCaret() && editor.IsModified() );      // control characters stay out
        editor.MoveCaret( -3, false );
        ASSERT_TRUE( 5 == editor.GetCaret() && editor.InsertText( "-" ) && "caf\xC3\xA9s-tab" == editor.GetText() );
        ASSERT_TRUE( editor.Backspace() && editor.DeleteForward() && "caf\xC3\xA9sab" == editor.GetText() && 5 == editor.GetCaret() );
        editor.MoveToStart( true );
        ASSERT_TRUE( editor.HasSelection() && "caf\xC3\xA9s" == editor.GetSelectedText() );
        ASSERT_TRUE( editor.InsertText( "X" ) && "Xab" == editor.GetText() && 1 == editor.GetCaret() );
        editor.SelectAll();
        ASSERT_TRUE( "Xab" == editor.GetSelectedText() && editor.DeleteSelection() && editor.GetText().IsNULLOrEmpty() && !editor.DeleteSelection() && !editor.Backspace() && !editor.DeleteForward() );
        ASSERT_TRUE( editor.SetText( "hello" ) && 5 == editor.GetCaret() && !editor.SetText( "hello" ) );
        editor.MoveCaretTo( 1, false );
        editor.MoveCaretTo( 3, true );
        ASSERT_TRUE( "el" == editor.GetSelectedText() && "he" == editor.GetTextBefore( 2 ) && "ll" == editor.GetTextBetween( 2, 4 ) );
        editor.MoveCaret( 1, false );                                     // a selection ends at the side the caret goes to
        ASSERT_TRUE( !editor.HasSelection() && 3 == editor.GetCaret() );
        editor.MoveToEnd( false );
        editor.MoveCaretTo( 100, false );
        ASSERT_TRUE( 5 == editor.GetCaret() );
        editor.SetError( "No good" );
        ASSERT_TRUE( editor.HasError() && "No good" == editor.GetError() );
        editor.SetError( "" );
        ASSERT_TRUE( !editor.HasError() );

        // a number editor takes numbers only
        editor.Open( TEditor::KIND_NUMBER, 0, 0, "", false );
        editor.SetExtraCharacters( "$%" );
        ASSERT_TRUE( !editor.InsertText( "abc" ) && editor.InsertText( "-$1,2.5e3 %xyz" ) && "-$1,2.5e3 %" == editor.GetText() );
        editor.Open( TEditor::KIND_NUMBER, 0, 0, "7", true );
        ASSERT_TRUE( editor.HasSelection() && editor.InsertText( "8" ) && "8" == editor.GetText() );
        editor.Open( TEditor::KIND_DATE, 0, 0, "", false );
        ASSERT_TRUE( editor.InsertText( "2021-02-03" ) && "2021-02-03" == editor.GetText() );

        // no more than the limit
        editor.Open( TEditor::KIND_TEXT, 0, 0, "", false );
        CORE::CString many;
        for ( UInt32 i=0; i<5000; ++i )
            many += 'a';
        ASSERT_TRUE( editor.InsertText( many ) && (UInt32) TEditor::MAX_CHARACTERS == editor.GetCharacterCount() && !editor.InsertText( "b" ) );

        // a list of choices
        GUCEF::vector< CORE::CString > choices;
        choices.push_back( "alpha" ); choices.push_back( "Beta" ); choices.push_back( "gamma" ); choices.push_back( "beige" );
        editor.Open( TEditor::KIND_CHOICE, 0, 0, "gamma", false );
        editor.SetChoices( choices );
        ASSERT_TRUE( 2 == editor.GetChoiceIndex() && editor.MoveChoice( 1 ) && "beige" == editor.GetText() && 3 == editor.GetChoiceIndex() );
        ASSERT_TRUE( editor.MoveChoice( 1 ) && 3 == editor.GetChoiceIndex() && editor.MoveChoice( -10 ) && 0 == editor.GetChoiceIndex() );
        ASSERT_TRUE( editor.TypeAheadChoice( "b" ) && "Beta" == editor.GetText() );           // case ignored
        ASSERT_TRUE( editor.TypeAheadChoice( "b" ) && "beige" == editor.GetText() );          // the next one that starts with it
        ASSERT_TRUE( editor.TypeAheadChoice( "be" ) && "Beta" == editor.GetText() && !editor.TypeAheadChoice( "zzz" ) );
        ASSERT_TRUE( !editor.SetChoiceIndex( 9 ) && !editor.SetChoiceIndex( -1 ) && editor.SetChoiceIndex( 0 ) && "alpha" == editor.GetText() );
        editor.Open( TEditor::KIND_CHOICE, 0, 0, "other", false );
        editor.SetChoices( choices );
        ASSERT_TRUE( -1 == editor.GetChoiceIndex() && editor.MoveChoice( 1 ) && "alpha" == editor.GetText() );
        editor.Close();
        ASSERT_TRUE( !editor.IsOpen() && -1 == editor.GetRow() && editor.GetText().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: Everything is off by default: a grid with editable cells in a model does not open an editor, and the keys move as they did" )
    try
    {
        SEditScene off( false, false );
        ASSERT_TRUE( off.IsReady() );
        if ( !off.IsReady() )
            return;
        ASSERT_TRUE( !off.grid->GetBoolProperty( "CellEditing" ) && !off.grid->GetBoolProperty( "CellClipboard" ) && !off.grid->IsEditingEnabled() );
        CEditEvents events( off.grid );
        ASSERT_TRUE( off.Select( 2, CTestModel::COL_NAME ) );
        PressKey( off.context, INPUT::KEYCODE_F2 );
        Type( off.context, "x" );
        ASSERT_TRUE( !off.grid->IsEditing() && 0 == off.model->setCalls && events.events.empty() );
        PressKey( off.context, INPUT::KEYCODE_RETURN );                    // moves down, as before
        ASSERT_TRUE( !off.grid->IsEditing() && 3 == off.grid->GetCursorRow() );
        ASSERT_TRUE( DoubleClickCell( off.context, off.grid, 4, CTestModel::COL_NAME ) && !off.grid->IsEditing() );
        PressKey( off.context, INPUT::KEYCODE_DELETE );
        PressKey( off.context, INPUT::KEYCODE_SPACE );
        Chord( off.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        Chord( off.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( 0 == off.model->setCalls && "name 4" == Shown( off.model, 4, CTestModel::COL_NAME ) && events.events.empty() );
        ASSERT_TRUE( !off.grid->IsCellEditable( 2, CTestModel::COL_NAME ) );

        // a grid in the row mode, and one without a model, have no editing either
        GUI::CWidget* screen = off.grid->GetParentWidget();
        GUI::CGridView* rowGrid = MakeGrid( off.context, screen, 10, 400, 400, 200 );
        rowGrid->SetModel( off.modelPtr );
        rowGrid->SetBoolProperty( "CellEditing", true );
        off.context->FlushChanges();
        ASSERT_TRUE( !rowGrid->IsEditingEnabled() && !rowGrid->BeginEdit( 1, 1 ) );
        GUI::CGridView* listGrid = MakeGrid( off.context, screen, 420, 400, 300, 200 );
        listGrid->SetProperty( "SelectionMode", "Cell" );
        listGrid->SetBoolProperty( "CellEditing", true );
        ASSERT_TRUE( !listGrid->IsEditingEnabled() && !listGrid->IsCellEditable( 0, 0 ) && !listGrid->ApplyCellEdit( 0, 0, CORE::CVariant( (Int32) 1 ) ).IsAccepted() );

        // a model that does not override the editing says read only to everything
        CTestModel* plain = GUCEF_NEW CTestModel( 10 );
        GUI::TTableViewModelPtr plainPtr( plain );
        GUI::CGridView* plainGrid = MakeEditGrid( off.context, plainPtr, true, true );
        plainGrid->SetPosition( GUI::CUVector( GUI::CUDim( 0.0f, 10 ), GUI::CUDim( 0.0f, 700 ) ) );
        off.context->FlushChanges();
        CEditEvents plainEvents( plainGrid );
        ASSERT_TRUE( plainGrid->IsEditingEnabled() && !plainGrid->IsCellEditable( 1, CTestModel::COL_NAME ) );
        ASSERT_TRUE( !plainGrid->BeginEdit( 1, CTestModel::COL_NAME ) && 1 == plainEvents.Count( GUI::CGridView::CellEditRefusedEvent ) );
        ASSERT_TRUE( GUI::CTableEditResult::STATUS_READ_ONLY == plainGrid->ApplyCellEdit( 1, CTestModel::COL_NAME, CORE::CVariant( "x" ) ).status );
        GUI::CITableViewModel& plainModel = *plainPtr;
        ASSERT_TRUE( GUI::CTableEditResult::STATUS_READ_ONLY == plainModel.SetCellValue( 1, 1, CORE::CVariant( "x" ) ).status && !plainModel.CanUndo() && !plainModel.Undo() );
        ASSERT_TRUE( plainModel.IsCellReadOnly( 0, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: F2, Enter, a double click and typing open the editor; Enter and Tab commit and move, Escape cancels" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );
        ASSERT_TRUE( grid->IsEditingEnabled() );

        // F2: the value as text, the caret at the end
        ASSERT_TRUE( scene.Select( 2, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( grid->IsEditing() && NULL != grid->GetCellEditor() );
        const TEditor* editor = grid->GetCellEditor();
        ASSERT_TRUE( TEditor::KIND_TEXT == editor->GetKind() && 2 == editor->GetRow() && CTestModel::COL_NAME == editor->GetColumn() && "name 2" == editor->GetText() && 6 == editor->GetCaret() && !editor->HasSelection() );
        ASSERT_TRUE( 1 == events.Count( GUI::CGridView::CellEditStartedEvent ) && 2 == events.Last( GUI::CGridView::CellEditStartedEvent )->report.row );
        Type( scene.context, "X" );
        ASSERT_TRUE( "name 2X" == grid->GetCellEditor()->GetText() && "name 2" == Shown( scene.model, 2, CTestModel::COL_NAME ) );     // nothing goes to the model before the commit

        // Enter: commit, and on to the cell below
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( !grid->IsEditing() && NULL == grid->GetCellEditor() && "name 2X" == Shown( scene.model, 2, CTestModel::COL_NAME ) && 1 == scene.model->setCalls );
        ASSERT_TRUE( 3 == grid->GetCursorRow() && CTestModel::COL_NAME == grid->GetCursorColumn() );
        const CEditEvents::SEvent* committed = events.Last( GUI::CGridView::CellEditCommittedEvent );
        ASSERT_TRUE( 1 == events.Count( GUI::CGridView::CellEditCommittedEvent ) && GUCEF_NULL != committed && 2 == committed->report.row && CTestModel::COL_NAME == committed->report.column && 1 == committed->report.cellsApplied );

        // Enter opens the editor of a single selected cell, Escape cancels
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( grid->IsEditing() && 3 == grid->GetCellEditor()->GetRow() );
        Type( scene.context, "ZZZ" );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( !grid->IsEditing() && "name 3" == Shown( scene.model, 3, CTestModel::COL_NAME ) && 1 == events.Count( GUI::CGridView::CellEditCancelledEvent ) && 3 == grid->GetCursorRow() );

        // typing replaces the text, Tab commits and goes to the next cell, Shift + Tab back
        Type( scene.context, "zz" );
        ASSERT_TRUE( grid->IsEditing() && "zz" == grid->GetCellEditor()->GetText() && 2 == grid->GetCellEditor()->GetCaret() );
        PressKey( scene.context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( !grid->IsEditing() && "zz" == Shown( scene.model, 3, CTestModel::COL_NAME ) && 3 == grid->GetCursorRow() && CTestModel::COL_NAME + 1 == grid->GetCursorColumn() );
        Chord( scene.context, INPUT::KEYCODE_TAB, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( 3 == grid->GetCursorRow() && CTestModel::COL_NAME == grid->GetCursorColumn() );

        // Shift + Enter commits and goes up
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        Type( scene.context, "!" );
        Chord( scene.context, INPUT::KEYCODE_RETURN, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( !grid->IsEditing() && "zz!" == Shown( scene.model, 3, CTestModel::COL_NAME ) && 2 == grid->GetCursorRow() );

        // a double click
        ASSERT_TRUE( DoubleClickCell( scene.context, grid, 6, CTestModel::COL_NAME ) );
        ASSERT_TRUE( grid->IsEditing() && 6 == grid->GetCellEditor()->GetRow() && "name 6" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );

        // Enter at the last row stays there
        ASSERT_TRUE( scene.Select( 99, CTestModel::COL_NAME ) );
        Type( scene.context, "last" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "last" == Shown( scene.model, 99, CTestModel::COL_NAME ) && 99 == grid->GetCursorRow() );

        // a block of cells is walked through, one cell is edited: Enter does not open an editor in a block
        scene.context->FlushChanges();
        grid->SelectRange( 10, CTestModel::COL_NAME, 12, CTestModel::COL_NAME );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( !grid->IsEditing() && 11 == grid->GetCursorRow() );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( grid->IsEditing() && 11 == grid->GetCellEditor()->GetRow() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( !grid->IsEditing() );

        // an editor that is committed unchanged is a cancel, and never reaches the model
        const UInt32 calls = scene.model->setCalls;
        ASSERT_TRUE( scene.Select( 20, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        PressKey( scene.context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( !grid->IsEditing() && calls == scene.model->setCalls );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: The editor is the one of the type of the column: a number editor takes numbers only, dates are read by their pattern, an empty text is a null" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );

        // a number: letters stay out, the value is a float again and is shown by the format of the column
        ASSERT_TRUE( scene.Select( 10, CTestModel::COL_PRICE ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( grid->IsEditing() && TEditor::KIND_NUMBER == grid->GetCellEditor()->GetKind() && "2.5" == grid->GetCellEditor()->GetText() );
        Type( scene.context, "abc" );
        ASSERT_TRUE( "2.5" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        Type( scene.context, "x" );
        ASSERT_TRUE( !grid->IsEditing() );                           // typing a letter on a number does not open anything
        Type( scene.context, "1234" );
        Type( scene.context, ".5" );
        ASSERT_TRUE( grid->IsEditing() && "1234.5" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( !grid->IsEditing() && 1234.5 == scene.model->Current( 10, CTestModel::COL_PRICE ).AsFloat64() && "$1,234.50" == Shown( scene.model, 10, CTestModel::COL_PRICE ) );

        // the prefix and the grouping that are shown can be typed too
        ASSERT_TRUE( scene.Select( 11, CTestModel::COL_PRICE ) );
        Type( scene.context, "$2,000" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 2000.0 == scene.model->Current( 11, CTestModel::COL_PRICE ).AsFloat64() );

        // a whole number and a decimal
        ASSERT_TRUE( scene.Select( 12, CTestModel::COL_MAYBE ) );
        Type( scene.context, "77" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( 77 == scene.model->Current( 12, CTestModel::COL_MAYBE ).AsInt32() && GUCEF_DATATYPE_INT32 == scene.model->Current( 12, CTestModel::COL_MAYBE ).GetTypeId() );
        ASSERT_TRUE( scene.Select( 12, CTestModel::COL_AMOUNT ) );
        Type( scene.context, "5.125" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( scene.model->Current( 12, CTestModel::COL_AMOUNT ).IsDecimal() && "5.125" == scene.model->Current( 12, CTestModel::COL_AMOUNT ).AsDecimal().ToString() );

        // empty: a null
        ASSERT_TRUE( scene.Select( 12, CTestModel::COL_MAYBE ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( "77" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_BACK );
        PressKey( scene.context, INPUT::KEYCODE_BACK );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( scene.model->Current( 12, CTestModel::COL_MAYBE ).IsNULL() );
        GUI::SCellContent content;
        ASSERT_TRUE( grid->GetCellContent( 12, CTestModel::COL_MAYBE, content ) && GUI::SCellContent::KIND_NULL == content.kind );

        // a date: the pattern of the column; a date that is no date marks the editor
        ASSERT_TRUE( scene.Select( 13, CTestModel::COL_DATE ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( TEditor::KIND_DATE == grid->GetCellEditor()->GetKind() && "2019-04-27" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        Type( scene.context, "2020-13-45" );
        ASSERT_TRUE( grid->IsEditing() && grid->GetCellEditor()->HasError() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( grid->IsEditing() && 1 == events.Count( GUI::CGridView::CellEditRefusedEvent ) );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        Type( scene.context, "2021-02-03" );
        ASSERT_TRUE( !grid->GetCellEditor()->HasError() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "2021-02-03" == Shown( scene.model, 13, CTestModel::COL_DATE ) && GUCEF_DATATYPE_LE_DATE_IN_DAYS_SINCE_UNIX_EPOCH == scene.model->Current( 13, CTestModel::COL_DATE ).GetTypeId() );

        // a timestamp keeps its unit
        ASSERT_TRUE( scene.Select( 14, CTestModel::COL_STAMP ) );
        Type( scene.context, "2024-05-06 07:08:09" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "2024-05-06 07:08:09" == Shown( scene.model, 14, CTestModel::COL_STAMP ) );

        // Delete clears the selected cells: text becomes empty text, the others a null
        ASSERT_TRUE( scene.Select( 15, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( scene.model->Current( 15, CTestModel::COL_NAME ).IsString() && scene.model->Current( 15, CTestModel::COL_NAME ).AsString().IsNULLOrEmpty() );
        ASSERT_TRUE( scene.Select( 15, CTestModel::COL_PRICE ) );
        PressKey( scene.context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( scene.model->Current( 15, CTestModel::COL_PRICE ).IsNULL() && 2 == events.Count( GUI::CGridView::RangeEditResultEvent ) );
        const CEditEvents::SEvent* cleared = events.Last( GUI::CGridView::RangeEditResultEvent );
        ASSERT_TRUE( GUCEF_NULL != cleared && GUI::CGridEditReport::OPERATION_CLEAR == cleared->report.operation && 1 == cleared->report.cellsApplied && !cleared->report.refused );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: A list of choices comes from the definition of the column and is picked with the keys, letters or the mouse" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );
        GUCEF::vector< CORE::CString >& choices = scene.model->EditColumn( CTestModel::COL_NAME ).choices;
        choices.push_back( "alpha" ); choices.push_back( "beta" ); choices.push_back( "gamma" ); choices.push_back( "delta" );

        ASSERT_TRUE( scene.Select( 4, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( grid->IsEditing() && TEditor::KIND_CHOICE == grid->GetCellEditor()->GetKind() && 4 == grid->GetCellEditor()->GetChoices().size() && -1 == grid->GetCellEditor()->GetChoiceIndex() );
        PressKey( scene.context, INPUT::KEYCODE_DOWN );
        PressKey( scene.context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( "beta" == grid->GetCellEditor()->GetText() && 1 == grid->GetCellEditor()->GetChoiceIndex() && 4 == grid->GetCursorRow() );        // the arrows pick, they do not move the cursor
        PressKey( scene.context, INPUT::KEYCODE_UP );
        ASSERT_TRUE( "alpha" == grid->GetCellEditor()->GetText() );
        Type( scene.context, "g" );
        ASSERT_TRUE( "gamma" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_END );
        ASSERT_TRUE( "delta" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_HOME );
        ASSERT_TRUE( "alpha" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( !grid->IsEditing() && "alpha" == Shown( scene.model, 4, CTestModel::COL_NAME ) );

        // typing on the cell is a letter of the list
        ASSERT_TRUE( scene.Select( 5, CTestModel::COL_NAME ) );
        Type( scene.context, "d" );
        ASSERT_TRUE( grid->IsEditing() && "delta" == grid->GetCellEditor()->GetText() );
        Type( scene.context, "zzz" );
        ASSERT_TRUE( "delta" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "delta" == Shown( scene.model, 5, CTestModel::COL_NAME ) );

        // the list is under the cell, a click on a choice picks it and commits
        ASSERT_TRUE( scene.Select( 6, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        const GUI::CGridEditController* controller = grid->GetEditController();
        GUI::CGridEditController::SEditorLayout layout;
        ASSERT_TRUE( GUCEF_NULL != controller && controller->GetEditorLayout( layout ) && layout.hasList && 4 == layout.itemCount && 0 == layout.firstItem );
        ASSERT_TRUE( layout.list.top == layout.cell.top + grid->GetRowHeight() && layout.list.left == layout.cell.left && layout.list.width >= layout.cell.width );
        scene.context->FlushChanges();
        const Int32 x = grid->GetRect().left + layout.list.left + 10;
        const Int32 y = grid->GetRect().top + layout.list.top + 2 * layout.itemHeight + layout.itemHeight / 2;
        ClickAt( scene.context, x, y );
        ASSERT_TRUE( !grid->IsEditing() && "gamma" == Shown( scene.model, 6, CTestModel::COL_NAME ) && 6 == grid->GetCursorRow() );

        // the list of a cell low in the grid opens upwards
        ASSERT_TRUE( scene.Select( 12, CTestModel::COL_NAME ) );
        grid->ScrollToRow( 90 );
        ASSERT_TRUE( scene.Select( 99, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( controller->GetEditorLayout( layout ) && layout.hasList && layout.list.top + layout.list.height <= layout.cell.top );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );

        // a click outside the list commits the choice that is shown
        ASSERT_TRUE( scene.Select( 7, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        PressKey( scene.context, INPUT::KEYCODE_DOWN );
        ASSERT_TRUE( scene.Select( 9, CTestModel::COL_ID ) );
        ASSERT_TRUE( !grid->IsEditing() && "alpha" == Shown( scene.model, 7, CTestModel::COL_NAME ) && 9 == grid->GetCursorRow() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: A check box is toggled by a click or Space, through the model, and not when the cell is read only" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );
        ASSERT_TRUE( !scene.model->Current( 4, CTestModel::COL_FLAG ).AsBool() );

        ASSERT_TRUE( ClickCell( scene.context, grid, 4, CTestModel::COL_FLAG ) );
        ASSERT_TRUE( scene.model->Current( 4, CTestModel::COL_FLAG ).AsBool() && scene.model->Current( 4, CTestModel::COL_FLAG ).IsBoolean() && 4 == grid->GetCursorRow() && !grid->IsEditing() );
        ASSERT_TRUE( 1 == events.Count( GUI::CGridView::CellEditCommittedEvent ) && GUI::CGridEditReport::OPERATION_TOGGLE == events.Last( GUI::CGridView::CellEditCommittedEvent )->report.operation );
        PressKey( scene.context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( !scene.model->Current( 4, CTestModel::COL_FLAG ).AsBool() );
        PressKey( scene.context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( scene.model->Current( 4, CTestModel::COL_FLAG ).AsBool() && 3 == scene.model->setCalls );

        // a double click is one toggle, not two, and opens no editor
        ASSERT_TRUE( DoubleClickCell( scene.context, grid, 6, CTestModel::COL_FLAG ) );
        ASSERT_TRUE( scene.model->Current( 6, CTestModel::COL_FLAG ).AsBool() && !grid->IsEditing() );
        ASSERT_TRUE( 4 == scene.model->setCalls );

        // Enter and F2 are not for check boxes; a null becomes true
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( !grid->IsEditing() );
        scene.model->EditColumn( CTestModel::COL_FLAG ).booleanDisplay = GUI::CTableViewColumn::BOOLEAN_TEXT;
        ASSERT_TRUE( ClickCell( scene.context, grid, 8, CTestModel::COL_FLAG ) && scene.model->Current( 8, CTestModel::COL_FLAG ).AsBool() );
        scene.model->EditColumn( CTestModel::COL_FLAG ).booleanDisplay = GUI::CTableViewColumn::BOOLEAN_CHECK;

        // read only: nothing happens to the value, the refusal is reported
        scene.model->readOnlyIds.insert( 10 );
        const UInt32 calls = scene.model->setCalls;
        ASSERT_TRUE( ClickCell( scene.context, grid, 10, CTestModel::COL_FLAG ) );
        PressKey( scene.context, INPUT::KEYCODE_SPACE );
        ASSERT_TRUE( calls == scene.model->setCalls && !scene.model->Current( 10, CTestModel::COL_FLAG ).AsBool() );
        ASSERT_TRUE( events.Count( GUI::CGridView::CellEditRefusedEvent ) >= 1 );
        scene.model->EditColumn( CTestModel::COL_FLAG ).readOnly = true;
        ASSERT_TRUE( ClickCell( scene.context, grid, 12, CTestModel::COL_FLAG ) && calls == scene.model->setCalls );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: Cells that are read only by column or by cell are not edited, and the model has the say about a value while it is typed and when it is committed" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );

        // a column that is read only
        ASSERT_TRUE( scene.Select( 2, CTestModel::COL_ID ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( !grid->IsEditing() && 1 == events.Count( GUI::CGridView::CellEditRefusedEvent ) && events.Last( GUI::CGridView::CellEditRefusedEvent )->report.refused );
        ASSERT_TRUE( 2 == events.Last( GUI::CGridView::CellEditRefusedEvent )->report.row && !events.Last( GUI::CGridView::CellEditRefusedEvent )->report.message.IsNULLOrEmpty() );
        Type( scene.context, "5" );
        ASSERT_TRUE( !grid->IsEditing() && 1 == events.Count( GUI::CGridView::CellEditRefusedEvent ) );          // typing says nothing
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( !grid->IsEditing() && 3 == grid->GetCursorRow() );

        // a cell
        scene.model->readOnlyIds.insert( 5 );
        ASSERT_TRUE( scene.Select( 5, CTestModel::COL_NAME ) && scene.Select( 6, CTestModel::COL_NAME ) && scene.Select( 5, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( !grid->IsEditing() && 2 == events.Count( GUI::CGridView::CellEditRefusedEvent ) && !grid->IsCellEditable( 5, CTestModel::COL_NAME ) && grid->IsCellEditable( 6, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( "name 5" == Shown( scene.model, 5, CTestModel::COL_NAME ) );

        // the model refuses while the user types: the editor is marked with the reason, and the value is not taken
        ASSERT_TRUE( scene.Select( 7, CTestModel::COL_PRICE ) );
        Type( scene.context, "-3" );
        ASSERT_TRUE( grid->IsEditing() && grid->GetCellEditor()->HasError() && "The price must not be negative" == grid->GetCellEditor()->GetError() );
        const UInt32 validations = scene.model->validateCalls;
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( grid->IsEditing() && scene.model->validateCalls > validations && 0 == scene.model->setCalls && 7 == grid->GetCursorRow() );
        const CEditEvents::SEvent* refused = events.Last( GUI::CGridView::CellEditRefusedEvent );
        ASSERT_TRUE( GUCEF_NULL != refused && "The price must not be negative" == refused->report.message && 7 == refused->report.row && CTestModel::COL_PRICE == refused->report.column );

        // the tooltip over the editor says why
        Int32 x = 0, y = 0;
        ASSERT_TRUE( CellPoint( scene.context, grid, 7, CTestModel::COL_PRICE, x, y ) );
        ASSERT_TRUE( "The price must not be negative" == grid->GetTooltipAt( x - grid->GetRect().left, y - grid->GetRect().top ) );
        ASSERT_TRUE( grid->GetTooltipAt( 5, 150 ).IsNULLOrEmpty() );

        // a fixed value is taken
        PressKey( scene.context, INPUT::KEYCODE_BACK );
        PressKey( scene.context, INPUT::KEYCODE_BACK );
        ASSERT_TRUE( !grid->GetCellEditor()->HasError() );
        Type( scene.context, "8" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( !grid->IsEditing() && 8.0 == scene.model->Current( 7, CTestModel::COL_PRICE ).AsFloat64() && 1 == scene.model->setCalls );

        // a text that is no number is refused by the grid before the model is asked
        ASSERT_TRUE( scene.Select( 8, CTestModel::COL_PRICE ) );
        Type( scene.context, "1e" );
        ASSERT_TRUE( grid->GetCellEditor()->HasError() && "Not a number" == grid->GetCellEditor()->GetError() );
        PressKey( scene.context, INPUT::KEYCODE_TAB );
        ASSERT_TRUE( grid->IsEditing() && 1 == scene.model->setCalls && 8 == grid->GetCursorRow() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );

        // a model that refuses everything, also at the commit and for a text
        scene.model->refuseAll = true;
        ASSERT_TRUE( scene.Select( 9, CTestModel::COL_NAME ) );
        Type( scene.context, "ok" );
        ASSERT_TRUE( grid->GetCellEditor()->HasError() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( grid->IsEditing() && "name 9" == Shown( scene.model, 9, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        scene.model->refuseAll = false;

        // name rule: not "!"
        Type( scene.context, "!bang" );
        ASSERT_TRUE( grid->IsEditing() && grid->GetCellEditor()->HasError() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( !grid->IsEditing() && "name 9" == Shown( scene.model, 9, CTestModel::COL_NAME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: Ctrl+Z and Ctrl+Y go to the model, which keeps the history; a model without one is not asked" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );
        ASSERT_TRUE( scene.Select( 3, CTestModel::COL_NAME ) );
        Type( scene.context, "one" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( scene.Select( 3, CTestModel::COL_NAME ) );
        Type( scene.context, "two" );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "two" == Shown( scene.model, 3, CTestModel::COL_NAME ) && scene.model->CanUndo() && !scene.model->CanRedo() );

        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "one" == Shown( scene.model, 3, CTestModel::COL_NAME ) && scene.model->CanRedo() );
        ASSERT_TRUE( events.Last( GUI::CGridView::RangeEditResultEvent ) && GUI::CGridEditReport::OPERATION_UNDO == events.Last( GUI::CGridView::RangeEditResultEvent )->report.operation );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "name 3" == Shown( scene.model, 3, CTestModel::COL_NAME ) && !scene.model->CanUndo() );
        const UInt32 results = events.Count( GUI::CGridView::RangeEditResultEvent );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );                  // nothing left: the key is not used
        ASSERT_TRUE( results == events.Count( GUI::CGridView::RangeEditResultEvent ) );
        Chord( scene.context, INPUT::KEYCODE_Y, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "one" == Shown( scene.model, 3, CTestModel::COL_NAME ) );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT );          // Ctrl + Shift + Z is redo
        ASSERT_TRUE( "two" == Shown( scene.model, 3, CTestModel::COL_NAME ) && !scene.model->CanRedo() );

        // while an editor is open the keys are for the editor
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        Chord( scene.context, INPUT::KEYCODE_Z, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( grid->IsEditing() && "two" == Shown( scene.model, 3, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: The editor follows its row when the model inserts, removes or re-orders rows, and closes when the row goes" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        CEditEvents events( grid );
        ASSERT_TRUE( scene.Select( 5, CTestModel::COL_NAME ) );
        Type( scene.context, "five" );
        scene.model->InsertRows( 0, 3 );
        ASSERT_TRUE( grid->IsEditing() && 8 == grid->GetCellEditor()->GetRow() && "five" == grid->GetCellEditor()->GetText() );
        scene.model->Reverse();
        ASSERT_TRUE( grid->IsEditing() && 102 - 8 == grid->GetCellEditor()->GetRow() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( !grid->IsEditing() && "five" == Shown( scene.model, 94, CTestModel::COL_NAME ) && 5 == scene.model->IdOf( 94 ) );

        scene.model->Reverse();
        ASSERT_TRUE( scene.Select( 20, CTestModel::COL_NAME ) );
        Type( scene.context, "gone" );
        ASSERT_TRUE( grid->IsEditing() );
        scene.model->RemoveRows( 20, 1 );
        ASSERT_TRUE( !grid->IsEditing() && 1 == events.Count( GUI::CGridView::CellEditCancelledEvent ) );

        // other rows removed: the editor moves up
        ASSERT_TRUE( scene.Select( 30, CTestModel::COL_NAME ) );
        Type( scene.context, "moved" );
        scene.model->RemoveRows( 0, 10 );
        ASSERT_TRUE( grid->IsEditing() && 20 == grid->GetCellEditor()->GetRow() );
        PressKey( scene.context, INPUT::KEYCODE_RETURN );
        ASSERT_TRUE( "moved" == Shown( scene.model, 20, CTestModel::COL_NAME ) );

        // a new model, or the end of editing: the editor goes
        Type( scene.context, "again" );
        ASSERT_TRUE( grid->IsEditing() );
        CTestModel* other = GUCEF_NEW CTestModel( 5 );
        grid->SetModel( GUI::TTableViewModelPtr( other ) );
        ASSERT_TRUE( !grid->IsEditing() );
        grid->SetModel( scene.modelPtr );
        grid->SetCursor( 3, CTestModel::COL_NAME );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        ASSERT_TRUE( grid->IsEditing() );
        grid->CancelEdit();
        ASSERT_TRUE( !grid->IsEditing() && grid->CommitEdit() );
        ASSERT_TRUE( grid->BeginEdit( 3, CTestModel::COL_NAME ) && grid->IsEditing() && grid->CommitEdit() && !grid->IsEditing() );
        ASSERT_TRUE( !grid->BeginEdit( 500, 0 ) && !grid->BeginEdit( 0, 99 ) && !grid->BeginEdit( -1, 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: A click puts the caret, a click outside commits, a click outside with a refused value keeps the editor, and losing the focus commits" )
    try
    {
        SEditScene scene;
        ASSERT_TRUE( scene.IsReady() );
        if ( !scene.IsReady() )
            return;
        GUI::CGridView* grid = scene.grid;
        ASSERT_TRUE( scene.Select( 2, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        Int32 x = 0, y = 0;
        GUI::CGridEditController::SEditorLayout layout;
        ASSERT_TRUE( grid->GetEditController()->GetEditorLayout( layout ) );

        // a click at the start of the text puts the caret there
        ClickAt( scene.context, grid->GetRect().left + layout.textLeft + 1, grid->GetRect().top + layout.cell.top + 3 );
        ASSERT_TRUE( grid->IsEditing() && 0 == grid->GetCellEditor()->GetCaret() );
        Type( scene.context, "A" );
        ASSERT_TRUE( "Aname 2" == grid->GetCellEditor()->GetText() );
        // a click in the middle of the third character
        ASSERT_TRUE( grid->GetEditController()->GetEditorLayout( layout ) );
        ClickAt( scene.context, grid->GetRect().left + layout.textLeft + 2 * 8 + 6, grid->GetRect().top + layout.cell.top + 3 );
        ASSERT_TRUE( 3 == grid->GetCellEditor()->GetCaret() );
        PressKey( scene.context, INPUT::KEYCODE_HOME );
        Chord( scene.context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_SHIFT );
        Chord( scene.context, INPUT::KEYCODE_RIGHT, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( "An" == grid->GetCellEditor()->GetSelectedText() );
        Type( scene.context, "_" );
        ASSERT_TRUE( "_ame 2" == grid->GetCellEditor()->GetText() );
        Chord( scene.context, INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "_ame 2" == grid->GetCellEditor()->GetSelectedText() );
        Type( scene.context, "new" );
        PressKey( scene.context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( "new" == grid->GetCellEditor()->GetText() );
        PressKey( scene.context, INPUT::KEYCODE_HOME );
        PressKey( scene.context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( "ew" == grid->GetCellEditor()->GetText() );

        // a click on another cell commits and selects that cell
        ASSERT_TRUE( ClickCell( scene.context, grid, 6, CTestModel::COL_PRICE ) );
        ASSERT_TRUE( !grid->IsEditing() && "ew" == Shown( scene.model, 2, CTestModel::COL_NAME ) && 6 == grid->GetCursorRow() && CTestModel::COL_PRICE == grid->GetCursorColumn() );

        // a refused value: the click is taken by the editor
        Type( scene.context, "-1" );
        ASSERT_TRUE( grid->IsEditing() && grid->GetCellEditor()->HasError() );
        ASSERT_TRUE( ClickCell( scene.context, grid, 9, CTestModel::COL_NAME ) );
        ASSERT_TRUE( grid->IsEditing() && 6 == grid->GetCursorRow() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );

        // the editor takes its text from the clipboard of the system through the hook, one line of it
        CFakeClipboard clipboard;
        CFakeClipboard::Text() = "pasted\nsecond line";
        ASSERT_TRUE( scene.Select( 7, CTestModel::COL_NAME ) );
        PressKey( scene.context, INPUT::KEYCODE_F2 );
        Chord( scene.context, INPUT::KEYCODE_A, INPUT::KEYMOD_CTRL );
        Chord( scene.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "pasted" == grid->GetCellEditor()->GetText() );
        Chord( scene.context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_SHIFT );
        Chord( scene.context, INPUT::KEYCODE_LEFT, INPUT::KEYMOD_SHIFT );
        Chord( scene.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "ed" == CFakeClipboard::Text() );
        Chord( scene.context, INPUT::KEYCODE_X, INPUT::KEYMOD_CTRL );
        ASSERT_TRUE( "past" == grid->GetCellEditor()->GetText() && "ed" == CFakeClipboard::Text() );
        PressKey( scene.context, INPUT::KEYCODE_ESCAPE );

        // the focus goes elsewhere: the value is kept
        ASSERT_TRUE( scene.Select( 8, CTestModel::COL_NAME ) );
        Type( scene.context, "kept" );
        GUI::CWidget* other = scene.context->CreateWidget( "Editbox", "Other", grid->GetParentWidget() );
        scene.context->FlushChanges();
        ASSERT_TRUE( scene.context->SetFocus( other ) );
        ASSERT_TRUE( !grid->IsEditing() && "kept" == Shown( scene.model, 8, CTestModel::COL_NAME ) );

        // the focus goes elsewhere with a refused value: the editor closes and the value is not taken
        ASSERT_TRUE( scene.context->SetFocus( grid ) );
        ASSERT_TRUE( scene.Select( 9, CTestModel::COL_PRICE ) );
        Type( scene.context, "-9" );
        ASSERT_TRUE( grid->IsEditing() && grid->GetCellEditor()->HasError() );
        scene.context->SetFocus( other );
        ASSERT_TRUE( !grid->IsEditing() && 2.25 == scene.model->Current( 9, CTestModel::COL_PRICE ).AsFloat64() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "GridEditing: A model on a data table is read only: editing refuses, and nothing changes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = MakeContext( 1280, 960 );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        CORE::CDataTableBufferPool pool;
        CORE::CDataTableDefinition definition;
        definition.name = "editgrid";
        definition.schema.AddColumn( CORE::CDataTableColumnDef( "id", CORE::DATATABLE_TYPE_INT64, false ) );
        definition.schema.AddColumn( CORE::CDataTableColumnDef( "name", CORE::DATATABLE_TYPE_UTF8 ) );
        CORE::CIDataTablePtr table( GUCEF_NEW CORE::CMemoryDataTable( definition, &pool ) );
        CORE::CDataTableBatchBuilder builder( table->GetSchema(), pool );
        for ( UInt32 i=0; i<20; ++i )
        {
            CORE::CVariant cells[ 2 ];
            cells[ 0 ] = (Int64) i;
            cells[ 1 ] = CORE::ToUtf8String( CORE::CString( "row " ) + CORE::UInt32ToString( i ) );
            builder.AppendRow( cells, 2 );
        }
        CORE::CDataTableBatchPtr batch;
        CORE::CString error;
        CORE::CDataTableWriteResult result;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == builder.Build( batch, error ) && CORE::DATATABLE_STATUS_OK == table->Write( *batch, CORE::DATATABLE_WRITE_INSERT, result ) );

        GUI::CDataTableViewModel* raw = GUCEF_NEW GUI::CDataTableViewModel( table );
        GUI::TTableViewModelPtr model( raw );
        GUI::CGridView* grid = MakeEditGrid( context, model, true, true );
        CEditEvents events( grid );
        const GUI::CITableViewModel& reader = *grid->GetModel();
        ASSERT_TRUE( 20 == reader.GetRowCount() && 2 == reader.GetColumnCount() );
        ASSERT_TRUE( "row 3" == reader.GetCellText( 3, 1 ) );
        ASSERT_TRUE( reader.IsCellReadOnly( 3, 1 ) && reader.GetColumn( 1 ).readOnly );
        ASSERT_TRUE( !reader.CanUndo() );
        ASSERT_TRUE( GUI::CTableEditResult::STATUS_READ_ONLY == raw->SetCellValue( 3, 1, CORE::CVariant( "x" ) ).status );
        ASSERT_TRUE( ClickCell( context, grid, 3, 1 ) );
        PressKey( context, INPUT::KEYCODE_F2 );
        Type( context, "q" );
        PressKey( context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( !grid->IsEditing() && "row 3" == reader.GetCellText( 3, 1 ) && events.Count( GUI::CGridView::CellEditRefusedEvent ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
