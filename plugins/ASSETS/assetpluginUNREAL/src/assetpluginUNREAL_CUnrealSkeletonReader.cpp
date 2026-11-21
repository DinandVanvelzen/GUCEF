/*
 *  assetpluginUNREAL: Reads the reference skeleton and the sockets of a Skeleton asset
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#include "assetpluginUNREAL_CUnrealObjectProperties.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H
#include "assetpluginUNREAL_CUnrealSkeletonReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSKELETONREADER_H ? */

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

CUnrealSkeletonReader::CUnrealSkeletonReader( void )
    : m_error()
    , m_socketCount( 0 )
    , m_virtualBoneCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealSkeletonReader::Read( CUnrealPackage& package, UInt32 exportIndex, CUnrealReferenceSkeleton& skeleton )
{GUCEF_TRACE;

    m_error = CORE::CString();
    m_socketCount = 0;
    m_virtualBoneCount = 0;
    skeleton.Clear();

    CUnrealArchiveReader reader;
    if ( !package.OpenExportRange( exportIndex, reader ) )
    {
        m_error = "The data of the export " + CORE::UInt32ToString( exportIndex ) + " cannot be opened";
        return false;
    }
    CORE::CDataNode root( "root" );
    CUnrealPropertyReader properties( package );
    if ( !properties.ReadExport( reader, root, "Skeleton" ) )
    {
        m_error = "The properties of the skeleton cannot be read: " + reader.GetError();
        return false;
    }
    const CUnrealObjectProperties values( root );
    m_socketCount = values.GetCount( "Sockets" );
    m_virtualBoneCount = values.GetCount( "VirtualBones" );

    if ( reader.ReadBool32() )
    {
        CUnrealGuid guid;
        reader.ReadGuid( guid );
    }
    if ( !skeleton.Read( reader, package.GetNames(), package.GetVersions(), 0 == ( package.GetSummary().packageFlags & 0x80000000 ) ) )
    {
        m_error = "The reference skeleton cannot be read: " + reader.GetError();
        skeleton.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUnrealSkeletonReader::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealSkeletonReader::GetSocketCount( void ) const
{GUCEF_TRACE;

    return m_socketCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealSkeletonReader::GetVirtualBoneCount( void ) const
{GUCEF_TRACE;

    return m_virtualBoneCount;
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
