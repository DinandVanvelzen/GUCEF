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

#include "guidriverImGui_CImGuiTextureCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImGuiTextureCache::SEntry::SEntry( void )
    : texture( GUCEF_NULL )
    , revision( 0 )
    , source( GUCEF_NULL )
    , stale( false )
{
}

/*-------------------------------------------------------------------------*/

CImGuiTextureCache::CImGuiTextureCache( GUI::CGUIContext& context )
    : m_context( context )
    , m_images()
    , m_fonts()
    , m_dying()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImGuiTextureCache::~CImGuiTextureCache()
{GUCEF_TRACE;

    if ( !m_images.empty() || !m_fonts.empty() || !m_dying.empty() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "ImGui: The texture cache goes with textures that were not released" );
}

/*-------------------------------------------------------------------------*/

bool
CImGuiTextureCache::FitsTexture( const ::ImTextureData& texture, const IMAGE::CPixelMap& pixels )
{GUCEF_TRACE;

    return texture.Width == (int) pixels.GetWidthInPixels() && texture.Height == (int) pixels.GetHeightInPixels() &&
           ImTextureFormat_RGBA32 == texture.Format;
}

/*-------------------------------------------------------------------------*/

void
CImGuiTextureCache::CopyPixels( ::ImTextureData& texture, const IMAGE::CPixelMap& pixels )
{GUCEF_TRACE;

    // the pixels of a registered image are RGBA with 8 bits per channel (CGUIContext::RegisterImage() converts others)
    if ( IMAGE::PSF_RGBA == pixels.GetPixelStorageFormat() && GUCEF::DATATYPE_UINT8 == pixels.GetPixelComponentDataType() )
    {
        memcpy( texture.Pixels, pixels.GetDataPtr(), (size_t) texture.GetSizeInBytes() );
        return;
    }
    IMAGE::CPixelMap converted( pixels );
    if ( converted.ConvertFormatTo( IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) )
        memcpy( texture.Pixels, converted.GetDataPtr(), (size_t) texture.GetSizeInBytes() );
    else
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImGui: An image cannot be converted to RGBA for a texture" );
}

/*-------------------------------------------------------------------------*/

::ImTextureData*
CImGuiTextureCache::MakeTexture( const IMAGE::CPixelMap& pixels )
{GUCEF_TRACE;

    ::ImTextureData* texture = IM_NEW( ImTextureData )();
    texture->Create( ImTextureFormat_RGBA32, (int) pixels.GetWidthInPixels(), (int) pixels.GetHeightInPixels() );
    texture->UseColors = true;
    texture->UsedRect.x = 0;
    texture->UsedRect.y = 0;
    texture->UsedRect.w = (unsigned short) texture->Width;
    texture->UsedRect.h = (unsigned short) texture->Height;
    CopyPixels( *texture, pixels );
    // a user texture: the cache is the only caller of this experimental part of Dear ImGui
    ImGui::RegisterUserTexture( texture );
    return texture;
}

/*-------------------------------------------------------------------------*/

void
CImGuiTextureCache::ScheduleDestroy( ::ImTextureData* texture )
{GUCEF_TRACE;

    if ( GUCEF_NULL == texture )
        return;
    // Dear ImGui asks the renderer to destroy it at the next frame; a texture that is no longer drawn counts as unused
    texture->WantDestroyNextFrame = true;
    texture->UnusedFrames = 1;
    m_dying.push_back( texture );
}

/*-------------------------------------------------------------------------*/

