/*
 *  assetpluginGODOT: resolves the resources a Godot section refers to
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CIGODOTRESOURCERESOLVER_H
#define GUCEF_ASSETPLUGIN_GODOT_CIGODOTRESOURCERESOLVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#include "assetpluginGODOT_CGodotDocument.h"
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTDOCUMENT_H ? */

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
 *  What a reader of a part of a Godot scene needs from the scene reader that owns the documents, the materials and the
 *  meshes: a reference (ExtResource or SubResource) resolved to the section of its resource, and a material or a mesh
 *  resource read into the scene once. Unresolved references are reported by the implementation.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CIGodotResourceResolver
{
    public:

    /** The section of the resource a reference names (a sub resource, or the main resource of another file) and its document */
    virtual bool ResolveResourceSection( const CGodotDocument& document            ,
                                         const CORE::CDataNode* value              ,
                                         const CGodotDocument*& resourceDocument   ,
                                         const CORE::CDataNode*& section           ) = 0;

    /** The material a reference names, read into the scene; -1 when it cannot be read */
    virtual Int32 ResolveMaterialResource( const CGodotDocument& document, const CORE::CDataNode* value ) = 0;

    /** The mesh a section of a mesh resource makes, read into the scene; -1 when it cannot be read */
    virtual Int32 ResolveMeshSection( const CGodotDocument& document, const CORE::CDataNode& section ) = 0;

    CIGodotResourceResolver( void );

    virtual ~CIGodotResourceResolver();

    private:

    CIGodotResourceResolver( const CIGodotResourceResolver& src );            /**< not implemented */
    CIGodotResourceResolver& operator=( const CIGodotResourceResolver& src ); /**< not implemented */
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

#endif /* GUCEF_ASSETPLUGIN_GODOT_CIGODOTRESOURCERESOLVER_H ? */
