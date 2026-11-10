/*
 *  guidriverNull: GUCEF GUI driver that keeps the widgets but draws nothing, for tests and tools
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

#include "guidriverNull_CNullWidgetHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SNullNative::SNullNative( void )
    : properties()
    , propertyApplyCount( 0 )
    , layoutApplyCount( 0 )
    , stateApplyCount( 0 )
    , parentApplyCount( 0 )
    , childOrderApplyCount( 0 )
    , commandCount( 0 )
    , rect()
    , state( 0 )
    , parent( GUCEF_NULL )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CNullWidgetHandler::CNullWidgetHandler( const CORE::CString& widgetType )
    : GUI::CWidgetHandler()
{GUCEF_TRACE;

    // every property of the type is mapped, so the capability report lists them all
    GUI::CWidgetTypeRegistry::TDescriptorVector descriptors = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetProperties( widgetType );
    for ( CORE::UInt32 i=0; i<(CORE::UInt32)descriptors.size(); ++i )
        MapProperty( descriptors[ i ]->name, &CNullWidgetHandler::RecordProperty );
    MapCommand( "ScrollTo", GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

CNullWidgetHandler::~CNullWidgetHandler()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

bool
CNullWidgetHandler::RecordProperty( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CORE::CString& value )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullWidgetHandler::CreateNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    TNullNative* native = GUCEF_NEW TNullNative();
    native->parent = widget.GetParentWidget();
    widget.SetImplementationPtr( native );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNullWidgetHandler::DestroyNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    delete static_cast< TNullNative* >( widget.GetImplementationPtr() );
    widget.SetImplementationPtr( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

void
CNullWidgetHandler::ApplyParent( GUI::CWidget& widget )
{GUCEF_TRACE;

    TNullNative* native = static_cast< TNullNative* >( widget.GetImplementationPtr() );
    if ( GUCEF_NULL == native )
        return;
    native->parent = widget.GetParentWidget();
    ++native->parentApplyCount;
}

/*-------------------------------------------------------------------------*/

void
CNullWidgetHandler::ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect )
{GUCEF_TRACE;

    TNullNative* native = static_cast< TNullNative* >( widget.GetImplementationPtr() );
    if ( GUCEF_NULL == native )
        return;
    native->rect = rect;
    ++native->layoutApplyCount;
}

/*-------------------------------------------------------------------------*/

void
CNullWidgetHandler::ApplyChildOrder( GUI::CWidget& widget )
{GUCEF_TRACE;

    TNullNative* native = static_cast< TNullNative* >( widget.GetImplementationPtr() );
    if ( GUCEF_NULL != native )
        ++native->childOrderApplyCount;
}

/*-------------------------------------------------------------------------*/

void
CNullWidgetHandler::ApplyState( GUI::CWidget& widget )
{GUCEF_TRACE;

    TNullNative* native = static_cast< TNullNative* >( widget.GetImplementationPtr() );
    if ( GUCEF_NULL == native )
        return;
    native->state = widget.GetState();
    ++native->stateApplyCount;
}

/*-------------------------------------------------------------------------*/

bool
CNullWidgetHandler::ApplyProperty( GUI::CWidget& widget, const CORE::CString& name, const CORE::CString& value )
{GUCEF_TRACE;

    TNullNative* native = static_cast< TNullNative* >( widget.GetImplementationPtr() );
    if ( GUCEF_NULL == native )
        return false;
    native->properties[ name ] = value;
    ++native->propertyApplyCount;
    GUI::CWidgetHandler::ApplyProperty( widget, name, value );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNullWidgetHandler::RunCommand( GUI::CWidget& widget, const CORE::CString& command, const CORE::CValueList& arguments )
{GUCEF_TRACE;

    TNullNative* native = static_cast< TNullNative* >( widget.GetImplementationPtr() );
    if ( GUCEF_NULL == native )
        return false;
    ++native->commandCount;
    return true;
}

/*-------------------------------------------------------------------------*/

void*
CNullWidgetHandler::GetNativeHandle( const GUI::CWidget& widget ) const
{GUCEF_TRACE;

    return widget.GetImplementationPtr();
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
