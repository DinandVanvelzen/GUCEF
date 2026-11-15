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

#ifndef GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H
#define GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWIDGETHANDLER_H
#include "gucefGUI_CWidgetHandler.h"
#define GUCEF_GUI_CWIDGETHANDLER_H
#endif /* GUCEF_GUI_CWIDGETHANDLER_H ? */

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI { class CGUIContext; }
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The handler of every widget type in an ImGui context (one object per type, all of this class). A retained widget is
 *  drawn with the commands of CWidgetPainter::Paint(), kept per widget and painted again only when the context passes a
 *  change of the widget to the handler (ApplyProperty(), ApplyLayout(), ApplyState(), ApplyParent(), ApplyChildOrder()),
 *  when an image of the context changed or when a bitmap font the drawing uses was registered again. CImGuiDrawListPainter
 *  replays the commands into a draw list of Dear ImGui every frame.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiWidgetHandler : public GUI::CWidgetHandler
{
    public:

    typedef GUI::CDrawSurface::TCommandVector TCommandVector;

    explicit CImGuiWidgetHandler( const CString& widgetType );

    virtual ~CImGuiWidgetHandler();

    virtual bool CreateNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyNative( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyParent( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyLayout( GUI::CWidget& widget, const GUI::CRect& rect ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyChildOrder( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void ApplyState( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplyProperty( GUI::CWidget& widget, const CString& name, const CString& value ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void* GetNativeHandle( const GUI::CWidget& widget ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The drawing commands of the widget relative to the widget, painted again when they are out of date; 'imageGeneration'
     *  is the count of image changes of the context (CImGuiContext::GetImageGeneration())
     */
    static const TCommandVector& GetCommands( const GUI::CWidget& widget, GUI::CGUIContext& context, UInt32 imageGeneration );

    /** Marks the drawing of the widget out of date */
    static void Invalidate( const GUI::CWidget& widget );

    /** How often the widget was painted: for tests of the cache */
    static UInt32 GetPaintCount( const GUI::CWidget& widget );

    private:

    CImGuiWidgetHandler( const CImGuiWidgetHandler& src );
    CImGuiWidgetHandler& operator=( const CImGuiWidgetHandler& src );

    /** Every property is shown, through the painter */
    static bool DrawnByPainter( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CString& value );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUIWIDGETHANDLER_H ? */
