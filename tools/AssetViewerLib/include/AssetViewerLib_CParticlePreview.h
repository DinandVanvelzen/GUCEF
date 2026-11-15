/*
 *  AssetViewerLib: plays the Ogre particle systems of what the viewer shows
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

#ifndef ASSETVIEWERLIB_CPARTICLEPREVIEW_H
#define ASSETVIEWERLIB_CPARTICLEPREVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETVIEWERLIB_CVIEWERSTAGE_H
#include "AssetViewerLib_CViewerStage.h"
#define ASSETVIEWERLIB_CVIEWERSTAGE_H
#endif /* ASSETVIEWERLIB_CVIEWERSTAGE_H ? */

#ifndef ASSETVIEWERLIB_CORBITCONTROLLER_H
#include "AssetViewerLib_COrbitController.h"
#define ASSETVIEWERLIB_CORBITCONTROLLER_H
#endif /* ASSETVIEWERLIB_CORBITCONTROLLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace Ogre {
class ParticleSystem;
class SceneNode;
}

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Plays the Ogre particle systems below a scene node (those a DotScene made) or of the templates of a particle script,
 *  driven by the viewer's time line instead of Ogre's frame time: their speed factor is 0, and Advance() runs their
 *  updates in steps of at most 1/30 s. SetTime() clears the systems, starts their emitters again, applies the prewarm
 *  (the "particlePrewarm" user data the cooker writes on the node) and runs them up to the time, so a scrub shows the
 *  state at that time. Needs the ParticleFX plugin of Ogre for the emitters and affectors of the scripts.
 */
class ASSETVIEWERLIB_PUBLIC_CPP CParticlePreview
{
    public:

    CParticlePreview( void );

    ~CParticlePreview();

    /**
     *  Parses the material scripts and the particle scripts of the folder of a file of a resource group (each file once
     *  per group), when that folder has a particle script: a cooked scene needs both before it is loaded. The number of
     *  particle scripts parsed.
     */
    static UInt32 ParseScriptsNear( const CORE::CString& groupName, const CORE::CString& fileName );

    /** Parses one particle script (once per group) and gives the names of the templates it defines */
    static bool ParseParticleScript( const CORE::CString& groupName, const CORE::CString& fileName, CORE::CString::StringVector& templates );

    /** Takes over the particle systems below the node; the number found */
    UInt32 Collect( Ogre::SceneNode* root );

    /** Makes a system of every template below the node, side by side along X; the number made (destroyed by Clear()) */
    UInt32 CreateFromTemplates( Ogre::SceneManager* sceneManager, Ogre::SceneNode* parent, const CORE::CString::StringVector& templates );

    /** Forgets the systems; those CreateFromTemplates() made are destroyed */
    void Clear( void );

    UInt32 GetSystemCount( void ) const;

    /** The particles alive in all systems */
    UInt32 GetParticleCount( void ) const;

    /** A length for the time line: the longest emission cycle plus lifetime of the systems, at least a second */
    UInt32 GetLengthInMs( void ) const;

    void SetPlaying( bool playing );
    bool IsPlaying( void ) const;

    /** Clears the systems, starts their emitters again and applies their prewarm */
    void Restart( void );

    /** Restart() and run up to the time */
    void SetTime( Float32 seconds );

    /** Runs the systems on by the time */
    void Advance( Float32 seconds );

    Float32 GetTime( void ) const;

    /** The box of the particles of all systems in the world; false while they have none */
    bool GetBoundingSphere( COrbitController::SVector3& center, Float32& radius ) const;

    /** "Particles": the number of systems and particles, and per system the template, the quota and the particles alive */
    void WriteInspectorData( CORE::CDataNode& root ) const;

    private:

    CParticlePreview( const CParticlePreview& src );
    CParticlePreview& operator=( const CParticlePreview& src );

    void CollectBelow( Ogre::SceneNode* node );

    struct SSystem
    {
        Ogre::ParticleSystem* system;
        Float32 prewarm;
        bool owned;
    };
    typedef GUCEF::vector< SSystem > TSystemVector;

    TSystemVector m_systems;
    GUCEF::vector< Ogre::SceneNode* > m_ownedNodes;
    Ogre::SceneManager* m_ownedSceneManager;
    Float32 m_time;
    bool m_playing;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CPARTICLEPREVIEW_H ? */
