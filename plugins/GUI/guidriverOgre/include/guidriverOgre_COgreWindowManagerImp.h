/*
 *  guidriverOgre: gucefGUI window backend that gives Ogre render windows on the platform windows
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

#ifndef GUIDRIVEROGRE_COGREWINDOWMANAGERIMP_H
#define GUIDRIVEROGRE_COGREWINDOWMANAGERIMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUIDRIVEROGRE_MACROS_H
#include "guidriverOgre_macros.h"
#define GUIDRIVEROGRE_MACROS_H
#endif /* GUIDRIVEROGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The window backend "Ogre": windows with an Ogre render window (COgreWindowContext) on a window of a platform
 *  backend. Window parameter "PlatformBackend" (XWinGL on Linux, Win32, AndroidGLES, Emscripten, or Offscreen);
 *  the parameter GraphicsApi is not taken, the windows are drawn with Ogre
 */
class GUIDRIVEROGRE_PUBLIC_CPP COgreWindowManagerImp : public GUI::CWindowManagerBackend
{
    public:

    /** "Ogre" */
    static const CORE::CString BackendName;

    COgreWindowManagerImp( void );

    virtual ~COgreWindowManagerImp();

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title ,
                                                        const GUI::UInt32 width   ,
                                                        const GUI::UInt32 height  ,
                                                        const bool fullscreen     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title             ,
                                                        const GUI::CVideoSettings& settings   ,
                                                        const CORE::CValueList* params = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title      ,
                                                        const GUI::UInt32 width        ,
                                                        const GUI::UInt32 height       ,
                                                        const bool fullscreen          ,
                                                        const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyWindowContext( GUI::TWindowContextPtr& windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                     const GUI::CVideoSettings& settings   ) GUCEF_VIRTUAL_OVERRIDE;

    /** None of the values: the windows are drawn by Ogre, which makes its own surface */
    virtual bool IsGraphicsApiSupported( const GUI::CString& graphicsApi ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    COgreWindowManagerImp( const COgreWindowManagerImp& src );              /**< not implemented */
    COgreWindowManagerImp& operator=( const COgreWindowManagerImp& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< COgreWindowManagerImp, MT::CMutex > TOgreWindowManagerImpPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVEROGRE_COGREWINDOWMANAGERIMP_H ? */
