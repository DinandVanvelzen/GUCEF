/*
 *  assetpluginUNITY: the reader of Unity animation clips (.anim)
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYANIMATIONCLIPREADER_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYANIMATIONCLIPREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#include "assetpluginUNITY_CUnityProject.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYPROJECT_H ? */

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
 *  Reads an AnimationClip (class 74) of a .anim file with generic transform curves into one animation, on nodes made
 *  from the paths of its curves (the root node is the animated object itself, path ""):
 *
 *  - m_PositionCurves, m_RotationCurves (quaternions) and m_ScaleCurves are Hermite curves with in and out slopes per
 *    second: without weighted tangents they are exactly glTF cubic splines; keys with infinite slopes are steps;
 *  - m_EulerCurves and the float curves of the Transform (m_LocalPosition.x, localEulerAnglesRaw.y, ...) are combined
 *    per property and sampled at the sample rate of the clip, as are curves with weighted tangents (reported);
 *  - humanoid muscle curves (Animator) and curves of other components are reported as not convertible;
 *  - m_AnimationClipSettings gives the time range and the loop flag.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityAnimationClipReader
{
    public:

    explicit CUnityAnimationClipReader( CUnityProject& project );

    /** Reads the .anim file into the scene of the project */
    bool ReadAsset( const CORE::CString& vfsPath );

    /** Reads the AnimationClip object; its curves target the nodes below 'rootNode' named by their paths (made where missing) */
    bool ReadClip( const CORE::CDataNode& clipObject, const CORE::CString& subject, UInt32 rootNode, ASSETCONVERTER::CAssetAnimation& animation );

    private:

    struct SKey
    {
        Float32 time;
        Float32 value[ 4 ];
        Float32 inSlope[ 4 ];
        Float32 outSlope[ 4 ];
        bool weighted;
    };
    typedef GUCEF::vector< SKey > TKeyVector;

    /** The keys of a curve; 'components' 1 for a float curve, 3 or 4 for vectors and quaternions */
    static void ReadKeys( const CORE::CDataNode* curve, UInt32 components, TKeyVector& keys );

    /** The value of one component of a Hermite curve at a time */
    static Float32 Evaluate( const TKeyVector& keys, UInt32 component, Float32 time );

    UInt32 GetNode( UInt32 rootNode, const CORE::CString& path );

    void AddVectorChannel( const TKeyVector& keys, UInt32 node, ASSETCONVERTER::TAnimationPath path, UInt32 components, Float32 sampleRate, const CORE::CString& subject,
                           ASSETCONVERTER::CAssetAnimation& animation );

    CUnityAnimationClipReader( const CUnityAnimationClipReader& src );              /**< not implemented */
    CUnityAnimationClipReader& operator=( const CUnityAnimationClipReader& src );   /**< not implemented */

    CUnityProject& m_project;
    GUCEF::map< CORE::CString, UInt32 > m_nodes;
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYANIMATIONCLIPREADER_H ? */
