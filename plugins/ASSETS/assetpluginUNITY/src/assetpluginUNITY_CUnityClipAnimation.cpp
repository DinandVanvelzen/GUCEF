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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#include "assetpluginUNITY_CUnityYamlReader.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */

#include "assetpluginUNITY_CUnityClipAnimation.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityClipAnimation::CUnityClipAnimation( void )
    : name()
    , takeName()
    , firstFrame( 0.0f )
    , lastFrame( 0.0f )
    , loopTime( false )
    , loop( false )
    , wrapMode( 0 )
    , mirror( false )
    , cycleOffset( 0.0f )
    , eventCount( 0 )
    , internalId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnityClipAnimation::Read( const CORE::CDataNode& item )
{GUCEF_TRACE;

    *this = CUnityClipAnimation();
    if ( GUCEF_DATATYPE_OBJECT != item.GetNodeType() )
        return false;
    name = CUnityYamlReader::GetString( item, "name" );
    takeName = CUnityYamlReader::GetString( item, "takeName" );
    firstFrame = CUnityYamlReader::GetFloat( item, "firstFrame", 0.0f );
    lastFrame = CUnityYamlReader::GetFloat( item, "lastFrame", 0.0f );
    loopTime = CUnityYamlReader::GetBool( item, "loopTime", false );
    loop = CUnityYamlReader::GetBool( item, "loop", false );
    wrapMode = CUnityYamlReader::GetInt32( item, "wrapMode", 0 );
    mirror = CUnityYamlReader::GetBool( item, "mirror", false );
    cycleOffset = CUnityYamlReader::GetFloat( item, "cycleOffset", 0.0f );
    internalId = CUnityYamlReader::GetInt64( item, "internalID", 0 );
    CUnityYamlReader::TNodeList events;
    CUnityYamlReader::GetItems( item.FindChild( "events" ), events );
    eventCount = (UInt32) events.size();
    return !name.IsNULLOrEmpty() || !takeName.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
