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

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXT_H
#define GUIDRIVERIMGUI_CIMGUICONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "imgui.h"

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZER_H
#include "gucefGUI_CFontRasterizer.h"
#define GUCEF_GUI_CFONTRASTERIZER_H
#endif /* GUCEF_GUI_CFONTRASTERIZER_H ? */

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
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CImGuiDriver;
class CImGuiFontLoader;
class CImGuiTextureCache;
class CImGuiSoftwareRenderer;

/**
 *  A gucefGUI context that is also a Dear ImGui context: a full gucefGUI layer (logical resolution, present rectangle,
 *  images, fonts, theme and retained widgets, drawn below the ImGui windows) with one ImGuiContext and its own font atlas.
 *
 *  A frame, in OnRender(), which RenderFrame() calls once per frame on the thread of the window:
 *  1. the ImGui context becomes current (CImGuiContextScope); io.DisplaySize is the logical size, io.DisplayFramebufferScale
 *     the present scale (the render scale without a window), io.DeltaTime the context time since the last frame (at
 *     least 1 ms);
 *  2. OnRendererNewFrame() and ImGui::NewFrame();
 *  3. the retained roots into the background draw list, the gucefGUI overlay (tooltip, cursor picture) into the
 *     foreground draw list (CImGuiDrawListPainter);
 *  4. BuildFrameEvent: the immediate-mode code of the application runs in its handlers, with the context current;
 *  5. ImGui::Render(), the cursor shape, RenderDrawData() with the draw data, IniSettingsChangedEvent when Dear ImGui
 *     wants its settings saved.
 *
 *  Input: every Inject*() call reaches Dear ImGui first through the previews (always queued, so a release is never lost);
 *  the event is consumed, and the retained widgets do not get it, while io.WantCaptureMouse (pointer and wheel),
 *  io.WantCaptureKeyboard (keys) or io.WantTextInput (text) of the last frame is set. Between layers the context takes
 *  the pointer while it shows a root or Dear ImGui wants the mouse, and the keyboard likewise.
 *
 *  Rendering: this class draws nothing on a screen. RenderDrawData() serves the texture requests in memory and
 *  CaptureFrame() rasterizes the last draw data with CImGuiSoftwareRenderer, which makes the context work headless. A
 *  renderer driver derives its context from this class and overrides the renderer hooks (see RenderDrawData()).
 *
 *  All calls are made on the ImGui thread (CImGuiGlobal): the thread of the first ImGui context of the process.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiContext : public GUI::CGUIContext
{
    public:

    /** Between ImGui::NewFrame() and ImGui::Render(), with the context current; TCloneableUInt32 with the frame time in milliseconds */
    static const CORE::CEvent BuildFrameEvent;

    /** Dear ImGui wants its settings saved: GetIniSettings() gives them, the application stores them */
    static const CORE::CEvent IniSettingsChangedEvent;

    static void RegisterEvents( void );

    typedef GUI::CFontRasterizer::TCodePointRanges TCodePointRanges;

    /** What Dear ImGui last said about text input (ImGuiPlatformImeData), for a text input area or soft keyboard */
    struct STextInputState
    {
        bool wantVisible;           /**< a widget wants the input method shown */
        bool wantTextInput;         /**< a widget wants text */
        Float32 inputX;             /**< where the text cursor is, in logical pixels */
        Float32 inputY;
        Float32 lineHeight;
        UInt32 updates;             /**< how often Dear ImGui set the data */

        STextInputState( void );
    };
    typedef struct STextInputState TTextInputState;

    /** The mapping of draw data to pixels: the fields DisplayPos, DisplaySize and FramebufferScale of ImDrawData */
    struct SDrawDataMapping
    {
        ImVec2 displayPos;
        ImVec2 displaySize;
        ImVec2 framebufferScale;
    };
    typedef struct SDrawDataMapping TDrawDataMapping;

    CImGuiContext( CImGuiDriver* driver, GUI::TWindowContextPtr windowContext );

    /** Releases the widgets, the renderer (when the renderer context did not), the textures, then the Dear ImGui context */
    virtual ~CImGuiContext();

    /** Whether the Dear ImGui context exists and has a font: false when the context was refused (see the log) */
    bool IsReady( void ) const;

    ::ImGuiContext* GetImGuiContext( void ) const;

    /** How many frames were built (BuildFrameEvent notified) */
    UInt32 GetBuiltFrameCount( void ) const;

    /** Whether a frame is being built now: between ImGui::NewFrame() and ImGui::Render() */
    bool IsBuildingFrame( void ) const;

    /** The draw data of the last frame, valid until the next frame; NULL before the first frame */
    ImDrawData* GetDrawData( void );

    /*
     *  Settings of Dear ImGui (the .ini text): no file is written, the application stores them
     */

    bool GetIniSettings( CString& settings ) const;

    bool SetIniSettings( const CString& settings );

    /** The last text input data of Dear ImGui */
    const TTextInputState& GetTextInputState( void ) const;

    /*
     *  Look
     */

    /** Scales the sizes of the style and the fonts; 1 is the unscaled style */
    void SetUiScale( Float32 scale );

    Float32 GetUiScale( void ) const;

    /** Dark, Light or Classic: the colours of the style; false for another name */
    bool SetStyle( const CString& styleName );

    const CString& GetStyle( void ) const;

    /*
     *  Fonts: rasterized by the font rasterizers of gucefGUI (CImGuiFontLoader)
     */

    /** Adds an outline font (a VFS path) at a size in logical pixels (the height of a line), with Latin-1 and the bullet */
    ImFont* AddFont( const CString& vfsPath, Float32 sizePixels );

    ImFont* AddFont( const CString& vfsPath, Float32 sizePixels, const TCodePointRanges& ranges );

    /** Adds a bitmap font registered with this context (RegisterFont()) */
    ImFont* AddBitmapFont( const CString& fontName );

    /** Adds the embedded ProggyClean font of Dear ImGui, rasterized by the font loader */
    ImFont* AddDefaultFont( void );

    CImGuiFontLoader& GetFontLoader( void );

    /*
     *  Images
     */

    /** The images and font atlases as textures of Dear ImGui */
    CImGuiTextureCache& GetTextureCache( void );

    /** A texture reference for ImGui::Image() and the draw lists: an image of the context (alias or VFS path) */
    ImTextureRef GetImageTextureRef( const CString& reference );

    /** Draws an image at its logical size (pixels divided by its density) in the current ImGui window; false when it does not resolve */
    bool Image( const CString& reference );

    /** Goes up with every change of an image of the context */
    UInt32 GetImageGeneration( void ) const;

    /**
     *  A renderer context that has textures of its own backend to show (a render texture of the application) can give the
     *  texture reference of an image of the context here. The image stays registered with the context as the stand-in
     *  that the drawing commands name, its size is the one the source rectangles of the commands refer to; when this
     *  returns true the retained painter draws the image with the reference, over the whole backend texture for the whole
     *  stand-in, with linear sampling, and makes no texture of the pixels of the stand-in. False by default
     */
    virtual bool GetBackendImageTextureRef( const CString& reference, ImTextureRef& textureRef ) const;

    /*
     *  For renderers
     */

    /** The CPU rasterizer and texture server of the context: the default renderer and the default capture */
    CImGuiSoftwareRenderer& GetSoftwareRenderer( void );

    /** The mapping that puts logical (0,0) at the origin of the present rectangle of the window, at the present scale */
    TDrawDataMapping GetPresentMapping( void ) const;

    /** The mapping that puts the logical picture over a whole target of the given size */
    TDrawDataMapping GetTargetMapping( UInt32 width, UInt32 height ) const;

    static TDrawDataMapping GetDrawDataMapping( const ImDrawData& drawData );

    static void SetDrawDataMapping( ImDrawData& drawData, const TDrawDataMapping& mapping );

    /*
     *  gucefGUI
     */

    /** A shown root, or Dear ImGui wants the mouse (a window or a popup is under the pointer, or an ImGui item holds it) */
    virtual bool IsTakingPointerInput( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A shown root, or Dear ImGui wants the keyboard; nothing while the context passes the keyboard on */
    virtual bool IsTakingKeyboardInput( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The last frame at the render size, drawn again by RenderDrawDataOffscreen() without a new frame */
    virtual bool CaptureFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnRender( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnImageChanged( const CString& alias ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnImageRegionChanged( const CString& alias, const GUI::CRect& region ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool PreviewMouseMove( Int32 x, Int32 y ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool PreviewMouseButton( GUI::TMouseButton button, bool down ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool PreviewMouseWheel( Int32 delta ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool PreviewKey( INPUT::KeyCode keyCode, bool down, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool PreviewText( const CString& text ) GUCEF_VIRTUAL_OVERRIDE;

    /*
     *  The renderer hooks. They are called on the ImGui thread with the ImGui context current. A renderer context:
     *  - sets up its backend in its constructor (with the ImGui context current: CImGuiContextScope( GetImGuiContext() )),
     *    sets io.BackendRendererName and keeps ImGuiBackendFlags_RendererHasTextures, and sets the standard draw
     *    callbacks of the platform IO (DrawCallback_SetSamplerNearest, DrawCallback_SetSamplerLinear,
     *    DrawCallback_ResetRenderState), which the retained painter uses;
     *  - overrides the hooks below;
     *  - calls DestroyAllWidgets() and ShutdownRenderer() in its destructor, while its backend still exists.
     */

    /** Before ImGui::NewFrame(); false skips the frame. Nothing by default */
    virtual bool OnRendererNewFrame( void );

    /**
     *  Draws the draw data of a frame and serves its texture requests (drawData.Textures). The draw data is valid until the
     *  next frame. By default the requests are served in memory and nothing is drawn. A renderer that draws in the window
     *  applies GetPresentMapping() first (SetDrawDataMapping()) and clears the window when it is the bottom layer
     */
    virtual bool RenderDrawData( ImDrawData& drawData );

    /**
     *  Draws the draw data again into an RGBA picture of the given size for CaptureFrame(), without serving texture
     *  requests and without a new frame. By default the software rasterizer draws it on opaque black
     */
    virtual bool RenderDrawDataOffscreen( ImDrawData& drawData, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels );

    /** Destroys what the renderer made: its textures (those of platform_io.Textures included) and its backend */
    virtual void ReleaseRenderer( void );

    /** Calls ReleaseRenderer() once, with the ImGui context current; the destructor of a renderer context calls it */
    void ShutdownRenderer( void );

    private:

    friend class CImGuiContextPlatformFunctions;

    typedef CORE::CTEventHandlerFunctor< CImGuiContext > TEventCallback;

    CImGuiContext( const CImGuiContext& src );
    CImGuiContext& operator=( const CImGuiContext& src );

    bool Initialize( void );
    bool ApplyStyle( void );

    void OnWindowActivation( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

    bool CheckThread( const char* action ) const;

    private:

    CImGuiDriver* m_imguiDriver;
    ::ImGuiContext* m_imguiContext;
    CImGuiFontLoader* m_fontLoader;
    CImGuiTextureCache* m_textureCache;
    CImGuiSoftwareRenderer* m_softwareRenderer;
    ImGuiStyle* m_unscaledStyle;
    Float32 m_uiScale;
    CString m_styleName;
    CString m_clipboardText;
    TTextInputState m_textInput;
    UInt64 m_lastFrameTime;
    UInt32 m_frameCount;
    UInt32 m_imageGeneration;
    bool m_registered;
    bool m_ready;
    bool m_inFrame;
    bool m_frameBuilt;
    bool m_rendererReleased;
    bool m_windowHasCursorShapes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUICONTEXT_H ? */
