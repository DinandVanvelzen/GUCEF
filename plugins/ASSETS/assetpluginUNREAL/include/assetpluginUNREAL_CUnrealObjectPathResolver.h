/*
 *  assetpluginUNREAL: Maps package names to the files that hold them
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#include "AssetConverterLib_CAssetPackContext.h"
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETSCENE_H
#include "AssetConverterLib_CAssetScene.h"
#define ASSETCONVERTERLIB_CASSETSCENE_H
#endif /* ASSETCONVERTERLIB_CASSETSCENE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Finds the file of a package by its package name. A pack that was indexed (CAssetPackContext, index "unreal.package")
 *  is asked first; packages that were added by hand or found by scanning a folder of the VFS come next. Engine and plugin
 *  content that is not in the pack is not found, which the callers report.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealObjectPathResolver
{
    public:

    CUnrealObjectPathResolver( void );

    /** The pack context to ask, not owned; may be NULL */
    void SetContext( ASSETCONVERTER::CAssetPackContext* context );

    void AddPackage( const CORE::CString& packageName, const CORE::CString& vfsPath );

    /** Opens every .uasset and .umap below the VFS folder for its package name and adds it; returns how many were added */
    UInt32 AddPackagesOf( const CORE::CString& vfsFolder );

    /** @param objectPath a package name or an object path: everything after the first dot is ignored */
    bool Resolve( const CORE::CString& objectPath, CORE::CString& vfsPath ) const;

    /** The package name of an object path: the text in front of the first dot */
    static CORE::CString PackageNameOf( const CORE::CString& objectPath );

    /**
     *  The package name of a package file: the one its summary carries, or, when the summary carries none (packages of UE 4 store
     *  "None"), the one its place gives: the path below a folder "Content" with /Game in front, below "Plugins/<Name>/Content" with
     *  /<Name> in front, without the extension. Empty when neither is there.
     */
    static CORE::CString PackageNameOfFile( const CORE::CString& summaryPackageName, const CORE::CString& path );

    /** The scene that ResolveReference() records the normalized relations of the import in (converter design 5.8), not owned; may be NULL */
    void SetRecorder( ASSETCONVERTER::CAssetScene* scene );

    /**
     *  Resolve(), and records the outcome as a relation of the kind in the recorder: resolved with the object path as the original
     *  reference, or unresolved (engine and plugin content that is not in the pack)
     */
    bool ResolveReference( const CORE::CString& objectPath, const CORE::CString& kind, CORE::CString& vfsPath, bool optional = false ) const;

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString > TPathMap;

    ASSETCONVERTER::CAssetPackContext* m_context;
    ASSETCONVERTER::CAssetScene* m_recorder;
    TPathMap m_paths;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPATHRESOLVER_H ? */