::ImTextureData*
CImGuiTextureCache::GetImageTexture( const CString& reference )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr pixels;
    UInt32 revision = 0;
    if ( reference.IsNULLOrEmpty() || !m_context.ResolveImage( reference, pixels, revision ) || pixels.IsNULL() ||
         0 == pixels->GetWidthInPixels() || 0 == pixels->GetHeightInPixels() )
        return GUCEF_NULL;

    SEntry& entry = m_images[ reference ];
    if ( GUCEF_NULL != entry.texture && !entry.stale && entry.revision == revision )
        return entry.texture;

    if ( GUCEF_NULL != entry.texture && FitsTexture( *entry.texture, *pixels ) )
    {
        // the whole texture changes
        CopyPixels( *entry.texture, *pixels );
        if ( ImTextureStatus_OK == entry.texture->Status || ImTextureStatus_WantUpdates == entry.texture->Status )
        {
            ImTextureRect all;
            all.x = 0;
            all.y = 0;
            all.w = (unsigned short) entry.texture->Width;
            all.h = (unsigned short) entry.texture->Height;
            entry.texture->Updates.resize( 0 );
            entry.texture->Updates.push_back( all );
            entry.texture->UpdateRect = all;
            entry.texture->SetStatus( ImTextureStatus_WantUpdates );
        }
    }
    else
    {
        ScheduleDestroy( entry.texture );
        entry.texture = MakeTexture( *pixels );
    }
    entry.revision = revision;
    entry.stale = false;
    return entry.texture;
}

/*-------------------------------------------------------------------------*/

::ImTextureData*
CImGuiTextureCache::GetFontTexture( const CString& fontName )
{GUCEF_TRACE;

    GUI::TBitmapFontPtr font = m_context.GetBitmapFont( fontName );
    if ( font.IsNULL() || font->GetAtlas().IsNULL() )
        return GUCEF_NULL;
    const IMAGE::CPixelMap& atlas = *font->GetAtlas();
    if ( 0 == atlas.GetWidthInPixels() || 0 == atlas.GetHeightInPixels() )
        return GUCEF_NULL;

    // a font registered again under the name, or a font that got another atlas, is another texture
    SEntry& entry = m_fonts[ fontName ];
    const void* source = font->GetAtlas().GetPointerAlways();
    if ( GUCEF_NULL != entry.texture && entry.source == source )
        return entry.texture;
    ScheduleDestroy( entry.texture );
    entry.texture = MakeTexture( atlas );
    entry.source = source;
    return entry.texture;
}

/*-------------------------------------------------------------------------*/

void
CImGuiTextureCache::OnImageChanged( const CString& alias )
{GUCEF_TRACE;

    TEntryMap::iterator i = m_images.find( alias );
    if ( i == m_images.end() )
        return;
    if ( !m_context.HasImage( alias ) )
    {
        // unregistered: its texture goes
        ScheduleDestroy( (*i).second.texture );
        m_images.erase( i );
        return;
    }
    (*i).second.stale = true;
}

/*-------------------------------------------------------------------------*/

void
CImGuiTextureCache::Update( void )
{GUCEF_TRACE;

    TTextureVector::iterator i = m_dying.begin();
    while ( i != m_dying.end() )
    {
        ::ImTextureData* texture = (*i);
        if ( ImTextureStatus_Destroyed == texture->Status )
        {
            ImGui::UnregisterUserTexture( texture );
            IM_DELETE( texture );
            i = m_dying.erase( i );
            continue;
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CImGuiTextureCache::ReleaseAll( void )
{GUCEF_TRACE;

    TTextureVector textures = m_dying;
    for ( TEntryMap::iterator i=m_images.begin(); i!=m_images.end(); ++i )
        if ( GUCEF_NULL != (*i).second.texture )
            textures.push_back( (*i).second.texture );
    for ( TEntryMap::iterator i=m_fonts.begin(); i!=m_fonts.end(); ++i )
        if ( GUCEF_NULL != (*i).second.texture )
            textures.push_back( (*i).second.texture );
    m_dying.clear();
    m_images.clear();
    m_fonts.clear();

    for ( TTextureVector::iterator i=textures.begin(); i!=textures.end(); ++i )
    {
        ImGui::UnregisterUserTexture( *i );
        IM_DELETE( *i );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiTextureCache::GetImageTextureCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_images.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiTextureCache::GetFontTextureCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_fonts.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CImGuiTextureCache::GetPendingDestroyCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_dying.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
