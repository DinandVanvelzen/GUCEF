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

#include <string.h>

#include "imgui.h"
#include "imgui_internal.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "guidriverImGui_CImGuiFontLoader.h"

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

const UInt32 CImGuiFontLoader::NominalEmSize = 64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** What the font data of a bitmap font source holds: a marker no font file starts with, and the font of the loader */
struct SBitmapFontHandle
{
    char marker[ 8 ];
    UInt32 id;
};

const char BitmapFontMarker[ 8 ] = { 'G', 'U', 'C', 'E', 'F', 'B', 'F', '\0' };

bool
IsBitmapFontHandle( const void* data, int size )
{
    return GUCEF_NULL != data && size == (int) sizeof( SBitmapFontHandle ) &&
           0 == memcmp( static_cast< const SBitmapFontHandle* >( data )->marker, BitmapFontMarker, sizeof( BitmapFontMarker ) );
}

}

/*-------------------------------------------------------------------------*/

/** The functions Dear ImGui calls, with access to the loader */
class CImGuiFontLoaderFunctions
{
    public:

    static bool FontSrcInit( ImFontAtlas* atlas, ImFontConfig* src )
    {
        CImGuiFontLoader* loader = CImGuiFontLoader::GetLoaderOf( *atlas );
        if ( GUCEF_NULL == loader || GUCEF_NULL == src->FontData || src->FontDataSize <= 0 )
            return false;

        CImGuiFontLoader::SSource* source = GUCEF_NEW CImGuiFontLoader::SSource();
        if ( IsBitmapFontHandle( src->FontData, src->FontDataSize ) )
        {
            const SBitmapFontHandle* handle = static_cast< const SBitmapFontHandle* >( src->FontData );
            CImGuiFontLoader::TBitmapFontMap::iterator i = loader->m_bitmapFonts.find( handle->id );
            if ( i == loader->m_bitmapFonts.end() || (*i).second.IsNULL() )
            {
                GUCEF_DELETE source;
                return false;
            }
            source->isBitmap = true;
            source->font = (*i).second;
        }
        else
        {
            // the data stays with the atlas for as long as the font exists: linked, not copied
            source->data.LinkTo( src->FontData, (UInt32) src->FontDataSize );
            source->ranges = CImGuiFontLoader::ReadGlyphRanges( *src );
            source->font = loader->Rasterize( source->data, CImGuiFontLoader::NominalEmSize, source->ranges );
            if ( source->font.IsNULL() || source->font->GetLineHeight() <= 0 )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The font \"" << src->Name << "\" cannot be read by a font rasterizer" );
                GUCEF_DELETE source;
                return false;
            }
            source->emPerLineHeight = (Float32) CImGuiFontLoader::NominalEmSize / (Float32) source->font->GetLineHeight();
        }
        src->FontLoaderData = source;
        return true;
    }

    static void FontSrcDestroy( ImFontAtlas* atlas, ImFontConfig* src )
    {
        CImGuiFontLoader::SSource* source = static_cast< CImGuiFontLoader::SSource* >( src->FontLoaderData );
        GUCEF_DELETE source;
        src->FontLoaderData = GUCEF_NULL;
    }

    static int FontSrcGetGlyphIndexFromCodepoint( ImFontAtlas* atlas, ImFontConfig* src, ImWchar codepoint )
    {
        // the glyphs are known by their code point: the index is the code point, 0 when the source has no glyph for it
        const CImGuiFontLoader::SSource* source = static_cast< const CImGuiFontLoader::SSource* >( src->FontLoaderData );
        if ( GUCEF_NULL == source || source->font.IsNULL() || 0 == codepoint )
            return 0;
        return GUCEF_NULL != source->font->GetGlyph( (UInt32) codepoint ) ? (int) codepoint : 0;
    }

    static bool FontSrcContainsGlyph( ImFontAtlas* atlas, ImFontConfig* src, int glyphIndex )
    {
        const CImGuiFontLoader::SSource* source = static_cast< const CImGuiFontLoader::SSource* >( src->FontLoaderData );
        return GUCEF_NULL != source && !source->font.IsNULL() && glyphIndex > 0 && GUCEF_NULL != source->font->GetGlyph( (UInt32) glyphIndex );
    }

    static bool FontBakedInit( ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loaderData )
    {
        CImGuiFontLoader* loader = CImGuiFontLoader::GetLoaderOf( *atlas );
        CImGuiFontLoader::SSource* source = static_cast< CImGuiFontLoader::SSource* >( src->FontLoaderData );
        CImGuiFontLoader::SBaked** slot = static_cast< CImGuiFontLoader::SBaked** >( loaderData );
        *slot = GUCEF_NULL;
        if ( GUCEF_NULL == loader || GUCEF_NULL == source || source->font.IsNULL() )
            return false;

        CImGuiFontLoader::SBaked* bakedFont = GUCEF_NEW CImGuiFontLoader::SBaked();
        if ( source->isBitmap )
        {
            // the bitmap font at its own line height; other sizes scale the quads
            bakedFont->font = source->font;
            const Int32 lineHeight = source->font->GetLineHeight();
            const Float32 scale = lineHeight > 0 ? baked->Size / (Float32) lineHeight : 1.0f;
            bakedFont->metricScale = scale;
            bakedFont->sizeScale = scale / (Float32) ( 0 == source->font->GetDensity() ? 1 : source->font->GetDensity() );
        }
        else
        {
            // rasterized at the density the font is shown at, the metrics then are pixels of that density
            Float32 density = src->RasterizerDensity * baked->RasterizerDensity;
            if ( density <= 0.0f )
                density = 1.0f;
            UInt32 emSize = (UInt32) ( baked->Size * density * source->emPerLineHeight + 0.5f );
            if ( 0 == emSize )
                emSize = 1;
            bakedFont->font = loader->Rasterize( source->data, emSize, source->ranges );
            bakedFont->metricScale = 1.0f / density;
            bakedFont->sizeScale = 1.0f / density;
            if ( bakedFont->font.IsNULL() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The font \"" << src->Name << "\" cannot be rasterized at " << emSize << " pixels" );
                GUCEF_DELETE bakedFont;
                return false;
            }
        }
        *slot = bakedFont;

        if ( !src->MergeMode )
        {
            const Float32 baseline = (Float32) bakedFont->font->GetBaseline() * bakedFont->metricScale;
            const Float32 lineHeight = (Float32) bakedFont->font->GetLineHeight() * bakedFont->metricScale;
            baked->Ascent = baseline;
            baked->Descent = baseline - lineHeight;
        }
        return true;
    }

    static void FontBakedDestroy( ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loaderData )
    {
        CImGuiFontLoader::SBaked** slot = static_cast< CImGuiFontLoader::SBaked** >( loaderData );
        if ( GUCEF_NULL == slot )
            return;
        GUCEF_DELETE *slot;
        *slot = GUCEF_NULL;
    }

    static bool FontBakedLoadGlyph( ImFontAtlas* atlas, ImFontConfig* src, ImFontBaked* baked, void* loaderData, int glyphIndex,
                                    ImFontGlyph* outGlyph, float* outAdvanceX )
    {
        const CImGuiFontLoader::SBaked* bakedFont = *static_cast< CImGuiFontLoader::SBaked** >( loaderData );
        if ( GUCEF_NULL == bakedFont || bakedFont->font.IsNULL() || glyphIndex <= 0 )
            return false;
        const GUI::CBitmapFont& font = *bakedFont->font;
        const GUI::CBitmapFont::TGlyph* glyph = font.GetGlyph( (UInt32) glyphIndex );
        if ( GUCEF_NULL == glyph )
            return false;

        const Float32 advance = (Float32) ( glyph->advance + font.GetSpacing() ) * bakedFont->metricScale;
        if ( GUCEF_NULL != outAdvanceX )
        {
            // only the metrics: nothing is packed
            *outAdvanceX = advance;
            return true;
        }

        outGlyph->AdvanceX = advance;
        const Int32 width = glyph->source.width;
        const Int32 height = glyph->source.height;
        if ( width <= 0 || height <= 0 || font.GetAtlas().IsNULL() )
            return true;

        const IMAGE::CPixelMap& atlasPixels = *font.GetAtlas();
        if ( IMAGE::PSF_RGBA != atlasPixels.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != atlasPixels.GetPixelComponentDataType() ||
             glyph->source.left < 0 || glyph->source.top < 0 || glyph->source.Right() > (Int32) atlasPixels.GetWidthInPixels() ||
             glyph->source.Bottom() > (Int32) atlasPixels.GetHeightInPixels() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The glyph of code point " << glyphIndex << " is not in the RGBA atlas of its font" );
            return true;
        }

        ImFontAtlasRectId packId = ImFontAtlasPackAddRect( atlas, width, height );
        if ( ImFontAtlasRectId_Invalid == packId )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The atlas has no room for a glyph of " << width << " x " << height << " pixels" );
            return false;
        }
        ImTextureRect* rect = ImFontAtlasPackGetRect( atlas, packId );

        // the coverage of the glyph: the alpha of the white glyph in the atlas of the font
        ImVector< unsigned char >& coverage = atlas->Builder->TempBuffer;
        coverage.resize( width * height );
        const UInt8* pixels = static_cast< const UInt8* >( atlasPixels.GetDataPtr() );
        const UInt32 atlasWidth = atlasPixels.GetWidthInPixels();
        for ( Int32 y=0; y<height; ++y )
        {
            const UInt8* row = pixels + ( (size_t) ( glyph->source.top + y ) * atlasWidth + glyph->source.left ) * 4;
            for ( Int32 x=0; x<width; ++x )
                coverage[ y * width + x ] = row[ x * 4 + 3 ];
        }

        outGlyph->X0 = (Float32) glyph->offsetX * bakedFont->metricScale;
        outGlyph->Y0 = (Float32) glyph->offsetY * bakedFont->metricScale;
        outGlyph->X1 = outGlyph->X0 + (Float32) width * bakedFont->sizeScale;
        outGlyph->Y1 = outGlyph->Y0 + (Float32) height * bakedFont->sizeScale;
        outGlyph->Visible = true;
        outGlyph->PackId = packId;
        ImFontAtlasBakedSetFontGlyphBitmap( atlas, baked, src, outGlyph, rect, coverage.Data, ImTextureFormat_Alpha8, width );
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiFontLoader::SSource::SSource( void )
    : isBitmap( false )
    , data()
    , ranges()
    , emPerLineHeight( 0.0f )
    , font()
{
}

