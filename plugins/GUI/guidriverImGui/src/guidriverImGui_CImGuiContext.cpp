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

#include "imgui.h"
#include "imgui_internal.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#include "gucefGUI_CFontRasterizerRegistry.h"
#define GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#endif /* GUCEF_GUI_CFONTRASTERIZERREGISTRY_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#include "guidriverImGui_CImGuiGlobal.h"
#define GUIDRIVERIMGUI_CIMGUIGLOBAL_H
#endif /* GUIDRIVERIMGUI_CIMGUIGLOBAL_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIDRIVER_H
#include "guidriverImGui_CImGuiDriver.h"
#define GUIDRIVERIMGUI_CIMGUIDRIVER_H
#endif /* GUIDRIVERIMGUI_CIMGUIDRIVER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#include "guidriverImGui_CImGuiContextScope.h"
#define GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H
#endif /* GUIDRIVERIMGUI_CIMGUICONTEXTSCOPE_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H
#include "guidriverImGui_CImGuiInputMapper.h"
#define GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H
#endif /* GUIDRIVERIMGUI_CIMGUIINPUTMAPPER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIFONTLOADER_H
#include "guidriverImGui_CImGuiFontLoader.h"
#define GUIDRIVERIMGUI_CIMGUIFONTLOADER_H
#endif /* GUIDRIVERIMGUI_CIMGUIFONTLOADER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H
#include "guidriverImGui_CImGuiTextureCache.h"
#define GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H
#endif /* GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H
#include "guidriverImGui_CImGuiSoftwareRenderer.h"
#define GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H
#endif /* GUIDRIVERIMGUI_CIMGUISOFTWARERENDERER_H ? */

#ifndef GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H
#include "guidriverImGui_CImGuiDrawListPainter.h"
#define GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H
#endif /* GUIDRIVERIMGUI_CIMGUIDRAWLISTPAINTER_H ? */

#include "guidriverImGui_CImGuiContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CImGuiContext::BuildFrameEvent = "GUCEF::GUIDRIVERIMGUI::CImGuiContext::BuildFrameEvent";
const CORE::CEvent CImGuiContext::IniSettingsChangedEvent = "GUCEF::GUIDRIVERIMGUI::CImGuiContext::IniSettingsChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The functions of the platform IO of Dear ImGui, with access to the context */
class CImGuiContextPlatformFunctions
{
    public:

    static CImGuiContext* ContextOf( ImGuiContext* imguiContext, void* userData )
    {
        return static_cast< CImGuiContext* >( userData );
    }

    static const char* GetClipboardText( ImGuiContext* imguiContext )
    {
        CImGuiContext* context = ContextOf( imguiContext, imguiContext->PlatformIO.Platform_ClipboardUserData );
        if ( GUCEF_NULL == context )
            return GUCEF_NULL;
        // the clipboard of gucefGUI, which the widgets and the test hooks share; kept until the next call
        if ( !GUI::CSecretInput::ReadClipboardText( context->m_clipboardText ) )
            return GUCEF_NULL;
        return context->m_clipboardText.C_String();
    }

    static void SetClipboardText( ImGuiContext* imguiContext, const char* text )
    {
        CImGuiContext* context = ContextOf( imguiContext, imguiContext->PlatformIO.Platform_ClipboardUserData );
        if ( GUCEF_NULL == context || GUCEF_NULL == text )
            return;
        GUI::CSecretInput::WriteClipboardText( CString( text ) );
    }

    static void SetImeData( ImGuiContext* imguiContext, ImGuiViewport* viewport, ImGuiPlatformImeData* data )
    {
        CImGuiContext* context = ContextOf( imguiContext, imguiContext->PlatformIO.Platform_ImeUserData );
        if ( GUCEF_NULL == context || GUCEF_NULL == data )
            return;
        // kept for a text input area of a window driver, which gucefGUI does not have yet
        CImGuiContext::TTextInputState& state = context->m_textInput;
        state.wantVisible = data->WantVisible;
        state.wantTextInput = data->WantTextInput;
        state.inputX = data->InputPos.x;
        state.inputY = data->InputPos.y;
        state.lineHeight = data->InputLineHeight;
        ++state.updates;
    }

