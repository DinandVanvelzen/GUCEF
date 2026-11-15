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

#ifndef GUCEF_guidriverMyGUI_CMYGUICONTEXT_H
#define GUCEF_guidriverMyGUI_CMYGUICONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#include "guidriverMyGUI_macros.h"
#define GUCEF_guidriverMyGUI_MACROS_H
#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MyGUI { class Gui; class Widget; class FontManager; class ITexture; }

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A gucefGUI context drawn by MyGUI. Every gucefGUI widget has a MyGUI widget (made by the handler of its
 *  type), all of them in one MyGUI layer, placed at the rectangle gucefGUI laid out and stacked in the order
 *  of the forms and the widgets. MyGUI itself gets no input: the context of gucefGUI routes it and tells the
 *  MyGUI widgets how they look (hovered, pressed, focused, enabled).
 *
 *  MyGUI has process wide singletons (the Gui, the render manager, the data manager), so a driver has one
 *  context at a time. The renderer driver derives its context from this class: it starts the render manager
 *  of its platform, calls StartGui(), draws with BeginFrame() / EndFrame() around the drawing of its
 *  platform and calls ReleaseGui() before the render manager goes.
 */
class GUCEF_guidriverMyGUI_PUBLIC_CPP CMyGUIContext : public GUI::CGUIContext
{
    public:

    /** The layer all widgets are in */
    static const char* WidgetLayerName;

    /** The name the MSDF font shader of the render platform is registered with */
    static const char* MsdfShaderName;

    CMyGUIContext( GUI::CGUIDriver* driver, GUI::TWindowContextPtr windowContext );

    virtual ~CMyGUIContext();

    /** Starts MyGUI with the resources of the data manager (MyGUI_Core.xml); the render manager must be running */
    bool StartGui( void );

    /** Destroys the widgets and shuts MyGUI down; the render manager may go after it */
    void ReleaseGui( void );

    bool IsGuiStarted( void ) const;

    MyGUI::Gui* GetGui( void ) const;

    /*
     *  For the widget handlers
     */

    /** The MyGUI widget of a widget, NULL when it has none */
    static MyGUI::Widget* GetNativeWidget( const GUI::CWidget& widget );

    static void SetNativeWidget( GUI::CWidget& widget, MyGUI::Widget* native );

    /** Makes a MyGUI widget of a MyGUI type ("Button") with a skin ("Button") in the layer of the widgets */
    MyGUI::Widget* CreateNativeWidget( const char* type, const char* skin, const std::string& name );

    void DestroyNativeWidget( MyGUI::Widget* native );

    /** Places the MyGUI widget at the rectangle in logical pixels, scaled by the render scale */
    void PlaceNativeWidget( MyGUI::Widget* native, const GUI::CRect& rect ) const;

    /** The order of the widgets changed */
    void InvalidateStacking( void );

    /** The name of the MyGUI font for a gucefGUI font name; the default font of MyGUI when it has no such font */
    std::string GetMyGUIFontName( const CString& fontName ) const;

    /** Whether MyGUI has a font with the name */
    bool HasMyGUIFont( const CString& fontName ) const;

    /**
     *  Makes (when the picture changed: again) the MyGUI texture for an image of gucefGUI (a registered alias or a
     *  VFS path), true when the texture exists; 'textureName' is the name MyGUI knows it by
     */
    bool PrepareTexture( const CString& imageReference, std::string& textureName, Int32& width, Int32& height );

    /** Loads a TrueType font from the VFS as a MyGUI font under the name, at a point size */
    bool LoadFont( const CString& name, const CString& vfsPath, UInt32 size );

    /**
     *  Loads a TrueType font from the VFS as a multi-channel signed distance field (MSDF) font under the name:
     *  MyGUI generates the distance fields of the glyphs once, at 'atlasSize', and the MSDF shader of the
     *  render platform draws them sharp at every text height. The text has the size a bitmap font of 'size'
     *  has (see GetMyGUIFontHeight()). False when the platform has no MSDF shader (SupportsMsdfFonts()) or the
     *  font cannot be loaded; the caller falls back to LoadFont().
     */
    bool LoadMsdfFont( const CString& name, const CString& vfsPath, UInt32 size, UInt32 atlasSize );

    /** Whether the render platform of this context draws MSDF fonts: it has a shader for them */
    virtual bool SupportsMsdfFonts( void ) const;

    /** Whether the MyGUI font was loaded with LoadMsdfFont() */
    bool IsMsdfFont( const CString& fontName ) const;

    /**
     *  The height in render pixels to give the MyGUI text widgets that use the font: an MSDF font is drawn at the
     *  logical text height times the render scale (it stays sharp), a bitmap font, or a name that is no font, has 0
     *  which is the default height of the font
     */
    Int32 GetMyGUIFontHeight( const CString& fontName ) const;

    virtual bool MeasureText( const CString& fontName, const CString& text, Int32 maxWidth, Int32& width, Int32& height ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Whether the MyGUI platform reads the bytes of an "R8G8B8A8" texture as B, G, R, A (the OpenGL and Ogre
     *  platforms) instead of R, G, B, A (OpenGL ES); the pictures of gucefGUI are written to the textures that way
     */
    virtual bool IsBgraTextureMemory( void ) const;

    protected:

    /**
     *  Registers the MSDF shader of the render platform with the render manager of MyGUI under MsdfShaderName;
     *  false when the platform has none or it does not compile. Only a platform that says SupportsMsdfFonts() is asked.
     */
    virtual bool RegisterMsdfShader( void );

    /**
     *  Brings the MyGUI widgets up to date for a frame: visibility, stacking and the size of the view; the render
     *  manager of the platform lets MyGUI run its animations while it draws
     */
    void PrepareFrame( void );

    virtual void OnRootOrderChanged( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnImageChanged( const CString& alias ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct STextureEntry
    {
        UInt32 revision;
        Int32 width;
        Int32 height;
        std::string name;
    };
    typedef struct STextureEntry TTextureEntry;
    typedef GUCEF::map< CString, TTextureEntry > TTextureMap;
    typedef GUCEF::map< CString, Int32 > TMsdfFontMap;

    CMyGUIContext( const CMyGUIContext& src );
    CMyGUIContext& operator=( const CMyGUIContext& src );

    void RefreshVisibility( void );

    void RefreshStacking( void );

    void StackWidget( GUI::CWidget* widget );

    void ReleaseTextures( void );

    bool WriteTexturePixels( MyGUI::ITexture& texture, IMAGE::CPixelMap& pixels ) const;

    bool LoadFontDefinition( const CString& definition, const CString& name, const CString& vfsPath );

    void ApplyMsdfFontHeights( void );

    private:

    MyGUI::Gui* m_gui;
    bool m_stackingDirty;
    UInt32 m_appliedScale;
    UInt32 m_textureCounter;
    TTextureMap m_textures;
    TMsdfFontMap m_msdfFonts;
    bool m_msdfShaderReady;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_CMYGUICONTEXT_H ? */
