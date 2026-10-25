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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/RenderManager.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#include "guidriverRmlUI_CRmlUIWidgetElement.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H
#include "guidriverRmlUI_CRmlUIFormBackend.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H ? */

#include "guidriverRmlUI_CRmlUIContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CString
PixelsProperty( Int32 value )
{
    return CORE::Int32ToString( value ) + "px";
}

/*-------------------------------------------------------------------------*/

int
RmlModifiers( UInt32 modifiers )
{
    int result = 0;
    if ( 0 != ( modifiers & INPUT::KEYMOD_CTRL ) ) result |= Rml::Input::KM_CTRL;
    if ( 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) ) result |= Rml::Input::KM_SHIFT;
    if ( 0 != ( modifiers & INPUT::KEYMOD_ALT ) ) result |= Rml::Input::KM_ALT;
    if ( 0 != ( modifiers & INPUT::KEYMOD_META ) ) result |= Rml::Input::KM_META;
    if ( 0 != ( modifiers & INPUT::KEYMOD_CAPSLOCK ) ) result |= Rml::Input::KM_CAPSLOCK;
    if ( 0 != ( modifiers & INPUT::KEYMOD_NUMLOCK ) ) result |= Rml::Input::KM_NUMLOCK;
    if ( 0 != ( modifiers & INPUT::KEYMOD_SCROLLLOCK ) ) result |= Rml::Input::KM_SCROLLLOCK;
    return result;
}

/*-------------------------------------------------------------------------*/

