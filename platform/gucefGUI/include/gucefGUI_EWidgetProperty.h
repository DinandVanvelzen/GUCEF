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

#ifndef GUCEF_GUI_EWIDGETPROPERTY_H
#define GUCEF_GUI_EWIDGETPROPERTY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_ETYPES_H
#include "gucefGUI_ETypes.h"
#define GUCEF_GUI_ETYPES_H
#endif /* GUCEF_GUI_ETYPES_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The standard properties of the widgets gucefGUI comes with: every property the stock widget types
 *  describe (CWidgetTypeRegistry) and the painters, the layout and the input read. A widget keeps the
 *  values of these in a table indexed by the id, the theme and the type descriptors are indexed the same
 *  way, so reading a property never compares text. The name of a property (the layout files, the
 *  theme, the editor property grid, the drivers) stays text: WidgetPropertyToString() and
 *  StringToWidgetProperty() convert, once, where text comes in or goes out.
 *
 *  A property that is not in this list (a hint for one backend, a property a newer layout carries, a
 *  property of a widget type an application registers) has the id EWidgetProperty_Unknown and is
 *  reached by name: the string overloads of CWidget keep working for every property and take the
 *  shortcut by themselves when the name is a standard one.
 *
 *  The ids are in the alphabetical order of the names (a byte by byte comparison, so upper case before
 *  lower case), and so are the changes a driver gets in a frame. The ids are not stored anywhere: they
 *  change when a property is added, text is what files hold.
 */
