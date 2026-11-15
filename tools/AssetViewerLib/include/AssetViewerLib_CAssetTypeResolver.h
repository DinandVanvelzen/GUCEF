/*
 *  AssetViewerLib: the kinds of assets the viewer shows and the names of those kinds in the content browser
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

#ifndef ASSETVIEWERLIB_CASSETTYPERESOLVER_H
#define ASSETVIEWERLIB_CASSETTYPERESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CICONTENTTYPERESOLVER_H
#include "gucefGUI_CIContentTypeResolver.h"
#define GUCEF_GUI_CICONTENTTYPERESOLVER_H
#endif /* GUCEF_GUI_CICONTENTTYPERESOLVER_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The kinds of files the viewer has a view for */
enum EAssetKind
{
    ASSETKIND_OTHER = 0 ,   /**< nothing to show */
    ASSETKIND_MODEL     ,   /**< an Ogre .mesh or a source model that the Assimp codec of Ogre reads */
    ASSETKIND_TEXTURE   ,   /**< a picture: gucefIMAGE and Ogre codecs, DDS and KTX included */
    ASSETKIND_MATERIAL  ,   /**< an Ogre .material script */
    ASSETKIND_SCENE     ,   /**< a DotScene .scene */
    ASSETKIND_SKELETON  ,   /**< an Ogre .skeleton */
    ASSETKIND_FOLDER    ,   /**< a directory, for the type column */
    ASSETKIND_PARTICLE      /**< an Ogre .particle script: its templates are played side by side */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Says what kind of asset a path is from its extension, without reading the content: the resolver of the content browser
 *  of the viewer, and the choice of the view for an asset. The type names are Model, Texture, Material, Scene, Skeleton and
 *  Other (Folder for a directory).
 */
class ASSETVIEWERLIB_PUBLIC_CPP CAssetTypeResolver : public GUI::CIContentTypeResolver
{
    public:

    CAssetTypeResolver( void );

    virtual ~CAssetTypeResolver();

    virtual CORE::CString GetTypeName( const CORE::CString& path, bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetIconName( const CORE::CString& path, bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE;

    /** The kind of a path by its extension (case does not matter) */
    static EAssetKind GetKind( const CORE::CString& path );

    /** The name of a kind, as the type column shows it */
    static CORE::CString GetKindName( EAssetKind kind );

    /** The extensions (lower case, without the dot) of the files of a kind, in the order of the table of the resolver */
    static void GetExtensions( EAssetKind kind, CORE::CString::StringVector& extensions );

    /** The extension of a path without the dot, in lower case; empty without one */
    static CORE::CString GetExtension( const CORE::CString& path );

    private:

    CAssetTypeResolver( const CAssetTypeResolver& src );
    CAssetTypeResolver& operator=( const CAssetTypeResolver& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CASSETTYPERESOLVER_H ? */
