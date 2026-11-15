/*
 *  assetpluginUNITY: a reference to an object of a Unity file
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYREFERENCE_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYREFERENCE_H

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
 *  A reference of a Unity file, the flow mapping {fileID: <id>, guid: <guid>, type: <type>}: the file ID of the
 *  object (signed 64 bit) in the asset with the GUID, or in the same file when the GUID is missing. Type 0 refers to
 *  the built-in resources of the editor (the GUIDs 0000000000000000e000000000000000 and 0000000000000000f000000000000000),
 *  2 to an asset that is serialised by the editor (.mat, .prefab, .asset), 3 to an object an importer makes from a
 *  file (models, textures, shaders). A value type.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityReference
{
    public:

    static const CORE::CString BuiltinDefaultGuid;  /**< "0000000000000000e000000000000000": built-in meshes, textures */
    static const CORE::CString BuiltinExtraGuid;    /**< "0000000000000000f000000000000000": built-in shaders, materials */

    Int64 fileId;
    CORE::CString guid;
    Int32 type;

    CUnityReference( void );

    explicit CUnityReference( Int64 fileId, const CORE::CString& guid = CORE::CString::Empty, Int32 type = 0 );

    /** No object: file ID 0 */
    bool IsNull( void ) const;

    /** An object of the same file: no GUID */
    bool IsLocal( void ) const;

    /** An object of the built-in resources of the editor */
    bool IsBuiltin( void ) const;

    /** Reads a reference mapping; false (and a null reference) when the node is none */
    bool Read( const CORE::CDataNode* node );

    /** The reference that is the member 'key' of the mapping; a null reference when there is none */
    static CUnityReference Get( const CORE::CDataNode& mapping, const CORE::CString& key );

    /** "{fileID: <id>, guid: <guid>, type: <type>}", the way Unity writes it */
    CORE::CString ToString( void ) const;

    bool operator==( const CUnityReference& other ) const;

    bool operator!=( const CUnityReference& other ) const;

    bool operator<( const CUnityReference& other ) const;
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYREFERENCE_H ? */
