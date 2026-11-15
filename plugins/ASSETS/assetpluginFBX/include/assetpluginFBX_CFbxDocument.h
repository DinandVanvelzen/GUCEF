/*
 *  assetpluginFBX: an FBX file read through the VFS with ufbx
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

#ifndef GUCEF_ASSETPLUGIN_FBX_CFBXDOCUMENT_H
#define GUCEF_ASSETPLUGIN_FBX_CFBXDOCUMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETPLUGIN_FBX_MACROS_H
#include "assetpluginFBX_macros.h"
#define GUCEF_ASSETPLUGIN_FBX_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_FBX_MACROS_H ? */

#include "ufbx.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An FBX file, binary or ASCII: the bytes are read through the VFS and parsed by ufbx from memory, so a file in an
 *  archive of the VFS loads like any other. The scene is loaded as the file has it: the axes and the unit of the file
 *  stay (the importer states them for the normalisation), while what a scene graph of glTF cannot express is
 *  resolved by ufbx on load: geometry transforms are applied to the geometry (helper nodes where a mesh is shared),
 *  pivots become node transforms, scale inheritance is compensated, normals are generated where the file has none.
 *  Not thread-safe; one instance per file and thread.
 */
class GUCEF_ASSETPLUGIN_FBX_EXPORT_CPP CFbxDocument
{
    public:

    CFbxDocument( void );

    ~CFbxDocument();

    /** Reads and parses the file; 'problem' says what went wrong */
    bool Load( const CORE::CString& vfsPath, CORE::CString& problem );

    void Clear( void );

    bool IsLoaded( void ) const;

    const ufbx_scene* GetScene( void ) const;

    const CORE::CString& GetPath( void ) const;

    private:

    CFbxDocument( const CFbxDocument& src );            /**< not implemented */
    CFbxDocument& operator=( const CFbxDocument& src ); /**< not implemented */

    private:

    ufbx_scene* m_scene;
    CORE::CString m_path;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_FBX_CFBXDOCUMENT_H ? */
