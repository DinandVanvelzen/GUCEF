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

#include "gucefGUI_CWidgetTypeRegistry.h"
#include "gucefGUI_CWidget.h"
#include "gucefGUI_CPanel.h"
#include "gucefGUI_CWindow.h"
#include "gucefGUI_CGroupBox.h"
#include "gucefGUI_CScrollPanel.h"
#include "gucefGUI_CLabel.h"
#include "gucefGUI_CButton.h"
#include "gucefGUI_CPushButton.h"
#include "gucefGUI_CCheckbox.h"
#include "gucefGUI_CEditbox.h"
#include "gucefGUI_CTextbox.h"
#include "gucefGUI_CListBox.h"
#include "gucefGUI_CCombobox.h"
#include "gucefGUI_CSpinner.h"
#include "gucefGUI_CSlider.h"
#include "gucefGUI_CProgressBar.h"
#include "gucefGUI_CImageFrame.h"
#include "gucefGUI_CDrawSurface.h"
#include "gucefGUI_CRenderContext.h"
#include "gucefGUI_CVideoView.h"
#include "gucefGUI_CAudioPlayer.h"
#include "gucefGUI_CTabControl.h"
#include "gucefGUI_CTabContentPane.h"
#include "gucefGUI_CTreeview.h"
#include "gucefGUI_CMenuBar.h"
#include "gucefGUI_CPopupMenu.h"
#include "gucefGUI_CGridView.h"

#include "gucefGUI_StandardWidgetTypes.h"

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

typedef CPropertyDescriptor PD;

const UInt32 DEFAULTFLAGS = PD::FLAG_DEFAULT;
const UInt32 LAYOUTFLAGS = PD::FLAG_DEFAULT | PD::FLAG_AFFECTSLAYOUT;
const UInt32 RUNTIMEFLAGS = PD::FLAG_RUNTIME;

PD
Prop( EWidgetProperty property, PD::TValueType type, const char* defaultValue, UInt32 flags, const char* category, const char* description )
{
    return PD( property, type, defaultValue, flags, category, description );
}

PD
EnumProp( EWidgetProperty property, const char* defaultValue, const char* values, UInt32 flags, const char* category, const char* description )
{
    PD descriptor( property, PD::VALUETYPE_ENUM, defaultValue, flags, category, description );
    CString::StringVector parts = CString( values ).ParseElements( ',', false );
    for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
        descriptor.enumValues.push_back( parts[ i ] );
    return descriptor;
}

CWidgetTypeInfo
Type( const char* name, const char* base, const char* category, CWidgetTypeInfo::TWidgetCreator creator, const char* description )
{
    CWidgetTypeInfo info;
    info.typeName = name;
    info.baseTypeName = base;
    info.category = category;
    info.creator = creator;
    info.description = description;
    return info;
}

/** The text properties several types share */
void
AddTextProperties( CWidgetTypeInfo& info, const char* textColour )
{
    info.AddProperty( Prop( EWidgetProperty_Font, PD::VALUETYPE_FONT, "", LAYOUTFLAGS, "Text", "The font, a name registered with the context or known to the driver; empty for the driver's default" ) );
    info.AddProperty( Prop( EWidgetProperty_TextColour, PD::VALUETYPE_COLOUR, textColour, DEFAULTFLAGS, "Text", "The colour of the text" ) );
}

