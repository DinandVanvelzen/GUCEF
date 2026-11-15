/*
 *  assetpluginUNITY: a clip split of a model's .meta file
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYCLIPANIMATION_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYCLIPANIMATION_H

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
 *  One item of animations.clipAnimations of a model .meta file: a named clip cut from a take of the model file,
 *  from 'firstFrame' to 'lastFrame' (frames of the file's timeline at the frame rate of the file), with its loop
 *  settings. A value type.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityClipAnimation
{
    public:

    CORE::CString name;
    CORE::CString takeName;
    Float32 firstFrame;
    Float32 lastFrame;
    bool loopTime;
    bool loop;
    Int32 wrapMode;
    bool mirror;
    Float32 cycleOffset;
    UInt32 eventCount;
    Int64 internalId;           /**< "internalID" of newer files, 0 when absent */

    CUnityClipAnimation( void );

    /** Reads a clipAnimations item */
    bool Read( const CORE::CDataNode& item );
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYCLIPANIMATION_H ? */
