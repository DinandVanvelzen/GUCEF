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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "MyGUI.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H
#include "guidriverMyGUI_CMyGUIVfsDataStream.h"
#define GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H
#endif /* GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H ? */

#include "guidriverMyGUI_CMyGUIContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CMyGUIContext::WidgetLayerName = "Overlapped";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMyGUIContext::CMyGUIContext( GUI::CGUIDriver* driver, GUI::TWindowContextPtr windowContext )
    : GUI::CGUIContext( driver, windowContext )
    , m_gui( GUCEF_NULL )
    , m_stackingDirty( true )
    , m_appliedScale( 1 )
    , m_textureCounter( 0 )
    , m_textures()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMyGUIContext::~CMyGUIContext()
{GUCEF_TRACE;

    // a renderer context releases MyGUI before its render manager goes, this is the net for one that did not
    ReleaseGui();
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::StartGui( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_gui )
        return true;
    if ( GUCEF_NULL == MyGUI::DataManager::getInstancePtr() || GUCEF_NULL == MyGUI::RenderManager::getInstancePtr() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIContext: MyGUI needs its data manager and render manager before it starts" );
        return false;
    }

    try
    {
        m_gui = new MyGUI::Gui();
        m_gui->initialise( "MyGUI_Core.xml" );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIContext: MyGUI could not be started: " << e.what() );
        delete m_gui;
        m_gui = GUCEF_NULL;
        return false;
    }

    // the pointer of the context is the one gucefGUI draws (SetCursor), MyGUI gets no pointer input
    if ( GUCEF_NULL != MyGUI::PointerManager::getInstancePtr() )
        MyGUI::PointerManager::getInstance().setVisible( false );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIContext: Started MyGUI " << MYGUI_VERSION_MAJOR << "." << MYGUI_VERSION_MINOR << "." << MYGUI_VERSION_PATCH );
    m_stackingDirty = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::ReleaseGui( void )
{GUCEF_TRACE;

    // the widgets first: their MyGUI widgets go with them
    DestroyAllWidgets();
    if ( GUCEF_NULL != m_gui )
    {
        ReleaseTextures();
        m_gui->shutdown();
        delete m_gui;
        m_gui = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::IsGuiStarted( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_gui;
}

/*-------------------------------------------------------------------------*/

MyGUI::Gui*
CMyGUIContext::GetGui( void ) const
{GUCEF_TRACE;

    return m_gui;
}

/*-------------------------------------------------------------------------*/

MyGUI::Widget*
CMyGUIContext::GetNativeWidget( const GUI::CWidget& widget )
{GUCEF_TRACE;

    return static_cast< MyGUI::Widget* >( widget.GetImplementationPtr() );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::SetNativeWidget( GUI::CWidget& widget, MyGUI::Widget* native )
{GUCEF_TRACE;

    widget.SetImplementationPtr( native );
}

/*-------------------------------------------------------------------------*/

MyGUI::Widget*
CMyGUIContext::CreateNativeWidget( const char* type, const char* skin, const std::string& name )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_gui )
        return GUCEF_NULL;
    try
    {
        MyGUI::Widget* native = m_gui->createWidgetT( type, skin, MyGUI::IntCoord( 0, 0, 16, 16 ), MyGUI::Align::Default, WidgetLayerName, name );
        if ( GUCEF_NULL != native )
        {
            // gucefGUI routes the input, MyGUI only draws
            native->setNeedMouseFocus( false );
            native->setNeedKeyFocus( false );
            native->setVisible( false );
            m_stackingDirty = true;
        }
        return native;
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIContext: The MyGUI widget " << type << " with skin " << skin << " could not be made: " << e.what() );
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::DestroyNativeWidget( MyGUI::Widget* native )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_gui && GUCEF_NULL != native )
        m_gui->destroyWidget( native );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::PlaceNativeWidget( MyGUI::Widget* native, const GUI::CRect& rect ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == native )
        return;
    const Int32 scale = (Int32) GetRenderScale();
    native->setCoord( rect.left * scale, rect.top * scale, rect.width * scale, rect.height * scale );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::InvalidateStacking( void )
{GUCEF_TRACE;

    m_stackingDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::OnRootOrderChanged( void )
{GUCEF_TRACE;

    m_stackingDirty = true;
    GUI::CGUIContext::OnRootOrderChanged();
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::OnImageChanged( const CString& alias )
{GUCEF_TRACE;

    // a picture that is a texture already is written again, in place, so the widgets that show it keep it
    TTextureMap::iterator i = m_textures.find( alias );
    if ( i != m_textures.end() )
    {
        std::string textureName;
        Int32 width = 0, height = 0;
        PrepareTexture( alias, textureName, width, height );
    }
    GUI::CGUIContext::OnImageChanged( alias );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::ReleaseTextures( void )
{GUCEF_TRACE;

    MyGUI::RenderManager* renderer = MyGUI::RenderManager::getInstancePtr();
    if ( GUCEF_NULL != renderer )
    {
        for ( TTextureMap::iterator i=m_textures.begin(); i!=m_textures.end(); ++i )
        {
            MyGUI::ITexture* texture = renderer->getTexture( (*i).second.name );
            if ( GUCEF_NULL != texture )
                renderer->destroyTexture( texture );
        }
    }
    m_textures.clear();
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::WriteTexturePixels( MyGUI::ITexture& texture, IMAGE::CPixelMap& pixels ) const
{GUCEF_TRACE;

    unsigned char* data = static_cast< unsigned char* >( texture.lock( MyGUI::TextureUsage::Write ) );
    if ( GUCEF_NULL == data )
        return false;
    const UInt32 h = pixels.GetHeightInPixels();
    const size_t rowSize = (size_t) pixels.GetWidthInPixels() * 4;
    const bool swap = IsBgraTextureMemory();
    for ( UInt32 y=0; y<h; ++y )
    {
        unsigned char* row = data + y * rowSize;
        memcpy( row, pixels.GetDataAtScanLine( y ), rowSize );
        if ( swap )
        {
            for ( size_t x=0; x<rowSize; x+=4 )
            {
                unsigned char red = row[ x ];
                row[ x ] = row[ x + 2 ];
                row[ x + 2 ] = red;
            }
        }
    }
    texture.unlock();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::PrepareTexture( const CString& imageReference, std::string& textureName, Int32& width, Int32& height )
{GUCEF_TRACE;

    MyGUI::RenderManager* renderer = MyGUI::RenderManager::getInstancePtr();
    if ( GUCEF_NULL == renderer || imageReference.IsNULLOrEmpty() )
        return false;

    IMAGE::TPixelMapPtr pixels;
    UInt32 revision = 0;
    if ( !ResolveImage( imageReference, pixels, revision ) || pixels.IsNULL() )
        return false;

    TTextureMap::iterator i = m_textures.find( imageReference );
    if ( i != m_textures.end() && (*i).second.revision == revision )
    {
        textureName = (*i).second.name;
        width = (*i).second.width;
        height = (*i).second.height;
        return true;
    }
    if ( i != m_textures.end() )
    {
        MyGUI::ITexture* old = renderer->getTexture( (*i).second.name );
        if ( GUCEF_NULL != old )
        {
            if ( (UInt32) (*i).second.width == pixels->GetWidthInPixels() && (UInt32) (*i).second.height == pixels->GetHeightInPixels() &&
                 IMAGE::PSF_RGBA == pixels->GetPixelStorageFormat() && GUCEF::DATATYPE_UINT8 == pixels->GetPixelComponentDataType() )
            {
                // the same size: the pixels are written to the texture that is there
                try
                {
                    if ( WriteTexturePixels( *old, *pixels ) )
                    {
                        (*i).second.revision = revision;
                        textureName = (*i).second.name;
                        width = (*i).second.width;
                        height = (*i).second.height;
                        return true;
                    }
                }
                catch ( const std::exception& )
                {
                }
            }
            renderer->destroyTexture( old );
        }
        m_textures.erase( i );
    }

    const UInt32 w = pixels->GetWidthInPixels();
    const UInt32 h = pixels->GetHeightInPixels();
    if ( 0 == w || 0 == h || IMAGE::PSF_RGBA != pixels->GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() )
        return false;

    TTextureEntry entry;
    entry.revision = revision;
    entry.width = (Int32) w;
    entry.height = (Int32) h;
    entry.name = std::string( "gucef:" ) + imageReference.C_String() + "#" + CORE::UInt32ToString( ++m_textureCounter ).C_String();

    try
    {
        MyGUI::ITexture* texture = renderer->createTexture( entry.name );
        texture->createManual( (int) w, (int) h, MyGUI::TextureUsage::Static | MyGUI::TextureUsage::Write, MyGUI::PixelFormat( MyGUI::PixelFormat::R8G8B8A8 ) );
        if ( !WriteTexturePixels( *texture, *pixels ) )
        {
            renderer->destroyTexture( texture );
            return false;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIContext: The texture for " << imageReference << " could not be made: " << e.what() );
        return false;
    }

    m_textures[ imageReference ] = entry;
    textureName = entry.name;
    width = entry.width;
    height = entry.height;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::IsBgraTextureMemory( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

std::string
CMyGUIContext::GetMyGUIFontName( const CString& fontName ) const
{GUCEF_TRACE;

    if ( !fontName.IsNULLOrEmpty() && HasMyGUIFont( fontName ) )
        return fontName.C_String();
    return "Default";
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::HasMyGUIFont( const CString& fontName ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_gui || GUCEF_NULL == MyGUI::FontManager::getInstancePtr() )
        return false;
    return GUCEF_NULL != MyGUI::FontManager::getInstance().getByName( fontName.C_String() );
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::LoadFont( const CString& name, const CString& vfsPath, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_gui || !MyGUI::DataManager::getInstance().isDataExist( vfsPath.C_String() ) )
        return false;

    CString definition = CString( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<MyGUI type=\"Resource\" version=\"1.1\">\n" ) +
                         "<Resource type=\"ResourceTrueTypeFont\" name=\"" + name + "\">\n" +
                         "<Property key=\"Source\" value=\"" + vfsPath + "\"/>\n" +
                         "<Property key=\"Size\" value=\"" + CORE::UInt32ToString( size ) + "\"/>\n" +
                         "<Codes><Code range=\"32 126\"/><Code range=\"160 255\"/></Codes>\n" +
                         "</Resource>\n</MyGUI>\n";
    try
    {
        CMyGUIVfsDataStream stream( definition.C_String(), definition.Length() );
        MyGUI::xml::Document document;
        if ( !document.open( &stream ) )
            return false;
        MyGUI::ResourceManager::getInstance().loadFromXmlNode( document.getRoot(), "gucef", MyGUI::Version( 1, 1 ) );
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MyGUIContext: The font " << vfsPath << " could not be loaded: " << e.what() );
        return false;
    }
    return HasMyGUIFont( name );
}

/*-------------------------------------------------------------------------*/

bool
CMyGUIContext::MeasureText( const CString& fontName, const CString& text, Int32 maxWidth, Int32& width, Int32& height )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_gui )
        return GUI::CGUIContext::MeasureText( fontName, text, maxWidth, width, height );

    MyGUI::IFont* font = MyGUI::FontManager::getInstance().getByName( GetMyGUIFontName( fontName ) );
    if ( GUCEF_NULL == font )
        return GUI::CGUIContext::MeasureText( fontName, text, maxWidth, width, height );

    // MyGUI draws fonts at the scale of the render, layout is in logical pixels
    const float scale = 1.0f / (float) GetRenderScale();
    float widest = 0.0f;
    float line = 0.0f;
    UInt32 lines = 1;
    MyGUI::UString converted( std::string_view( text.C_String(), text.Length() ) );
    for ( size_t c=0; c<converted.size(); ++c )
    {
        MyGUI::Char ch = converted[ c ];
        if ( '\n' == ch )
        {
            if ( line > widest ) widest = line;
            line = 0.0f;
            ++lines;
            continue;
        }
        const MyGUI::GlyphInfo* glyph = font->getGlyphInfo( ch );
        if ( GUCEF_NULL != glyph )
            line += glyph->advance;
    }
    if ( line > widest ) widest = line;
    width = (Int32) ( widest * scale + 0.5f );
    height = (Int32) ( (float) ( font->getDefaultHeight() * (int) lines ) * scale + 0.5f );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::RefreshVisibility( void )
{GUCEF_TRACE;

    TWidgetSet widgets = GetOwnedWidgets();
    for ( TWidgetSet::iterator i=widgets.begin(); i!=widgets.end(); ++i )
    {
        GUI::CWidget* widget = (*i);
        MyGUI::Widget* native = GetNativeWidget( *widget );
        if ( GUCEF_NULL == native )
            continue;

        GUI::CWidget* root = widget;
        while ( GUCEF_NULL != root->GetParentWidget() )
            root = root->GetParentWidget();
        const bool visible = widget->IsEffectivelyVisible() && IsRootShown( root );
        if ( native->getVisible() != visible )
            native->setVisible( visible );
    }
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::StackWidget( GUI::CWidget* widget )
{GUCEF_TRACE;

    MyGUI::Widget* native = GetNativeWidget( *widget );
    if ( GUCEF_NULL != native )
        MyGUI::LayerManager::getInstance().upLayerItem( native );
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( UInt32 i=0; i<(UInt32) children.size(); ++i )
        StackWidget( children[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::RefreshStacking( void )
{GUCEF_TRACE;

    // from the back to the front: what is raised last is on top
    const TWidgetVector& roots = GetRootWidgets();
    for ( UInt32 i=0; i<(UInt32) roots.size(); ++i )
        StackWidget( roots[ i ] );
    m_stackingDirty = false;
}

/*-------------------------------------------------------------------------*/

void
CMyGUIContext::PrepareFrame( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_gui )
        return;

    // the picture MyGUI draws is the logical resolution times the render scale
    MyGUI::RenderManager& renderer = MyGUI::RenderManager::getInstance();
    const int width = (int) GetRenderWidth();
    const int height = (int) GetRenderHeight();
    if ( width > 0 && height > 0 && ( renderer.getViewSize().width != width || renderer.getViewSize().height != height ) )
        renderer.setViewSize( width, height );

    const UInt32 scale = GetRenderScale();
    if ( scale != m_appliedScale )
    {
        m_appliedScale = scale;
        TWidgetSet widgets = GetOwnedWidgets();
        for ( TWidgetSet::iterator i=widgets.begin(); i!=widgets.end(); ++i )
            PlaceNativeWidget( GetNativeWidget( **i ), (*i)->GetRect() );
    }

    RefreshVisibility();
    if ( m_stackingDirty )
        RefreshStacking();
}

/*-------------------------------------------------------------------------*/

const CString&
CMyGUIContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::guidriverMyGUI::CMyGUIContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
