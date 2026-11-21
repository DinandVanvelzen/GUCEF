/*
 *  AssetWorkbenchLib: plays the Ogre particle systems of what the workbench shows
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

#ifndef ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H
#define ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H
#include "AssetWorkbenchLib_CWorkbenchStage.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSTAGE_H ? */

#ifndef ASSETWORKBENCHLIB_CORBITCONTROLLER_H
#include "AssetWorkbenchLib_COrbitController.h"
#define ASSETWORKBENCHLIB_CORBITCONTROLLER_H
#endif /* ASSETWORKBENCHLIB_CORBITCONTROLLER_H ? */

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
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Plays the Ogre particle systems below a scene node (those a DotScene made) or of the templates of a particle script,
 *  driven by the workbench's time line instead of Ogre's frame time: their speed factor is 0, and Advance() runs their
 *  updates in steps of at most 1/30 s. SetTime() clears the systems, starts their emitters again, applies the prewarm
 *  (the "particlePrewarm" user data the cooker writes on the node) and runs them up to the time, so a scrub shows the
 *  state at that time. Needs the ParticleFX plugin of Ogre for the emitters and affectors of the scripts.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CParticlePreview
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

    /**
     *  Parses the material scripts of the folder of a file of a resource group (each file once per group): the meshes of a
     *  cooked scene name the materials of the script next to it. The number of material scripts of the folder.
     */
    static UInt32 ParseMaterialsNear( const CORE::CString& groupName, const CORE::CString& fileName );

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

    /**
     *  Runs the systems for 'seconds' in steps of a quarter of a second, adds the box of the particles at every step to what was seen
     *  (see GetBoundingBox()), and puts the time back where it was. For the camera to know how far an effect reaches before it plays
     */
    void ObserveRun( Float32 seconds );

    /** Whether ObserveRun() ran for the systems that are there */
    bool HasObservedRun( void ) const;

    /**
     *  The box that the systems filled while they were looked at (every call and every Advance() adds what the particles are
     *  now), for the camera to frame. A box with no side bigger than 'minimumSize' (a system that has not run yet, or that
     *  has no particles) is made 'minimumSize' wide around its middle, so that there is always something sensible to frame.
     *  False when no system has a box yet and none can be told from the emitters
     */
    bool GetBoundingBox( COrbitController::SVector3& minimum, COrbitController::SVector3& maximum, Float32 minimumSize ) const;

    /** "Particles": the number of systems and particles, and per system the template, the quota and the particles alive */
    void WriteInspectorData( CORE::CDataNode& root ) const;

    private:

    CParticlePreview( const CParticlePreview& src );
    CParticlePreview& operator=( const CParticlePreview& src );

    void CollectBelow( Ogre::SceneNode* node );

    /** Adds the box of the particles as they are now to what was seen */
    void Observe( void ) const;

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
    bool m_observedRun;
    mutable bool m_hasObserved;
    mutable Float32 m_observed[ 6 ];     /**< minimum x, y, z and maximum x, y, z of what the particles reached */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H ? */
