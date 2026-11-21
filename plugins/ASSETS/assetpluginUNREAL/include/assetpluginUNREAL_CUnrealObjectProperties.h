/*
 *  assetpluginUNREAL: Typed access to a property tree by name path
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  A view on the tree of CUnrealPropertyReader. A path names the properties from the root, separated by "/", array and
 *  set elements by their position: "Skin/Settings/Tint/r", "Materials/0". A missing path gives the default.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealObjectProperties
{
    public:

    explicit CUnrealObjectProperties( const CORE::CDataNode& root );

    const CORE::CDataNode* Find( const CORE::CString& path ) const;

    bool Has( const CORE::CString& path ) const;

    bool GetBool( const CORE::CString& path, bool defaultValue = false ) const;

    Int32 GetInt32( const CORE::CString& path, Int32 defaultValue = 0 ) const;

    Int64 GetInt64( const CORE::CString& path, Int64 defaultValue = 0 ) const;

    Float64 GetDouble( const CORE::CString& path, Float64 defaultValue = 0.0 ) const;

    /** Strings, names and enumerators; for object references the resolved path */
    CORE::CString GetString( const CORE::CString& path, const CORE::CString& defaultValue = CORE::CString::Empty ) const;

    /** The components of a binary struct (x, y, z, r, g, b, a, ...) as attributes of the node at the path */
    Float64 GetComponent( const CORE::CString& path, const char* component, Float64 defaultValue = 0.0 ) const;

    /** The number of elements of an array or set, or properties of a struct at the path */
    UInt32 GetCount( const CORE::CString& path ) const;

    /** The type text of the property at the path, empty when absent */
    CORE::CString GetType( const CORE::CString& path ) const;

    private:

    const CORE::CDataNode* m_root;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTPROPERTIES_H ? */
