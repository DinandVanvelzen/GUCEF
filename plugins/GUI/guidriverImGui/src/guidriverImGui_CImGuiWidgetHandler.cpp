/*
 *  guidriverImGui: the gucefGUI driver library of Dear ImGui, shared by its renderer drivers
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

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "guidriverImGui_CImGuiWidgetHandler.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** What a widget keeps of its drawing: the native object of a widget in an ImGui context */
struct SWidgetDrawing
{
    struct SFontUse
    {
        CString name;
        const void* font;       /**< the bitmap font the name gave when the commands were painted */
    };

    CImGuiWidgetHandler::TCommandVector commands;
    GUCEF::vector< SFontUse > fonts;
    UInt32 imageGeneration;
    UInt32 paints;
    bool valid;

    SWidgetDrawing( void )
        : commands()
        , fonts()
        , imageGeneration( 0 )
        , paints( 0 )
        , valid( false )
    {
    }
};

SWidgetDrawing*
DrawingOf( const GUI::CWidget& widget )
{
    return static_cast< SWidgetDrawing* >( widget.GetImplementationPtr() );
}

/** Whether every bitmap font the drawing used is still the font registered under its name */
bool
FontsUnchanged( const SWidgetDrawing& drawing, GUI::CGUIContext& context )
{
    for ( UInt32 i=0; i<(UInt32)drawing.fonts.size(); ++i )
    {
        GUI::TBitmapFontPtr font = context.GetBitmapFont( drawing.fonts[ i ].name );
        if ( font.GetPointerAlways() != drawing.fonts[ i ].font )
            return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiWidgetHandler::CImGuiWidgetHandler( const CString& widgetType )
    : GUI::CWidgetHandler()
{GUCEF_TRACE;

    GUI::CWidgetTypeRegistry::TDescriptorVector descriptors = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetProperties( widgetType );
    for ( UInt32 i=0; i<(UInt32)descriptors.size(); ++i )
        MapProperty( descriptors[ i ]->name, &CImGuiWidgetHandler::DrawnByPainter );
}

/*-------------------------------------------------------------------------*/

CImGuiWidgetHandler::~CImGuiWidgetHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImGuiWidgetHandler::DrawnByPainter( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CString& value )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiWidgetHandler::CreateNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    if ( GUCEF_NULL == DrawingOf( widget ) )
        widget.SetImplementationPtr( GUCEF_NEW SWidgetDrawing() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiWidgetHandler::DestroyNative( GUI::CWidget& widget )
{GUCEF_TRACE;

    SWidgetDrawing* drawing = DrawingOf( widget );
    widget.SetImplementationPtr( GUCEF_NULL );
    GUCEF_DELETE drawing;
}

/*-------------------------------------------------------------------------*/

void
CImGuiWidgetHandler::ApplyParent( GUI::CWidget& widget )
{GUCEF_TRACE;

    Invalidate( widget );
}

/*-------------------------------------------------------------------------*/

void
CImGuiWidgetHandler::ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect )
{GUCEF_TRACE;

    Invalidate( widget );
}

/*-------------------------------------------------------------------------*/

void
CImGuiWidgetHandler::ApplyChildOrder( GUI::CWidget& widget )
{GUCEF_TRACE;

    Invalidate( widget );
}

/*-------------------------------------------------------------------------*/

void
CImGuiWidgetHandler::ApplyState( GUI::CWidget& widget )
{GUCEF_TRACE;

    Invalidate( widget );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiWidgetHandler::ApplyProperty( GUI::CWidget& widget, const CString& name, const CString& value )
{GUCEF_TRACE;

    Invalidate( widget );
    return GUI::CWidgetHandler::ApplyProperty( widget, name, value );
}

/*-------------------------------------------------------------------------*/

void*
CImGuiWidgetHandler::GetNativeHandle( const GUI::CWidget& widget ) const
{GUCEF_TRACE;

    return widget.GetImplementationPtr();
}

/*-------------------------------------------------------------------------*/

void
CImGuiWidgetHandler::Invalidate( const GUI::CWidget& widget )
{GUCEF_TRACE;

    SWidgetDrawing* drawing = DrawingOf( widget );
    if ( GUCEF_NULL != drawing )
        drawing->valid = false;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiWidgetHandler::GetPaintCount( const GUI::CWidget& widget )
{GUCEF_TRACE;

    const SWidgetDrawing* drawing = DrawingOf( widget );
    return GUCEF_NULL != drawing ? drawing->paints : 0;
}

/*-------------------------------------------------------------------------*/

const CImGuiWidgetHandler::TCommandVector&
CImGuiWidgetHandler::GetCommands( const GUI::CWidget& widget, GUI::CGUIContext& context, UInt32 imageGeneration )
{GUCEF_TRACE;

    static const TCommandVector noCommands;
    SWidgetDrawing* drawing = DrawingOf( widget );
    if ( GUCEF_NULL == drawing )
        return noCommands;
    if ( drawing->valid && drawing->imageGeneration == imageGeneration && FontsUnchanged( *drawing, context ) )
        return drawing->commands;

    drawing->commands.clear();
    GUI::CWidgetPainter::Paint( widget, context, drawing->commands );
    drawing->fonts.clear();
    for ( UInt32 i=0; i<(UInt32)drawing->commands.size(); ++i )
    {
        const GUI::CDrawSurface::TCommand& command = drawing->commands[ i ];
        if ( GUI::CDrawSurface::COMMAND_TEXT != command.type )
            continue;
        bool known = false;
        for ( UInt32 f=0; f<(UInt32)drawing->fonts.size() && !known; ++f )
            known = drawing->fonts[ f ].name == command.font;
        if ( known )
            continue;
        SWidgetDrawing::SFontUse use;
        use.name = command.font;
        use.font = context.GetBitmapFont( command.font ).GetPointerAlways();
        drawing->fonts.push_back( use );
    }
    drawing->imageGeneration = imageGeneration;
    drawing->valid = true;
    ++drawing->paints;
    return drawing->commands;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