/*-------------------------------------------------------------------------*/

CImGuiFontLoader::SBaked::SBaked( void )
    : font()
    , metricScale( 1.0f )
    , sizeScale( 1.0f )
{
}

/*-------------------------------------------------------------------------*/

CImGuiFontLoader::CImGuiFontLoader( GUI::CFontRasterizerRegistry& registry )
    : m_registry( registry )
    , m_bitmapFonts()
    , m_glyphRanges()
    , m_nextBitmapFontId( 0 )
    , m_rasterizeCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiFontLoader::~CImGuiFontLoader()
{GUCEF_TRACE;

    // the atlas is gone by now: the glyph ranges it pointed to can go
    for ( TPointerVector::iterator i=m_glyphRanges.begin(); i!=m_glyphRanges.end(); ++i )
        GUCEF_DELETE[] static_cast< ImWchar* >( *i );
    m_glyphRanges.clear();
}

/*-------------------------------------------------------------------------*/

const ::ImFontLoader*
CImGuiFontLoader::GetFontLoader( void )
{GUCEF_TRACE;

    static ImFontLoader loader;
    if ( GUCEF_NULL == loader.Name )
    {
        loader.Name = "gucefGUI";
        loader.LoaderInit = GUCEF_NULL;
        loader.LoaderShutdown = GUCEF_NULL;
        loader.FontSrcInit = &CImGuiFontLoaderFunctions::FontSrcInit;
        loader.FontSrcDestroy = &CImGuiFontLoaderFunctions::FontSrcDestroy;
        loader.FontSrcGetGlyphIndexFromCodepoint = &CImGuiFontLoaderFunctions::FontSrcGetGlyphIndexFromCodepoint;
        loader.FontSrcContainsGlyph = &CImGuiFontLoaderFunctions::FontSrcContainsGlyph;
        loader.FontBakedInit = &CImGuiFontLoaderFunctions::FontBakedInit;
        loader.FontBakedDestroy = &CImGuiFontLoaderFunctions::FontBakedDestroy;
        loader.FontBakedLoadGlyph = &CImGuiFontLoaderFunctions::FontBakedLoadGlyph;
        loader.FontBakedSrcLoaderDataSize = sizeof( SBaked* );
    }
    return &loader;
}

