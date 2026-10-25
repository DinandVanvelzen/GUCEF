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

#include <string.h>

#include "gucefGUI_EWidgetProperty.h"

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

/** The names in the order of the enumeration, which is the order of strcmp(): the lookup by name is a binary search */
const char* const g_propertyNames[ EWidgetProperty_Count ] =
{
    "",
    "ActiveTab",
    "ActiveTabColour",
    "AnchorRow",
    "AutoHeight",
    "AutoPlay",
    "BackgroundColour",
    "BackgroundImage",
    "BarColour",
    "BarImage",
    "BorderColour",
    "BorderWidth",
    "BottomPadding",
    "Caption",
    "CaretVisible",
    "Checked",
    "CheckedImage",
    "ClickSound",
    "Clip",
    "Collapsed",
    "Collapsible",
    "ColumnWidths",
    "Columns",
    "Decimals",
    "DisabledImage",
    "DisabledItems",
    "DisabledTextColour",
    "DropDownRows",
    "Enabled",
    "EnabledBy",
    "Expanded",
    "ExpandedItems",
    "FitColumns",
    "Focusable",
    "FoldButtonHoverColour",
    "FoldButtonHovered",
    "FoldButtonPressed",
    "FoldButtonPressedColour",
    "Font",
    "FrameColour",
    "HAlign",
    "HTextAlign",
    "HeaderColour",
    "HoverColour",
    "HoverImage",
    "HoverIndex",
    "Image",
    "Indent",
    "InputTransparent",
    "Items",
    "Loop",
    "Max",
    "MaxColumnWidth",
    "MaxLength",
    "MaxSize",
    "Min",
    "MinSize",
    "Movable",
    "MultiSelect",
    "NavDown",
    "NavLeft",
    "NavRight",
    "NavUp",
    "NormalImage",
    "Opacity",
    "OpenMenu",
    "Orientation",
    "Padding",
    "PageSize",
    "Playing",
    "Position",
    "PositionInMs",
    "PushedImage",
    "ReadOnly",
    "Revision",
    "RowHeight",
    "Rows",
    "ScaleMode",
    "ScrollBarWidth",
    "ScrollPosition",
    "ScrollStep",
    "SelectAllOnFocus",
    "Selected",
    "SelectedIndex",
    "SelectedIndices",
    "SelectedItem",
    "SelectedRow",
    "SelectedRows",
    "SelectionColour",
    "Size",
    "SkipOnInput",
    "Smooth",
    "SortAscending",
    "SortColumn",
    "Sortable",
    "Source",
    "Spacing",
    "Step",
    "StyleClass",
    "SubRect",
    "TabColour",
    "TabHeight",
    "TabIndex",
    "Tabs",
    "Text",
    "TextColour",
    "ThumbColour",
    "ThumbHoverColour",
    "ThumbHovered",
    "ThumbImage",
    "ThumbPressedColour",
    "ThumbSize",
    "Tint",
    "Title",
    "TitleBarColour",
    "TitleColour",
    "TitleHeight",
    "TitleVisible",
    "Tooltip",
    "TrackColour",
    "TrackEdgeColour",
    "TrackImage",
    "VAlign",
    "VTextAlign",
    "Value",
    "Visible",
    "Volume",
    "WordWrap",
};

/** The names as strings, made once; the enumeration order */
struct SPropertyStrings
{
    CString names[ EWidgetProperty_Count ];

    SPropertyStrings( void )
    {
        for ( UInt32 i=0; i<(UInt32)EWidgetProperty_Count; ++i )
            names[ i ] = g_propertyNames[ i ];
    }
};

/** Binary search over the sorted names, 1 .. Count-1 */
EWidgetProperty
FindPropertyByName( const char* name )
{
    if ( GUCEF_NULL == name || '\0' == name[ 0 ] )
        return EWidgetProperty_Unknown;

    Int32 low = 1;
    Int32 high = (Int32) EWidgetProperty_Count - 1;
    while ( low <= high )
    {
        Int32 middle = ( low + high ) / 2;
        const char* candidate = g_propertyNames[ middle ];

        // the first byte decides almost always, without a call
        Int32 order = (Int32)(unsigned char) name[ 0 ] - (Int32)(unsigned char) candidate[ 0 ];
        if ( 0 == order )
            order = strcmp( name, candidate );
        if ( 0 == order )
            return (EWidgetProperty) middle;
        if ( order < 0 )
            high = middle - 1;
        else
            low = middle + 1;
    }
    return EWidgetProperty_Unknown;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString&
WidgetPropertyToString( EWidgetProperty property )
{GUCEF_TRACE;

    // made on first use, which is the first widget made or the first layout read; the order of the static initializers of the modules never matters
    static const SPropertyStrings strings;
    if ( (UInt32) property >= (UInt32) EWidgetProperty_Count )
        return CString::Empty;
    return strings.names[ property ];
}

/*-------------------------------------------------------------------------*/

EWidgetProperty
StringToWidgetProperty( const CString& name )
{GUCEF_TRACE;

    return FindPropertyByName( name.C_String() );
}

/*-------------------------------------------------------------------------*/

EWidgetProperty
StringToWidgetProperty( const char* name )
{GUCEF_TRACE;

    return FindPropertyByName( name );
}

/*-------------------------------------------------------------------------*/

bool
IsStandardWidgetProperty( EWidgetProperty property )
{GUCEF_TRACE;

    return (UInt32) property > (UInt32) EWidgetProperty_Unknown && (UInt32) property < (UInt32) EWidgetProperty_Count;
}

/*-------------------------------------------------------------------------*/

bool
IsWidgetStateProperty( EWidgetProperty property )
{GUCEF_TRACE;

    return EWidgetProperty_Visible == property || EWidgetProperty_Enabled == property || EWidgetProperty_Checked == property;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
