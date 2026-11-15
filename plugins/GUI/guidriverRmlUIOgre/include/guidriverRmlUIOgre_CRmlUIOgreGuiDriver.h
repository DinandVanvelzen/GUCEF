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

#ifndef GUIDRIVERRMLUIOGRE_CRMLUIOGREGUIDRIVER_H
#define GUIDRIVERRMLUIOGRE_CRMLUIOGREGUIDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "guidriverRmlUI_CRmlUIDriver.h"

#ifndef GUIDRIVERRMLUIOGRE_MACROS_H
#include "guidriverRmlUIOgre_macros.h"
#define GUIDRIVERRMLUIOGRE_MACROS_H
#endif /* GUIDRIVERRMLUIOGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The GUI driver "RmlUiOgre": RmlUi drawn through Ogre in windows of the window backend "Ogre"
 *  (guidriverOgre), over the viewports of the application.
 *
 *  The driver registers with the GUI manager under the name "RmlUiOgre" (RegistrationName), which applications
 *  pass to CGUIManager::CreateGUIContext(). GetDriverName() stays "RmlUi", the name that the backend hints of layout
 *  files (BackendHints driver="RmlUi") are written for, so a layout gets the same hints from both RmlUi drivers.
 *  Contexts are made only for Ogre windows; other windows are refused with an error in the log.
 */
class GUIDRIVERRMLUIOGRE_PUBLIC_CPP CRmlUIOgreGuiDriver : public guidriverRmlUI::CRmlUIDriver
{
    public:

    /** "RmlUiOgre" */
    static const char* RegistrationName;

    CRmlUIOgreGuiDriver( void );

    virtual ~CRmlUIOgreGuiDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    /** Those of RmlUi, and CaptureFrame */
    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** "Renderer": Ogre and its render system, after the first context */
    virtual CORE::CString GetDriverProperty( const CORE::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRmlUIOgreGuiDriver( const CRmlUIOgreGuiDriver& src );
    CRmlUIOgreGuiDriver& operator=( const CRmlUIOgreGuiDriver& src );

    private:

    CORE::CString m_rendererDescription;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIOGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERRMLUIOGRE_CRMLUIOGREGUIDRIVER_H ? */
