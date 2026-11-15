/*
 *  assetpluginGLTF: the URIs of glTF files: percent encoding, decoding and resolution against the folder of the file
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#include "assetpluginGLTF_macros.h"
#define GUCEF_ASSETPLUGIN_GLTF_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_MACROS_H ? */

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
 *  The URIs of buffers and images in a glTF file are relative references, percent encoded. VFS paths here have '/'
 *  between the parts and no '/' at the start or the end (as the converter library uses them).
 */
class GUCEF_ASSETPLUGIN_GLTF_EXPORT_CPP CGltfUri
{
    public:

    /** Percent encodes everything but the unreserved characters and '/' */
    static CORE::CString Encode( const CORE::CString& path );

    /** Decodes the percent escapes */
    static CORE::CString Decode( const CORE::CString& uri );

    /** Whether the URI is a data URI ("data:...") */
    static bool IsDataUri( const CORE::CString& uri );

    /** The VFS path of a relative URI of a file in 'folder': decoded, with '.' and '..' parts resolved */
    static CORE::CString Resolve( const CORE::CString& folder, const CORE::CString& uri );

    /** The relative URI, encoded, from a file in 'folder' to the file at 'path' ('..' parts where it is not below the folder) */
    static CORE::CString MakeRelative( const CORE::CString& folder, const CORE::CString& path );

    private:

    CGltfUri( void );
    ~CGltfUri();
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

#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFURI_H ? */
