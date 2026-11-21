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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#include "gucefGUI_CViewportForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CViewportForm::ToggleChangedEvent = "GUCEF::GUI::CViewportForm::ToggleChangedEvent";
const CORE::CEvent CViewportForm::SurfaceResizedEvent = "GUCEF::GUI::CViewportForm::SurfaceResizedEvent";

namespace {

/** The height of the tool bar, and of the toggles in it */
const Int32 ToolbarHeight = 34;

CString
ToggleWidgetName( UInt32 index )
{
    return "Toggle_" + CORE::UInt32ToString( index );
}

CString
ToggleLabelName( UInt32 index )
{
    return "ToggleLabel_" + CORE::UInt32ToString( index );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CViewportForm::SToggle::SToggle( void )
    : id()
    , text()
    , tooltip()
    , checked( false )
    , choices()
    , choice( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CViewportForm::SToggle::SToggle( const CString& toggleId, const CString& toggleText, const CString& toggleTooltip, bool isChecked )
    : id( toggleId )
    , text( toggleText )
    , tooltip( toggleTooltip )
    , checked( isChecked )
    , choices()
    , choice( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CViewportForm::SToggleChange::SToggleChange( void )
    : id()
    , checked( false )
    , choice( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CViewportForm::RegisterEvents( void )
{GUCEF_TRACE;

    ToggleChangedEvent.Initialize();
    SurfaceResizedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CViewportForm::CViewportForm( void )
    : CFormEx()
    , m_toggles()
    , m_controller( GUCEF_NULL )
    , m_lastWidth( 0 )
    , m_lastHeight( 0 )
    , m_sizeKnown( false )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_labelWidths()
    , m_widgetWidths()
    , m_toolbarWidth( -1 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CViewportForm::CViewportForm( CIGUIContext* context )
    : CFormEx( context )
    , m_toggles()
    , m_controller( GUCEF_NULL )
    , m_lastWidth( 0 )
    , m_lastHeight( 0 )
    , m_sizeKnown( false )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
    , m_labelWidths()
    , m_widgetWidths()
    , m_toolbarWidth( -1 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CViewportForm::~CViewportForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_controller = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "ViewportForm", "0.1,0 0.1,0", "0.8,0 0.8,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "ViewportForm", "0,0 0,0", "1,0 1,0" );

    // the tool bar is there while the form has toggles
    CORE::CDataNode* toolbar = CFormLayout::AddWidgetNode( *content, "Panel", "Toolbar", "0,0 0,0", "1,0 0," + CORE::Int32ToString( ToolbarHeight ) );
    CFormLayout::SetWidgetNodeProperty( *toolbar, EWidgetProperty_Visible, "false" );
    CORE::CDataNode* surface = CFormLayout::AddWidgetNode( *content, "DrawSurface", "Surface", "0,0 0,0", "1,0 1,0" );
    CFormLayout::SetWidgetNodeProperty( *surface, EWidgetProperty_Focusable, "true" );
}

/*-------------------------------------------------------------------------*/

bool
CViewportForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    return Show();
}

/*-------------------------------------------------------------------------*/

bool
CViewportForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* surface = GetWidget( "Surface" );
    if ( GUCEF_NULL != surface )
    {
        TEventCallback moveCallback( this, &CViewportForm::OnMouseMove );
        SubscribeTo( surface, CWidget::MouseMoveEvent, moveCallback );
        TEventCallback downCallback( this, &CViewportForm::OnMouseDown );
        SubscribeTo( surface, CWidget::MouseDownEvent, downCallback );
        TEventCallback upCallback( this, &CViewportForm::OnMouseUp );
        SubscribeTo( surface, CWidget::MouseUpEvent, upCallback );
        TEventCallback wheelCallback( this, &CViewportForm::OnMouseWheel );
        SubscribeTo( surface, CWidget::MouseWheelEvent, wheelCallback );
        TEventCallback keyDownCallback( this, &CViewportForm::OnKeyDown );
        SubscribeTo( surface, CWidget::KeyDownEvent, keyDownCallback );
        TEventCallback keyUpCallback( this, &CViewportForm::OnKeyUp );
        SubscribeTo( surface, CWidget::KeyUpEvent, keyUpCallback );
        TEventCallback lostCallback( this, &CViewportForm::OnCaptureLost );
        SubscribeTo( surface, CWidget::MouseCaptureLostEvent, lostCallback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CViewportForm::OnFrameRendered );
        SubscribeTo( context, CGUIContext::FrameRenderedEvent, callback );
    }

    // a new surface: its size is told again when it is known
    m_sizeKnown = false;
    BuildToggleWidgets();
}

/*-------------------------------------------------------------------------*/

CDrawSurface*
CViewportForm::GetSurface( void )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( "Surface" );
    return GUCEF_NULL != widget && widget->IsRegisteredAs( "DrawSurface" ) ? static_cast< CDrawSurface* >( widget ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CRect
CViewportForm::GetSurfaceRect( void )
{GUCEF_TRACE;

    CWidget* widget = GetWidget( "Surface" );
    return GUCEF_NULL != widget ? widget->GetRect() : CRect();
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::GetSurfaceSize( UInt32& width, UInt32& height )
{GUCEF_TRACE;

    const CRect rect = GetSurfaceRect();
    width = rect.width > 0 ? (UInt32) rect.width : 0;
    height = rect.height > 0 ? (UInt32) rect.height : 0;
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::CheckSurfaceSize( void )
{GUCEF_TRACE;

    UInt32 width = 0, height = 0;
    GetSurfaceSize( width, height );
    if ( 0 == width || 0 == height )
        return;
    if ( m_sizeKnown && width == m_lastWidth && height == m_lastHeight )
        return;
    m_sizeKnown = true;
    m_lastWidth = width;
    m_lastHeight = height;
    if ( GUCEF_NULL != m_controller )
        m_controller->OnSurfaceResized( width, height );
    TSurfaceSize size;
    size.width = width;
    size.height = height;
    TSurfaceSizeEventData eventData( size );
    NotifyObservers( SurfaceResizedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::SetController( CIViewportController* controller )
{GUCEF_TRACE;

    m_controller = controller;
    if ( GUCEF_NULL != m_controller && m_sizeKnown )
        m_controller->OnSurfaceResized( m_lastWidth, m_lastHeight );
}

/*-------------------------------------------------------------------------*/

CIViewportController*
CViewportForm::GetController( void ) const
{GUCEF_TRACE;

    return m_controller;
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::SetToggles( const TToggleVector& toggles )
{GUCEF_TRACE;

    m_toggles = toggles;
    for ( UInt32 i=0; i<(UInt32)m_toggles.size(); ++i )
    {
        if ( m_toggles[ i ].choice < 0 || m_toggles[ i ].choice >= (Int32) m_toggles[ i ].choices.size() )
            m_toggles[ i ].choice = m_toggles[ i ].choices.empty() ? -1 : 0;
    }
    BuildToggleWidgets();
}

/*-------------------------------------------------------------------------*/

const CViewportForm::TToggleVector&
CViewportForm::GetToggles( void ) const
{GUCEF_TRACE;

    return m_toggles;
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::BuildToggleWidgets( void )
{GUCEF_TRACE;

    CWidget* toolbar = GetWidget( "Toolbar" );
    CWidget* surface = GetWidget( "Surface" );
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == toolbar || GUCEF_NULL == surface || GUCEF_NULL == context )
        return;

    // what was there goes: the toggles of a loaded layout belong to the form that saved it
    CWidget::TWidgetVector old = toolbar->GetChildren();
    for ( UInt32 i=0; i<(UInt32)old.size(); ++i )
        context->DestroyWidget( old[ i ] );

    // the widgets get their size here; where they are is decided by LayoutToolbar(), which wraps them into rows to the width of the tool bar
    m_updatingWidgets = true;
    m_labelWidths.assign( m_toggles.size(), 0 );
    m_widgetWidths.assign( m_toggles.size(), 0 );
    for ( UInt32 i=0; i<(UInt32)m_toggles.size(); ++i )
    {
        const TToggle& toggle = m_toggles[ i ];
        if ( toggle.choices.empty() )
        {
            CWidget* box = context->CreateWidget( "Checkbox", ToggleWidgetName( i ), toolbar );
            if ( GUCEF_NULL == box )
                continue;
            Int32 textWidth = 0, textHeight = 0;
            context->MeasureText( box->GetProperty( EWidgetProperty_Font ), toggle.text, 0, textWidth, textHeight );
            m_widgetWidths[ i ] = textWidth + 40;
            box->SetProperty( EWidgetProperty_Text, toggle.text );
            box->SetProperty( EWidgetProperty_Tooltip, toggle.tooltip );
            box->SetBoolProperty( EWidgetProperty_Checked, toggle.checked );
        }
        else
        {
            CWidget* label = context->CreateWidget( "Label", ToggleLabelName( i ), toolbar );
            Int32 labelWidth = 0, textHeight = 0;
            if ( GUCEF_NULL != label )
            {
                context->MeasureText( label->GetProperty( EWidgetProperty_Font ), toggle.text, 0, labelWidth, textHeight );
                m_labelWidths[ i ] = labelWidth + 10;
                label->SetProperty( EWidgetProperty_Text, toggle.text );
            }
            CWidget* list = context->CreateWidget( "Combobox", ToggleWidgetName( i ), toolbar );
            if ( GUCEF_NULL == list )
                continue;
            Int32 widest = 0;
            CString items;
            for ( UInt32 c=0; c<(UInt32)toggle.choices.size(); ++c )
            {
                Int32 choiceWidth = 0;
                context->MeasureText( list->GetProperty( EWidgetProperty_Font ), toggle.choices[ c ], 0, choiceWidth, textHeight );
                if ( choiceWidth > widest )
                    widest = choiceWidth;
                items += ( c > 0 ? "\n" : "" ) + toggle.choices[ c ];
            }
            m_widgetWidths[ i ] = widest + 44;
            list->SetProperty( EWidgetProperty_Items, items );
            list->SetProperty( EWidgetProperty_Tooltip, toggle.tooltip );
            list->SetIntProperty( EWidgetProperty_SelectedIndex, toggle.choice );
        }
    }
    m_updatingWidgets = false;
    SubscribeToggleWidgets();

    // the tool bar is there for toggles only; the surface takes the room it leaves
    toolbar->SetBoolProperty( EWidgetProperty_Visible, !m_toggles.empty() );
    m_toolbarWidth = -1;
    LayoutToolbar();
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::LayoutToolbar( void )
{GUCEF_TRACE;

    CWidget* toolbar = GetWidget( "Toolbar" );
    CWidget* surface = GetWidget( "Surface" );
    if ( GUCEF_NULL == toolbar || GUCEF_NULL == surface )
        return;
    // the room is the width the tool bar has as it was laid out last; until there is one the toggles are in one row
    const Int32 width = toolbar->GetRect().width > 0 ? toolbar->GetRect().width : 0x7fff;
    if ( width == m_toolbarWidth )
        return;
    m_toolbarWidth = width;

    // a toggle (its label and its check box or combo box) goes in the row it fits; the one that does not goes to the next row, and a
    // combo box wider than a row is cut to the row, so that nothing is drawn beyond the tool bar or over what is next to it
    const Int32 margin = 6;
    Int32 x = margin;
    Int32 row = 0;
    for ( UInt32 i=0; i<(UInt32)m_toggles.size() && i<(UInt32)m_widgetWidths.size(); ++i )
    {
        CWidget* widget = GetWidget( ToggleWidgetName( i ) );
        if ( GUCEF_NULL == widget )
            continue;
        const Int32 labelWidth = m_labelWidths[ i ];
        Int32 widgetWidth = m_widgetWidths[ i ];
        if ( x > margin && x + labelWidth + widgetWidth + margin > width )
        {
            x = margin;
            ++row;
        }
        if ( x + labelWidth + widgetWidth + margin > width )
        {
            widgetWidth = width - margin - x - labelWidth - margin;
            if ( widgetWidth < 48 )
                widgetWidth = 48;
        }
        const Int32 y = row * ToolbarHeight;
        if ( labelWidth > 0 )
        {
            CWidget* label = GetWidget( ToggleLabelName( i ) );
            if ( GUCEF_NULL != label )
            {
                label->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( x ) + " 0," + CORE::Int32ToString( y + 8 ) );
                label->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( labelWidth - 6 ) + " 0,24" );
            }
        }
        widget->SetProperty( EWidgetProperty_Position, "0," + CORE::Int32ToString( x + labelWidth ) + " 0," + CORE::Int32ToString( y + 3 ) );
        widget->SetProperty( EWidgetProperty_Size, "0," + CORE::Int32ToString( widgetWidth ) + " 0,28" );
        x += labelWidth + widgetWidth + margin;
    }

    const bool haveToggles = !m_toggles.empty();
    const Int32 height = haveToggles ? ( row + 1 ) * ToolbarHeight : 0;
    toolbar->SetProperty( EWidgetProperty_Size, "1,0 0," + CORE::Int32ToString( height ) );
    surface->SetProperty( EWidgetProperty_Position, "0,0 0," + CORE::Int32ToString( height ) );
    surface->SetProperty( EWidgetProperty_Size, "1,0 1,-" + CORE::Int32ToString( height ) );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::SubscribeToggleWidgets( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_toggles.size(); ++i )
    {
        CWidget* widget = GetWidget( ToggleWidgetName( i ) );
        if ( GUCEF_NULL == widget )
            continue;
        if ( m_toggles[ i ].choices.empty() )
        {
            TEventCallback checkedCallback( this, &CViewportForm::OnToggleChecked );
            SubscribeTo( widget, CCheckbox::BoxCheckedEvent, checkedCallback );
            TEventCallback uncheckedCallback( this, &CViewportForm::OnToggleUnchecked );
            SubscribeTo( widget, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
        }
        else
        {
            TEventCallback callback( this, &CViewportForm::OnToggleValueChanged );
            SubscribeTo( widget, CWidget::ValueChangedEvent, callback );
        }
    }
}

/*-------------------------------------------------------------------------*/

Int32
CViewportForm::FindToggle( const CString& id ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_toggles.size(); ++i )
        if ( m_toggles[ i ].id == id )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

Int32
CViewportForm::FindToggleOfWidget( CORE::CNotifier* widget ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_toggles.size(); ++i )
        if ( widget == const_cast< CViewportForm* >( this )->GetWidget( ToggleWidgetName( i ) ) )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

CWidget*
CViewportForm::GetToggleWidget( const CString& id )
{GUCEF_TRACE;

    const Int32 index = FindToggle( id );
    return index >= 0 ? GetWidget( ToggleWidgetName( (UInt32) index ) ) : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::ChangeToggle( UInt32 index, bool checked, Int32 choice )
{GUCEF_TRACE;

    TToggle& toggle = m_toggles[ index ];
    toggle.checked = checked;
    toggle.choice = choice;
    TToggleChange change;
    change.id = toggle.id;
    change.checked = checked;
    change.choice = toggle.choices.empty() ? -1 : choice;
    TToggleChangedEventData eventData( change );
    NotifyObservers( ToggleChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CViewportForm::SetToggleChecked( const CString& id, bool checked )
{GUCEF_TRACE;

    const Int32 index = FindToggle( id );
    if ( index < 0 || !m_toggles[ index ].choices.empty() )
        return false;
    if ( m_toggles[ index ].checked == checked )
        return true;
    CWidget* widget = GetWidget( ToggleWidgetName( (UInt32) index ) );
    if ( GUCEF_NULL != widget )
    {
        m_updatingWidgets = true;
        widget->SetBoolProperty( EWidgetProperty_Checked, checked );
        m_updatingWidgets = false;
    }
    ChangeToggle( (UInt32) index, checked, -1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewportForm::GetToggleChecked( const CString& id ) const
{GUCEF_TRACE;

    const Int32 index = FindToggle( id );
    return index >= 0 && m_toggles[ index ].checked;
}

/*-------------------------------------------------------------------------*/

bool
CViewportForm::SetToggleChoice( const CString& id, Int32 choice )
{GUCEF_TRACE;

    const Int32 index = FindToggle( id );
    if ( index < 0 || m_toggles[ index ].choices.empty() || choice < 0 || choice >= (Int32) m_toggles[ index ].choices.size() )
        return false;
    if ( m_toggles[ index ].choice == choice )
        return true;
    CWidget* widget = GetWidget( ToggleWidgetName( (UInt32) index ) );
    if ( GUCEF_NULL != widget )
    {
        m_updatingWidgets = true;
        widget->SetIntProperty( EWidgetProperty_SelectedIndex, choice );
        m_updatingWidgets = false;
    }
    ChangeToggle( (UInt32) index, false, choice );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewportForm::SetToggleChoiceSilently( const CString& id, Int32 choice )
{GUCEF_TRACE;

    const Int32 index = FindToggle( id );
    if ( index < 0 || m_toggles[ index ].choices.empty() || choice < 0 || choice >= (Int32) m_toggles[ index ].choices.size() )
        return false;
    CWidget* widget = GetWidget( ToggleWidgetName( (UInt32) index ) );
    if ( GUCEF_NULL != widget )
    {
        m_updatingWidgets = true;
        widget->SetIntProperty( EWidgetProperty_SelectedIndex, -2 );    // not the index of the choice, so that the choice is a change of the widget
        widget->SetIntProperty( EWidgetProperty_SelectedIndex, choice );
        m_updatingWidgets = false;
    }
    m_toggles[ index ].choice = choice;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CViewportForm::SetToggleDisplayText( const CString& id, const CString& text )
{GUCEF_TRACE;

    const Int32 index = FindToggle( id );
    if ( index < 0 || m_toggles[ index ].choices.empty() )
        return false;
    CWidget* widget = GetWidget( ToggleWidgetName( (UInt32) index ) );
    if ( GUCEF_NULL != widget )
    {
        m_updatingWidgets = true;
        widget->SetIntProperty( EWidgetProperty_SelectedIndex, -1 );
        widget->SetProperty( EWidgetProperty_Text, text );
        m_updatingWidgets = false;
    }
    m_toggles[ index ].choice = -1;
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CViewportForm::GetToggleChoice( const CString& id ) const
{GUCEF_TRACE;

    const Int32 index = FindToggle( id );
    return index >= 0 ? m_toggles[ index ].choice : -1;
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnMouseMove( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( GUCEF_NULL != pointer && GUCEF_NULL != m_controller )
        m_controller->OnPointerMove( pointer->localX, pointer->localY, pointer->buttonStates, pointer->modifiers );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnMouseDown( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( GUCEF_NULL != pointer && GUCEF_NULL != m_controller )
        m_controller->OnPointerButton( pointer->localX, pointer->localY, pointer->button, true, pointer->clickCount, pointer->modifiers );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnMouseUp( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    if ( GUCEF_NULL != pointer && GUCEF_NULL != m_controller )
        m_controller->OnPointerButton( pointer->localX, pointer->localY, pointer->button, false, pointer->clickCount, pointer->modifiers );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnMouseWheel( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CWheelEventData* wheel = static_cast< CWheelEventData* >( eventData );
    if ( GUCEF_NULL != wheel && GUCEF_NULL != m_controller )
    {
        m_controller->OnWheel( wheel->localX, wheel->localY, wheel->delta, wheel->modifiers );
        wheel->handled = true;      // the wheel is the controller's, it does not scroll what the form is in
    }
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnKeyDown( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( eventData );
    if ( GUCEF_NULL != key && GUCEF_NULL != m_controller )
        m_controller->OnKey( key->keyCode, true, key->modifiers );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnKeyUp( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( eventData );
    if ( GUCEF_NULL != key && GUCEF_NULL != m_controller )
        m_controller->OnKey( key->keyCode, false, key->modifiers );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnCaptureLost( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_controller )
        m_controller->OnPointerCaptureLost();
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnToggleChecked( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const Int32 index = m_updatingWidgets ? -1 : FindToggleOfWidget( notifier );
    if ( index >= 0 && !m_toggles[ index ].checked )
        ChangeToggle( (UInt32) index, true, -1 );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnToggleUnchecked( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const Int32 index = m_updatingWidgets ? -1 : FindToggleOfWidget( notifier );
    if ( index >= 0 && m_toggles[ index ].checked )
        ChangeToggle( (UInt32) index, false, -1 );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnToggleValueChanged( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const Int32 index = m_updatingWidgets ? -1 : FindToggleOfWidget( notifier );
    if ( index < 0 )
        return;
    CCombobox* list = CWidget::As< CCombobox >( GetWidget( ToggleWidgetName( (UInt32) index ) ) );
    if ( GUCEF_NULL != list && list->GetSelectedIndex() >= 0 && list->GetSelectedIndex() != m_toggles[ index ].choice )
        ChangeToggle( (UInt32) index, false, list->GetSelectedIndex() );
}

/*-------------------------------------------------------------------------*/

void
CViewportForm::OnFrameRendered( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    LayoutToolbar();
    CheckSurfaceSize();
}

/*-------------------------------------------------------------------------*/

const CString&
CViewportForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CViewportForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
