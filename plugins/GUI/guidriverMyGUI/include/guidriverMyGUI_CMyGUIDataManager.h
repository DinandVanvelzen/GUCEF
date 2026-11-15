/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_guidriverMyGUI_CMYGUIDATAMANAGER_H
#define GUCEF_guidriverMyGUI_CMYGUIDATAMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "MyGUI_DataManager.h"

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#include "guidriverMyGUI_macros.h"
#define GUCEF_guidriverMyGUI_MACROS_H
#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  MyGUI's access to resources (skins, layouts, fonts, shaders, pictures): everything comes out of the
 *  virtual file system, nothing is read from a disk directly. A resource name is looked up in the search
 *  paths in the order they were added (directories of the VFS, for example the mounted data of
 *  MyGUI.Engine), then as a VFS path of its own.
 */
class GUCEF_guidriverMyGUI_PUBLIC_CPP CMyGUIDataManager : public MyGUI::DataManager
{
    public:

    typedef GUCEF::vector< CORE::CString > TStringVector;

    CMyGUIDataManager( void );

    virtual ~CMyGUIDataManager();

    /** Adds a VFS directory resource names are looked up in; a path that is already there stays where it is */
    void AddSearchPath( const CORE::CString& vfsDirectory );

    void RemoveSearchPath( const CORE::CString& vfsDirectory );

    const TStringVector& GetSearchPaths( void ) const;

    /** The VFS path of a resource name; false when no file has that name */
    bool ResolvePath( const CORE::CString& name, CORE::CString& vfsPath ) const;

    virtual MyGUI::IDataStream* getData( const std::string& name ) const override;

    virtual void freeData( MyGUI::IDataStream* data ) override;

    virtual bool isDataExist( const std::string& name ) const override;

    virtual MyGUI::VectorString getDataListNames( const std::string& pattern ) const override;

    virtual std::string getDataPath( const std::string& name ) const override;

    private:

    CMyGUIDataManager( const CMyGUIDataManager& src );
    CMyGUIDataManager& operator=( const CMyGUIDataManager& src );

    private:

    TStringVector m_searchPaths;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_CMYGUIDATAMANAGER_H ? */
