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

#ifndef GUIDRIVERIMGUI_CIMGUIFONTLOADER_H
#define GUIDRIVERIMGUI_CIMGUIFONTLOADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#include "gucefGUI_CFontRasterizerRegistry.h"
#define GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#endif /* GUCEF_GUI_CFONTRASTERIZERREGISTRY_H ? */

#ifndef GUIDRIVERIMGUI_MACROS_H
#include "guidriverImGui_macros.h"
#define GUIDRIVERIMGUI_MACROS_H
#endif /* GUIDRIVERIMGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

struct ImFont;
struct ImFontAtlas;
struct ImFontBaked;
struct ImFontConfig;
struct ImFontLoader;

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/**
 *  The font loader of Dear ImGui (ImFontLoader) over the font rasterizers of gucefGUI: ImGui text has the glyph shapes
 *  and coverage of gucefGUI and RmlUi text from the same font (fontpluginFREETYPE), and Dear ImGui's own stb_truetype is
 *  not compiled. One loader object belongs to one font atlas (the atlas of one ImGui context); Attach() makes it the
 *  loader of the atlas and keeps it in the UserData of the atlas.
 *
 *  Sources:
 *  - TrueType or OpenType data (AddFont(), also Dear ImGui's embedded default fonts): rasterized once at a nominal size
 *    to learn which code points exist and the ratio of the em size to the height of a line, then once per size Dear
 *    ImGui bakes, all code points of the ranges of the source at once (Latin-1 and the bullet by default). Dear ImGui's
 *    size is the height of a line: the em size is round( size x rasterizer density x em per line height ).
 *  - a gucefGUI CBitmapFont (AddBitmapFont()): its glyphs at its own line height, other sizes scale the quads.
 *
 *  Glyphs are packed into the atlas of Dear ImGui with their coverage (ImTextureFormat_Alpha8). A code point the source
 *  lacks has no glyph, so Dear ImGui shows its fallback glyph. No LoaderInit or LoaderShutdown: the same loader serves as
 *  the loader of the atlas and of single sources.
 */
class GUIDRIVERIMGUI_PUBLIC_CPP CImGuiFontLoader
{
    public:

    typedef GUI::CFontRasterizer::TCodePointRanges TCodePointRanges;

    /** The nominal em size the sources are rasterized at to learn their metrics */
    static const UInt32 NominalEmSize;

    explicit CImGuiFontLoader( GUI::CFontRasterizerRegistry& registry );

    ~CImGuiFontLoader();

    /** Makes this the font loader of the atlas; done before any font is added */
    void Attach( ::ImFontAtlas& atlas );

    /** The loader object of an atlas, NULL when none is attached */
    static CImGuiFontLoader* GetLoaderOf( const ::ImFontAtlas& atlas );

    /** The functions Dear ImGui calls */
    static const ::ImFontLoader* GetFontLoader( void );

    /** Whether a font rasterizer is registered: without one no outline font can be used */
    bool HasRasterizer( void ) const;

    /** Adds an outline font from its data at a size (the height of a line in logical pixels); NULL when it cannot be read */
    ::ImFont* AddFont( ::ImFontAtlas& atlas, const CORE::CDynamicBuffer& fontData, Float32 sizePixels, const TCodePointRanges& ranges, const CString& name );

    /** Adds an outline font file read through the VFS */
    ::ImFont* AddFont( ::ImFontAtlas& atlas, const CString& vfsPath, Float32 sizePixels, const TCodePointRanges& ranges );

    /** Adds a gucefGUI bitmap font, at its line height */
    ::ImFont* AddBitmapFont( ::ImFontAtlas& atlas, const GUI::TBitmapFontPtr& font, const CString& name );

    /** The bitmap font a source of a baked font was rasterized into (the font itself for a bitmap source); NULL when none */
    static GUI::TBitmapFontPtr GetBakedFont( ::ImFontBaked& baked, UInt32 sourceIndex );

    /** The em size per height of a line that was measured for an outline source; 0 for other sources */
    static Float32 GetEmPerLineHeight( const ::ImFontConfig& source );

    /** How many times a font was rasterized for this atlas */
    UInt32 GetRasterizeCount( void ) const;

    private:

    friend class CImGuiFontLoaderFunctions;

    struct SSource
    {
        bool isBitmap;
        CORE::CDynamicBuffer data;      /**< linked to the font data Dear ImGui keeps, not a copy */
        TCodePointRanges ranges;
        Float32 emPerLineHeight;
        GUI::TBitmapFontPtr font;       /**< the nominal rasterization, or the bitmap font */

        SSource( void );
    };

    struct SBaked
    {
        GUI::TBitmapFontPtr font;
        Float32 metricScale;            /**< from the metrics of the font (offsets, advance) to logical pixels */
        Float32 sizeScale;              /**< from atlas pixels of the font to logical pixels */

        SBaked( void );
    };

    typedef GUCEF::map< UInt32, GUI::TBitmapFontPtr > TBitmapFontMap;
    typedef GUCEF::vector< void* > TPointerVector;

    CImGuiFontLoader( const CImGuiFontLoader& src );
    CImGuiFontLoader& operator=( const CImGuiFontLoader& src );

    const void* MakeGlyphRanges( const TCodePointRanges& ranges );
    static TCodePointRanges ReadGlyphRanges( const ::ImFontConfig& source );
    GUI::TBitmapFontPtr Rasterize( const CORE::CDynamicBuffer& data, UInt32 emSize, const TCodePointRanges& ranges );

    private:

    GUI::CFontRasterizerRegistry& m_registry;
    TBitmapFontMap m_bitmapFonts;
    TPointerVector m_glyphRanges;
    UInt32 m_nextBitmapFontId;
    UInt32 m_rasterizeCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUI_CIMGUIFONTLOADER_H ? */
