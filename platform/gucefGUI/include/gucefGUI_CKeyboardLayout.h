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

#ifndef GUCEF_GUI_CKEYBOARDLAYOUT_H
#define GUCEF_GUI_CKEYBOARDLAYOUT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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
 *  The keys of an on screen keyboard (COnScreenKeyboardForm) as data: rows of keys, each with the key code it sends
 *  (the positional code of gucefINPUT), what it shows and what it types, with and without shift. A layout is made in
 *  code or read from a tree (Load(): any DStore codec reads one from a file), and layouts are looked up by name in the
 *  CKeyboardLayoutRegistry, which has the layout "US" built in.
 *
 *  Key codes are positional, as everywhere in gucefINPUT: a layout changes what the keys show and the text they type,
 *  not what a consumer of the key events sees. The text of a key is what a keyboard of that layout would type; it is sent
 *  as text after the key goes down, the way the window drivers send it.
 *
 *  Tree format:
 *
 *      KeyboardLayout (attribute name)
 *          Row
 *              Key (attributes: code, label, shiftLabel, text, shiftText, width, caps)
 *
 *  code is the name of the key code (INPUT::KeyCodeToString(), "ESCAPE", "A", "LSHIFT"); a Key without a code is a gap of
 *  its width. width is in key widths, 1 by default. caps is true for keys that caps lock acts on (letters).
 */
class GUCEF_GUI_PUBLIC_CPP CKeyboardLayout
{
    public:

    /** One key, or a gap in a row when the key code is KEYCODE_UNASSIGNED */
    struct SKey
    {
        INPUT::KeyCode keyCode;     /**< what the key sends; KEYCODE_UNASSIGNED for a gap */
        CString label;              /**< what the key shows */
        CString shiftLabel;         /**< what it shows while shift is active; the label when empty */
        CString text;               /**< UTF-8 text the key types, empty for a key that types none */
        CString shiftText;          /**< the text with shift; the text when empty */
        Float32 width;              /**< in key widths */
        bool capsAffected;          /**< caps lock acts as shift for this key */

        SKey( void );
    };
    typedef struct SKey TKey;
    typedef GUCEF::vector< TKey > TKeyRow;
    typedef GUCEF::vector< TKeyRow > TKeyRows;

    CKeyboardLayout( void );

    explicit CKeyboardLayout( const CString& name );

    CKeyboardLayout( const CKeyboardLayout& src );

    ~CKeyboardLayout();

    CKeyboardLayout& operator=( const CKeyboardLayout& src );

    const CString& GetName( void ) const;

    void SetName( const CString& name );

    /** Adds an empty row below the last; its index */
    UInt32 AddRow( void );

    /** Adds a key at the end of a row; false when there is no such row */
    bool AddKey( UInt32 row, const TKey& key );

    /** Adds a key that types text, with its shifted form; letters have 'capsAffected' set */
    bool AddKey( UInt32 row, INPUT::KeyCode keyCode, const CString& label, const CString& shiftLabel,
                 const CString& text, const CString& shiftText, Float32 width = 1.0f, bool capsAffected = false );

    /** Adds a key that types nothing (a modifier, an editing or a function key) */
    bool AddKey( UInt32 row, INPUT::KeyCode keyCode, const CString& label, Float32 width = 1.0f );

    /** Adds a gap */
    bool AddGap( UInt32 row, Float32 width );

    UInt32 GetRowCount( void ) const;

    const TKeyRow& GetRow( UInt32 row ) const;

    const TKeyRows& GetRows( void ) const;

    /** The sum of the widths of the keys and gaps of a row */
    Float32 GetRowWidth( UInt32 row ) const;

    /** The width of the widest row, which is what the keyboard is as wide as */
    Float32 GetWidth( void ) const;

    /** Where a key code is (the first of it); false when the layout has no such key */
    bool FindKey( INPUT::KeyCode keyCode, UInt32& row, UInt32& index ) const;

    /** The number of keys that send a key code (gaps left out) */
    UInt32 GetKeyCount( void ) const;

    /** Whether the layout has a row and every key has a key code of a key (or is a gap) and a width above 0 */
    bool IsValid( void ) const;

    /** The tree of this layout, see the format above */
    void Save( CORE::CDataNode& tree ) const;

    /** Replaces this layout with the tree; false (and this layout is left as it was) when the tree is not a layout or has an unknown key code */
    bool Load( const CORE::CDataNode& tree );

    /** The US layout of the gucefINPUT key codes: function row, number row, letters, modifiers, the arrow keys */
    static CKeyboardLayout MakeUS( void );

    private:

    CString m_name;
    TKeyRows m_rows;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CKEYBOARDLAYOUT_H ? */
