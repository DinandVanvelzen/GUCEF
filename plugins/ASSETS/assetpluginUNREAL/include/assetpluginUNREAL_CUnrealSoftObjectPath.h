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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#include "assetpluginUNREAL_CUnrealNameTable.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The three stored forms (section 3.3 of the design): one name holding "/Game/Dir/Package.Asset" before UE5 version 1007,
 *  the top level asset path as two names from 1007, and from 1008 in editor packages an index into the soft object path
 *  list of the summary. Every form carries a sub object path string.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealSoftObjectPath
{
    public:

    typedef GUCEF::vector< CUnrealSoftObjectPath > TSoftObjectPathVector;

    CORE::CString packageName;
    CORE::CString assetName;
    CORE::CString subPath;

    CUnrealSoftObjectPath( void );

    bool IsNull( void ) const;

    /** "Package.Asset:SubPath", empty for a null path */
    CORE::CString ToString( void ) const;

    /**
     *  Reads one path in the form the versions call for. The list is the soft object path list of the package, NULL when
     *  the package has none; with a list and UE5 version 1008 or later the stored value is an index into it.
     */
    bool Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions, const TSoftObjectPathVector* list );

    /** Reads an entry of the list itself: the form without the index */
    bool ReadListEntry( CUnrealArchiveReader& reader, const CUnrealNameTable& names, const CUnrealVersionContext& versions );
};

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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H ? */
