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
#ifndef GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#define GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  What the application says about one value of the tree a CPropertyInspectorForm shows: the kind of value, so that the
 *  form gives it the right editor, and what the editor needs of it. The descriptor is found by the path of the value in the
 *  tree (see CPropertyInspectorForm). A value without a descriptor is shown as read only text.
 *
 *  The kinds:
 *      Text        any text
 *      Number      a number; with a range, a number of decimals (0: a whole number) and a unit that is shown after the value
 *      Bool        a check box
 *      Choice      one of the choices, in a list
 *      Colour      "r,g,b" or "r,g,b,a" with whole channels 0 to 255, or "#rrggbb" / "#rrggbbaa"
 *      Vector      2 to 4 numbers separated by commas, "1, 2.5, 3"
 *      Path        a path of a file or a folder, with a button that opens the dialog to choose it (CPathFieldForm)
 *
 *  The class is data with named constructors: the fields can be changed after.
 */
class GUCEF_GUI_PUBLIC_CPP CPropertyRowDescriptor
{
    public:

    enum EValueType
    {
        VALUETYPE_TEXT = 0  ,
        VALUETYPE_NUMBER    ,
        VALUETYPE_BOOL      ,
        VALUETYPE_CHOICE    ,
        VALUETYPE_COLOUR    ,
        VALUETYPE_VECTOR    ,
        VALUETYPE_PATH
    };

    EValueType type;
    CString label;                  /**< the name in the first column; the last part of the path when empty */
    CString tooltip;
    CString unit;                   /**< shown after a number, for example "m" or "%" */
    bool readOnly;
    bool hasRange;                  /**< numbers (and the components of a vector): the value is within [minimum, maximum] */
    Float64 minimum;
    Float64 maximum;
    Int32 decimals;                 /**< numbers: digits after the decimal point, 0 for whole numbers, -1 for the digits the value has */
    CString::StringVector choices;  /**< choices: what the list offers */
    UInt32 componentCount;          /**< vectors: 2, 3 or 4 */
    bool pathIsDirectory;           /**< paths: a folder (else a file) */

    /** A text value */
    CPropertyRowDescriptor( void );

    static CPropertyRowDescriptor Text( void );

    static CPropertyRowDescriptor Number( Int32 numberOfDecimals = -1 );

    static CPropertyRowDescriptor NumberInRange( Float64 lowest, Float64 highest, Int32 numberOfDecimals = -1 );

    /** A true or false value. Not named Bool: Xlib defines Bool as a macro */
    static CPropertyRowDescriptor Boolean( void );

    static CPropertyRowDescriptor Choice( const CString::StringVector& options );

    static CPropertyRowDescriptor Colour( void );

    static CPropertyRowDescriptor Vector( UInt32 components );

    static CPropertyRowDescriptor Path( bool isDirectory );

    /** The same descriptor, read only */
    CPropertyRowDescriptor AsReadOnly( void ) const;

    /** The same descriptor with a tooltip */
    CPropertyRowDescriptor WithTooltip( const CString& text ) const;

    /** The same descriptor with a unit */
    CPropertyRowDescriptor WithUnit( const CString& text ) const;

    /** The same descriptor with the name the first column shows */
    CPropertyRowDescriptor WithLabel( const CString& text ) const;

    /**
     *  Whether the text is a value of this kind and within the range: false with the reason when not. The text of a number
     *  may end in the unit
     */
    bool Validate( const CString& text, CString& problem ) const;

    /** The text as the value shown by the kind: a number with its decimals, a vector with ", " between the components, a colour as "r,g,b,a" */
    CString Normalize( const CString& text ) const;

    /** The text the value column shows: the normalized text, with the unit of a number */
    CString FormatForDisplay( const CString& text ) const;

    /** A number as text: with that many digits after the decimal point, or with -1 as many as it needs (up to 12 significant digits) */
    static CString NumberToText( Float64 value, Int32 numberOfDecimals );

    /** Reads the whole text as a number; false for any other text, also an empty one */
    static bool TextToNumber( const CString& text, Float64& value );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H ? */
