/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_GAMEGUI_TESTMATCHLAYOUTCHECKS_H
#define GUCEF_GAMEGUI_TESTMATCHLAYOUTCHECKS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefGAMEGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::vector< GUCEF::GUI::CWidget* > TMfWidgetVector;

inline void
MfCollect( GUCEF::GUI::CWidget* widget, TMfWidgetVector& all )
{
    all.push_back( widget );
    const GUCEF::GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( size_t i=0; i<children.size(); ++i )
        MfCollect( children[ i ], all );
}

/** The part of a widget that is seen: cut off at every ancestor that clips */
inline GUCEF::GUI::CRect
MfVisibleRect( const GUCEF::GUI::CWidget* widget )
{
    GUCEF::GUI::CRect rect = widget->GetRect();
    for ( const GUCEF::GUI::CWidget* p=widget->GetParentWidget(); GUCEF_NULL != p; p=p->GetParentWidget() )
        if ( p->GetBoolProperty( "Clip" ) )
            rect = rect.Intersection( p->GetRect() );
    return rect;
}

inline bool
MfIsContainerType( const GUCEF::CORE::CString& type )
{
    return type == "Panel" || type == "Form" || type == "VBox" || type == "HBox" || type == "Grid" || type == "Margin" ||
           type == "GroupBox" || type == "Window" || type == "ScrollPanel";
}

inline bool
MfInside( const GUCEF::GUI::CRect& inner, const GUCEF::GUI::CRect& outer )
{
    return inner.left >= outer.left && inner.top >= outer.top && inner.Right() <= outer.Right() && inner.Bottom() <= outer.Bottom();
}

/** The width of the text of a label or button in its font; false when it has no text or the font is not known */
inline bool
MfMeasureText( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* widget, GUCEF::Int32& width )
{
    const GUCEF::CORE::CString text = widget->GetProperty( GUCEF::GUI::EWidgetProperty_Text );
    if ( text.IsNULLOrEmpty() )
        return false;
    GUCEF::Int32 height = 0;
    return context->MeasureText( widget->GetProperty( GUCEF::GUI::EWidgetProperty_Font ), text, 0, width, height );
}

/**
 *  The first thing that is wrong with the layout of a form, empty when it is fine:
 *  - a widget outside the rectangle of its parent (in a scroll panel: outside the column left of the scroll bar; what is scrolled out
 *    vertically is cut off and is fine)
 *  - two controls that are seen and overlap
 *  - a button, check box or field that is seen and lower than 44 pixels (not the ones in 'touchExempt', a part that another form made)
 *  - the text of a label or button that is wider than the box it is in (labels that wrap are left out)
 */
inline GUCEF::CORE::CString
MfFindLayoutProblem( GUCEF::GUI::CGUIContext* context, GUCEF::GUI::CWidget* root, GUCEF::GUI::CWidget* touchExempt = GUCEF_NULL )
{
    using namespace GUCEF;
    TMfWidgetVector all;
    MfCollect( root, all );

    for ( size_t i=1; i<all.size(); ++i )
    {
        GUI::CWidget* widget = all[ i ];
        GUI::CWidget* parent = widget->GetParentWidget();
        if ( !widget->IsVisible() || widget->GetRect().IsEmpty() )
            continue;
        GUI::CRect parentRect = parent->GetRect();
        GUI::CRect rect = widget->GetRect();
        const GUI::CScrollPanel* scrollParent = parent->AsScrollPanel();
        if ( GUCEF_NULL != scrollParent )
        {
            const GUI::CRect viewport = scrollParent->GetViewportRect();
            if ( rect.left < parentRect.left + viewport.left || rect.Right() > parentRect.left + viewport.Right() )
                return "\"" + widget->GetName() + "\" is wider than the scroll panel \"" + parent->GetName() + "\"";
        }
        else
        if ( !MfInside( rect, parentRect ) )
            return "\"" + widget->GetName() + "\" (" + widget->GetType() + ") is outside \"" + parent->GetName() + "\"";
    }

    TMfWidgetVector controls;
    for ( size_t i=0; i<all.size(); ++i )
        if ( !MfIsContainerType( all[ i ]->GetType() ) && all[ i ]->IsEffectivelyVisible() && !MfVisibleRect( all[ i ] ).IsEmpty() )
            controls.push_back( all[ i ] );
    for ( size_t i=0; i<controls.size(); ++i )
    {
        const GUI::CRect a = MfVisibleRect( controls[ i ] );
        for ( size_t j=i+1; j<controls.size(); ++j )
            if ( !a.Intersection( MfVisibleRect( controls[ j ] ) ).IsEmpty() )
                return "\"" + controls[ i ]->GetName() + "\" overlaps \"" + controls[ j ]->GetName() + "\"";
    }

    for ( size_t i=0; i<all.size(); ++i )
    {
        GUI::CWidget* widget = all[ i ];
        if ( !widget->IsEffectivelyVisible() || widget->GetRect().IsEmpty() )
            continue;
        const CORE::CString& type = widget->GetType();
        bool exempt = false;
        for ( const GUI::CWidget* p=widget; GUCEF_NULL != touchExempt && !exempt && GUCEF_NULL != p; p=p->GetParentWidget() )
            exempt = p == touchExempt;
        if ( !exempt && ( type == "Button" || type == "Checkbox" || type == "Editbox" ) && widget->GetRect().height < 44 )
            return "\"" + widget->GetName() + "\" is " + CORE::Int32ToString( widget->GetRect().height ) + " pixels high";
        if ( ( type == "Label" || type == "Button" ) && !widget->GetBoolProperty( "WordWrap" ) )
        {
            Int32 width = 0;
            if ( MfMeasureText( context, widget, width ) && width > widget->GetRect().width )
                return "the text of \"" + widget->GetName() + "\" is " + CORE::Int32ToString( width ) + " pixels wide in a box of " + CORE::Int32ToString( widget->GetRect().width );
        }
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTMATCHLAYOUTCHECKS_H ? */
