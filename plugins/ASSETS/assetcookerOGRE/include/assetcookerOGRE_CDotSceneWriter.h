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

#ifndef GUCEF_ASSETCOOKER_OGRE_CDOTSCENEWRITER_H
#define GUCEF_ASSETCOOKER_OGRE_CDOTSCENEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H
#include "assetcookerOGRE_CSceneMath.h"
#define GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H
#endif /* GUCEF_ASSETCOOKER_OGRE_CSCENEMATH_H ? */

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
 *  Writes DotScene files (format version 1.1, as OgrePlugin_DotScene of 14.6.3 reads them) of the node tree of an
 *  asset or of one of its prefabs.
 *
 *  - Every node becomes a <node> with its local position, rotation and scale; names are those of the intermediate,
 *    made unique within the file (Ogre needs unique scene node names; the loader's "namePrefix" option keeps several
 *    loaded files apart). The levels of a LOD group after the first are left out: they are manual levels of the mesh.
 *  - A node with a mesh gets an <entity> of the same name with the cooked mesh. A skinned mesh is drawn in the space of
 *    the scene, as glTF skins are, so its entity is on a node of its own ("<node>.skin") at the top of the file, placed
 *    at the space of the scene relative to the root of the file.
 *  - A node with a light gets a <light>: glTF directional, point and spot lights with colour, the intensity as power
 *    scale (the photometric units are not converted), range and inverse square attenuation, and the spot cone (glTF
 *    gives half angles, Ogre full angles).
 *  - A node with a particle system gets a <particleSystem> of the same name with the template of the cooked particle
 *    script; the prewarm time, the start delay and whether the system plays from the start, which Ogre's scripts
 *    cannot hold, become <userData> properties "particlePrewarm", "particleStartDelay" and "particlePlayOnAwake".
 *  - Material overrides of prefab instances ("materials" in the overrides: material names of the scene, one per
 *    submesh) become the entity's "material" when there is one, else a user data property "materials".
 *  - The extras of a node (plain values) and the prefab it is an instance of (the node's reference, else the source
 *    prefab of an instance of the prefab written) become <userData> properties.
 *  - The clips that animate nodes that are not joints become <animations> of those nodes, named
 *    "<clip>/<node>", with keys relative to the written transform, as Ogre's node tracks are.
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CDotSceneWriter
{
    public:

    typedef GUCEF::vector< UInt32 > TUInt32Vector;
    typedef GUCEF::map< UInt32, CORE::CString > TNameMap;

    CDotSceneWriter( const ASSETCONVERTER::CAssetScene& scene, ASSETCONVERTER::CConversionReport& report );

    /** The cooked mesh of a node */
    void SetEntity( UInt32 node, const CORE::CString& meshName, bool skinned );

    /** The particle system template of a node, with the values the template cannot hold */
    void SetParticleSystem( UInt32 node, const CORE::CString& templateName, Float32 prewarm, Float32 startDelay, bool playOnAwake );

    /** A node to leave out, with its subtree */
    void Exclude( UInt32 node );

    /** The Ogre material names of the materials of the scene, for the overrides */
    void SetMaterialNames( const CORE::CString::StringVector& materialNames );

    /**
     *  The DotScene text of the subtrees of the roots. 'prefab' gives the overrides of its instances, NULL for none.
     *  'spaceRoot' is the node whose parent space is the space of the file, -1 for the scene.
     */
    CORE::CString Write( const TUInt32Vector& roots, const ASSETCONVERTER::CAssetPrefab* prefab, Int32 spaceRoot );

    /** The mesh resource names the last Write() referenced, each once */
    const CORE::CString::StringVector& GetReferences( void ) const;

    /** The name every node of the last Write() got */
    const TNameMap& GetNodeNames( void ) const;

    static CORE::CString Escape( const CORE::CString& text );

    private:

    void WriteNode( UInt32 node, UInt32 depth );

    void WriteTransform( const CSceneMath::STrs& trs, UInt32 depth, bool always );

    void WriteLight( UInt32 node, UInt32 depth );

    void WriteUserData( UInt32 node, UInt32 depth );

    void WriteParticleSystem( UInt32 node, UInt32 depth );

    void WriteAnimations( UInt32 node, UInt32 depth );

    void Line( UInt32 depth, const CORE::CString& text );

    CORE::CString GetName( UInt32 node );

    void AddReference( const CORE::CString& meshName );

    struct SEntity
    {
        CORE::CString mesh;
        bool skinned;
    };
    typedef GUCEF::map< UInt32, SEntity > TEntityMap;

    struct SParticles
    {
        CORE::CString templateName;
        Float32 prewarm;
        Float32 startDelay;
        bool playOnAwake;
    };
    typedef GUCEF::map< UInt32, SParticles > TParticleMap;

    private:

    const ASSETCONVERTER::CAssetScene& m_scene;
    ASSETCONVERTER::CConversionReport& m_report;
    TEntityMap m_entities;
    TParticleMap m_particles;
    GUCEF::set< UInt32 > m_excluded;
    CORE::CString::StringVector m_materialNames;
    const ASSETCONVERTER::CAssetPrefab* m_prefab;
    CORE::CString m_text;
    CORE::CString::StringVector m_references;
    TNameMap m_names;
    CORE::CString::StringSet m_usedNames;
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

#endif /* GUCEF_ASSETCOOKER_OGRE_CDOTSCENEWRITER_H ? */
