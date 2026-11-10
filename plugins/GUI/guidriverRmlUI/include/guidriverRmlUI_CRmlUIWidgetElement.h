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

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Geometry.h>

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRmlUIContext;

/**
 *  The RmlUi element of one gucefGUI widget (tag "gucefwidget").
 *
 *  RmlUi keeps the tree, the order of the documents, visibility and the frame; the element draws the
 *  widget the way gucefGUI's painter describes it (CWidgetPainter), so the widget looks the same as
 *  with every other driver. The drawing is turned into rectangles, clipped on the CPU to the area the
 *  widget may draw in, and kept as geometry until the drawing changes.
 *
 *  An element without a widget draws the overlay of the context (tooltip and cursor picture).
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CRmlUIWidgetElement : public Rml::Element
{
    public:

    static const char* TagName;

    CRmlUIWidgetElement( const Rml::String& tag );

    virtual ~CRmlUIWidgetElement();

    /** The widget drawn, NULL with 'overlay' for the overlay of the context */
    void SetWidget( CRmlUIContext* context, GUI::CWidget* widget, bool overlay );

    GUI::CWidget* GetWidget( void ) const;

    /** Forgets the geometry, it is built again at the next frame */
    void Invalidate( void );

    /** How many times the geometry was built; it is only built when the drawing changed */
    UInt32 GetBuildCount( void ) const;

    protected:

    virtual void OnRender( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SBatch
    {
        Rml::Geometry geometry;
        CString texture;        /**< image alias or font name, empty for plain colour */
        bool isFont;
        Int32 textureWidth;
        Int32 textureHeight;
        UInt32 density;         /**< of the image or font when the batch was made */
    };
    typedef struct SBatch TBatch;
    typedef GUCEF::vector< TBatch > TBatchVector;

    friend class CRmlUIQuadBuilder;

    bool IsUpToDate( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& clip, Float32 opacity ) const;

    void Build( const GUI::CDrawSurface::TCommandVector& commands, const GUI::CRect& clip, Float32 opacity );

    CRmlUIWidgetElement( const CRmlUIWidgetElement& src );
    CRmlUIWidgetElement& operator=( const CRmlUIWidgetElement& src );

    private:

    CRmlUIContext* m_context;
    GUI::CWidget* m_widget;
    bool m_overlay;
    bool m_valid;
    GUI::CDrawSurface::TCommandVector m_commands;
    GUI::CRect m_clip;
    Float32 m_opacity;
    UInt32 m_renderScale;       /**< of the context when the batches were made */
    TBatchVector m_batches;
    UInt32 m_buildCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H ? */
