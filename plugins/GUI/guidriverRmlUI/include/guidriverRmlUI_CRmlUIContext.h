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

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#define GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <RmlUi/Core/CallbackTexture.h>
#include <RmlUi/Core/Texture.h>

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace Rml { class Context; class ElementDocument; class RenderInterface; }

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRmlUIWidgetElement;

/**
 *  A gucefGUI context drawn by RmlUi.
 *
 *  Every root widget (form) is an RmlUi document holding an element per widget (CRmlUIWidgetElement);
 *  the documents are shown, hidden and stacked like the roots. A last document on top draws the
 *  overlay (tooltip, cursor picture). Images and bitmap font atlases become RmlUi textures, made
 *  again when their pixels change. RmlUi documents loaded from RML files (CRmlUIFormBackend) live
 *  beside the widget documents and get the input of their form.
 *
 *  The renderer driver makes the RmlUi context with its render interface and draws frames: it
 *  calls PrepareRml() and RenderRml() between the begin and end of its frame.
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CRmlUIContext : public GUI::CGUIContext
{
    public:

    CRmlUIContext( GUI::CGUIDriver* driver                 ,
                   GUI::TWindowContextPtr windowContext    ,
                   Rml::RenderInterface* renderInterface   );

    virtual ~CRmlUIContext();

    /** Whether the RmlUi context could be made */
    bool IsValid( void ) const;

    Rml::Context* GetRmlContext( void ) const;

    /*
     *  For the widget elements and handlers
     */

    CRmlUIWidgetElement* CreateWidgetElement( GUI::CWidget& widget );

    void DestroyWidgetElement( GUI::CWidget& widget );

    /** Puts the element of the widget below the element of its parent, or in a document of its own for a root */
    void AttachWidgetElement( GUI::CWidget& widget );

    /** Orders the elements of the children like the children of the widget */
    void OrderChildElements( GUI::CWidget& widget );

    /** Places the element of the widget at the rectangle (in context coordinates) */
    void PlaceWidgetElement( GUI::CWidget& widget, const GUI::CRect& rect );

    static CRmlUIWidgetElement* GetWidgetElement( const GUI::CWidget& widget );

    /** The size of the texture of a registered or loadable image; false when there is none */
    bool HasImageTexture( const CString& imageReference, Int32& width, Int32& height );

    Rml::Texture GetImageTexture( const CString& imageReference );

    /** The size of the atlas texture of a registered bitmap font; false when there is none */
    bool HasFontTexture( const CString& fontName, Int32& width, Int32& height );

    Rml::Texture GetFontTexture( const CString& fontName );

    /** How many textures were made, for tests: a picture that does not change is not uploaded again */
    UInt32 GetTextureCreationCount( void ) const;

    /*
     *  For the RML form backend
     */

    /** Shows the document with the root and gives it the input that reaches the root */
    void RegisterNativeDocument( GUI::CWidget* root, Rml::ElementDocument* document );

    void UnregisterNativeDocument( GUI::CWidget* root );

    virtual GUI::CFormBackend* CreateFormBackend( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Brings RmlUi up to date: the size, which documents are shown and their order, layout */
    void PrepareRml( void );

    /** Lets RmlUi draw the documents through the render interface */
    void RenderRml( void );

    /**
     *  Destroys the widgets and the RmlUi context; a renderer context calls it in its destructor, before
     *  its render interface goes
     */
    void ReleaseRml( void );

    virtual void OnRootOrderChanged( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnImageChanged( const CString& alias ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct STextureEntry
    {
        Rml::CallbackTexture texture;
        UInt32 revision;
        const void* pixels;
        Int32 width;
        Int32 height;
    };
    typedef struct STextureEntry TTextureEntry;
    typedef GUCEF::map< CString, TTextureEntry > TTextureMap;
    typedef GUCEF::map< GUI::CWidget*, Rml::ElementDocument* > TDocumentMap;

    Rml::ElementDocument* MakeDocument( void );

    void StyleDocument( Rml::ElementDocument* document );

    TTextureEntry* FindTexture( const CString& reference, bool isFont );

    void ForwardToNativeDocument( Rml::ElementDocument* document, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CRmlUIContext( const CRmlUIContext& src );
    CRmlUIContext& operator=( const CRmlUIContext& src );

    private:

    Rml::Context* m_rmlContext;
    CString m_rmlContextName;
    Rml::ElementDocument* m_overlayDocument;
    CRmlUIWidgetElement* m_overlay;
    TDocumentMap m_rootDocuments;
    TDocumentMap m_nativeDocuments;
    TTextureMap m_imageTextures;
    TTextureMap m_fontTextures;
    UInt32 m_textureCreations;
    bool m_orderDirty;
    UInt32 m_rmlWidth;
    UInt32 m_rmlHeight;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H ? */
