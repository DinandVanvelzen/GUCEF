/*
 *  assetpluginUNREAL: The memory and size budgets of the Unreal package readers
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#include "assetpluginUNREAL_CUnrealReaderLimits.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H ? */

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

CUnrealReaderLimits::CUnrealReaderLimits( void )
    : maxHeaderBytes( 64 * 1024 * 1024 )
    , maxPropertyBytes( 256 * 1024 * 1024 )
    , windowBytes( 1024 * 1024 )
    , maxNameLength( 1024 * 1024 )
    , maxDepth( 64 )
    , maxElements( 16 * 1024 * 1024 )
    , maxTableEntries( 16 * 1024 * 1024 )
    , maxPayloadRawBytes( (UInt64) 1024 * 1024 * 1024 )
    , maxBlockBytes( 16 * 1024 * 1024 )
{GUCEF_TRACE;

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
