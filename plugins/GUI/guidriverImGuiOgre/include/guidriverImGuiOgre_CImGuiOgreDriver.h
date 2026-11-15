/*
 *  guidriverImGuiOgre: gucefGUI driver ImGuiOgre: Dear ImGui drawn through Ogre over the 3D scene of an Ogre window
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

#ifndef GUIDRIVERIMGUIOGRE_CIMGUIOGREDRIVER_H
#define GUIDRIVERIMGUIOGRE_CIMGUIOGREDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUIDRIVERIMGUI_CIMGUIDRIVER_H
#include "guidriverImGui_CImGuiDriver.h"
#define GUIDRIVERIMGUI_CIMGUIDRIVER_H
#endif /* GUIDRIVERIMGUI_CIMGUIDRIVER_H ? */

#ifndef GUIDRIVERIMGUIOGRE_MACROS_H
#include "guidriverImGuiOgre_macros.h"
#define GUIDRIVERIMGUIOGRE_MACROS_H
#endif /* GUIDRIVERIMGUIOGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The GUI driver "ImGuiOgre": Dear ImGui drawn through Ogre, as overlays of the windows of the window backend Ogre
 *  (guidriverOgre), over the 3D scene. A window of another window backend is refused with an error log entry.
 *  Property "Renderer": the render system of Ogre.
 */
class GUIDRIVERIMGUIOGRE_PUBLIC_CPP CImGuiOgreDriver : public GUIDRIVERIMGUI::CImGuiDriver
{
    public:

    /** The name the driver registers under and reports: "ImGuiOgre" */
    static const char* RegistrationName;

    CImGuiOgreDriver( void );

    virtual ~CImGuiOgreDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDriverProperty( const CORE::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CImGuiOgreDriver( const CImGuiOgreDriver& src );
    CImGuiOgreDriver& operator=( const CImGuiOgreDriver& src );

    private:

    CORE::CString m_rendererDescription;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIOGRE_CIMGUIOGREDRIVER_H ? */