    static void OnError( ImGuiContext* imguiContext, void* userData, const char* message )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: " << ( GUCEF_NULL != message ? message : "" ) );
    }

    /*
     *  The standard draw callbacks of the default renderer: they do nothing themselves, the software rasterizer knows them
     */
    static void SetSamplerLinear( const ImDrawList* drawList, const ImDrawCmd* command ) {}
    static void SetSamplerNearest( const ImDrawList* drawList, const ImDrawCmd* command ) {}
    static void ResetRenderState( const ImDrawList* drawList, const ImDrawCmd* command ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CImGuiContext::RegisterEvents( void )
{GUCEF_TRACE;

    BuildFrameEvent.Initialize();
    IniSettingsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CImGuiContext::STextInputState::STextInputState( void )
    : wantVisible( false )
    , wantTextInput( false )
    , inputX( 0.0f )
    , inputY( 0.0f )
    , lineHeight( 0.0f )
    , updates( 0 )
{
}

/*-------------------------------------------------------------------------*/

CImGuiContext::CImGuiContext( CImGuiDriver* driver, GUI::TWindowContextPtr windowContext )
    : GUI::CGUIContext( driver, windowContext )
    , m_imguiDriver( driver )
    , m_imguiContext( GUCEF_NULL )
    , m_fontLoader( GUCEF_NULL )
    , m_textureCache( GUCEF_NULL )
    , m_softwareRenderer( GUCEF_NULL )
    , m_unscaledStyle( GUCEF_NULL )
    , m_uiScale( 1.0f )
    , m_styleName()
    , m_clipboardText()
    , m_textInput()
    , m_lastFrameTime( 0 )
    , m_frameCount( 0 )
    , m_imageGeneration( 0 )
    , m_registered( false )
    , m_ready( false )
    , m_inFrame( false )
    , m_frameBuilt( false )
    , m_rendererReleased( false )
    , m_windowHasCursorShapes( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_ready = Initialize();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::Initialize( void )
{GUCEF_TRACE;

    // one current context per process: every ImGui context lives on the thread of the first one
    m_registered = CImGuiGlobal::Instance()->RegisterContext( this );
    if ( !m_registered )
        return false;

    GUI::CFontRasterizerRegistry& rasterizers = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry();
    if ( !rasterizers.HasRasterizers() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: No font rasterizer is registered, an ImGui context needs one: load the plugin fontpluginFREETYPE" );
        return false;
    }

    // the data layout this library was built with must be the one of the imgui module (32-bit indices and characters)
    if ( !ImGui::DebugCheckVersionAndDataLayout( IMGUI_VERSION, sizeof( ImGuiIO ), sizeof( ImGuiStyle ), sizeof( ImVec2 ), sizeof( ImVec4 ), sizeof( ImDrawVert ), sizeof( ImDrawIdx ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: The imgui module does not match the headers guidriverImGui was built with" );
        return false;
    }

    m_imguiContext = ImGui::CreateContext();
    if ( GUCEF_NULL == m_imguiContext )
        return false;
    CImGuiGlobal::Instance()->SetImGuiContext( this, m_imguiContext );

    CImGuiContextScope scope( m_imguiContext );
    const CImGuiDriver::TSettings settings = GUCEF_NULL != m_imguiDriver ? m_imguiDriver->GetSettings() : CImGuiDriver::TSettings();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = GUCEF_NULL;        // no files: the application stores the settings (IniSettingsChangedEvent)
    io.LogFilename = GUCEF_NULL;
    io.BackendPlatformName = "gucefGUI";
    io.BackendRendererName = "gucefGUI software";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    if ( settings.keyboardNavigation )
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.MouseDrawCursor = settings.softwareCursor;
    io.ConfigErrorRecoveryEnableAssert = settings.configErrorRecoveryAsserts;
    #ifdef GUCEF_DEBUG_MODE
    io.ConfigErrorRecoveryEnableTooltip = true;
    #else
    io.ConfigErrorRecoveryEnableTooltip = false;
    #endif
    io.ConfigErrorRecoveryEnableDebugLog = true;

    // the pointer shapes of the window: asked once whether it has them
    GUI::TWindowContextPtr window = GetWindowContext();
    if ( !window.IsNULL() )
    {
        m_windowHasCursorShapes = window->SetSystemCursorShape( GUI::CURSORSHAPE_ARROW );
        TEventCallback activationCallback( this, &CImGuiContext::OnWindowActivation );
        SubscribeTo( window.GetPointerAlways(), GUI::CWindowContext::WindowContextActivateEvent, activationCallback );
    }
    if ( m_windowHasCursorShapes )
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    platformIO.Platform_GetClipboardTextFn = &CImGuiContextPlatformFunctions::GetClipboardText;
    platformIO.Platform_SetClipboardTextFn = &CImGuiContextPlatformFunctions::SetClipboardText;
    platformIO.Platform_ClipboardUserData = this;
    platformIO.Platform_SetImeDataFn = &CImGuiContextPlatformFunctions::SetImeData;
    platformIO.Platform_ImeUserData = this;
    platformIO.Platform_OpenInShellFn = GUCEF_NULL;
    platformIO.DrawCallback_SetSamplerLinear = &CImGuiContextPlatformFunctions::SetSamplerLinear;
    platformIO.DrawCallback_SetSamplerNearest = &CImGuiContextPlatformFunctions::SetSamplerNearest;
    platformIO.DrawCallback_ResetRenderState = &CImGuiContextPlatformFunctions::ResetRenderState;
    m_imguiContext->ErrorCallback = &CImGuiContextPlatformFunctions::OnError;
    m_imguiContext->ErrorCallbackUserData = this;

    m_softwareRenderer = GUCEF_NEW CImGuiSoftwareRenderer();
    m_textureCache = GUCEF_NEW CImGuiTextureCache( *this );

    // fonts through the font rasterizers of gucefGUI, set right after the context is made
    m_fontLoader = GUCEF_NEW CImGuiFontLoader( rasterizers );
    m_fontLoader->Attach( *io.Fonts );
    ImFont* font = GUCEF_NULL;
    if ( !settings.fontPath.IsNULLOrEmpty() )
    {
        if ( settings.fontPath == GUI::CGuiGlobal::DefaultUiFontPath )
            GUI::CGuiGlobal::Instance()->MountData();
        font = AddFont( settings.fontPath, settings.fontSize );
    }
    if ( GUCEF_NULL == font )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The font \"" << settings.fontPath << "\" cannot be used, the embedded font of Dear ImGui is used instead" );
        font = AddDefaultFont();
    }
    if ( GUCEF_NULL == font )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: No font can be rasterized for the context" );
        return false;
    }
    if ( GUCEF_NULL != m_imguiDriver )
    {
        GUI::CGUIDriver::TStringSet fonts = m_imguiDriver->GetAvailableFonts();
        for ( GUI::CGUIDriver::TStringSet::iterator i=fonts.begin(); i!=fonts.end(); ++i )
            AddFont( (*i), settings.fontSize );
    }

    // the style: the colours by name, the sizes by the UI scale, from a kept unscaled copy (ScaleAllSizes() is cumulative)
    m_unscaledStyle = GUCEF_NEW ImGuiStyle();
    m_unscaledStyle->FontSizeBase = settings.fontSize;
    m_uiScale = settings.uiScale > 0.0f ? settings.uiScale : 1.0f;
    if ( !SetStyle( settings.style ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The style \"" << settings.style << "\" is not Dark, Light or Classic: Dark is used" );
        SetStyle( "Dark" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CImGuiContext::~CImGuiContext()
{GUCEF_TRACE;

    // the release order: widgets, renderer (with the GPU textures), the textures of the cache, then Dear ImGui
    DestroyAllWidgets();
    if ( GUCEF_NULL != m_imguiContext )
    {
        ShutdownRenderer();
        if ( GUCEF_NULL != m_textureCache )
        {
            CImGuiContextScope scope( m_imguiContext );
            m_textureCache->ReleaseAll();
        }
        if ( m_registered )
            CImGuiGlobal::Instance()->SetImGuiContext( this, GUCEF_NULL );
        ImGui::DestroyContext( m_imguiContext );
        m_imguiContext = GUCEF_NULL;
    }
    GUCEF_DELETE m_textureCache;
    m_textureCache = GUCEF_NULL;
    GUCEF_DELETE m_fontLoader;
    m_fontLoader = GUCEF_NULL;
    GUCEF_DELETE m_softwareRenderer;
    m_softwareRenderer = GUCEF_NULL;
    GUCEF_DELETE m_unscaledStyle;
    m_unscaledStyle = GUCEF_NULL;
    if ( m_registered )
    {
        m_registered = false;
        CImGuiGlobal::Instance()->UnregisterContext( this );
    }
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::CheckThread( const char* action ) const
{GUCEF_TRACE;

    return CImGuiGlobal::HasInstance() && CImGuiGlobal::Instance()->CheckImGuiThread( action );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::IsReady( void ) const
{GUCEF_TRACE;

    return m_ready;
}

/*-------------------------------------------------------------------------*/

::ImGuiContext*
CImGuiContext::GetImGuiContext( void ) const
{GUCEF_TRACE;

    return m_imguiContext;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiContext::GetBuiltFrameCount( void ) const
{GUCEF_TRACE;

    return m_frameCount;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::IsBuildingFrame( void ) const
{GUCEF_TRACE;

    return m_inFrame;
}

/*-------------------------------------------------------------------------*/

ImDrawData*
CImGuiContext::GetDrawData( void )
{GUCEF_TRACE;

    if ( !m_ready || !m_frameBuilt )
        return GUCEF_NULL;
    CImGuiContextScope scope( m_imguiContext );
    return ImGui::GetDrawData();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::GetIniSettings( CString& settings ) const
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "the settings of an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    size_t size = 0;
    const char* text = ImGui::SaveIniSettingsToMemory( &size );
    settings = GUCEF_NULL != text ? CString( text, (UInt32) size ) : CString();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::SetIniSettings( const CString& settings )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "the settings of an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    ImGui::LoadIniSettingsFromMemory( settings.C_String(), (size_t) settings.ByteSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

const CImGuiContext::TTextInputState&
CImGuiContext::GetTextInputState( void ) const
{GUCEF_TRACE;

    return m_textInput;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::ApplyStyle( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imguiContext || GUCEF_NULL == m_unscaledStyle )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    ImGuiStyle style = *m_unscaledStyle;
    style.ScaleAllSizes( m_uiScale );
    style.FontScaleDpi = m_uiScale;
    ImGui::GetStyle() = style;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiContext::SetUiScale( Float32 scale )
{GUCEF_TRACE;

    if ( scale <= 0.0f || !CheckThread( "the scale of an ImGui context" ) )
        return;
    m_uiScale = scale;
    ApplyStyle();
}

/*-------------------------------------------------------------------------*/

Float32
CImGuiContext::GetUiScale( void ) const
{GUCEF_TRACE;

    return m_uiScale;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::SetStyle( const CString& styleName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_unscaledStyle || !CheckThread( "the style of an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    CString name = styleName.Lowercase();
    if ( "dark" == name )
        ImGui::StyleColorsDark( m_unscaledStyle );
    else
    if ( "light" == name )
        ImGui::StyleColorsLight( m_unscaledStyle );
    else
    if ( "classic" == name )
        ImGui::StyleColorsClassic( m_unscaledStyle );
    else
        return false;
    m_styleName = styleName;
    return ApplyStyle();
}

/*-------------------------------------------------------------------------*/

const CString&
CImGuiContext::GetStyle( void ) const
{GUCEF_TRACE;

    return m_styleName;
}

/*-------------------------------------------------------------------------*/

ImFont*
CImGuiContext::AddFont( const CString& vfsPath, Float32 sizePixels )
{GUCEF_TRACE;

    return AddFont( vfsPath, sizePixels, GUI::CFontRasterizer::GetUiRanges() );
}

/*-------------------------------------------------------------------------*/

ImFont*
CImGuiContext::AddFont( const CString& vfsPath, Float32 sizePixels, const TCodePointRanges& ranges )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_fontLoader || GUCEF_NULL == m_imguiContext || !CheckThread( "a font of an ImGui context" ) )
        return GUCEF_NULL;
    CImGuiContextScope scope( m_imguiContext );
    return m_fontLoader->AddFont( *ImGui::GetIO().Fonts, vfsPath, sizePixels, ranges );
}

/*-------------------------------------------------------------------------*/

ImFont*
CImGuiContext::AddBitmapFont( const CString& fontName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_fontLoader || GUCEF_NULL == m_imguiContext || !CheckThread( "a font of an ImGui context" ) )
        return GUCEF_NULL;
    GUI::TBitmapFontPtr font = GetBitmapFont( fontName );
    if ( font.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: No bitmap font \"" << fontName << "\" is registered with the context" );
        return GUCEF_NULL;
    }
    CImGuiContextScope scope( m_imguiContext );
    return m_fontLoader->AddBitmapFont( *ImGui::GetIO().Fonts, font, fontName );
}

/*-------------------------------------------------------------------------*/

ImFont*
CImGuiContext::AddDefaultFont( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_fontLoader || GUCEF_NULL == m_imguiContext || !CheckThread( "a font of an ImGui context" ) )
        return GUCEF_NULL;
    CImGuiContextScope scope( m_imguiContext );
    // the embedded ProggyClean data, decompressed by Dear ImGui and rasterized by the font loader
    return ImGui::GetIO().Fonts->AddFontDefaultBitmap();
}

/*-------------------------------------------------------------------------*/

CImGuiFontLoader&
CImGuiContext::GetFontLoader( void )
{GUCEF_TRACE;

    return *m_fontLoader;
}

/*-------------------------------------------------------------------------*/

CImGuiTextureCache&
CImGuiContext::GetTextureCache( void )
{GUCEF_TRACE;

    return *m_textureCache;
}

/*-------------------------------------------------------------------------*/

ImTextureRef
CImGuiContext::GetImageTextureRef( const CString& reference )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "an image of an ImGui context" ) )
        return ImTextureRef();
    CImGuiContextScope scope( m_imguiContext );
    ImTextureData* texture = m_textureCache->GetImageTexture( reference );
    return GUCEF_NULL != texture ? texture->GetTexRef() : ImTextureRef();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::Image( const CString& reference )
{GUCEF_TRACE;

    if ( !m_ready || !m_inFrame )
        return false;
    ImTextureData* texture = m_textureCache->GetImageTexture( reference );
    if ( GUCEF_NULL == texture )
        return false;
    const Float32 density = (Float32) GetImageDensity( reference );
    ImGui::Image( texture->GetTexRef(), ImVec2( texture->Width / density, texture->Height / density ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::GetBackendImageTextureRef( const CString& reference, ImTextureRef& textureRef ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiContext::GetImageGeneration( void ) const
{GUCEF_TRACE;

    return m_imageGeneration;
}

/*-------------------------------------------------------------------------*/

CImGuiSoftwareRenderer&
CImGuiContext::GetSoftwareRenderer( void )
{GUCEF_TRACE;

    return *m_softwareRenderer;
}

/*-------------------------------------------------------------------------*/

CImGuiContext::TDrawDataMapping
CImGuiContext::GetTargetMapping( UInt32 width, UInt32 height ) const
{GUCEF_TRACE;

    const Float32 logicalWidth = (Float32) GetLogicalWidth();
    const Float32 logicalHeight = (Float32) GetLogicalHeight();
    TDrawDataMapping mapping;
    mapping.displayPos = ImVec2( 0.0f, 0.0f );
    mapping.displaySize = ImVec2( logicalWidth, logicalHeight );
    mapping.framebufferScale = ImVec2( logicalWidth > 0.0f ? width / logicalWidth : 1.0f, logicalHeight > 0.0f ? height / logicalHeight : 1.0f );
    return mapping;
}

/*-------------------------------------------------------------------------*/

CImGuiContext::TDrawDataMapping
CImGuiContext::GetPresentMapping( void ) const
{GUCEF_TRACE;

    GUI::TWindowContextPtr window = GetWindowContext();
    if ( window.IsNULL() )
        return GetTargetMapping( GetRenderWidth(), GetRenderHeight() );

    // logical (0,0) at the origin of the present rectangle, at the present scale per axis, over the whole window
    const GUI::CRect present = GetPresentRect();
    const Float32 logicalWidth = (Float32) GetLogicalWidth();
    const Float32 logicalHeight = (Float32) GetLogicalHeight();
    TDrawDataMapping mapping;
    const Float32 scaleX = logicalWidth > 0.0f && present.width > 0 ? present.width / logicalWidth : 1.0f;
    const Float32 scaleY = logicalHeight > 0.0f && present.height > 0 ? present.height / logicalHeight : 1.0f;
    mapping.framebufferScale = ImVec2( scaleX, scaleY );
    mapping.displaySize = ImVec2( window->GetWidth() / scaleX, window->GetHeight() / scaleY );
    mapping.displayPos = ImVec2( -present.left / scaleX, -present.top / scaleY );
    return mapping;
}

/*-------------------------------------------------------------------------*/

CImGuiContext::TDrawDataMapping
CImGuiContext::GetDrawDataMapping( const ImDrawData& drawData )
{GUCEF_TRACE;

    TDrawDataMapping mapping;
    mapping.displayPos = drawData.DisplayPos;
    mapping.displaySize = drawData.DisplaySize;
    mapping.framebufferScale = drawData.FramebufferScale;
    return mapping;
}

/*-------------------------------------------------------------------------*/

void
CImGuiContext::SetDrawDataMapping( ImDrawData& drawData, const TDrawDataMapping& mapping )
{GUCEF_TRACE;

    drawData.DisplayPos = mapping.displayPos;
    drawData.DisplaySize = mapping.displaySize;
    drawData.FramebufferScale = mapping.framebufferScale;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::IsTakingPointerInput( void ) const
{GUCEF_TRACE;

    if ( GUI::CGUIContext::IsTakingPointerInput() )
        return true;
    return m_ready && m_imguiContext->IO.WantCaptureMouse;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::IsTakingKeyboardInput( void ) const
{GUCEF_TRACE;

    if ( GUI::CGUIContext::IsTakingKeyboardInput() )
        return true;
    return m_ready && !IsKeyboardInputPassThrough() && m_imguiContext->IO.WantCaptureKeyboard;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::PreviewMouseMove( Int32 x, Int32 y )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "a pointer move for an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent( (float) x, (float) y );
    return io.WantCaptureMouse;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::PreviewMouseButton( GUI::TMouseButton button, bool down )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "a mouse button for an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    ImGuiIO& io = ImGui::GetIO();
    const Int32 imguiButton = CImGuiInputMapper::ToImGuiMouseButton( button );
    if ( imguiButton >= 0 )
        io.AddMouseButtonEvent( imguiButton, down );
    return io.WantCaptureMouse;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::PreviewMouseWheel( Int32 delta )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "the wheel for an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent( 0.0f, (float) delta );
    return io.WantCaptureMouse;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::PreviewKey( INPUT::KeyCode keyCode, bool down, UInt32 modifiers )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "a key for an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    ImGuiIO& io = ImGui::GetIO();
    CImGuiInputMapper::QueueModifiers( io, modifiers );
    const ImGuiKey key = CImGuiInputMapper::ToImGuiKey( keyCode );
    if ( ImGuiKey_None != key )
        io.AddKeyEvent( key, down );
    return io.WantCaptureKeyboard;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::PreviewText( const CString& text )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "text for an ImGui context" ) )
        return false;
    CImGuiContextScope scope( m_imguiContext );
    ImGuiIO& io = ImGui::GetIO();
    if ( !text.IsNULLOrEmpty() )
        io.AddInputCharactersUTF8( text.C_String() );
    return io.WantTextInput;
}

/*-------------------------------------------------------------------------*/

void
CImGuiContext::OnWindowActivation( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_ready || !CheckThread( "the focus of an ImGui context" ) )
        return;
    // the window says whether it is active now (window drivers notify the event without data)
    GUI::TWindowContextPtr window = GetWindowContext();
    const bool active = !window.IsNULL() && window->IsActive();
    CImGuiContextScope scope( m_imguiContext );
    ImGui::GetIO().AddFocusEvent( active );
}

/*-------------------------------------------------------------------------*/

void
CImGuiContext::OnImageChanged( const CString& alias )
{GUCEF_TRACE;

    ++m_imageGeneration;
    if ( !m_ready || GUCEF_NULL == m_textureCache || !CheckThread( "an image of an ImGui context" ) )
        return;
    CImGuiContextScope scope( m_imguiContext );
    m_textureCache->OnImageChanged( alias );
}

/*-------------------------------------------------------------------------*/

void
CImGuiContext::OnRender( void )
{GUCEF_TRACE;

    if ( !m_ready || m_inFrame || !CheckThread( "a frame of an ImGui context" ) )
        return;

    CImGuiContextScope scope( m_imguiContext );
    ImGuiIO& io = ImGui::GetIO();

    // the logical size, the present scale (the render scale without a window) and the time since the last frame
    const Float32 logicalWidth = (Float32) GetLogicalWidth();
    const Float32 logicalHeight = (Float32) GetLogicalHeight();
    io.DisplaySize = ImVec2( logicalWidth, logicalHeight );
    io.DisplayFramebufferScale = GetPresentMapping().framebufferScale;
    const UInt64 now = GetTimeInMs();
    const UInt32 delta = (UInt32) ( now - m_lastFrameTime );
    m_lastFrameTime = now;
    io.DeltaTime = ( delta < 1 ? 1 : delta ) / 1000.0f;

    m_textureCache->Update();
    if ( !OnRendererNewFrame() )
        return;
    ImGui::NewFrame();
    m_inFrame = true;

    // the retained widgets below the ImGui windows, the overlay of gucefGUI above them
    {
        CImGuiDrawListPainter background( *this, *ImGui::GetBackgroundDrawList() );
        background.PaintRoots();
    }
    {
        CImGuiDrawListPainter foreground( *this, *ImGui::GetForegroundDrawList() );
        foreground.PaintOverlay();
    }

    // the immediate-mode content of the application
    ++m_frameCount;
    CORE::TCloneableUInt32 eventData( delta );
    if ( !NotifyObservers( BuildFrameEvent, &eventData ) )
        return;

    ImGui::Render();
    m_inFrame = false;
    m_frameBuilt = true;

    // the pointer shape Dear ImGui asks for, shown by the window while this layer takes the pointer
    if ( m_windowHasCursorShapes && !io.MouseDrawCursor )
        SetSystemCursorShape( CImGuiInputMapper::ToCursorShape( ImGui::GetMouseCursor() ) );

    RenderDrawData( *ImGui::GetDrawData() );

    if ( io.WantSaveIniSettings )
    {
        io.WantSaveIniSettings = false;
        NotifyObservers( IniSettingsChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::OnRendererNewFrame( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::RenderDrawData( ImDrawData& drawData )
{GUCEF_TRACE;

    // no GPU: the textures keep their pixels in memory, a capture draws them on the CPU
    m_softwareRenderer->ServeTextureRequests( drawData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::RenderDrawDataOffscreen( ImDrawData& drawData, UInt32 width, UInt32 height, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    return m_softwareRenderer->Render( drawData, ImGui::GetPlatformIO(), width, height, true, pixels );
}

/*-------------------------------------------------------------------------*/

void
CImGuiContext::ReleaseRenderer( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_softwareRenderer )
        m_softwareRenderer->DestroyTextures( ImGui::GetPlatformIO().Textures );
}

/*-------------------------------------------------------------------------*/

void
CImGuiContext::ShutdownRenderer( void )
{GUCEF_TRACE;

    if ( m_rendererReleased || GUCEF_NULL == m_imguiContext )
        return;
    m_rendererReleased = true;
    CImGuiContextScope scope( m_imguiContext );
    ReleaseRenderer();
}

/*-------------------------------------------------------------------------*/

bool
CImGuiContext::CaptureFrame( IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !m_ready || !m_frameBuilt || m_inFrame || !CheckThread( "a capture of an ImGui context" ) )
        return false;
    const UInt32 width = GetRenderWidth();
    const UInt32 height = GetRenderHeight();
    if ( 0 == width || 0 == height )
        return false;

    CImGuiContextScope scope( m_imguiContext );
    ImDrawData* drawData = ImGui::GetDrawData();
    if ( GUCEF_NULL == drawData || !drawData->Valid )
        return false;

    // the last frame again, at the render size: no new frame, so Dear ImGui's time and state do not move
    const TDrawDataMapping previous = GetDrawDataMapping( *drawData );
    SetDrawDataMapping( *drawData, GetTargetMapping( width, height ) );
    const bool captured = RenderDrawDataOffscreen( *drawData, width, height, pixels );
    SetDrawDataMapping( *drawData, previous );
    return captured;
}

/*-------------------------------------------------------------------------*/

const CString&
CImGuiContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUIDRIVERIMGUI::CImGuiContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
