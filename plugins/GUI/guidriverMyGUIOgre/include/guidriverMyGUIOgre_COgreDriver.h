/*
 *  guidriverMyGUIOgre: GUI backend using MyGUI with Ogre
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

#ifndef GUCEF_guidriverMyGUIOgre_COGREDRIVER_H
#define GUCEF_guidriverMyGUIOgre_COGREDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverMyGUI_CMYGUIDRIVER_H
#include "guidriverMyGUI_CMyGUIDriver.h"
#define GUCEF_guidriverMyGUI_CMYGUIDRIVER_H
#endif /* GUCEF_guidriverMyGUI_CMYGUIDRIVER_H ? */

#ifndef GUCEF_guidriverMyGUIOgre_MACROS_H
#include "guidriverMyGUIOgre_macros.h"
#define GUCEF_guidriverMyGUIOgre_MACROS_H
#endif /* GUCEF_guidriverMyGUIOgre_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MyGUI { class OgreDataManager; }

namespace GUCEF {
namespace guidriverMyGUIOgre {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The MyGUI GUI driver for Ogre render windows (the windows of guidriverOgre: the window properties
 *  "Ogre::RenderWindow" and "Ogre::SceneManager" give what MyGUI draws in).
 *
 *  The Ogre platform of MyGUI reads everything (skins, fonts, shaders, textures) through the resource system of
 *  Ogre. The data of MyGUI.Engine is added to an Ogre resource group, "MyGUI", as a location of the archive type
 *  "GucefVFS", which the Ogre plugin OgrePlugin_GucefVFS registers: that is how the resource system of Ogre
 *  reaches the virtual file system. Pictures that Ogre decodes go through OgreCodec_GucefIMAGE the same way.
 */
class GUCEF_guidriverMyGUIOgre_PUBLIC_CPP COgreDriver : public guidriverMyGUI::CMyGUIDriver
{
    public:

    static const char* DriverName;

    /** The Ogre resource group the data of MyGUI is in */
    static const char* ResourceGroupName;

    /** The type of the Ogre resource archive that reads the virtual file system */
    static const char* ArchiveTypeName;

    COgreDriver( void );

    virtual ~COgreDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    COgreDriver( const COgreDriver& src );
    COgreDriver& operator=( const COgreDriver& src );

    bool PrepareResources( void );

    private:

    MyGUI::OgreDataManager* m_ogreDataManager;
    bool m_resourcesReady;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOgre */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUIOgre_COGREDRIVER_H ? */
