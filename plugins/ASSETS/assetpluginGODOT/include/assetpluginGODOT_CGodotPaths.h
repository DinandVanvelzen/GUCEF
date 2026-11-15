/*
 *  assetpluginGODOT: paths and identities of a Godot project
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#include "assetpluginGODOT_macros.h"
#define GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The paths of a Godot project inside a pack. A project is the folder with project.godot; "res://" is that folder.
 *  The pack can be a project, a folder of projects, or a folder of assets without project file (then "res://" is the
 *  root of the pack, and a path that is not found there is looked for by its tail in the whole pack). A "uid://" is
 *  resolved through the index "uid", which is filled from the .import, .tscn, .tres, .res and .mesh files when a path
 *  leads to no file (a path that is there is taken as it is). A uid whose file is not in the
 *  pack, such as that of a source file the project imported and does not hold, does not resolve.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotPaths
{
    public:

    static const CORE::CString UidIndexName;        /**< "uid" */
    static const CORE::CString LowerIndexName;      /**< "lower": the paths of the pack by their lower case text, filled when a path is found by another letter case */
    static const CORE::CString IndexBuiltKey;       /**< "<built>": the key that marks an index as filled */
    static const CORE::CString ResPrefix;           /**< "res://" */
    static const CORE::CString UidPrefix;           /**< "uid://" */

    /** The folder with project.godot that holds the file (a path in the pack; empty for the root of the pack); false without such a folder */
    static bool FindProjectRoot( const ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& assetPath, CORE::CString& projectRoot );

    /**
     *  The path in the pack of a resource of a file: the res:// path (against the project of the file, the root of the
     *  pack, the folders above the file, then in another letter case), then the uid, then the tail of the path. False
     *  when the file is not in the pack.
     */
    static bool Resolve( ASSETCONVERTER::CAssetPackContext& context, const CORE::CString& referrer, const CORE::CString& path, const CORE::CString& uid, CORE::CString& assetPath );

    /** The path of "res://a/b" as "a/b"; the text itself for a path without the prefix */
    static CORE::CString StripScheme( const CORE::CString& path );

    /** The first line of a text file of the VFS (up to 512 bytes); empty when it cannot be read */
    static CORE::CString ReadHeaderLine( const CORE::CString& vfsPath );

    /** The uid of the first line of a scene or resource file; empty without one */
    static CORE::CString ReadHeaderUid( const CORE::CString& vfsPath );

    /**
     *  Adds the uids of the .import, .tscn, .tres, .res and .mesh files of the pack to the index "uid" of the context,
     *  once per context. A file of an archive is read by scanning the archive up to it, so Resolve() does this only when a
     *  path leads to no file, and not for the whole pack up front. Returns the number of uids added.
     */
    static UInt32 IndexUids( ASSETCONVERTER::CAssetPackContext& context );

    private:

    static void EnsureLowerIndex( ASSETCONVERTER::CAssetPackContext& context );

    CGodotPaths( void );
    ~CGodotPaths();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTPATHS_H ? */
