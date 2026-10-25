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

#ifndef GUCEF_GUI_CGROUPBOX_H
#define GUCEF_GUI_CGROUPBOX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CPANEL_H
#include "gucefGUI_CPanel.h"
#define GUCEF_GUI_CPANEL_H
#endif /* GUCEF_GUI_CPANEL_H ? */

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
 *  A frame around widgets that belong together, with a caption that says what they are about in a gap of the top
 *  line, as settings pages often have. Registered as "GroupBox", a "VBox": the children are placed below each other
 *  inside the frame (Padding, Spacing), below the caption. Without a size it is as large as its children need.
 */
class GUCEF_GUI_PUBLIC_CPP CGroupBox : public CPanel
{
    public:

    CGroupBox( void );

    virtual ~CGroupBox();

    bool SetCaption( const CString& caption );

    CString GetCaption( void ) const;

    /** The height the caption takes at the top: a line of its font, 0 without a caption */
    Int32 GetCaptionHeight( void ) const;

    /** The size of the children below each other, with the padding and the caption */
    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "GroupBox"; }

    static CWidget* Create( void );

    private:

    CGroupBox( const CGroupBox& src );
    CGroupBox& operator=( const CGroupBox& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGROUPBOX_H ? */
