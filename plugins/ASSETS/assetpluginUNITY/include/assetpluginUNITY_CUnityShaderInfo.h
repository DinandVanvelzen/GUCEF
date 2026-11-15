/*
 *  assetpluginUNITY: the properties a shader of a package declares
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYSHADERINFO_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYSHADERINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#include "assetpluginUNITY_macros.h"
#define GUCEF_ASSETPLUGIN_UNITY_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The name and the declared properties of a shader shipped in a package: a ShaderLab .shader file (the name on the
 *  first line, Shader "<name>", and the Properties block) or a shader graph (.shadergraph, JSON; the reference names
 *  and types of its serialised properties). Materials keep the properties of every shader they were ever assigned,
 *  so the declared list tells which of their values the shader uses, and with which type ("_Opacity" can be a colour).
 *
 *  Property types: "2D", "3D", "Cube", "Color", "Vector", "Float", "Range", "Int". A value type.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityShaderInfo
{
    public:

    struct SProperty
    {
        CORE::CString name;
        CORE::CString displayName;
        CORE::CString type;
        CORE::CString::StringVector attributes;     /**< "HideInInspector", "Normal", "HDR", "NoScaleOffset", ... */
        CORE::CString defaultValue;
    };
    typedef GUCEF::vector< SProperty > TPropertyVector;

    CORE::CString name;
    CORE::CString kind;             /**< "shader" or "shaderGraph" */
    TPropertyVector properties;
    bool transparent;               /**< alpha:fade/blend, Queue Transparent, or a graph with a transparent surface */
    bool alphaTested;               /**< Queue AlphaTest or alphatest: */

    CUnityShaderInfo( void );

    /** Reads a ShaderLab text: the name and the Properties block */
    bool ParseShaderLab( const CORE::CString& text );

    /** Reads a shader graph text; 'assetName' (the file name without extension) names it "Shader Graphs/<assetName>" */
    bool ParseShaderGraph( const CORE::CString& text, const CORE::CString& assetName );

    const SProperty* FindProperty( const CORE::CString& propertyName ) const;

    bool HasAttribute( const SProperty& property, const CORE::CString& attribute ) const;

    /** Whether the type is a texture type */
    static bool IsTextureType( const CORE::CString& type );

    /** Whether the type is a colour */
    static bool IsColourType( const CORE::CString& type );

    /** Whether the type is a number (Float, Range, Int) */
    static bool IsNumberType( const CORE::CString& type );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYSHADERINFO_H ? */
