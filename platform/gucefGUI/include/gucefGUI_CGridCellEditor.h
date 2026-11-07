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
#ifndef GUCEF_GUI_CGRIDCELLEDITOR_H
#define GUCEF_GUI_CGRIDCELLEDITOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

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
 *  The state of the editor of one cell of a grid: the text that is being typed with its caret and selection, the
 *  kind of editor (text, number, date or a list of choices), and whether the model refused the value (with its message).
 *  The editor knows nothing of the grid or the model; the session that owns it (CGridEditController) gives it the
 *  keys and the text, parses the text into a value of the column and asks the model. The painter draws it.
 *
 *  A number editor takes digits, a sign, the decimal point, the grouping comma, an exponent and the characters
 *  given with SetExtraCharacters() (the prefix and suffix of the column); every other character is ignored, so
 *  what is typed can only be a number. The other kinds take any text (no control characters). A list of choices has
 *  the text of one of the choices, chosen with the keys Up and Down, a click or the first letters of a choice.
 */
class GUCEF_GUI_PUBLIC_CPP CGridCellEditor
{
    public:

    enum EKind
    {
        KIND_NONE = 0   ,   /**< no editor is open */
        KIND_TEXT       ,
        KIND_NUMBER     ,
        KIND_DATE       ,   /**< text that is read as a date or a time by the pattern of the column */
        KIND_CHOICE
    };

    /** The most characters an editor takes */
    enum ELimits
    {
        MAX_CHARACTERS = 4096
    };

    CGridCellEditor( void );

    /** Opens the editor on a cell with the text; with selectAll the text is selected, so that typing replaces it, else the caret is at the end */
    void Open( EKind kind, Int32 row, Int32 column, const CString& text, bool selectAll );

    void Close( void );

    bool IsOpen( void ) const;

    EKind GetKind( void ) const;

    Int32 GetRow( void ) const;

    Int32 GetColumn( void ) const;

    /** The row moved (rows were inserted or removed before it) */
    void SetRow( Int32 row );

    const CString& GetText( void ) const;

    /** The text the editor was opened with */
    const CString& GetOriginalText( void ) const;

    bool IsModified( void ) const;

    UInt32 GetCharacterCount( void ) const;

    /** The first 'index' code points of the text, for measuring the place of the caret */
    CString GetTextBefore( UInt32 index ) const;

    /** The code points [first, last) of the text */
    CString GetTextBetween( UInt32 first, UInt32 last ) const;

    /*
     *  Caret and selection
     */

    UInt32 GetCaret( void ) const;

    bool HasSelection( void ) const;

    /** The selected code points: [first, last) */
    void GetSelection( UInt32& first, UInt32& last ) const;

    CString GetSelectedText( void ) const;

    void SelectAll( void );

    void MoveCaretTo( UInt32 index, bool extend );

    void MoveCaret( Int32 delta, bool extend );

    void MoveToStart( bool extend );

    void MoveToEnd( bool extend );

    /*
     *  Changing the text; each gives true when the text changed
     */

    /** Replaces the selection (else inserts at the caret) with the text, without the characters the kind does not take */
    bool InsertText( const CString& text );

    bool Backspace( void );

    bool DeleteForward( void );

    /** Removes the selected text; false when nothing is selected */
    bool DeleteSelection( void );

    /** Replaces the whole text and puts the caret at the end */
    bool SetText( const CString& text );

    /** More characters a number editor takes, besides the digits, signs, decimal point, grouping and exponent: the prefix and suffix of the column */
    void SetExtraCharacters( const CString& characters );

    /*
     *  A list of choices
     */

    void SetChoices( const GUCEF::vector< CString >& choices );

    const GUCEF::vector< CString >& GetChoices( void ) const;

    /** The index of the choice that is the text, -1 when none is */
    Int32 GetChoiceIndex( void ) const;

    /** Makes a choice the text; false when the index is no choice */
    bool SetChoiceIndex( Int32 index );

    /** Moves to the next (or previous) choice, staying at the ends */
    bool MoveChoice( Int32 step );

    /** The first choice (after the current one) that starts with the typed text, case ignored, becomes the text */
    bool TypeAheadChoice( const CString& typed );

    /*
     *  The model's opinion
     */

    /** Marks the value as refused, with the reason; an empty message clears the mark */
    void SetError( const CString& message );

    bool HasError( void ) const;

    const CString& GetError( void ) const;

    private:

    bool AcceptsCharacter( UInt32 codePoint ) const;
    void TextChanged( void );
    bool RemoveRange( UInt32 first, UInt32 last );

    EKind m_kind;
    Int32 m_row;
    Int32 m_column;
    TCodePointVector m_codePoints;
    mutable CString m_text;
    mutable bool m_textValid;
    CString m_original;
    UInt32 m_caret;
    UInt32 m_anchor;                    /**< the other end of the selection; equal to the caret when nothing is selected */
    CString m_extraCharacters;
    GUCEF::vector< CString > m_choices;
    CString m_error;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCELLEDITOR_H ? */
