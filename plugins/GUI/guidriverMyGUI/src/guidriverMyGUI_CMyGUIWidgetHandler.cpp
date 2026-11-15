/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#include "MyGUI.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#include "guidriverMyGUI_CMyGUIContext.h"
#define GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#endif /* GUCEF_guidriverMyGUI_CMYGUICONTEXT_H ? */

#include "guidriverMyGUI_CMyGUIWidgetHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CMyGUIContext*
ContextOf( const GUI::CWidget& widget )
{
    return static_cast< CMyGUIContext* >( widget.GetContext() );
}

/*-------------------------------------------------------------------------*/

MyGUI::Colour
ToMyGUIColour( const GUI::CColour& colour )
{
    return MyGUI::Colour( (float) colour.r / 255.0f, (float) colour.g / 255.0f, (float) colour.b / 255.0f, (float) colour.a / 255.0f );
}

/*-------------------------------------------------------------------------*/

/** The text of a property: the "\n" of the property value breaks a line */
MyGUI::UString
ToCaption( const CString& text )
{
    std::string converted;
    const char* p = text.C_String();
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        if ( '\\' == p[ i ] && i+1 < text.Length() && 'n' == p[ i+1 ] )
        {
            converted += '\n';
            ++i;
        }
        else
            converted += p[ i ];
    }
    return MyGUI::UString( converted );
}

/*-------------------------------------------------------------------------*/

MyGUI::Align
ToAlign( const CString& horizontal, const CString& vertical )
{
    MyGUI::Align align = MyGUI::Align::Left;
    if ( "Center" == horizontal )
        align = MyGUI::Align::HCenter;
    else
    if ( "Right" == horizontal )
        align = MyGUI::Align::Right;

    if ( "Center" == vertical )
        align |= MyGUI::Align::VCenter;
    else
    if ( "Bottom" == vertical )
        align |= MyGUI::Align::Bottom;
    else
        align |= MyGUI::Align::Top;
    return align;
}

/*-------------------------------------------------------------------------*/

