/*
 *  guidriverRmlUIOgre: gucefGUI driver drawing RmlUi through Ogre, over the 3D scene of an Ogre window
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "guidriverRmlUIOgre_CRmlUIOgreContext.h"
#include "guidriverRmlUIOgre_CRmlUIOgreRenderContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CRmlUIOgreContext*
GetOgreContext( const GUI::CWidget& widget )
{
    GUI::CGUIContext* context = widget.GetContext();
    if ( GUCEF_NULL == context || CRmlUIOgreContext::ClassTypeName != context->GetClassTypeName() )
        return GUCEF_NULL;
    return static_cast< CRmlUIOgreContext* >( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CRmlUIOgreRenderContext::GetImageAlias( const GUI::CWidget& widget )
{GUCEF_TRACE;

    return "RmlUiOgre/Texture/" + CORE::PointerToString( &widget );
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreRenderContext::SetOgreTexture( GUI::CDrawSurface& widget        ,
                                         const Ogre::TexturePtr& texture  ,
                                         const GUI::CRect& target         )
{GUCEF_TRACE;

    CRmlUIOgreContext* context = GetOgreContext( widget );
    if ( GUCEF_NULL == context || !texture )
        return false;
    const CORE::CString alias = GetImageAlias( widget );
    if ( !context->RegisterOgreTexture( alias, texture ) )
        return false;
    widget.Clear();
    widget.DrawImage( alias, target, GUI::CRect( 0, 0, 1, 1 ) );
    widget.Commit();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRmlUIOgreRenderContext::ClearOgreTexture( GUI::CDrawSurface& widget )
{GUCEF_TRACE;

    CRmlUIOgreContext* context = GetOgreContext( widget );
    if ( GUCEF_NULL == context )
        return false;
    widget.Clear();
    widget.Commit();
    return context->UnregisterOgreTexture( GetImageAlias( widget ) );
}

/*-------------------------------------------------------------------------*/

Ogre::TexturePtr
CRmlUIOgreRenderContext::GetOgreTexture( const GUI::CDrawSurface& widget )
{GUCEF_TRACE;

    CRmlUIOgreContext* context = GetOgreContext( widget );
    if ( GUCEF_NULL == context )
        return Ogre::TexturePtr();
    return context->GetRegisteredOgreTexture( GetImageAlias( widget ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
