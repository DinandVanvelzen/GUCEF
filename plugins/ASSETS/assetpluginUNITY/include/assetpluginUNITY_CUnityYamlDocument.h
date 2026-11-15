/*
 *  assetpluginUNITY: one document of a text-serialised Unity file
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLDOCUMENT_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLDOCUMENT_H

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
 *  One document of a Unity YAML file. A document of an asset, prefab or scene starts with the header
 *  "--- !u!<class ID> &<file ID>", optionally followed by "stripped", and holds a mapping with one key, the name of
 *  the class ("GameObject", "Material", ...), whose value is the object. A file without headers (a .meta file) is one
 *  document without header.
 *
 *  The values are held as CDataNode trees: a mapping is an object node whose scalar members are attributes (the text
 *  of the scalar, unquoted) and whose mapping and sequence members are child nodes named after their key; a sequence
 *  is an array node with unnamed children (a scalar item is a child with a value). The text of a scalar is kept as it
 *  is written, so "-0" and ".5" stay as they are; CUnityYamlReader converts them.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityYamlDocument
{
    public:

    Int32 classId;              /**< the class ID of the header, -1 without header */
    Int64 fileId;               /**< the file ID of the header (signed 64 bit), 0 without header */
    bool stripped;              /**< the header ends with "stripped": a placeholder of an object of a prefab instance */
    CORE::CString typeName;     /**< the key of the document with a header: the class name; empty without header */
    CORE::CDataNode body;       /**< the object under that key; without header the whole mapping of the document */

    CUnityYamlDocument( void );

    bool HasHeader( void ) const;
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLDOCUMENT_H ? */
