/*
 *  guidriverRmlUI: GUI backend using RmlUi
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

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#include "guidriverRmlUI_CRmlUIContext.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#include "guidriverRmlUI_CRmlUIWidgetElement.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H ? */

#include "guidriverRmlUI_CRmlUIWidgetHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CRmlUIWidgetHandler::RcssHintPrefix = "rcss:";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CRmlUIContext*
ContextOf( const GUI::CWidget& widget )
{
    return static_cast< CRmlUIContext* >( widget.GetContext() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIWidgetHandler::CRmlUIWidgetHandler( const CString& widgetType )
    : GUI::CWidgetHandler()
{GUCEF_TRACE;

    // every property of the type is shown: the element draws it through the painter
    GUI::CWidgetTypeRegistry::TDescriptorVector descriptors = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetProperties( widgetType );
    for ( UInt32 i=0; i<(UInt32)descriptors.size(); ++i )
        MapProperty( descriptors[ i ]->name, &CRmlUIWidgetHandler::DrawnByElement );
}

/*-------------------------------------------------------------------------*/

CRmlUIWidgetHandler::~CRmlUIWidgetHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CRmlUIWidgetHandler::DrawnByElement( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CString& value )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIWidgetHandler::CreateNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    CRmlUIContext* context = ContextOf( widget );
    return GUCEF_NULL != context && GUCEF_NULL != context->CreateWidgetElement( widget );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetHandler::DestroyNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    CRmlUIContext* context = ContextOf( widget );
    if ( GUCEF_NULL != context )
        context->DestroyWidgetElement( widget );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetHandler::ApplyParent( GUI::CWidget& widget )
{GUCEF_TRACE;

    CRmlUIContext* context = ContextOf( widget );
    if ( GUCEF_NULL != context )
        context->AttachWidgetElement( widget );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetHandler::ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect )
{GUCEF_TRACE;

    CRmlUIContext* context = ContextOf( widget );
    if ( GUCEF_NULL == context )
        return;
    context->PlaceWidgetElement( widget, rect );

    // the children are placed within this one, the context gives them their new rectangles as well
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetHandler::ApplyChildOrder( GUI::CWidget& widget )
{GUCEF_TRACE;

    CRmlUIContext* context = ContextOf( widget );
    if ( GUCEF_NULL != context )
        context->OrderChildElements( widget );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIWidgetHandler::ApplyState( GUI::CWidget& widget )
{GUCEF_TRACE;

    // the element draws the state through the painter at every frame
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIWidgetHandler::ApplyProperty( GUI::CWidget& widget, const CString& name, const CString& value )
{GUCEF_TRACE;

    CRmlUIWidgetElement* element = CRmlUIContext::GetWidgetElement( widget );
    if ( GUCEF_NULL == element )
        return false;

    if ( "Visible" == name )
    {
        element->SetProperty( "display", widget.IsVisible() ? "block" : "none" );
    }
    else
    if ( name.StartsWith( RcssHintPrefix ) )
    {
        CString rcssName = name.CutChars( 5, true, 0 );
        if ( value.IsNULLOrEmpty() )
            element->RemoveProperty( rcssName.C_String() );
        else
            element->SetProperty( rcssName.C_String(), value.C_String() );
        return true;
    }
    else
    if ( "rml:class" == name )
    {
        element->SetClassNames( value.C_String() );
        return true;
    }
    return GUI::CWidgetHandler::ApplyProperty( widget, name, value );
}

/*-------------------------------------------------------------------------*/

void*
CRmlUIWidgetHandler::GetNativeHandle( const GUI::CWidget& widget ) const
{GUCEF_TRACE;

    return CRmlUIContext::GetWidgetElement( widget );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