void
FillItems( MyGUI::ListBox* list, const GUI::CWidget::TStringVector& items )
{
    list->removeAllItems();
    for ( UInt32 i=0; i<(UInt32) items.size(); ++i )
        list->addItem( MyGUI::UString( std::string( items[ i ].C_String() ) ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CMyGUIWidgetHandler::GetKindOfType( const CString& widgetType, EKind& kind )
{GUCEF_TRACE;

    if ( "Widget" == widgetType )           { kind = KIND_WIDGET; return true; }
    if ( "Label" == widgetType )            { kind = KIND_LABEL; return true; }
    if ( "Button" == widgetType )           { kind = KIND_BUTTON; return true; }
    if ( "Checkbox" == widgetType )         { kind = KIND_CHECKBOX; return true; }
    if ( "Editbox" == widgetType )          { kind = KIND_EDITBOX; return true; }
    if ( "Listbox" == widgetType )          { kind = KIND_LISTBOX; return true; }
    if ( "Combobox" == widgetType )         { kind = KIND_COMBOBOX; return true; }
    if ( "ProgressBar" == widgetType )      { kind = KIND_PROGRESSBAR; return true; }
    if ( "Slider" == widgetType )           { kind = KIND_SLIDER; return true; }
    if ( "ImageFrame" == widgetType )       { kind = KIND_IMAGEFRAME; return true; }
    if ( "Window" == widgetType )           { kind = KIND_WINDOW; return true; }
    return false;
}

/*-------------------------------------------------------------------------*/

CMyGUIWidgetHandler::CMyGUIWidgetHandler( EKind kind )
    : GUI::CWidgetHandler()
    , m_kind( kind )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMyGUIWidgetHandler::~CMyGUIWidgetHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMyGUIWidgetHandler::EKind
CMyGUIWidgetHandler::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIWidgetHandler::CreateNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    CMyGUIContext* context = ContextOf( widget );
    if ( GUCEF_NULL == context || !context->IsGuiStarted() )
        return false;

    const char* type = "Widget";
    const char* skin = "PanelEmpty";
    switch ( m_kind )
    {
        case KIND_LABEL:        { type = "TextBox";     skin = "TextBox"; break; }
        case KIND_BUTTON:       { type = "Button";      skin = "Button"; break; }
        case KIND_CHECKBOX:     { type = "Button";      skin = "CheckBox"; break; }
        case KIND_EDITBOX:      { type = "EditBox";     skin = "EditBox"; break; }
        case KIND_LISTBOX:      { type = "ListBox";     skin = "ListBox"; break; }
        case KIND_COMBOBOX:     { type = "ComboBox";    skin = "ComboBox"; break; }
        case KIND_PROGRESSBAR:  { type = "ProgressBar"; skin = "ProgressBar"; break; }
        case KIND_SLIDER:       { type = "ScrollBar";   skin = "SliderH"; break; }
        case KIND_IMAGEFRAME:   { type = "ImageBox";    skin = "ImageBox"; break; }
        case KIND_WINDOW:       { type = "Window";      skin = "Window"; break; }
        default:                break;
    }

    MyGUI::Widget* native = context->CreateNativeWidget( type, skin, std::string() );
    if ( GUCEF_NULL == native )
        return false;
    // the keyboard focus only shows the caret of an edit box
    if ( KIND_EDITBOX == m_kind || KIND_COMBOBOX == m_kind )
        native->setNeedKeyFocus( true );
    CMyGUIContext::SetNativeWidget( widget, native );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIWidgetHandler::DestroyNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    CMyGUIContext* context = ContextOf( widget );
    MyGUI::Widget* native = CMyGUIContext::GetNativeWidget( widget );
    if ( GUCEF_NULL != context && GUCEF_NULL != native )
        context->DestroyNativeWidget( native );
    CMyGUIContext::SetNativeWidget( widget, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIWidgetHandler::ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect )
{GUCEF_TRACE;

    CMyGUIContext* context = ContextOf( widget );
    if ( GUCEF_NULL != context )
        context->PlaceNativeWidget( CMyGUIContext::GetNativeWidget( widget ), rect );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIWidgetHandler::ApplyChildOrder( GUI::CWidget& widget )
{GUCEF_TRACE;

    CMyGUIContext* context = ContextOf( widget );
    if ( GUCEF_NULL != context )
        context->InvalidateStacking();
}

/*-------------------------------------------------------------------------*/

void
CMyGUIWidgetHandler::ApplyState( GUI::CWidget& widget )
{GUCEF_TRACE;

    MyGUI::Widget* native = CMyGUIContext::GetNativeWidget( widget );
    if ( GUCEF_NULL == native )
        return;

    native->setEnabled( widget.IsEffectivelyEnabled() );

    MyGUI::Button* button = native->castType< MyGUI::Button >( false );
    if ( GUCEF_NULL != button )
    {
        // the states of a button come from MyGUI's mouse and key focus: tell it what gucefGUI knows
        button->_setMouseFocus( widget.IsHovered() );
        button->_setKeyFocus( widget.HasFocus() );
        if ( widget.IsPressed() && widget.IsEffectivelyEnabled() )
            button->_setWidgetState( "pushed" );
    }

    if ( KIND_EDITBOX == m_kind || KIND_COMBOBOX == m_kind )
    {
        MyGUI::InputManager* input = MyGUI::InputManager::getInstancePtr();
        if ( GUCEF_NULL != input )
        {
            if ( widget.HasFocus() )
                input->setKeyFocusWidget( native );
            else
            if ( input->getKeyFocusWidget() == native )
                input->resetKeyFocusWidget();
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIWidgetHandler::ApplyProperty( GUI::CWidget& widget, const CString& name, const CString& value )
{GUCEF_TRACE;

    CMyGUIContext* context = ContextOf( widget );
    MyGUI::Widget* native = CMyGUIContext::GetNativeWidget( widget );
    if ( GUCEF_NULL == context || GUCEF_NULL == native )
        return false;

    if ( "Visible" == name )
    {
        // the context decides what is shown, with the roots and the parents taken into account
        return true;
    }
    if ( "Enabled" == name )
    {
        native->setEnabled( widget.IsEffectivelyEnabled() );
        return true;
    }
    if ( "Opacity" == name )
    {
        native->setAlpha( widget.GetFloatProperty( GUI::EWidgetProperty_Opacity ) );
        return true;
    }
    if ( "BackgroundColour" == name )
    {
        GUI::CColour colour;
        if ( KIND_WIDGET == m_kind && widget.TryGetColourProperty( GUI::EWidgetProperty_BackgroundColour, colour ) && colour.a > 0 )
        {
            native->changeWidgetSkin( "WhiteSkin" );
            native->setColour( ToMyGUIColour( colour ) );
            native->setAlpha( (float) colour.a / 255.0f );
        }
        else
        if ( KIND_WIDGET == m_kind )
            native->changeWidgetSkin( "PanelEmpty" );
        return true;
    }

    if ( "Text" == name )
    {
        MyGUI::TextBox* text = native->castType< MyGUI::TextBox >( false );
        if ( GUCEF_NULL == text )
            return false;
        MyGUI::EditBox* edit = native->castType< MyGUI::EditBox >( false );
        if ( GUCEF_NULL != edit )
            edit->setOnlyText( ToCaption( value ) );
        else
            text->setCaption( ToCaption( value ) );
        return true;
    }
    if ( "TextColour" == name )
    {
        MyGUI::TextBox* text = native->castType< MyGUI::TextBox >( false );
        if ( GUCEF_NULL == text )
            return false;
        text->setTextColour( ToMyGUIColour( widget.GetColourProperty( GUI::EWidgetProperty_TextColour ) ) );
        return true;
    }
    if ( "Font" == name )
    {
        MyGUI::TextBox* text = native->castType< MyGUI::TextBox >( false );
        if ( GUCEF_NULL == text )
            return false;
        text->setFontName( context->GetMyGUIFontName( value ) );
        text->setFontHeight( context->GetMyGUIFontHeight( value ) );
        return true;
    }
    if ( "HTextAlign" == name || "VTextAlign" == name )
    {
        MyGUI::TextBox* text = native->castType< MyGUI::TextBox >( false );
        if ( GUCEF_NULL == text )
            return false;
        text->setTextAlign( ToAlign( widget.GetProperty( GUI::EWidgetProperty_HTextAlign ), widget.GetProperty( GUI::EWidgetProperty_VTextAlign ) ) );
        return true;
    }

    switch ( m_kind )
    {
        case KIND_CHECKBOX:
        {
            MyGUI::Button* button = native->castType< MyGUI::Button >( false );
            if ( "Checked" == name && GUCEF_NULL != button )
            {
                button->setStateSelected( widget.GetBoolProperty( GUI::EWidgetProperty_Checked ) );
                return true;
            }
            break;
        }
        case KIND_EDITBOX:
        {
            MyGUI::EditBox* edit = native->castType< MyGUI::EditBox >( false );
            if ( GUCEF_NULL == edit )
                break;
            if ( "ReadOnly" == name )
            {
                edit->setEditReadOnly( widget.GetBoolProperty( GUI::EWidgetProperty_ReadOnly ) );
                return true;
            }
            if ( "MaxLength" == name )
            {
                Int32 max = widget.GetIntProperty( GUI::EWidgetProperty_MaxLength );
                edit->setMaxTextLength( max > 0 ? (size_t) max : (size_t) 2048 );
                return true;
            }
            break;
        }
        case KIND_LISTBOX:
        {
            MyGUI::ListBox* list = native->castType< MyGUI::ListBox >( false );
            if ( GUCEF_NULL == list )
                break;
            if ( "Items" == name )
            {
                FillItems( list, widget.GetStringListProperty( GUI::EWidgetProperty_Items ) );
                Int32 selected = widget.GetIntProperty( GUI::EWidgetProperty_SelectedIndex );
                if ( selected >= 0 && selected < (Int32) list->getItemCount() )
                    list->setIndexSelected( (size_t) selected );
                return true;
            }
            if ( "SelectedIndex" == name )
            {
                Int32 selected = widget.GetIntProperty( GUI::EWidgetProperty_SelectedIndex );
                if ( selected >= 0 && selected < (Int32) list->getItemCount() )
                    list->setIndexSelected( (size_t) selected );
                else
                    list->clearIndexSelected();
                return true;
            }
            break;
        }
        case KIND_COMBOBOX:
        {
            MyGUI::ComboBox* combo = native->castType< MyGUI::ComboBox >( false );
            if ( GUCEF_NULL == combo )
                break;
            if ( "Items" == name )
            {
                combo->removeAllItems();
                GUI::CWidget::TStringVector items = widget.GetStringListProperty( GUI::EWidgetProperty_Items );
                for ( UInt32 i=0; i<(UInt32) items.size(); ++i )
                    combo->addItem( MyGUI::UString( std::string( items[ i ].C_String() ) ) );
                Int32 selected = widget.GetIntProperty( GUI::EWidgetProperty_SelectedIndex );
                if ( selected >= 0 && selected < (Int32) combo->getItemCount() )
                    combo->setIndexSelected( (size_t) selected );
                return true;
            }
            if ( "SelectedIndex" == name )
            {
                Int32 selected = widget.GetIntProperty( GUI::EWidgetProperty_SelectedIndex );
                if ( selected >= 0 && selected < (Int32) combo->getItemCount() )
                    combo->setIndexSelected( (size_t) selected );
                else
                    combo->clearIndexSelected();
                return true;
            }
            break;
        }
        case KIND_PROGRESSBAR:
        case KIND_SLIDER:
        {
            if ( "Value" == name || "Min" == name || "Max" == name )
            {
                const Float32 minimum = widget.GetFloatProperty( GUI::EWidgetProperty_Min );
                const Float32 maximum = widget.GetFloatProperty( GUI::EWidgetProperty_Max );
                Float32 value = widget.GetFloatProperty( GUI::EWidgetProperty_Value );
                if ( value < minimum ) value = minimum;
                if ( value > maximum ) value = maximum;
                const Float32 range = maximum > minimum ? maximum - minimum : 1.0f;
                // a thousand steps of resolution are plenty for what a widget this size can show
                const size_t steps = 1000;
                const size_t position = (size_t) ( ( value - minimum ) / range * (Float32) steps + 0.5f );
                MyGUI::ProgressBar* progress = native->castType< MyGUI::ProgressBar >( false );
                if ( GUCEF_NULL != progress )
                {
                    progress->setProgressRange( steps );
                    progress->setProgressPosition( position );
                    return true;
                }
                MyGUI::ScrollBar* scroll = native->castType< MyGUI::ScrollBar >( false );
                if ( GUCEF_NULL != scroll )
                {
                    scroll->setScrollRange( steps + 1 );
                    scroll->setScrollPosition( position );
                    return true;
                }
            }
            break;
        }
        case KIND_IMAGEFRAME:
        {
            MyGUI::ImageBox* image = native->castType< MyGUI::ImageBox >( false );
            if ( GUCEF_NULL == image )
                break;
            if ( "Image" == name || "SubRect" == name )
            {
                std::string textureName;
                Int32 width = 0, height = 0;
                if ( context->PrepareTexture( widget.GetProperty( GUI::EWidgetProperty_Image ), textureName, width, height ) )
                {
                    image->setImageTexture( textureName );
                    GUI::CRect source;
                    if ( widget.IsPropertySet( GUI::EWidgetProperty_SubRect ) && GUI::CRect::FromString( widget.GetProperty( GUI::EWidgetProperty_SubRect ), source ) && source.width > 0 && source.height > 0 )
                        image->setImageCoord( MyGUI::IntCoord( source.left, source.top, source.width, source.height ) );
                    else
                        image->setImageCoord( MyGUI::IntCoord( 0, 0, width, height ) );
                }
                else
                    image->setImageTexture( std::string() );
                return true;
            }
            break;
        }
        case KIND_WINDOW:
        {
            MyGUI::Window* window = native->castType< MyGUI::Window >( false );
            if ( "Title" == name && GUCEF_NULL != window )
            {
                window->setCaption( ToCaption( value ) );
                return true;
            }
            break;
        }
        default:
            break;
    }
    return GUI::CWidgetHandler::ApplyProperty( widget, name, value );
}

/*-------------------------------------------------------------------------*/

void*
CMyGUIWidgetHandler::GetNativeHandle( const GUI::CWidget& widget ) const
{GUCEF_TRACE;

    return CMyGUIContext::GetNativeWidget( widget );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