Rml::Input::KeyIdentifier
RmlKey( INPUT::KeyCode keyCode )
{
    static const INPUT::KeyCode letters[ 26 ] = { INPUT::KEYCODE_A, INPUT::KEYCODE_B, INPUT::KEYCODE_C, INPUT::KEYCODE_D, INPUT::KEYCODE_E,
                                                  INPUT::KEYCODE_F, INPUT::KEYCODE_G, INPUT::KEYCODE_H, INPUT::KEYCODE_I, INPUT::KEYCODE_J,
                                                  INPUT::KEYCODE_K, INPUT::KEYCODE_L, INPUT::KEYCODE_M, INPUT::KEYCODE_N, INPUT::KEYCODE_O,
                                                  INPUT::KEYCODE_P, INPUT::KEYCODE_Q, INPUT::KEYCODE_R, INPUT::KEYCODE_S, INPUT::KEYCODE_T,
                                                  INPUT::KEYCODE_U, INPUT::KEYCODE_V, INPUT::KEYCODE_W, INPUT::KEYCODE_X, INPUT::KEYCODE_Y,
                                                  INPUT::KEYCODE_Z };
    for ( int i=0; i<26; ++i )
        if ( letters[ i ] == keyCode )
            return (Rml::Input::KeyIdentifier) ( Rml::Input::KI_A + i );

    if ( keyCode >= INPUT::KEYCODE_1 && keyCode <= INPUT::KEYCODE_9 )
        return (Rml::Input::KeyIdentifier) ( Rml::Input::KI_1 + ( keyCode - INPUT::KEYCODE_1 ) );
    if ( keyCode >= INPUT::KEYCODE_F1 && keyCode <= INPUT::KEYCODE_F10 )
        return (Rml::Input::KeyIdentifier) ( Rml::Input::KI_F1 + ( keyCode - INPUT::KEYCODE_F1 ) );

    switch ( keyCode )
    {
        case INPUT::KEYCODE_0: return Rml::Input::KI_0;
        case INPUT::KEYCODE_F11: return Rml::Input::KI_F11;
        case INPUT::KEYCODE_F12: return Rml::Input::KI_F12;
        case INPUT::KEYCODE_SPACE: return Rml::Input::KI_SPACE;
        case INPUT::KEYCODE_BACK: return Rml::Input::KI_BACK;
        case INPUT::KEYCODE_TAB: return Rml::Input::KI_TAB;
        case INPUT::KEYCODE_RETURN: return Rml::Input::KI_RETURN;
        case INPUT::KEYCODE_NUMPADENTER: return Rml::Input::KI_NUMPADENTER;
        case INPUT::KEYCODE_ESCAPE: return Rml::Input::KI_ESCAPE;
        case INPUT::KEYCODE_LEFT: return Rml::Input::KI_LEFT;
        case INPUT::KEYCODE_RIGHT: return Rml::Input::KI_RIGHT;
        case INPUT::KEYCODE_UP: return Rml::Input::KI_UP;
        case INPUT::KEYCODE_DOWN: return Rml::Input::KI_DOWN;
        case INPUT::KEYCODE_HOME: return Rml::Input::KI_HOME;
        case INPUT::KEYCODE_END: return Rml::Input::KI_END;
        case INPUT::KEYCODE_PGUP: return Rml::Input::KI_PRIOR;
        case INPUT::KEYCODE_PGDOWN: return Rml::Input::KI_NEXT;
        case INPUT::KEYCODE_INSERT: return Rml::Input::KI_INSERT;
        case INPUT::KEYCODE_DELETE: return Rml::Input::KI_DELETE;
        case INPUT::KEYCODE_LSHIFT: return Rml::Input::KI_LSHIFT;
        case INPUT::KEYCODE_RSHIFT: return Rml::Input::KI_RSHIFT;
        case INPUT::KEYCODE_LCONTROL: return Rml::Input::KI_LCONTROL;
        case INPUT::KEYCODE_RCONTROL: return Rml::Input::KI_RCONTROL;
        case INPUT::KEYCODE_LMENU: return Rml::Input::KI_LMENU;
        case INPUT::KEYCODE_RMENU: return Rml::Input::KI_RMENU;
        case INPUT::KEYCODE_MINUS: return Rml::Input::KI_OEM_MINUS;
        case INPUT::KEYCODE_EQUALS: return Rml::Input::KI_OEM_PLUS;
        case INPUT::KEYCODE_COMMA: return Rml::Input::KI_OEM_COMMA;
        case INPUT::KEYCODE_PERIOD: return Rml::Input::KI_OEM_PERIOD;
        case INPUT::KEYCODE_NUMPAD0: return Rml::Input::KI_NUMPAD0;
        case INPUT::KEYCODE_NUMPAD1: return Rml::Input::KI_NUMPAD1;
        case INPUT::KEYCODE_NUMPAD2: return Rml::Input::KI_NUMPAD2;
        case INPUT::KEYCODE_NUMPAD3: return Rml::Input::KI_NUMPAD3;
        case INPUT::KEYCODE_NUMPAD4: return Rml::Input::KI_NUMPAD4;
        case INPUT::KEYCODE_NUMPAD5: return Rml::Input::KI_NUMPAD5;
        case INPUT::KEYCODE_NUMPAD6: return Rml::Input::KI_NUMPAD6;
        case INPUT::KEYCODE_NUMPAD7: return Rml::Input::KI_NUMPAD7;
        case INPUT::KEYCODE_NUMPAD8: return Rml::Input::KI_NUMPAD8;
        case INPUT::KEYCODE_NUMPAD9: return Rml::Input::KI_NUMPAD9;
        default: return Rml::Input::KI_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

/** RGBA pixels with the colour multiplied by alpha, the form RmlUi's renderers take */
bool
MakePremultiplied( const IMAGE::CPixelMap& pixels, Rml::Vector< Rml::byte >& data )
{
    if ( IMAGE::PSF_RGBA != pixels.GetPixelStorageFormat() || GUCEF::DATATYPE_UINT8 != pixels.GetPixelComponentDataType() )
        return false;
    size_t count = (size_t) pixels.GetWidthInPixels() * pixels.GetHeightInPixels();
    const UInt8* in = static_cast< const UInt8* >( pixels.GetDataPtr() );
    data.resize( count * 4 );
    for ( size_t i=0; i<count; ++i, in+=4 )
    {
        UInt32 a = in[ 3 ];
        data[ i*4 ] = (Rml::byte) ( ( in[ 0 ] * a + 127 ) / 255 );
        data[ i*4+1 ] = (Rml::byte) ( ( in[ 1 ] * a + 127 ) / 255 );
        data[ i*4+2 ] = (Rml::byte) ( ( in[ 2 ] * a + 127 ) / 255 );
        data[ i*4+3 ] = (Rml::byte) a;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRmlUIContext::CRmlUIContext( GUI::CGUIDriver* driver                 ,
                              GUI::TWindowContextPtr windowContext    ,
                              Rml::RenderInterface* renderInterface   )
    : GUI::CGUIContext( driver, windowContext )
    , m_rmlContext( GUCEF_NULL )
    , m_rmlContextName( "gucefGUI-" + CORE::PointerToString( this ) )
    , m_overlayDocument( GUCEF_NULL )
    , m_overlay( GUCEF_NULL )
    , m_rootDocuments()
    , m_nativeDocuments()
    , m_imageTextures()
    , m_fontTextures()
    , m_textureCreations( 0 )
    , m_orderDirty( true )
    , m_rmlWidth( 0 )
    , m_rmlHeight( 0 )
{GUCEF_TRACE;

    m_rmlWidth = GetRenderWidth() > 0 ? GetRenderWidth() : 1;
    m_rmlHeight = GetRenderHeight() > 0 ? GetRenderHeight() : 1;
    m_rmlContext = Rml::CreateContext( m_rmlContextName.C_String(), Rml::Vector2i( (int) m_rmlWidth, (int) m_rmlHeight ), renderInterface );
    if ( GUCEF_NULL == m_rmlContext )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIContext: RmlUi could not make a context" );
        return;
    }

    // the overlay is a document of its own that always stays on top
    m_overlayDocument = MakeDocument();
    Rml::ElementPtr overlay = m_overlayDocument->CreateElement( CRmlUIWidgetElement::TagName );
    m_overlay = static_cast< CRmlUIWidgetElement* >( m_overlayDocument->AppendChild( std::move( overlay ) ) );
    m_overlay->SetWidget( this, GUCEF_NULL, true );
    m_overlay->SetProperty( "position", "absolute" );
    m_overlay->SetProperty( "left", "0px" );
    m_overlay->SetProperty( "top", "0px" );
    m_overlayDocument->Show( Rml::ModalFlag::None, Rml::FocusFlag::None );
}

/*-------------------------------------------------------------------------*/

CRmlUIContext::~CRmlUIContext()
{GUCEF_TRACE;

    ReleaseRml();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::ReleaseRml( void )
{GUCEF_TRACE;

    // the widgets go while their elements still exist
    DestroyAllWidgets();
    m_imageTextures.clear();
    m_fontTextures.clear();
    m_rootDocuments.clear();
    m_nativeDocuments.clear();
    m_overlay = GUCEF_NULL;
    m_overlayDocument = GUCEF_NULL;
    if ( GUCEF_NULL != m_rmlContext )
        Rml::RemoveContext( m_rmlContextName.C_String() );
    m_rmlContext = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIContext::IsValid( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_rmlContext;
}

/*-------------------------------------------------------------------------*/

Rml::Context*
CRmlUIContext::GetRmlContext( void ) const
{GUCEF_TRACE;

    return m_rmlContext;
}

/*-------------------------------------------------------------------------*/

Rml::ElementDocument*
CRmlUIContext::MakeDocument( void )
{GUCEF_TRACE;

    Rml::ElementDocument* document = m_rmlContext->CreateDocument();
    StyleDocument( document );
    return document;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::StyleDocument( Rml::ElementDocument* document )
{GUCEF_TRACE;

    // a document is the whole logical picture; the widget elements are placed absolutely within it
    document->SetProperty( "position", "absolute" );
    document->SetProperty( "left", "0px" );
    document->SetProperty( "top", "0px" );
    document->SetProperty( "width", PixelsProperty( (Int32) m_rmlWidth ).C_String() );
    document->SetProperty( "height", PixelsProperty( (Int32) m_rmlHeight ).C_String() );
    document->SetProperty( "margin", "0px" );
    document->SetProperty( "padding", "0px" );
}

/*-------------------------------------------------------------------------*/

CRmlUIWidgetElement*
CRmlUIContext::GetWidgetElement( const GUI::CWidget& widget )
{GUCEF_TRACE;

    return static_cast< CRmlUIWidgetElement* >( widget.GetImplementationPtr() );
}

/*-------------------------------------------------------------------------*/

CRmlUIWidgetElement*
CRmlUIContext::CreateWidgetElement( GUI::CWidget& widget )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_rmlContext )
        return GUCEF_NULL;

    GUI::CWidget* parent = widget.GetParentWidget();
    CRmlUIWidgetElement* parentElement = GUCEF_NULL != parent ? GetWidgetElement( *parent ) : GUCEF_NULL;
    Rml::Element* container = parentElement;
    if ( GUCEF_NULL == container )
    {
        Rml::ElementDocument* document = MakeDocument();
        m_rootDocuments[ &widget ] = document;
        container = document;
        m_orderDirty = true;
    }

    Rml::ElementPtr elementPtr = container->GetOwnerDocument()->CreateElement( CRmlUIWidgetElement::TagName );
    CRmlUIWidgetElement* element = static_cast< CRmlUIWidgetElement* >( container->AppendChild( std::move( elementPtr ) ) );
    element->SetWidget( this, &widget, false );
    element->SetProperty( "position", "absolute" );
    if ( !widget.GetName().IsNULLOrEmpty() )
        element->SetId( widget.GetName().C_String() );
    widget.SetImplementationPtr( element );
    return element;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::DestroyWidgetElement( GUI::CWidget& widget )
{GUCEF_TRACE;

    CRmlUIWidgetElement* element = GetWidgetElement( widget );
    widget.SetImplementationPtr( GUCEF_NULL );
    UnregisterNativeDocument( &widget );
    if ( GUCEF_NULL == element )
        return;
    element->SetWidget( GUCEF_NULL, GUCEF_NULL, false );

    TDocumentMap::iterator i = m_rootDocuments.find( &widget );
    if ( i != m_rootDocuments.end() )
    {
        // RmlUi deletes the document with the element at its next update
        if ( GUCEF_NULL != m_rmlContext )
            m_rmlContext->UnloadDocument( (*i).second );
        m_rootDocuments.erase( i );
        m_orderDirty = true;
        return;
    }
    Rml::Element* parentElement = element->GetParentNode();
    if ( GUCEF_NULL != parentElement )
        parentElement->RemoveChild( element );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::AttachWidgetElement( GUI::CWidget& widget )
{GUCEF_TRACE;

    CRmlUIWidgetElement* element = GetWidgetElement( widget );
    if ( GUCEF_NULL == element )
        return;

    GUI::CWidget* parent = widget.GetParentWidget();
    Rml::Element* wanted = GUCEF_NULL != parent ? GetWidgetElement( *parent ) : GUCEF_NULL;
    TDocumentMap::iterator own = m_rootDocuments.find( &widget );
    if ( GUCEF_NULL == wanted )
    {
        if ( own != m_rootDocuments.end() )
            return;
        wanted = MakeDocument();
        m_rootDocuments[ &widget ] = static_cast< Rml::ElementDocument* >( wanted );
        m_orderDirty = true;
    }

    Rml::Element* current = element->GetParentNode();
    if ( current == wanted )
        return;
    Rml::ElementPtr moved = current->RemoveChild( element );
    if ( own != m_rootDocuments.end() && GUCEF_NULL != parent )
    {
        // it was a root and is not any more: its document goes
        m_rmlContext->UnloadDocument( (*own).second );
        m_rootDocuments.erase( own );
        m_orderDirty = true;
    }
    wanted->AppendChild( std::move( moved ) );
    element->Invalidate();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::OrderChildElements( GUI::CWidget& widget )
{GUCEF_TRACE;

    CRmlUIWidgetElement* element = GetWidgetElement( widget );
    if ( GUCEF_NULL == element )
        return;
    const GUI::CWidget::TWidgetVector& children = widget.GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
    {
        CRmlUIWidgetElement* child = GetWidgetElement( *children[ i ] );
        if ( GUCEF_NULL == child || child->GetParentNode() != element )
            continue;
        Rml::ElementPtr moved = element->RemoveChild( child );
        element->AppendChild( std::move( moved ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::PlaceWidgetElement( GUI::CWidget& widget, const GUI::CRect& rect )
{GUCEF_TRACE;

    CRmlUIWidgetElement* element = GetWidgetElement( widget );
    if ( GUCEF_NULL == element )
        return;

    // RmlUi places an element within its parent
    Int32 left = rect.left, top = rect.top;
    GUI::CWidget* parent = widget.GetParentWidget();
    if ( GUCEF_NULL != parent )
    {
        left -= parent->GetRect().left;
        top -= parent->GetRect().top;
    }
    element->SetProperty( "left", PixelsProperty( left ).C_String() );
    element->SetProperty( "top", PixelsProperty( top ).C_String() );
    element->SetProperty( "width", PixelsProperty( rect.width ).C_String() );
    element->SetProperty( "height", PixelsProperty( rect.height ).C_String() );
}

/*-------------------------------------------------------------------------*/

CRmlUIContext::TTextureEntry*
CRmlUIContext::FindTexture( const CString& reference, bool isFont )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_rmlContext || reference.IsNULLOrEmpty() )
        return GUCEF_NULL;

    IMAGE::TPixelMapPtr pixels;
    UInt32 revision = 0;
    if ( isFont )
    {
        GUI::TBitmapFontPtr font = GetBitmapFont( reference );
        if ( font.IsNULL() || font->GetAtlas().IsNULL() )
            return GUCEF_NULL;
        pixels = font->GetAtlas();
    }
    else
    {
        if ( !ResolveImage( reference, pixels, revision ) || pixels.IsNULL() )
            return GUCEF_NULL;
    }

    TTextureMap& textures = isFont ? m_fontTextures : m_imageTextures;
    TTextureMap::iterator i = textures.find( reference );
    if ( i != textures.end() && (*i).second.revision == revision && (*i).second.pixels == pixels.GetPointerAlways() )
        return &(*i).second;

    // new or changed pixels: a new texture, uploaded when RmlUi first draws with it
    TTextureEntry& entry = textures[ reference ];
    entry.texture.Release();
    entry.revision = revision;
    entry.pixels = pixels.GetPointerAlways();
    entry.width = (Int32) pixels->GetWidthInPixels();
    entry.height = (Int32) pixels->GetHeightInPixels();

    CRmlUIContext* self = this;
    CString name = reference;
    entry.texture = m_rmlContext->GetRenderManager().MakeCallbackTexture( [self, name, isFont]( const Rml::CallbackTextureInterface& textureInterface ) -> bool
        {
            IMAGE::TPixelMapPtr source;
            UInt32 sourceRevision = 0;
            if ( isFont )
            {
                GUI::TBitmapFontPtr font = self->GetBitmapFont( name );
                if ( !font.IsNULL() )
                    source = font->GetAtlas();
            }
            else
            {
                self->ResolveImage( name, source, sourceRevision );
            }
            Rml::Vector< Rml::byte > data;
            if ( source.IsNULL() || !MakePremultiplied( *source, data ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RmlUIContext: No RGBA pixels for the texture of \"" << name << "\"" );
                return false;
            }
            ++self->m_textureCreations;
            return textureInterface.GenerateTexture( data, Rml::Vector2i( (int) source->GetWidthInPixels(), (int) source->GetHeightInPixels() ) );
        } );
    return &entry;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIContext::HasImageTexture( const CString& imageReference, Int32& width, Int32& height )
{GUCEF_TRACE;

    TTextureEntry* entry = FindTexture( imageReference, false );
    if ( GUCEF_NULL == entry )
        return false;
    width = entry->width;
    height = entry->height;
    return true;
}

/*-------------------------------------------------------------------------*/

Rml::Texture
CRmlUIContext::GetImageTexture( const CString& imageReference )
{GUCEF_TRACE;

    TTextureEntry* entry = FindTexture( imageReference, false );
    if ( GUCEF_NULL == entry )
        return Rml::Texture();
    return entry->texture;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIContext::HasFontTexture( const CString& fontName, Int32& width, Int32& height )
{GUCEF_TRACE;

    TTextureEntry* entry = FindTexture( fontName, true );
    if ( GUCEF_NULL == entry )
        return false;
    width = entry->width;
    height = entry->height;
    return true;
}

/*-------------------------------------------------------------------------*/

Rml::Texture
CRmlUIContext::GetFontTexture( const CString& fontName )
{GUCEF_TRACE;

    TTextureEntry* entry = FindTexture( fontName, true );
    if ( GUCEF_NULL == entry )
        return Rml::Texture();
    return entry->texture;
}

/*-------------------------------------------------------------------------*/

UInt32
CRmlUIContext::GetTextureCreationCount( void ) const
{GUCEF_TRACE;

    return m_textureCreations;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::OnImageChanged( const CString& alias )
{GUCEF_TRACE;

    // the texture is made again from the new pixels at its next use
    TTextureMap::iterator i = m_imageTextures.find( alias );
    if ( i != m_imageTextures.end() )
    {
        (*i).second.texture.Release();
        m_imageTextures.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::OnRootOrderChanged( void )
{GUCEF_TRACE;

    m_orderDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::RegisterNativeDocument( GUI::CWidget* root, Rml::ElementDocument* document )
{GUCEF_TRACE;

    if ( GUCEF_NULL == root || GUCEF_NULL == document )
        return;
    StyleDocument( document );
    m_nativeDocuments[ root ] = document;
    SubscribeTo( root, GUI::CWidget::MouseMoveEvent );
    SubscribeTo( root, GUI::CWidget::MouseDownEvent );
    SubscribeTo( root, GUI::CWidget::MouseUpEvent );
    SubscribeTo( root, GUI::CWidget::MouseWheelEvent );
    SubscribeTo( root, GUI::CWidget::KeyDownEvent );
    SubscribeTo( root, GUI::CWidget::KeyUpEvent );
    SubscribeTo( root, GUI::CWidget::TextInputEvent );
    m_orderDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::UnregisterNativeDocument( GUI::CWidget* root )
{GUCEF_TRACE;

    TDocumentMap::iterator i = m_nativeDocuments.find( root );
    if ( i == m_nativeDocuments.end() )
        return;
    UnsubscribeFrom( root );
    if ( GUCEF_NULL != m_rmlContext )
        m_rmlContext->UnloadDocument( (*i).second );
    m_nativeDocuments.erase( i );
    m_orderDirty = true;
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::ForwardToNativeDocument( Rml::ElementDocument* document, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUI::CWidget::MouseMoveEvent == eventId || GUI::CWidget::MouseDownEvent == eventId || GUI::CWidget::MouseUpEvent == eventId )
    {
        const GUI::CPointerEventData* data = static_cast< const GUI::CPointerEventData* >( eventData );
        int modifiers = RmlModifiers( data->modifiers );
        m_rmlContext->ProcessMouseMove( data->x, data->y, modifiers );
        if ( GUI::MOUSEBUTTON_NONE == data->button )
            return;
        // gucefGUI counts the buttons from 1 (left, right, middle), RmlUi from 0
        int button = (int) data->button - 1;
        if ( GUI::CWidget::MouseDownEvent == eventId )
            m_rmlContext->ProcessMouseButtonDown( button, modifiers );
        else
        if ( GUI::CWidget::MouseUpEvent == eventId )
            m_rmlContext->ProcessMouseButtonUp( button, modifiers );
    }
    else
    if ( GUI::CWidget::MouseWheelEvent == eventId )
    {
        const GUI::CWheelEventData* data = static_cast< const GUI::CWheelEventData* >( eventData );
        // RmlUi scrolls down for a positive delta, gucefGUI's positive delta is up
        m_rmlContext->ProcessMouseWheel( (float) -data->delta, RmlModifiers( data->modifiers ) );
    }
    else
    if ( GUI::CWidget::KeyDownEvent == eventId || GUI::CWidget::KeyUpEvent == eventId )
    {
        const GUI::CKeyEventData* data = static_cast< const GUI::CKeyEventData* >( eventData );
        Rml::Input::KeyIdentifier key = RmlKey( data->keyCode );
        if ( Rml::Input::KI_UNKNOWN == key )
            return;
        if ( GUI::CWidget::KeyDownEvent == eventId )
            m_rmlContext->ProcessKeyDown( key, RmlModifiers( data->modifiers ) );
        else
            m_rmlContext->ProcessKeyUp( key, RmlModifiers( data->modifiers ) );
    }
    else
    if ( GUI::CWidget::TextInputEvent == eventId )
    {
        const GUI::CTextEventData* data = static_cast< const GUI::CTextEventData* >( eventData );
        m_rmlContext->ProcessTextInput( Rml::String( data->text.C_String() ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::OnNotify( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CGUIContext::OnNotify( notifier, eventId, eventData );

    if ( GUCEF_NULL == m_rmlContext || GUCEF_NULL == eventData || m_nativeDocuments.empty() )
        return;
    GUI::CRoutedEventData* routed = GUI::CRoutedEventData::FromCloneable( eventData );
    if ( GUCEF_NULL == routed || routed->source != notifier || GUI::EVENTPHASE_TARGET != routed->phase )
        return;
    TDocumentMap::iterator i = m_nativeDocuments.find( static_cast< GUI::CWidget* >( routed->source ) );
    if ( i != m_nativeDocuments.end() )
        ForwardToNativeDocument( (*i).second, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

GUI::CFormBackend*
CRmlUIContext::CreateFormBackend( void )
{GUCEF_TRACE;

    return GUCEF_NEW CRmlUIFormBackend( *this );
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::PrepareRml( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_rmlContext )
        return;

    // RmlUi works in the pixels of the frame: the logical resolution times the render scale
    UInt32 width = GetRenderWidth() > 0 ? GetRenderWidth() : 1;
    UInt32 height = GetRenderHeight() > 0 ? GetRenderHeight() : 1;
    if ( width != m_rmlWidth || height != m_rmlHeight )
    {
        m_rmlWidth = width;
        m_rmlHeight = height;
        m_rmlContext->SetDimensions( Rml::Vector2i( (int) width, (int) height ) );
        for ( TDocumentMap::iterator i=m_rootDocuments.begin(); i!=m_rootDocuments.end(); ++i )
            StyleDocument( (*i).second );
        for ( TDocumentMap::iterator i=m_nativeDocuments.begin(); i!=m_nativeDocuments.end(); ++i )
            StyleDocument( (*i).second );
        StyleDocument( m_overlayDocument );
    }

    if ( m_orderDirty )
    {
        m_orderDirty = false;

        // the documents of hidden roots are hidden; the shown ones are stacked like the roots, the overlay on top
        for ( TDocumentMap::iterator i=m_rootDocuments.begin(); i!=m_rootDocuments.end(); ++i )
            if ( !IsRootShown( (*i).first ) )
                (*i).second->Hide();
        for ( TDocumentMap::iterator i=m_nativeDocuments.begin(); i!=m_nativeDocuments.end(); ++i )
            if ( !IsRootShown( (*i).first ) )
                (*i).second->Hide();

        const TWidgetVector& roots = GetRootWidgets();
        for ( UInt32 r=0; r<(UInt32)roots.size(); ++r )
        {
            TDocumentMap::iterator d = m_rootDocuments.find( roots[ r ] );
            if ( d != m_rootDocuments.end() )
            {
                (*d).second->Show( Rml::ModalFlag::None, Rml::FocusFlag::None );
                (*d).second->PullToFront();
            }
            TDocumentMap::iterator n = m_nativeDocuments.find( roots[ r ] );
            if ( n != m_nativeDocuments.end() )
            {
                (*n).second->Show( Rml::ModalFlag::None, Rml::FocusFlag::None );
                (*n).second->PullToFront();
            }
        }
        m_overlayDocument->PullToFront();
    }

    m_rmlContext->Update();
}

/*-------------------------------------------------------------------------*/

void
CRmlUIContext::RenderRml( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_rmlContext )
        m_rmlContext->Render();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRmlUIContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::guidriverRmlUI::CRmlUIContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