void
AddRowProperties( CWidgetTypeInfo& info )
{
    info.AddProperty( Prop( EWidgetProperty_RowHeight, PD::VALUETYPE_INT, "0", DEFAULTFLAGS, "Rows", "Height of a row in pixels; 0 for the line height of the font" ) );
    info.AddProperty( Prop( EWidgetProperty_ScrollPosition, PD::VALUETYPE_INT, "0", DEFAULTFLAGS, "Rows", "The first row shown" ) );
    info.AddProperty( Prop( EWidgetProperty_SelectionColour, PD::VALUETYPE_COLOUR, "60,90,160,255", DEFAULTFLAGS, "Rows", "Background of the selected row" ) );
    info.AddProperty( Prop( EWidgetProperty_HoverColour, PD::VALUETYPE_COLOUR, "", DEFAULTFLAGS, "Rows", "Outline of the row under the pointer; empty for none" ) );
    info.AddProperty( Prop( EWidgetProperty_HoverIndex, PD::VALUETYPE_INT, "-1", RUNTIMEFLAGS, "State", "The row under the pointer" ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
RegisterStandardWidgetTypes( CWidgetTypeRegistry& registry )
{GUCEF_TRACE;

    CWidgetTypeInfo widget = Type( "Widget", "", "Base", GUCEF_NULL, "What every widget has" );
    widget.AddProperty( Prop( EWidgetProperty_Visible, PD::VALUETYPE_BOOL, "true", LAYOUTFLAGS, "Common", "Whether the widget and its children are shown" ) );
    widget.AddProperty( Prop( EWidgetProperty_Enabled, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Common", "Whether the widget reacts to input" ) );
    widget.AddProperty( Prop( EWidgetProperty_EnabledBy, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Common", "The name of a check box in the same tree: the widget is enabled while it is checked, as an option that depends on another" ) );
    widget.AddProperty( Prop( EWidgetProperty_Position, PD::VALUETYPE_UVECTOR, "0,0 0,0", LAYOUTFLAGS, "Layout", "Position in the parent: per axis a part of the parent's size plus pixels" ) );
    widget.AddProperty( Prop( EWidgetProperty_Size, PD::VALUETYPE_UVECTOR, "", LAYOUTFLAGS, "Layout", "Size: per axis a part of the parent's size plus pixels; empty for the natural size" ) );
    widget.AddProperty( EnumProp( EWidgetProperty_HAlign, "Left", "Left,Center,Right", LAYOUTFLAGS, "Layout", "Which part of the widget is at the horizontal position" ) );
    widget.AddProperty( EnumProp( EWidgetProperty_VAlign, "Top", "Top,Center,Bottom", LAYOUTFLAGS, "Layout", "Which part of the widget is at the vertical position" ) );
    widget.AddProperty( Prop( EWidgetProperty_Clip, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Layout", "Children are cut off at the edge of the widget" ) );
    widget.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    widget.AddProperty( Prop( EWidgetProperty_TabIndex, PD::VALUETYPE_INT, "-1", DEFAULTFLAGS, "Input", "Place in the Tab order; -1 for after the numbered ones, in tree order" ) );
    widget.AddProperty( Prop( EWidgetProperty_NavUp, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Input", "The widget (name or path) the focus moves to on Up" ) );
    widget.AddProperty( Prop( EWidgetProperty_NavDown, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Input", "The widget the focus moves to on Down" ) );
    widget.AddProperty( Prop( EWidgetProperty_NavLeft, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Input", "The widget the focus moves to on Left" ) );
    widget.AddProperty( Prop( EWidgetProperty_NavRight, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Input", "The widget the focus moves to on Right" ) );
    widget.AddProperty( Prop( EWidgetProperty_InputTransparent, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Input", "Pointer input goes to what is below the widget" ) );
    widget.AddProperty( Prop( EWidgetProperty_Tooltip, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Common", "Text shown when the pointer rests on the widget" ) );
    widget.AddProperty( Prop( EWidgetProperty_StyleClass, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Common", "Theme classes, separated by spaces" ) );
    widget.AddProperty( Prop( EWidgetProperty_Opacity, PD::VALUETYPE_FLOAT, "1", DEFAULTFLAGS, "Common", "1 opaque, 0 invisible" ) );
    widget.AddProperty( Prop( EWidgetProperty_BackgroundColour, PD::VALUETYPE_COLOUR, "", DEFAULTFLAGS, "Look", "Fills the widget; empty for none" ) );
    registry.RegisterType( widget );

    CWidgetTypeInfo panel = Type( "Panel", "Widget", "Containers", &CPanel::Create, "A rectangle holding other widgets" );
    panel.AddProperty( Prop( EWidgetProperty_BackgroundImage, PD::VALUETYPE_IMAGE, "", LAYOUTFLAGS, "Look", "A picture filling the panel" ) );
    panel.AddProperty( Prop( EWidgetProperty_BorderColour, PD::VALUETYPE_COLOUR, "", DEFAULTFLAGS, "Look", "Colour of the border; empty for none" ) );
    panel.AddProperty( Prop( EWidgetProperty_BorderWidth, PD::VALUETYPE_INT, "1", DEFAULTFLAGS, "Look", "Width of the border in pixels" ) );
    registry.RegisterType( panel );

    registry.RegisterType( Type( "Form", "Panel", "Containers", &CPanel::Create, "The usual root of a layout" ) );

    CWidgetTypeInfo hbox = Type( "HBox", "Panel", "Containers", &CPanel::Create, "Places its children side by side, left to right" );
    hbox.AddProperty( Prop( EWidgetProperty_Padding, PD::VALUETYPE_INT, "0", LAYOUTFLAGS, "Layout", "Pixels between the edge and the children" ) );
    hbox.AddProperty( Prop( EWidgetProperty_Spacing, PD::VALUETYPE_INT, "0", LAYOUTFLAGS, "Layout", "Pixels between two children" ) );
    hbox.AddProperty( Prop( EWidgetProperty_AutoHeight, PD::VALUETYPE_BOOL, "false", LAYOUTFLAGS, "Layout", "The height is what the children need (HBox and VBox, the group box has it on), the width comes from Size" ) );
    registry.RegisterType( hbox );

    CWidgetTypeInfo vbox = hbox;
    vbox.typeName = "VBox";
    vbox.description = "Places its children below each other, top to bottom";
    registry.RegisterType( vbox );

    CWidgetTypeInfo grid = hbox;
    grid.typeName = "Grid";
    grid.description = "Places its children in rows of equal columns";
    grid.AddProperty( Prop( EWidgetProperty_Columns, PD::VALUETYPE_INT, "1", LAYOUTFLAGS, "Layout", "The number of columns" ) );
    registry.RegisterType( grid );

    CWidgetTypeInfo margin = hbox;
    margin.typeName = "Margin";
    margin.description = "Gives every child its area less the padding";
    registry.RegisterType( margin );

    CWidgetTypeInfo groupBox = Type( "GroupBox", "VBox", "Containers", &CGroupBox::Create, "A frame around related widgets, its caption in a gap of the top line; the children are placed below each other in it" );
    groupBox.AddProperty( Prop( EWidgetProperty_Caption, PD::VALUETYPE_STRING, "", LAYOUTFLAGS, "Text", "What the group is about, in a gap of the top line" ) );
    AddTextProperties( groupBox, "255,255,255,255" );
    groupBox.AddProperty( Prop( EWidgetProperty_FrameColour, PD::VALUETYPE_COLOUR, "120,120,130,255", DEFAULTFLAGS, "Look", "The colour of the frame" ) );
    groupBox.AddProperty( Prop( EWidgetProperty_AutoHeight, PD::VALUETYPE_BOOL, "true", LAYOUTFLAGS, "Layout", "The height is what the caption and the children need, the width comes from Size" ) );
    groupBox.AddProperty( Prop( EWidgetProperty_Padding, PD::VALUETYPE_INT, "10", LAYOUTFLAGS, "Layout", "Pixels between the frame and the children (the caption adds its height at the top)" ) );
    groupBox.AddProperty( Prop( EWidgetProperty_Spacing, PD::VALUETYPE_INT, "4", LAYOUTFLAGS, "Layout", "Pixels between two children" ) );
    registry.RegisterType( groupBox );

    CWidgetTypeInfo window = Type( "Window", "Panel", "Containers", &CWindow::Create, "A panel with a title bar" );
    window.AddProperty( Prop( EWidgetProperty_Title, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Window", "The text in the title bar" ) );
    window.AddProperty( Prop( EWidgetProperty_TitleVisible, PD::VALUETYPE_BOOL, "true", LAYOUTFLAGS, "Window", "Whether the title bar is shown" ) );
    window.AddProperty( Prop( EWidgetProperty_TitleHeight, PD::VALUETYPE_INT, "20", LAYOUTFLAGS, "Window", "Height of the title bar" ) );
    window.AddProperty( Prop( EWidgetProperty_TitleBarColour, PD::VALUETYPE_COLOUR, "40,60,110,255", DEFAULTFLAGS, "Window", "Colour of the title bar" ) );
    window.AddProperty( Prop( EWidgetProperty_TitleColour, PD::VALUETYPE_COLOUR, "255,255,255,255", DEFAULTFLAGS, "Window", "Colour of the title" ) );
    window.AddProperty( Prop( EWidgetProperty_Movable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Window", "Whether the user can move the window by its title bar" ) );
    window.AddProperty( Prop( EWidgetProperty_Collapsible, PD::VALUETYPE_BOOL, "true", LAYOUTFLAGS, "Window", "Whether the user can fold the window into its title bar (the fold button, a double click on the title bar)" ) );
    window.AddProperty( Prop( EWidgetProperty_Collapsed, PD::VALUETYPE_BOOL, "false", LAYOUTFLAGS, "Window", "Whether the window is folded into its title bar: its children are not shown and take no input" ) );
    window.AddProperty( Prop( EWidgetProperty_MinSize, PD::VALUETYPE_UVECTOR, "", LAYOUTFLAGS, "Window", "The smallest size the layout gives the window: per axis pixels (and a part of the parent), 0 or empty for no limit" ) );
    window.AddProperty( Prop( EWidgetProperty_MaxSize, PD::VALUETYPE_UVECTOR, "", LAYOUTFLAGS, "Window", "The largest size the layout gives the window: per axis pixels (and a part of the parent), 0 or empty for no limit" ) );
    window.AddProperty( Prop( EWidgetProperty_FoldButtonHoverColour, PD::VALUETYPE_COLOUR, "255,255,255,30", DEFAULTFLAGS, "Window", "Fill of the fold button while the pointer is on it" ) );
    window.AddProperty( Prop( EWidgetProperty_FoldButtonPressedColour, PD::VALUETYPE_COLOUR, "255,255,255,60", DEFAULTFLAGS, "Window", "Fill of the fold button while it is pressed" ) );
    window.AddProperty( Prop( EWidgetProperty_FoldButtonHovered, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "The pointer is on the fold button" ) );
    window.AddProperty( Prop( EWidgetProperty_FoldButtonPressed, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "The fold button is pressed" ) );
    registry.RegisterType( window );

    CWidgetTypeInfo scrollPanel = Type( "ScrollPanel", "Panel", "Containers", &CScrollPanel::Create, "A panel that cuts its children off at its edge and scrolls them vertically with a scroll bar" );
    scrollPanel.AddProperty( Prop( EWidgetProperty_Clip, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Layout", "Children are cut off at the edge of the widget" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_ScrollPosition, PD::VALUETYPE_INT, "0", LAYOUTFLAGS, "Scrolling", "How many pixels of the content are scrolled out at the top" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_ScrollStep, PD::VALUETYPE_INT, "48", DEFAULTFLAGS, "Scrolling", "Pixels the content moves for a notch of the mouse wheel" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_ScrollBarWidth, PD::VALUETYPE_INT, "16", LAYOUTFLAGS, "Scrolling", "Width of the scroll bar at the right edge of the column, kept free of children also when everything fits" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_MaxColumnWidth, PD::VALUETYPE_INT, "0", LAYOUTFLAGS, "Scrolling", "The widest the column of the children and the scroll bar gets, in the middle of the panel; 0 is the whole panel. The margins at the sides scroll it too" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_BottomPadding, PD::VALUETYPE_INT, "0", LAYOUTFLAGS, "Scrolling", "Pixels of room below the lowest child at the end of the content" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_TrackColour, PD::VALUETYPE_COLOUR, "18,20,26,255", DEFAULTFLAGS, "Look", "The colour of the scroll bar track" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_TrackEdgeColour, PD::VALUETYPE_COLOUR, "58,64,78,255", DEFAULTFLAGS, "Look", "The colour of the line along the left edge of the scroll bar track" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_ThumbColour, PD::VALUETYPE_COLOUR, "96,106,134,255", DEFAULTFLAGS, "Look", "The colour of the scroll bar thumb" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_ThumbHoverColour, PD::VALUETYPE_COLOUR, "138,152,188,255", DEFAULTFLAGS, "Look", "The colour of the thumb while the pointer is on it" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_ThumbPressedColour, PD::VALUETYPE_COLOUR, "176,192,232,255", DEFAULTFLAGS, "Look", "The colour of the thumb while it is held" ) );
    scrollPanel.AddProperty( Prop( EWidgetProperty_ThumbHovered, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "The pointer is on the thumb" ) );
    registry.RegisterType( scrollPanel );

    CWidgetTypeInfo label = Type( "Label", "Widget", "Common", &CLabel::Create, "Text" );
    label.AddProperty( Prop( EWidgetProperty_Text, PD::VALUETYPE_STRING, "", LAYOUTFLAGS, "Text", "The text, '\\n' breaks a line" ) );
    AddTextProperties( label, "255,255,255,255" );
    label.AddProperty( EnumProp( EWidgetProperty_HTextAlign, "Left", "Left,Center,Right", DEFAULTFLAGS, "Text", "Horizontal place of the text in the label" ) );
    label.AddProperty( EnumProp( EWidgetProperty_VTextAlign, "Top", "Top,Center,Bottom", DEFAULTFLAGS, "Text", "Vertical place of the text in the label" ) );
    label.AddProperty( Prop( EWidgetProperty_WordWrap, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Text", "Lines too wide for the label are broken between words" ) );
    registry.RegisterType( label );

    CWidgetTypeInfo button = Type( "Button", "Widget", "Common", &CButton::Create, "A button with text, pictures or both" );
    button.AddProperty( Prop( EWidgetProperty_Text, PD::VALUETYPE_STRING, "", LAYOUTFLAGS, "Text", "The text on the button" ) );
    AddTextProperties( button, "255,255,255,255" );
    button.AddProperty( Prop( EWidgetProperty_NormalImage, PD::VALUETYPE_IMAGE, "", LAYOUTFLAGS, "Look", "The picture of the button" ) );
    button.AddProperty( Prop( EWidgetProperty_HoverImage, PD::VALUETYPE_IMAGE, "", DEFAULTFLAGS, "Look", "The picture while the pointer is on the button" ) );
    button.AddProperty( Prop( EWidgetProperty_PushedImage, PD::VALUETYPE_IMAGE, "", DEFAULTFLAGS, "Look", "The picture while the button is pressed" ) );
    button.AddProperty( Prop( EWidgetProperty_DisabledImage, PD::VALUETYPE_IMAGE, "", DEFAULTFLAGS, "Look", "The picture while the button is disabled" ) );
    button.AddProperty( Prop( EWidgetProperty_ClickSound, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Common", "A sound id for the application to play when the button is activated" ) );
    button.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    registry.RegisterType( button );

    registry.RegisterType( Type( "PushButton", "Button", "Common", &CPushButton::Create, "A button (the same as Button)" ) );

    CWidgetTypeInfo checkbox = Type( "Checkbox", "Button", "Common", &CCheckbox::Create, "A button that is on or off" );
    checkbox.AddProperty( Prop( EWidgetProperty_Checked, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Common", "Whether it is on" ) );
    checkbox.AddProperty( Prop( EWidgetProperty_CheckedImage, PD::VALUETYPE_IMAGE, "", DEFAULTFLAGS, "Look", "The picture while it is on" ) );
    registry.RegisterType( checkbox );

    CWidgetTypeInfo editbox = Type( "Editbox", "Widget", "Common", &CEditbox::Create, "One line of text the user types" );
    editbox.AddProperty( Prop( EWidgetProperty_Text, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Text", "The text" ) );
    AddTextProperties( editbox, "255,255,255,255" );
    editbox.AddProperty( Prop( EWidgetProperty_MaxLength, PD::VALUETYPE_INT, "0", DEFAULTFLAGS, "Text", "Most characters the text may have; 0 for no limit" ) );
    editbox.AddProperty( Prop( EWidgetProperty_ReadOnly, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Text", "The user cannot change the text" ) );
    editbox.AddProperty( Prop( EWidgetProperty_SelectAllOnFocus, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Text", "The text is selected when the box gets the focus, so typing replaces it" ) );
    editbox.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    editbox.AddProperty( Prop( EWidgetProperty_CaretVisible, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "Whether the blinking caret is shown now" ) );
    editbox.AddProperty( Prop( EWidgetProperty_Selected, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "Whether the text is selected" ) );
    registry.RegisterType( editbox );

    CWidgetTypeInfo textbox = Type( "Textbox", "Widget", "Common", &CTextbox::Create, "Several lines of text, with scrolling" );
    textbox.AddProperty( Prop( EWidgetProperty_Text, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Text", "The text, '\\n' breaks a line" ) );
    AddTextProperties( textbox, "255,255,255,255" );
    textbox.AddProperty( Prop( EWidgetProperty_WordWrap, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Text", "Lines too wide are broken between words" ) );
    textbox.AddProperty( Prop( EWidgetProperty_RowHeight, PD::VALUETYPE_INT, "0", DEFAULTFLAGS, "Rows", "Height of a line in pixels; 0 for the line height of the font" ) );
    textbox.AddProperty( Prop( EWidgetProperty_ScrollPosition, PD::VALUETYPE_INT, "0", DEFAULTFLAGS, "Rows", "The first line shown" ) );
    registry.RegisterType( textbox );

    CWidgetTypeInfo listbox = Type( "Listbox", "Widget", "Lists", &CListbox::Create, "A list of text rows to choose from" );
    listbox.AddProperty( Prop( EWidgetProperty_Items, PD::VALUETYPE_STRINGLIST, "", DEFAULTFLAGS, "Items", "The rows" ) );
    listbox.AddProperty( Prop( EWidgetProperty_SelectedIndex, PD::VALUETYPE_INT, "-1", DEFAULTFLAGS, "Items", "The selected row, -1 for none" ) );
    listbox.AddProperty( Prop( EWidgetProperty_MultiSelect, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Items", "Whether several rows can be selected (SelectedIndices)" ) );
    listbox.AddProperty( Prop( EWidgetProperty_SelectedIndices, PD::VALUETYPE_INTLIST, "", DEFAULTFLAGS, "Items", "The selected rows when MultiSelect is on" ) );
    listbox.AddProperty( Prop( EWidgetProperty_DisabledItems, PD::VALUETYPE_INTLIST, "", DEFAULTFLAGS, "Items", "Rows that cannot be chosen" ) );
    AddTextProperties( listbox, "255,255,255,255" );
    listbox.AddProperty( Prop( EWidgetProperty_DisabledTextColour, PD::VALUETYPE_COLOUR, "128,128,128,255", DEFAULTFLAGS, "Text", "The colour of the text of a disabled row" ) );
    listbox.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    AddRowProperties( listbox );
    registry.RegisterType( listbox );

    CWidgetTypeInfo combobox = Type( "Combobox", "Widget", "Lists", &CCombobox::Create, "One choice of a list that drops down" );
    combobox.AddProperty( Prop( EWidgetProperty_Items, PD::VALUETYPE_STRINGLIST, "", DEFAULTFLAGS, "Items", "The choices" ) );
    combobox.AddProperty( Prop( EWidgetProperty_SelectedIndex, PD::VALUETYPE_INT, "-1", DEFAULTFLAGS, "Items", "The choice, -1 for none" ) );
    combobox.AddProperty( Prop( EWidgetProperty_Text, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Text", "The text shown, the chosen item" ) );
    combobox.AddProperty( Prop( EWidgetProperty_DropDownRows, PD::VALUETYPE_INT, "8", DEFAULTFLAGS, "Items", "Most rows the open list shows" ) );
    AddTextProperties( combobox, "255,255,255,255" );
    combobox.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    combobox.AddProperty( Prop( EWidgetProperty_Expanded, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "Whether the list is open" ) );
    registry.RegisterType( combobox );

    CWidgetTypeInfo spinner = Type( "Spinner", "Widget", "Common", &CSpinner::Create, "A number stepped up and down" );
    spinner.AddProperty( Prop( EWidgetProperty_Value, PD::VALUETYPE_FLOAT, "0", DEFAULTFLAGS, "Value", "The number" ) );
    spinner.AddProperty( Prop( EWidgetProperty_Min, PD::VALUETYPE_FLOAT, "0", DEFAULTFLAGS, "Value", "The lowest value" ) );
    spinner.AddProperty( Prop( EWidgetProperty_Max, PD::VALUETYPE_FLOAT, "100", DEFAULTFLAGS, "Value", "The highest value" ) );
    spinner.AddProperty( Prop( EWidgetProperty_Step, PD::VALUETYPE_FLOAT, "1", DEFAULTFLAGS, "Value", "How much one step changes the value" ) );
    spinner.AddProperty( Prop( EWidgetProperty_Decimals, PD::VALUETYPE_INT, "0", DEFAULTFLAGS, "Value", "Digits shown after the point" ) );
    AddTextProperties( spinner, "255,255,255,255" );
    spinner.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    registry.RegisterType( spinner );

    CWidgetTypeInfo slider = Type( "Slider", "Widget", "Common", &CSlider::Create, "A value picked by dragging a thumb" );
    slider.AddProperty( Prop( EWidgetProperty_Value, PD::VALUETYPE_FLOAT, "0", DEFAULTFLAGS, "Value", "The value" ) );
    slider.AddProperty( Prop( EWidgetProperty_Min, PD::VALUETYPE_FLOAT, "0", DEFAULTFLAGS, "Value", "The lowest value" ) );
    slider.AddProperty( Prop( EWidgetProperty_Max, PD::VALUETYPE_FLOAT, "100", DEFAULTFLAGS, "Value", "The highest value" ) );
    slider.AddProperty( Prop( EWidgetProperty_Step, PD::VALUETYPE_FLOAT, "1", DEFAULTFLAGS, "Value", "Values are multiples of the step from Min; 0 for any value" ) );
    slider.AddProperty( EnumProp( EWidgetProperty_Orientation, "Horizontal", "Horizontal,Vertical", DEFAULTFLAGS, "Look", "Direction of the track" ) );
    slider.AddProperty( Prop( EWidgetProperty_ThumbSize, PD::VALUETYPE_INT, "12", DEFAULTFLAGS, "Look", "Length of the thumb in pixels" ) );
    slider.AddProperty( Prop( EWidgetProperty_TrackColour, PD::VALUETYPE_COLOUR, "80,80,80,255", DEFAULTFLAGS, "Look", "Colour of the track" ) );
    slider.AddProperty( Prop( EWidgetProperty_ThumbColour, PD::VALUETYPE_COLOUR, "200,200,200,255", DEFAULTFLAGS, "Look", "Colour of the thumb" ) );
    slider.AddProperty( Prop( EWidgetProperty_TrackImage, PD::VALUETYPE_IMAGE, "", DEFAULTFLAGS, "Look", "Picture of the track" ) );
    slider.AddProperty( Prop( EWidgetProperty_ThumbImage, PD::VALUETYPE_IMAGE, "", DEFAULTFLAGS, "Look", "Picture of the thumb" ) );
    slider.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    registry.RegisterType( slider );

    CWidgetTypeInfo scrollbar = Type( "Scrollbar", "Slider", "Common", &CSlider::CreateScrollbar, "A slider whose thumb shows the visible part of a range" );
    scrollbar.AddProperty( Prop( EWidgetProperty_PageSize, PD::VALUETYPE_FLOAT, "10", DEFAULTFLAGS, "Value", "How much of the range is visible; sets the length of the thumb" ) );
    scrollbar.AddProperty( EnumProp( EWidgetProperty_Orientation, "Vertical", "Horizontal,Vertical", DEFAULTFLAGS, "Look", "Direction of the track" ) );
    registry.RegisterType( scrollbar );

    CWidgetTypeInfo progress = Type( "ProgressBar", "Widget", "Common", &CProgressBar::Create, "Shows how far something is" );
    progress.AddProperty( Prop( EWidgetProperty_Value, PD::VALUETYPE_FLOAT, "0", DEFAULTFLAGS, "Value", "How far" ) );
    progress.AddProperty( Prop( EWidgetProperty_Min, PD::VALUETYPE_FLOAT, "0", DEFAULTFLAGS, "Value", "Nothing done" ) );
    progress.AddProperty( Prop( EWidgetProperty_Max, PD::VALUETYPE_FLOAT, "100", DEFAULTFLAGS, "Value", "All done" ) );
    progress.AddProperty( Prop( EWidgetProperty_BarColour, PD::VALUETYPE_COLOUR, "60,140,60,255", DEFAULTFLAGS, "Look", "Colour of the bar" ) );
    progress.AddProperty( Prop( EWidgetProperty_BarImage, PD::VALUETYPE_IMAGE, "", DEFAULTFLAGS, "Look", "Picture of the bar" ) );
    registry.RegisterType( progress );

    CWidgetTypeInfo image = Type( "ImageFrame", "Widget", "Media", &CImageFrame::Create, "A picture, still or animated" );
    image.AddProperty( Prop( EWidgetProperty_Image, PD::VALUETYPE_IMAGE, "", LAYOUTFLAGS, "Look", "The picture: a name registered with the context or a VFS path" ) );
    image.AddProperty( Prop( EWidgetProperty_SubRect, PD::VALUETYPE_RECT, "", LAYOUTFLAGS, "Look", "The part of the picture shown, in logical pixels of the picture; empty for all of it" ) );
    image.AddProperty( EnumProp( EWidgetProperty_ScaleMode, "Stretch", "Stretch,None,Fit,Integer,Center", DEFAULTFLAGS, "Look", "How the picture fills the frame" ) );
    image.AddProperty( Prop( EWidgetProperty_Tint, PD::VALUETYPE_COLOUR, "255,255,255,255", DEFAULTFLAGS, "Look", "Multiplies the colours of the picture" ) );
    image.AddProperty( Prop( EWidgetProperty_Smooth, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Look", "Bilinear filtering when scaled; off keeps pixels sharp" ) );
    registry.RegisterType( image );

    CWidgetTypeInfo video = Type( "VideoView", "ImageFrame", "Media", &CVideoView::Create, "Plays a movie" );
    video.AddProperty( Prop( EWidgetProperty_Source, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Media", "The movie file, a VFS path" ) );
    video.AddProperty( Prop( EWidgetProperty_Loop, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Media", "Starts again at the end" ) );
    video.AddProperty( Prop( EWidgetProperty_SkipOnInput, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Media", "A click or a key ends the movie" ) );
    video.AddProperty( Prop( EWidgetProperty_AutoPlay, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Media", "Plays as soon as Source is set" ) );
    video.AddProperty( Prop( EWidgetProperty_Playing, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "Whether the movie plays" ) );
    video.AddProperty( EnumProp( EWidgetProperty_ScaleMode, "Fit", "Stretch,None,Fit,Integer,Center", DEFAULTFLAGS, "Look", "How the picture fills the frame" ) );
    registry.RegisterType( video );

    CWidgetTypeInfo audio = Type( "AudioPlayer", "Widget", "Media", &CAudioPlayer::Create, "Plays a sound file, with a play / pause control" );
    audio.AddProperty( Prop( EWidgetProperty_Source, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Media", "The sound file, a VFS path" ) );
    audio.AddProperty( Prop( EWidgetProperty_Volume, PD::VALUETYPE_FLOAT, "1", DEFAULTFLAGS, "Media", "0 silent to 1 full" ) );
    audio.AddProperty( Prop( EWidgetProperty_Loop, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Media", "Plays until stopped" ) );
    audio.AddProperty( Prop( EWidgetProperty_AutoPlay, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Media", "Plays as soon as Source is set" ) );
    AddTextProperties( audio, "255,255,255,255" );
    audio.AddProperty( Prop( EWidgetProperty_BarColour, PD::VALUETYPE_COLOUR, "60,140,60,255", DEFAULTFLAGS, "Look", "Colour of the control" ) );
    audio.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    audio.AddProperty( Prop( EWidgetProperty_Playing, PD::VALUETYPE_BOOL, "false", RUNTIMEFLAGS, "State", "Whether the sound plays" ) );
    audio.AddProperty( Prop( EWidgetProperty_PositionInMs, PD::VALUETYPE_INT, "0", RUNTIMEFLAGS, "State", "How far it has played" ) );
    registry.RegisterType( audio );

    CWidgetTypeInfo draw = Type( "DrawSurface", "Widget", "Media", &CDrawSurface::Create, "Drawn by the application with 2D commands" );
    draw.AddProperty( Prop( EWidgetProperty_Clip, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Layout", "Nothing is drawn outside the widget" ) );
    draw.AddProperty( Prop( EWidgetProperty_Revision, PD::VALUETYPE_INT, "0", RUNTIMEFLAGS, "State", "Goes up with every committed drawing" ) );
    registry.RegisterType( draw );

    registry.RegisterType( Type( "RenderContext", "DrawSurface", "Media", &CRenderContext::Create, "A draw surface under its older name" ) );

    CWidgetTypeInfo tabs = Type( "TabControl", "Widget", "Containers", &CTabControl::Create, "Pages with tabs" );
    tabs.AddProperty( Prop( EWidgetProperty_Tabs, PD::VALUETYPE_STRINGLIST, "", LAYOUTFLAGS, "Items", "The tab titles, one per page" ) );
    tabs.AddProperty( Prop( EWidgetProperty_ActiveTab, PD::VALUETYPE_INT, "0", DEFAULTFLAGS, "Items", "The page shown" ) );
    tabs.AddProperty( Prop( EWidgetProperty_TabHeight, PD::VALUETYPE_INT, "20", LAYOUTFLAGS, "Look", "Height of the tabs" ) );
    tabs.AddProperty( Prop( EWidgetProperty_TabColour, PD::VALUETYPE_COLOUR, "50,50,50,255", DEFAULTFLAGS, "Look", "Colour of a tab" ) );
    tabs.AddProperty( Prop( EWidgetProperty_ActiveTabColour, PD::VALUETYPE_COLOUR, "90,90,90,255", DEFAULTFLAGS, "Look", "Colour of the active tab" ) );
    AddTextProperties( tabs, "255,255,255,255" );
    tabs.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    registry.RegisterType( tabs );

    registry.RegisterType( Type( "TabContentPane", "Panel", "Containers", &CTabContentPane::Create, "A page of a tab control" ) );

    CWidgetTypeInfo tree = Type( "Treeview", "Widget", "Lists", &CTreeview::Create, "A tree of items" );
    tree.AddProperty( Prop( EWidgetProperty_Items, PD::VALUETYPE_STRINGLIST, "", DEFAULTFLAGS, "Items", "One path per item, levels separated by '/'" ) );
    tree.AddProperty( Prop( EWidgetProperty_ExpandedItems, PD::VALUETYPE_STRINGLIST, "", DEFAULTFLAGS, "Items", "The paths that are open" ) );
    tree.AddProperty( Prop( EWidgetProperty_SelectedItem, PD::VALUETYPE_STRING, "", DEFAULTFLAGS, "Items", "The path of the selected item" ) );
    tree.AddProperty( Prop( EWidgetProperty_Indent, PD::VALUETYPE_INT, "12", DEFAULTFLAGS, "Look", "Pixels per level" ) );
    AddTextProperties( tree, "255,255,255,255" );
    tree.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    AddRowProperties( tree );
    registry.RegisterType( tree );

    CWidgetTypeInfo menubar = Type( "MenuBar", "Widget", "Menus", &CMenuBar::Create, "A bar of menus" );
    menubar.AddProperty( Prop( EWidgetProperty_Items, PD::VALUETYPE_STRINGLIST, "", LAYOUTFLAGS, "Items", "One \"Menu/Item\" path per menu item" ) );
    AddTextProperties( menubar, "255,255,255,255" );
    menubar.AddProperty( Prop( EWidgetProperty_HoverColour, PD::VALUETYPE_COLOUR, "70,70,70,255", DEFAULTFLAGS, "Look", "Background of the menu under the pointer" ) );
    menubar.AddProperty( Prop( EWidgetProperty_OpenMenu, PD::VALUETYPE_INT, "-1", RUNTIMEFLAGS, "State", "The open menu" ) );
    menubar.AddProperty( Prop( EWidgetProperty_HoverIndex, PD::VALUETYPE_INT, "-1", RUNTIMEFLAGS, "State", "The menu under the pointer" ) );
    registry.RegisterType( menubar );

    CWidgetTypeInfo popup = Type( "PopupMenu", "Widget", "Menus", &CPopupMenu::Create, "A menu on top of everything" );
    popup.AddProperty( Prop( EWidgetProperty_Items, PD::VALUETYPE_STRINGLIST, "", LAYOUTFLAGS, "Items", "The items; \"-\" is a separator" ) );
    popup.AddProperty( Prop( EWidgetProperty_DisabledItems, PD::VALUETYPE_INTLIST, "", DEFAULTFLAGS, "Items", "Items that cannot be chosen" ) );
    AddTextProperties( popup, "255,255,255,255" );
    popup.AddProperty( Prop( EWidgetProperty_DisabledTextColour, PD::VALUETYPE_COLOUR, "128,128,128,255", DEFAULTFLAGS, "Text", "The colour of a disabled item" ) );
    popup.AddProperty( Prop( EWidgetProperty_RowHeight, PD::VALUETYPE_INT, "0", LAYOUTFLAGS, "Rows", "Height of an item; 0 for the line height of the font" ) );
    popup.AddProperty( Prop( EWidgetProperty_HoverColour, PD::VALUETYPE_COLOUR, "60,90,160,255", DEFAULTFLAGS, "Look", "Background of the item under the pointer" ) );
    popup.AddProperty( Prop( EWidgetProperty_BackgroundColour, PD::VALUETYPE_COLOUR, "30,30,30,255", DEFAULTFLAGS, "Look", "Fills the menu" ) );
    popup.AddProperty( Prop( EWidgetProperty_HoverIndex, PD::VALUETYPE_INT, "-1", RUNTIMEFLAGS, "State", "The item under the pointer" ) );
    registry.RegisterType( popup );

    CWidgetTypeInfo gridview = Type( "GridView", "Widget", "Lists", &CGridView::Create, "A table of text cells" );
    gridview.AddProperty( Prop( EWidgetProperty_Columns, PD::VALUETYPE_STRINGLIST, "", DEFAULTFLAGS, "Items", "The column titles" ) );
    gridview.AddProperty( Prop( EWidgetProperty_ColumnWidths, PD::VALUETYPE_INTLIST, "", DEFAULTFLAGS, "Items", "Widths of the columns in pixels; empty for equal widths" ) );
    gridview.AddProperty( Prop( EWidgetProperty_FitColumns, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Items", "The widths of the columns are relative to each other: together they are as wide as the grid" ) );
    gridview.AddProperty( Prop( EWidgetProperty_Rows, PD::VALUETYPE_STRINGLIST, "", DEFAULTFLAGS, "Items", "One entry per row, the cells separated by a tab" ) );
    gridview.AddProperty( Prop( EWidgetProperty_SelectedRow, PD::VALUETYPE_INT, "-1", DEFAULTFLAGS, "Items", "The selected row (the cursor of a multiple selection), -1 for none" ) );
    gridview.AddProperty( Prop( EWidgetProperty_MultiSelect, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Items", "Whether several rows can be selected (Ctrl and Shift with clicks and keys)" ) );
    gridview.AddProperty( Prop( EWidgetProperty_SelectedRows, PD::VALUETYPE_INTLIST, "", DEFAULTFLAGS, "Items", "The selected rows when MultiSelect is on" ) );
    gridview.AddProperty( Prop( EWidgetProperty_AnchorRow, PD::VALUETYPE_INT, "-1", RUNTIMEFLAGS, "State", "Where a Shift selection starts" ) );
    gridview.AddProperty( Prop( EWidgetProperty_Sortable, PD::VALUETYPE_BOOL, "false", DEFAULTFLAGS, "Items", "Whether a click on a column title sorts the rows by that column" ) );
    gridview.AddProperty( Prop( EWidgetProperty_SortColumn, PD::VALUETYPE_INT, "-1", DEFAULTFLAGS, "Items", "The column the rows are sorted by, -1 for none (shows the arrow in the header)" ) );
    gridview.AddProperty( Prop( EWidgetProperty_SortAscending, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Items", "Whether the sort goes up" ) );
    gridview.AddProperty( Prop( EWidgetProperty_HeaderColour, PD::VALUETYPE_COLOUR, "50,50,50,255", DEFAULTFLAGS, "Look", "Background of the header row" ) );
    AddTextProperties( gridview, "255,255,255,255" );
    gridview.AddProperty( Prop( EWidgetProperty_Focusable, PD::VALUETYPE_BOOL, "true", DEFAULTFLAGS, "Input", "Whether the widget can get the keyboard focus" ) );
    AddRowProperties( gridview );
    registry.RegisterType( gridview );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
