/*
 *  assetpluginUNREAL: The two strip flag bytes that start most native object data
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#include "assetpluginUNREAL_CUnrealStripDataFlags.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSTRIPDATAFLAGS_H ? */

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

CUnrealStripDataFlags::CUnrealStripDataFlags( void )
    : m_globalFlags( 0 )
    , m_classFlags( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealStripDataFlags::Read( CUnrealArchiveReader& reader )
{GUCEF_TRACE;

    m_globalFlags = reader.ReadUInt8();
    m_classFlags = reader.ReadUInt8();
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

UInt8
CUnrealStripDataFlags::GetGlobalFlags( void ) const
{GUCEF_TRACE;

    return m_globalFlags;
}

/*-------------------------------------------------------------------------*/

UInt8
CUnrealStripDataFlags::GetClassFlags( void ) const
{GUCEF_TRACE;

    return m_classFlags;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStripDataFlags::IsEditorDataStripped( void ) const
{GUCEF_TRACE;

    return 0 != ( m_globalFlags & 1 );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealStripDataFlags::IsDataStrippedForServer( void ) const
{GUCEF_TRACE;

    return 0 != ( m_globalFlags & 2 );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
