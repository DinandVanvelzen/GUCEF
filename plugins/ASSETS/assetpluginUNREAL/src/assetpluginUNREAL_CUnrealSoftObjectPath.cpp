/*
 *  assetpluginUNREAL: A soft reference to an asset: package, asset and sub object path
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#include "assetpluginUNREAL_UnrealVersions.h"
#define GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_UNREALVERSIONS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H
#include "assetpluginUNREAL_CUnrealSoftObjectPath.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H ? */

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

CUnrealSoftObjectPath::CUnrealSoftObjectPath( void )
    : packageName()
    , assetName()
    , subPath()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CUnrealSoftObjectPath::IsNull( void ) const
{GUCEF_TRACE;

    return packageName.IsNULLOrEmpty() && assetName.IsNULLOrEmpty() && subPath.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealSoftObjectPath::ToString( void ) const
{GUCEF_TRACE;

    if ( IsNull() )
        return CORE::CString();
    CORE::CString text = packageName;
    if ( !assetName.IsNULLOrEmpty() )
        text += "." + assetName;
    if ( !subPath.IsNULLOrEmpty() )
        text += ":" + subPath;
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSoftObjectPath::ReadListEntry( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions )
{GUCEF_TRACE;

    packageName.Clear();
    assetName.Clear();
    subPath.Clear();
    if ( versions.IsAtLeast( UE5_SOFTOBJECTPATH_REMOVE_ASSET_PATH_FNAMES ) )
    {
        names.ReadName( reader, packageName );
        names.ReadName( reader, assetName );
    }
    else
    {
        // one name with the whole path: "/Game/Dir/Package.Asset"
        CORE::CString path;
        names.ReadName( reader, path );
        const Int32 dot = path.HasChar( '.' );
        if ( dot >= 0 )
        {
            packageName = path.SubstrFromRange( 0, (UInt32) dot );
            assetName = path.SubstrFromRange( (UInt32) dot + 1, path.Length() );
        }
        else
            packageName = path;
    }
    reader.ReadString( subPath );
    if ( packageName == "None" )
        packageName.Clear();
    if ( assetName == "None" )
        assetName.Clear();
    return !reader.HasError();
}

/*-------------------------------------------------------------------------*/

bool
CUnrealSoftObjectPath::Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, const TSoftObjectPathVector* list )
{GUCEF_TRACE;

    if ( versions.IsAtLeast( UE5_ADD_SOFTOBJECTPATH_LIST ) && GUCEF_NULL != list )
    {
        const Int32 index = reader.ReadInt32();
        if ( reader.HasError() )
            return false;
        if ( index < 0 || (size_t) index >= list->size() )
            return reader.SetError( "Soft object path index " + CORE::Int32ToString( index ) + " is outside the list of " + CORE::UInt64ToString( list->size() ) );
        *this = (*list)[ (size_t) index ];
        return true;
    }
    return ReadListEntry( reader, names, versions );
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
