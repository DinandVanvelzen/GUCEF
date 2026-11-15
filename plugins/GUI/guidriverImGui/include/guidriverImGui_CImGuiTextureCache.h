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

#ifndef GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H
#define GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

struct ImTextureData;

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/**
 *  The images of a gucefGUI context and the atlases of its bitmap fonts as textures of Dear ImGui: ImTextureData
 *  objects registered with ImGui::RegisterUserTexture(), so that every renderer serves them through the texture requests
 *  of Dear ImGui (RendererHasTextures) and implements texture creation once.
 *
 *  - An image reference (a registered alias or a VFS path) is resolved with CGUIContext::ResolveImage() and its RGBA
 *    pixels are copied into the texture at the first use; a higher revision or CGUIContext::OnImageChanged() marks the
 *    whole texture for an update, a new size makes a new texture.
 *  - An image that is no longer registered has its texture destroyed: Dear ImGui asks the renderer to destroy it the
 *    next frame, after which the cache unregisters and deletes it.
 *  - Bitmap font atlases are cached under the name of the font, renewed when the font or its atlas is replaced.
 *
 *  All calls are made on the ImGui thread with the Dear ImGui context of the owner current.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiTextureCache
{
    public:

    explicit CImGuiTextureCache( GUI::CGUIContext& context );

    /** The textures must have been released with ReleaseAll() */
    ~CImGuiTextureCache();

    /** The texture of an image reference, made or brought up to date; NULL when the reference does not resolve */
    ::ImTextureData* GetImageTexture( const CString& reference );

    /** The texture of the atlas of a bitmap font registered with the context; NULL when there is none */
    ::ImTextureData* GetFontTexture( const CString& fontName );

    /** An image of the context was registered, changed or unregistered */
    void OnImageChanged( const CString& alias );

    /** Before a new frame: deletes the textures Dear ImGui has destroyed on request */
    void Update( void );

    /** Unregisters and deletes every texture; the renderer has destroyed its copies of them before */
    void ReleaseAll( void );

    /** The textures that exist for images and fonts, and those that wait to be destroyed */
    UInt32 GetImageTextureCount( void ) const;
    UInt32 GetFontTextureCount( void ) const;
    UInt32 GetPendingDestroyCount( void ) const;

    private:

    struct SEntry
    {
        ::ImTextureData* texture;
        UInt32 revision;
        const void* source;     /**< the font or atlas a font texture was made from */
        bool stale;

        SEntry( void );
    };
    typedef GUCEF::map< CString, SEntry > TEntryMap;
    typedef GUCEF::vector< ::ImTextureData* > TTextureVector;

    CImGuiTextureCache( const CImGuiTextureCache& src );
    CImGuiTextureCache& operator=( const CImGuiTextureCache& src );

    ::ImTextureData* MakeTexture( const IMAGE::CPixelMap& pixels );
    void CopyPixels( ::ImTextureData& texture, const IMAGE::CPixelMap& pixels );
    void ScheduleDestroy( ::ImTextureData* texture );
    static bool FitsTexture( const ::ImTextureData& texture, const IMAGE::CPixelMap& pixels );

    private:

    GUI::CGUIContext& m_context;
    TEntryMap m_images;
    TEntryMap m_fonts;
    TTextureVector m_dying;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUITEXTURECACHE_H ? */
