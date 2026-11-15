/*
 *  AssetViewerLib: the material view: the materials of a script on a sphere, a cube or a plane, with their values in the inspector
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

#ifndef ASSETVIEWERLIB_CMATERIALVIEW_H
#define ASSETVIEWERLIB_CMATERIALVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETVIEWERLIB_CMODELVIEW_H
#include "AssetViewerLib_CModelView.h"
#define ASSETVIEWERLIB_CMODELVIEW_H
#endif /* ASSETVIEWERLIB_CMODELVIEW_H ? */

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
 *  The materials of an Ogre .material script. The script is read from a resource group, the material that is picked is put on a
 *  preview shape (a sphere, a cube or a plane, made by Ogre) that sits in the stage like a model does (GetPreview()), and the
 *  inspector lists the first pass of the first technique: the lighting colours, the shininess, the culling and depth settings,
 *  and one row per texture slot with the size, format and memory of its texture. The colours, the shininess, the lighting switch
 *  and the depth write switch can be edited; the shaders of the scene are made again after an edit.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CMaterialView
{
    public:

    enum EShape
    {
        SHAPE_SPHERE = 0    ,
        SHAPE_CUBE          ,
        SHAPE_PLANE         ,
        SHAPE_COUNT
    };

    explicit CMaterialView( CViewerStage& stage );

    ~CMaterialView();

    /** Reads the script of a resource group and shows its first material */
    bool LoadScript( const CORE::CString& groupName, const CORE::CString& scriptName );

    /** Shows a material that Ogre knows already (also one of a model) */
    bool ShowMaterial( const CORE::CString& materialName, const CORE::CString& groupName );

    void Unload( void );

    const CORE::CString::StringVector& GetMaterialNames( void ) const;
    const CORE::CString& GetSelectedMaterial( void ) const;

    void SetShape( EShape shape );
    EShape GetShape( void ) const;
    static const char* GetShapeName( EShape shape );

    /** The preview shape, in the stage: the shell frames it and updates it like a model */
    CModelView& GetPreview( void );

    /** The names of the materials a script defines ("material <name>" at the start of a line, with the quotes of a name taken off) */
    static void ParseMaterialNames( const CORE::CString& scriptText, CORE::CString::StringVector& names );

    void WriteInspectorData( CORE::CDataNode& root ) const;
    void GetInspectorDescriptors( GUI::CPropertyInspectorForm::TDescriptorMap& descriptors ) const;
    bool ApplyProperty( const CORE::CString& path, const CORE::CString& value );

    private:

    CMaterialView( const CMaterialView& src );
    CMaterialView& operator=( const CMaterialView& src );

    Ogre::MaterialPtr GetMaterial( void ) const;
    bool ShowShape( void );

    private:

    CViewerStage& m_stage;
    CModelView m_preview;
    CORE::CString::StringVector m_materialNames;
    CORE::CString m_selectedMaterial;
    CORE::CString m_groupName;
    EShape m_shape;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CMATERIALVIEW_H ? */
