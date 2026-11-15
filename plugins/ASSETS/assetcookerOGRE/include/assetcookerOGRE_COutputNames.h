/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_ASSETCOOKER_OGRE_COUTPUTNAMES_H
#define GUCEF_ASSETCOOKER_OGRE_COUTPUTNAMES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The stable names of the cooked files and resources of one asset. Every name is a path relative to the folder of
 *  the cook profile and is also the Ogre resource name, so a game adds the profile folder as one recursive resource
 *  location and finds everything by these names:
 *
 *    <pack id>/<asset path without extension>/<mesh>.mesh, <skin>.skeleton, <stem>.material, <stem>.scene,
 *    <prefab>.scene, <stem>.collision.json, and the material names <pack id>/<asset path without extension>/<material>.
 *
 *  Packs never collide (the pack id leads), assets of a pack never collide (the pipeline gives every asset an
 *  intermediate of its own stem), and within an asset every name is made unique (letter case ignored, as file
 *  systems may) by appending _2, _3, ... in the order the names are asked for, which follows the order of the
 *  intermediate and is therefore stable. Characters that scripts, XML or file systems do not take become '_'.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP COutputNames
{
    public:

    COutputNames( const CORE::CString& packId, const CORE::CString& assetPath );

    /** <pack id>/<asset path without extension> */
    const CORE::CString& GetAssetFolder( void ) const;

    /** The file name of the asset without folder and extension */
    const CORE::CString& GetAssetStem( void ) const;

    /** <asset folder>/<name>.<extension>, unique among the files of the asset */
    CORE::CString MakeFileName( const CORE::CString& name, const CORE::CString& extension, const CORE::CString& fallback );

    /** <asset folder>/<name>, unique among the materials of the asset */
    CORE::CString MakeMaterialName( const CORE::CString& name, const CORE::CString& fallback );

    /** A name made safe for scripts, XML and file systems; 'fallback' when nothing is left */
    static CORE::CString Sanitize( const CORE::CString& name, const CORE::CString& fallback );

    /** Sanitizes every part of a path ('/' separated) */
    static CORE::CString SanitizePath( const CORE::CString& path );

    /** Makes 'name' unique in 'used' (letter case ignored) and adds it */
    static CORE::CString MakeUnique( const CORE::CString& name, const CORE::CString& extension, CORE::CString::StringSet& used );

    private:

    CORE::CString m_folder;
    CORE::CString m_stem;
    CORE::CString::StringSet m_files;
    CORE::CString::StringSet m_materials;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_COUTPUTNAMES_H ? */
