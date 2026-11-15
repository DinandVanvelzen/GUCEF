/*
 *  AssetViewerLib: the materials and textures the views draw with themselves
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

#ifndef ASSETVIEWERLIB_CVIEWERMATERIALS_H
#define ASSETVIEWERLIB_CVIEWERMATERIALS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The few materials and the one texture that the views make for themselves, in the internal resource group of Ogre:
 *    - Lines: unlit, the colour of the vertices, for the grid, the axes, the bounds, the normals and the bones;
 *    - UvChecker: a lit checkerboard that shows how the texture coordinates lie on a surface;
 *    - VertexColours: unlit, the colour of the vertices.
 *  They have no shader programs: the shader generator of Ogre makes them. Ensure() makes what is not there yet and can be
 *  called as often as needed; the materials go with the Root of Ogre.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CViewerMaterials
{
    public:

    static const CORE::CString LinesMaterialName;
    static const CORE::CString UvCheckerMaterialName;
    static const CORE::CString VertexColoursMaterialName;
    static const CORE::CString UvCheckerTextureName;

    /** Makes the materials and the texture; false when Ogre has no Root or no render system to make them in yet */
    static bool Ensure( void );

    /** Takes them out again (the materials may be used by objects that are still around: destroy those first) */
    static void Release( void );

    private:

    CViewerMaterials( void );
    CViewerMaterials( const CViewerMaterials& src );
    CViewerMaterials& operator=( const CViewerMaterials& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CVIEWERMATERIALS_H ? */
