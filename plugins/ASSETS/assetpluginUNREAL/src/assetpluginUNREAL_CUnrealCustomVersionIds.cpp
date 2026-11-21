/*
 *  assetpluginUNREAL: Names of the custom versions the readers know
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#include "assetpluginUNREAL_CUnrealCustomVersionIds.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCUSTOMVERSIONIDS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SCustomVersionName
{
    UInt32 a;
    UInt32 b;
    UInt32 c;
    UInt32 d;
    const char* name;
};

static const SCustomVersionName g_names[] =
{
    { 0xE4B068ED, 0xF49442E9, 0xA231DA0B, 0x2E46BB41, "EditorObjectVersion" },
    { 0x12F88B9F, 0x88754AFC, 0xA67CD90C, 0x383ABD29, "RenderingObjectVersion" },
    { 0x9C54D522, 0xA8264FBE, 0x94210746, 0x61B482D0, "ReleaseObjectVersion" },
    { 0xCFFC743F, 0x43B04480, 0x939114DF, 0x171D2073, "FrameworkObjectVersion" },
    { 0x697DD581, 0xE64F41AB, 0xAA4A51EC, 0xBEB7B628, "Ue5MainStreamObjectVersion" },
    { 0xD89B5E42, 0x24BD4D46, 0x8412ACA8, 0xDF641779, "Ue5ReleaseStreamObjectVersion" },
    { 0x601D1886, 0xAC644F84, 0xAA16D3DE, 0x0DEAC7D6, "FortniteMainBranchObjectVersion" },
    { 0xE7086368, 0x6B234C58, 0x84391B70, 0x16265E91, "FortniteReleaseBranchObjectVersion" },
    { 0xD78A4A00, 0xE8584697, 0xBAA819B5, 0x487D46B4, "SkeletalMeshCustomVersion" },
    { 0xAF43A65D, 0x7FD34947, 0x98733E8E, 0xD9C1BB05, "AnimationObjectVersion" },
    { 0x29E575DD, 0xE0A34627, 0x9D10D276, 0x232CDCEA, "AnimationPhysicsObjectVersion" },
    { 0x375EC13C, 0x06E448FB, 0xB50084F0, 0x262A717E, "CoreObjectVersion" },
    { 0xF2AED0AC, 0x9AFE416F, 0x8664AA7F, 0xFA26D6FC, "NiagaraObjectVersion" },
    { 0xE21E1CAA, 0xAF47425E, 0x89BF6AD4, 0x4C44A8BB, "InstancedStructCustomVersion" },
    { 0, 0, 0, 0, GUCEF_NULL }
};

/*-------------------------------------------------------------------------*/

const char*
CUnrealCustomVersionIds::GetName( const CUnrealGuid& id )
{GUCEF_TRACE;

    for ( const SCustomVersionName* entry = g_names; GUCEF_NULL != entry->name; ++entry )
        if ( entry->a == id.a && entry->b == id.b && entry->c == id.c && entry->d == id.d )
            return entry->name;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