/*-------------------------------------------------------------------------*/

void
CImGuiFontLoader::Attach( ::ImFontAtlas& atlas )
{GUCEF_TRACE;

    atlas.UserData = this;
    atlas.SetFontLoader( GetFontLoader() );
}

/*-------------------------------------------------------------------------*/

CImGuiFontLoader*
CImGuiFontLoader::GetLoaderOf( const ::ImFontAtlas& atlas )
{GUCEF_TRACE;

    if ( atlas.FontLoader != GetFontLoader() )
        return GUCEF_NULL;
    return static_cast< CImGuiFontLoader* >( atlas.UserData );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiFontLoader::HasRasterizer( void ) const
{GUCEF_TRACE;

    return m_registry.HasRasterizers();
}

/*-------------------------------------------------------------------------*/

GUI::TBitmapFontPtr
CImGuiFontLoader::Rasterize( const CORE::CDynamicBuffer& data, UInt32 emSize, const TCodePointRanges& ranges )
{GUCEF_TRACE;

    ++m_rasterizeCount;
    return m_registry.RasterizeFont( data, emSize, ranges );
}

/*-------------------------------------------------------------------------*/

const void*
CImGuiFontLoader::MakeGlyphRanges( const TCodePointRanges& ranges )
{GUCEF_TRACE;

    // pairs of first and last code point ending with 0, kept for as long as the atlas exists
    ImWchar* list = GUCEF_NEW ImWchar[ ranges.size() * 2 + 1 ];
    size_t n = 0;
    for ( size_t i=0; i<ranges.size(); ++i )
    {
        UInt32 first = ranges[ i ].first < 1 ? 1 : ranges[ i ].first;
        UInt32 last = ranges[ i ].last;
        if ( last > IM_UNICODE_CODEPOINT_MAX )
            last = IM_UNICODE_CODEPOINT_MAX;
        if ( last < first )
            continue;
        list[ n++ ] = (ImWchar) first;
        list[ n++ ] = (ImWchar) last;
    }
    list[ n ] = 0;
    m_glyphRanges.push_back( list );
    return list;
}

/*-------------------------------------------------------------------------*/

CImGuiFontLoader::TCodePointRanges
CImGuiFontLoader::ReadGlyphRanges( const ::ImFontConfig& source )
{GUCEF_TRACE;

    if ( GUCEF_NULL == source.GlyphRanges )
        return GUI::CFontRasterizer::GetUiRanges();
    TCodePointRanges ranges;
    for ( const ImWchar* p=source.GlyphRanges; 0 != p[ 0 ] && 0 != p[ 1 ]; p+=2 )
        ranges.push_back( GUI::CFontRasterizer::TCodePointRange( (UInt32) p[ 0 ], (UInt32) p[ 1 ] ) );
    return ranges;
}

/*-------------------------------------------------------------------------*/

::ImFont*
CImGuiFontLoader::AddFont( ::ImFontAtlas& atlas, const CORE::CDynamicBuffer& fontData, Float32 sizePixels, const TCodePointRanges& ranges, const CString& name )
{GUCEF_TRACE;

    if ( 0 == fontData.GetDataSize() )
        return GUCEF_NULL;

    // Dear ImGui keeps the data for as long as the atlas exists and frees it itself
    void* data = IM_ALLOC( fontData.GetDataSize() );
    memcpy( data, fontData.GetConstBufferPtr(), fontData.GetDataSize() );

    ImFontConfig config;
    config.FontData = data;
    config.FontDataSize = (int) fontData.GetDataSize();
    config.FontDataOwnedByAtlas = true;
    config.SizePixels = sizePixels;
    config.GlyphRanges = static_cast< const ImWchar* >( MakeGlyphRanges( ranges.empty() ? GUI::CFontRasterizer::GetUiRanges() : ranges ) );
    ImFormatString( config.Name, IM_ARRAYSIZE( config.Name ), "%s", name.C_String() );
    ImFont* font = atlas.AddFont( &config );
    if ( GUCEF_NULL == font )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The font \"" << name << "\" cannot be added" );
    return font;
}

