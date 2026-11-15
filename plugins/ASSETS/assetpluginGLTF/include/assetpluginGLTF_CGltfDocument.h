/*
 *  assetpluginGLTF: a glTF or GLB file read through the VFS with cgltf
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H

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

#ifndef GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#include "assetpluginGLTF_macros.h"
#define GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_MACROS_H ? */

#include "cgltf.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A glTF (.gltf with external or embedded buffers) or GLB file: the bytes are read through the VFS, parsed by
 *  cgltf from memory, and the buffers it names are read through the VFS as well (cgltf is given a read callback), so
 *  a file in an archive of the VFS loads like any other. Not thread-safe; one instance per file and thread.
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfDocument
{
    public:

    CGltfDocument( void );

    ~CGltfDocument();

    /** Reads and parses the file; 'problem' says what went wrong */
    bool Load( const CORE::CString& vfsPath, CORE::CString& problem );

    /** Reads the buffers the file names */
    bool LoadBuffers( CORE::CString& problem );

    /** cgltf_validate on the data, buffers loaded: the structure, the accessors against their views and buffers, the indices against the vertices */
    bool Validate( CORE::CString& problem );

    void Clear( void );

    bool IsLoaded( void ) const;

    const cgltf_data* GetData( void ) const;

    /** The JSON text of the file (for GLB the JSON chunk); valid while the document is loaded */
    CORE::CString GetJson( void ) const;

    const CORE::CString& GetPath( void ) const;

    /** The VFS folder of the file */
    CORE::CString GetFolder( void ) const;

    /** The text of a cgltf result */
    static CORE::CString GetResultText( cgltf_result result );

    private:

    CGltfDocument( const CGltfDocument& src );              /**< not implemented */
    CGltfDocument& operator=( const CGltfDocument& src );   /**< not implemented */

    private:

    CORE::CDynamicBuffer m_file;
    cgltf_data* m_data;
    CORE::CString m_path;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFDOCUMENT_H ? */
