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

#ifndef GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H
#define GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_ETYPES_H
#include "gucefGUI_ETypes.h"
#define GUCEF_GUI_ETYPES_H
#endif /* GUCEF_GUI_ETYPES_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Changes the text the painters draw, not the text the widgets hold: a context with a transform
 *  (CGUIContext::SetDisplayTextTransform()) shows every display text through it, while the properties, the models
 *  and the events keep the real text. Used for pseudo-localisation (CPseudoLocalizer), which makes the room a
 *  translation needs visible without a translation. Text the user types into an edit box is not transformed.
 */
class GUCEF_GUI_PUBLIC_CPP CIDisplayTextTransform
{
    public:

    CIDisplayTextTransform( void );

    virtual ~CIDisplayTextTransform();

    /** The text as it is shown */
    virtual CString TransformText( const CString& text ) = 0;

    /** What the line height of every font is multiplied by (a line spacing stress); 1 by default */
    virtual Float32 GetLineSpacing( void ) const;

    private:

    CIDisplayTextTransform( const CIDisplayTextTransform& src );
    CIDisplayTextTransform& operator=( const CIDisplayTextTransform& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H ? */