/*-------------------------------------------------------------------------*/

::ImFont*
CImGuiFontLoader::AddFont( ::ImFontAtlas& atlas, const CString& vfsPath, Float32 sizePixels, const TCodePointRanges& ranges )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: Cannot open the font " << vfsPath );
        return GUCEF_NULL;
    }
    CORE::CDynamicBuffer fontData;
    CORE::CIOAccess& access = *file->GetAccess();
    UInt64 size = access.GetSize();
    if ( 0 == size || (UInt32) size != access.Read( fontData, 1, (UInt32) size ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: Cannot read the font " << vfsPath );
        return GUCEF_NULL;
    }
    return AddFont( atlas, fontData, sizePixels, ranges, CORE::ExtractFilename( vfsPath ) );
}

/*-------------------------------------------------------------------------*/

::ImFont*
CImGuiFontLoader::AddBitmapFont( ::ImFontAtlas& atlas, const GUI::TBitmapFontPtr& font, const CString& name )
{GUCEF_TRACE;

    if ( font.IsNULL() || font->GetAtlas().IsNULL() || font->GetLineHeight() <= 0 )
        return GUCEF_NULL;

    UInt32 id = ++m_nextBitmapFontId;
    m_bitmapFonts[ id ] = font;

    // the font data of the source is a handle of the font, which the loader reads when Dear ImGui sets the source up
    SBitmapFontHandle* handle = static_cast< SBitmapFontHandle* >( IM_ALLOC( sizeof( SBitmapFontHandle ) ) );
    memcpy( handle->marker, BitmapFontMarker, sizeof( BitmapFontMarker ) );
    handle->id = id;

    ImFontConfig config;
    config.FontData = handle;
    config.FontDataSize = (int) sizeof( SBitmapFontHandle );
    config.FontDataOwnedByAtlas = true;
    config.FontLoader = GetFontLoader();
    config.SizePixels = (float) font->GetLineHeight();
    ImFormatString( config.Name, IM_ARRAYSIZE( config.Name ), "%s", name.C_String() );
    ImFont* imFont = atlas.AddFont( &config );
    if ( GUCEF_NULL == imFont )
    {
        m_bitmapFonts.erase( id );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: The bitmap font \"" << name << "\" cannot be added" );
    }
    return imFont;
}

/*-------------------------------------------------------------------------*/

GUI::TBitmapFontPtr
CImGuiFontLoader::GetBakedFont( ::ImFontBaked& baked, UInt32 sourceIndex )
{GUCEF_TRACE;

    if ( GUCEF_NULL == baked.OwnerFont || GUCEF_NULL == baked.FontLoaderDatas || sourceIndex >= (UInt32) baked.OwnerFont->Sources.Size )
        return GUI::TBitmapFontPtr();
    // every source of the font has one slot, all with this loader
    SBaked* const* slots = static_cast< SBaked* const* >( baked.FontLoaderDatas );
    const SBaked* bakedFont = slots[ sourceIndex ];
    return GUCEF_NULL != bakedFont ? bakedFont->font : GUI::TBitmapFontPtr();
}

/*-------------------------------------------------------------------------*/

Float32
CImGuiFontLoader::GetEmPerLineHeight( const ::ImFontConfig& source )
{GUCEF_TRACE;

    const SSource* data = static_cast< const SSource* >( source.FontLoaderData );
    return GUCEF_NULL != data && !data->isBitmap ? data->emPerLineHeight : 0.0f;
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiFontLoader::GetRasterizeCount( void ) const
{GUCEF_TRACE;

    return m_rasterizeCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
