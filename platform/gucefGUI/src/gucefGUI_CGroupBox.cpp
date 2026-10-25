/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CGroupBox.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGroupBox::CGroupBox( void )
    : CPanel( "GroupBox" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGroupBox::~CGroupBox()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWidget*
CGroupBox::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CGroupBox();
}

/*-------------------------------------------------------------------------*/

bool
CGroupBox::SetCaption( const CString& caption )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Caption, caption );
}

/*-------------------------------------------------------------------------*/

CString
CGroupBox::GetCaption( void ) const
{GUCEF_TRACE;

    return GetProperty( EWidgetProperty_Caption );
}

/*-------------------------------------------------------------------------*/

Int32
CGroupBox::GetCaptionHeight( void ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context || GetProperty( EWidgetProperty_Caption ).IsNULLOrEmpty() )
        return 0;
    return context->GetLineHeight( GetProperty( EWidgetProperty_Font ) );
}

/*-------------------------------------------------------------------------*/

bool
CGroupBox::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    if ( !GetStackNaturalSize( false, width, height ) )
        return false;
    height += GetCaptionHeight();
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CGroupBox::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGroupBox";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
