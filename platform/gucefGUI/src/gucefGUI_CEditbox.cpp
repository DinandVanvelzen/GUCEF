/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#include "gucefGUI_CEditbox.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CEditbox::SelectionChangedEvent = "GUCEF::GUI::CEditbox::SelectionChangedEvent";
const CORE::CEvent CEditbox::CaratPositionChangedEvent = "GUCEF::GUI::CEditbox::CaratPositionChangedEvent";
const CORE::CEvent CEditbox::TextChangedEvent = "GUCEF::GUI::CEditbox::TextChangedEvent";
const CORE::CEvent CEditbox::TextEnteredEvent = "GUCEF::GUI::CEditbox::TextEnteredEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CEditbox::RegisterEvents( void )
{GUCEF_TRACE;

    SelectionChangedEvent.Initialize();
    CaratPositionChangedEvent.Initialize();
    TextChangedEvent.Initialize();
    TextEnteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CEditbox::CEditbox( void )
    : CWidget( "Editbox" )
    , m_blinkTime( 0 )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CEditbox::~CEditbox()
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

CWidget*
CEditbox::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CEditbox();
}

/*-------------------------------------------------------------------------*/

bool
CEditbox::SetText( const CString& text )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Text, LimitText( text ) );
}

/*-------------------------------------------------------------------------*/

CString
CEditbox::GetText( void ) const
{GUCEF_TRACE;

    return GetProperty( EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CEditbox::SetSelectedText( const CString& text )
{GUCEF_TRACE;

    if ( GetBoolProperty( EWidgetProperty_Selected ) )
    {
        SetBoolProperty( EWidgetProperty_Selected, false );
        return SetText( text );
    }
    return SetText( GetText() + text );
}

/*-------------------------------------------------------------------------*/

bool
CEditbox::GetSelectedText( CString& text ) const
{GUCEF_TRACE;

    if ( !GetBoolProperty( EWidgetProperty_Selected ) )
    {
        text.Clear();
        return false;
    }
    text = GetText();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEditbox::Clear( void )
{GUCEF_TRACE;

    SetBoolProperty( EWidgetProperty_Selected, false );
    SetText( CString::Empty );
}

/*-------------------------------------------------------------------------*/

CString
CEditbox::LimitText( const CString& text ) const
{GUCEF_TRACE;

    Int32 maxLength = GetIntProperty( EWidgetProperty_MaxLength );
    if ( maxLength <= 0 )
        return text;
    TCodePointVector codePoints;
    DecodeUtf8( text, codePoints );
    if ( (Int32) codePoints.size() <= maxLength )
        return text;
    CString limited;
    for ( Int32 i=0; i<maxLength; ++i )
        limited.Append( codePoints[ i ] );
    return limited;
}

/*-------------------------------------------------------------------------*/

bool
CEditbox::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = height = 0;
    if ( GUCEF_NULL == GetContext() )
        return false;
    Int32 w = 0;
    if ( !GetContext()->MeasureText( GetProperty( EWidgetProperty_Font ), "M", 0, w, height ) )
        return false;
    width = w * 16;
    height += 4;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEditbox::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    if ( property == EWidgetProperty_Text )
        NotifyObservers( TextChangedEvent );
    else
    if ( property == EWidgetProperty_Selected )
        NotifyObservers( SelectionChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CEditbox::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return HasFocus() || GetBoolProperty( EWidgetProperty_CaretVisible );
}

/*-------------------------------------------------------------------------*/

void
CEditbox::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !HasFocus() )
    {
        SetBoolProperty( EWidgetProperty_CaretVisible, false );
        return;
    }
    // the caret blinks twice a second
    m_blinkTime += deltaInMs;
    if ( m_blinkTime >= 500 )
    {
        m_blinkTime %= 500;
        SetBoolProperty( EWidgetProperty_CaretVisible, !GetBoolProperty( EWidgetProperty_CaretVisible ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CEditbox::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( KeyboardFocusReceivedEvent == eventId )
    {
        if ( GetBoolProperty( EWidgetProperty_SelectAllOnFocus ) && !GetText().IsNULLOrEmpty() )
            SetBoolProperty( EWidgetProperty_Selected, true );
        m_blinkTime = 0;
        SetBoolProperty( EWidgetProperty_CaretVisible, true );
        return true;
    }
    if ( KeyboardFocusLostEvent == eventId )
    {
        SetBoolProperty( EWidgetProperty_Selected, false );
        SetBoolProperty( EWidgetProperty_CaretVisible, false );
        return true;
    }
    if ( TextInputEvent == eventId )
    {
        if ( GetBoolProperty( EWidgetProperty_ReadOnly ) )
            return false;
        CTextEventData* textData = static_cast< CTextEventData* >( &eventData );
        TCodePointVector codePoints;
        DecodeUtf8( textData->text, codePoints );
        CString typed;
        for ( UInt32 i=0; i<(UInt32)codePoints.size(); ++i )
            if ( codePoints[ i ] >= 32 && 127 != codePoints[ i ] )
                typed.Append( codePoints[ i ] );
        if ( typed.IsNULLOrEmpty() )
            return false;
        CString text = GetBoolProperty( EWidgetProperty_Selected ) ? CString::Empty : GetText();
        SetBoolProperty( EWidgetProperty_Selected, false );
        ChangeValueByUser( EWidgetProperty_Text, LimitText( text + typed ) );
        m_blinkTime = 0;
        SetBoolProperty( EWidgetProperty_CaretVisible, true );
        return true;
    }
    if ( KeyDownEvent == eventId )
    {
        CKeyEventData* key = static_cast< CKeyEventData* >( &eventData );
        if ( INPUT::KEYCODE_BACK == key->keyCode )
        {
            if ( GetBoolProperty( EWidgetProperty_ReadOnly ) )
                return true;
            CString text;
            if ( !GetBoolProperty( EWidgetProperty_Selected ) )
            {
                TCodePointVector codePoints;
                DecodeUtf8( GetText(), codePoints );
                for ( UInt32 i=0; i+1<(UInt32)codePoints.size(); ++i )
                    text.Append( codePoints[ i ] );
            }
            SetBoolProperty( EWidgetProperty_Selected, false );
            ChangeValueByUser( EWidgetProperty_Text, text );
            return true;
        }
        if ( INPUT::KEYCODE_RETURN == key->keyCode || INPUT::KEYCODE_NUMPADENTER == key->keyCode )
        {
            NotifyObservers( TextEnteredEvent );
            return true;
        }
        if ( INPUT::KEYCODE_LEFT == key->keyCode || INPUT::KEYCODE_RIGHT == key->keyCode ||
             INPUT::KEYCODE_HOME == key->keyCode || INPUT::KEYCODE_END == key->keyCode )
        {
            // the caret stays at the end; moving it only drops the selection
            SetBoolProperty( EWidgetProperty_Selected, false );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CEditbox::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CEditbox";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