enum EWidgetProperty
{
    EWidgetProperty_Unknown = 0,
    EWidgetProperty_ActiveTab,
    EWidgetProperty_ActiveTabColour,
    EWidgetProperty_AnchorRow,
    EWidgetProperty_AutoHeight,
    EWidgetProperty_AutoPlay,
    EWidgetProperty_BackgroundColour,
    EWidgetProperty_BackgroundImage,
    EWidgetProperty_BarColour,
    EWidgetProperty_BarImage,
    EWidgetProperty_BorderColour,
    EWidgetProperty_BorderWidth,
    EWidgetProperty_BottomPadding,
    EWidgetProperty_Caption,
    EWidgetProperty_CaretVisible,
    EWidgetProperty_Checked,
    EWidgetProperty_CheckedImage,
    EWidgetProperty_ClickSound,
    EWidgetProperty_Clip,
    EWidgetProperty_Collapsed,
    EWidgetProperty_Collapsible,
    EWidgetProperty_ColumnWidths,
    EWidgetProperty_Columns,
    EWidgetProperty_Decimals,
    EWidgetProperty_DisabledImage,
    EWidgetProperty_DisabledItems,
    EWidgetProperty_DisabledTextColour,
    EWidgetProperty_DropDownRows,
    EWidgetProperty_Enabled,
    EWidgetProperty_EnabledBy,
    EWidgetProperty_Expanded,
    EWidgetProperty_ExpandedItems,
    EWidgetProperty_FitColumns,
    EWidgetProperty_Focusable,
    EWidgetProperty_FoldButtonHoverColour,
    EWidgetProperty_FoldButtonHovered,
    EWidgetProperty_FoldButtonPressed,
    EWidgetProperty_FoldButtonPressedColour,
    EWidgetProperty_Font,
    EWidgetProperty_FrameColour,
    EWidgetProperty_HAlign,
    EWidgetProperty_HTextAlign,
    EWidgetProperty_HeaderColour,
    EWidgetProperty_HoverColour,
    EWidgetProperty_HoverImage,
    EWidgetProperty_HoverIndex,
    EWidgetProperty_Image,
    EWidgetProperty_Indent,
    EWidgetProperty_InputTransparent,
    EWidgetProperty_Items,
    EWidgetProperty_Loop,
    EWidgetProperty_Max,
    EWidgetProperty_MaxColumnWidth,
    EWidgetProperty_MaxLength,
    EWidgetProperty_MaxSize,
    EWidgetProperty_Min,
    EWidgetProperty_MinSize,
    EWidgetProperty_Movable,
    EWidgetProperty_MultiSelect,
    EWidgetProperty_NavDown,
    EWidgetProperty_NavLeft,
    EWidgetProperty_NavRight,
    EWidgetProperty_NavUp,
    EWidgetProperty_NormalImage,
    EWidgetProperty_Opacity,
    EWidgetProperty_OpenMenu,
    EWidgetProperty_Orientation,
    EWidgetProperty_Padding,
    EWidgetProperty_PageSize,
    EWidgetProperty_Playing,
    EWidgetProperty_Position,
    EWidgetProperty_PositionInMs,
    EWidgetProperty_PushedImage,
    EWidgetProperty_ReadOnly,
    EWidgetProperty_Revision,
    EWidgetProperty_RowHeight,
    EWidgetProperty_Rows,
    EWidgetProperty_ScaleMode,
    EWidgetProperty_ScrollBarWidth,
    EWidgetProperty_ScrollPosition,
    EWidgetProperty_ScrollStep,
    EWidgetProperty_SelectAllOnFocus,
    EWidgetProperty_Selected,
    EWidgetProperty_SelectedIndex,
    EWidgetProperty_SelectedIndices,
    EWidgetProperty_SelectedItem,
    EWidgetProperty_SelectedRow,
    EWidgetProperty_SelectedRows,
    EWidgetProperty_SelectionColour,
    EWidgetProperty_Size,
    EWidgetProperty_SkipOnInput,
    EWidgetProperty_Smooth,
    EWidgetProperty_SortAscending,
    EWidgetProperty_SortColumn,
    EWidgetProperty_Sortable,
    EWidgetProperty_Source,
    EWidgetProperty_Spacing,
    EWidgetProperty_Step,
    EWidgetProperty_StyleClass,
    EWidgetProperty_SubRect,
    EWidgetProperty_TabColour,
    EWidgetProperty_TabHeight,
    EWidgetProperty_TabIndex,
    EWidgetProperty_Tabs,
    EWidgetProperty_Text,
    EWidgetProperty_TextColour,
    EWidgetProperty_ThumbColour,
    EWidgetProperty_ThumbHoverColour,
    EWidgetProperty_ThumbHovered,
    EWidgetProperty_ThumbImage,
    EWidgetProperty_ThumbPressedColour,
    EWidgetProperty_ThumbSize,
    EWidgetProperty_Tint,
    EWidgetProperty_Title,
    EWidgetProperty_TitleBarColour,
    EWidgetProperty_TitleColour,
    EWidgetProperty_TitleHeight,
    EWidgetProperty_TitleVisible,
    EWidgetProperty_Tooltip,
    EWidgetProperty_TrackColour,
    EWidgetProperty_TrackEdgeColour,
    EWidgetProperty_TrackImage,
    EWidgetProperty_VAlign,
    EWidgetProperty_VTextAlign,
    EWidgetProperty_Value,
    EWidgetProperty_Visible,
    EWidgetProperty_Volume,
    EWidgetProperty_WordWrap,

    EWidgetProperty_Count               /**< the number of ids, not a property */
};
typedef enum EWidgetProperty TWidgetProperty;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The name of a standard property, as the layout files, the theme and the drivers know it.
 *  EWidgetProperty_Unknown and a value outside the enumeration give an empty string. The string lives as long as the module.
 */
GUCEF_GUI_PUBLIC_CPP const CString& WidgetPropertyToString( EWidgetProperty property );

/**
 *  The standard property with the name: the comparison is case sensitive, as the property names are everywhere.
 *  Names that are not standard give EWidgetProperty_Unknown; such a property is reached by its name.
 *  Does not allocate and does not compare more than a handful of names.
 */
GUCEF_GUI_PUBLIC_CPP EWidgetProperty StringToWidgetProperty( const CString& name );

GUCEF_GUI_PUBLIC_CPP EWidgetProperty StringToWidgetProperty( const char* name );

/** True for every id of the enumeration except EWidgetProperty_Unknown */
GUCEF_GUI_PUBLIC_CPP bool IsStandardWidgetProperty( EWidgetProperty property );

/**
 *  The state a theme selects on (Visible, Enabled, Checked): they are never taken from the theme
 */
GUCEF_GUI_PUBLIC_CPP bool IsWidgetStateProperty( EWidgetProperty property );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_EWIDGETPROPERTY_H ? */
