/*
 *  assetpluginUNITY: the reader of the YAML dialect of Unity's text-serialised files
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H
#include "assetpluginUNITY_CUnityYamlFile.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H ? */

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
 *  Reads the YAML that Unity writes (assets, prefabs, scenes, materials, .meta files) into CUnityYamlFile documents.
 *  It is a reader of that dialect, not of YAML in general:
 *
 *  - documents start with "--- !u!<class ID> &<file ID>", optionally followed by "stripped" (not valid YAML, so a
 *    general YAML parser rejects it); a text without such headers (a .meta file) is one document;
 *  - "%YAML" and "%TAG" directives, comments and "..." are skipped;
 *  - block mappings and block sequences, also a sequence at the indentation of its key and "- key: value" items;
 *  - flow mappings and sequences, also when they are wrapped over several lines;
 *  - plain scalars (also wrapped over several lines), single and double quoted scalars over several lines with
 *    their escapes, and literal and folded block scalars;
 *  - empty values ("m_Name:"), which are empty texts.
 *
 *  Values stay texts: file IDs are signed 64 bit (ParseInt64()), floats may start with a dot or be "-0", and the
 *  infinities and NaN are written as "Infinity", "-Infinity" and "NaN" (ParseFloat()). A document that cannot be read
 *  is reported and left out; the other documents of the file are kept.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityYamlReader
{
    public:

    typedef GUCEF::vector< const CORE::CDataNode* > TNodeList;

    /** Reads the text; false when a document could not be read ('problem' names the first), the others are in 'file' */
    static bool Parse( const char* text, UInt32 byteSize, CUnityYamlFile& file, CORE::CString& problem );

    static bool Parse( const CORE::CString& text, CUnityYamlFile& file, CORE::CString& problem );

    /** Reads a file of the VFS */
    static bool Load( const CORE::CString& vfsPath, CUnityYamlFile& file, CORE::CString& problem );

    /** A signed 64 bit integer: an optional sign and digits; false for other texts and overflows */
    static bool ParseInt64( const CORE::CString& text, Int64& value );

    /** A float as Unity writes it: "1", "-0", ".5", "-.25", "1e-05", "Infinity", "-Infinity", "NaN", ".inf", ".nan" */
    static bool ParseFloat( const CORE::CString& text, Float64& value );

    /** The scalar member 'key' of a mapping, the default when it is missing or not a scalar */
    static CORE::CString GetString( const CORE::CDataNode& mapping, const CORE::CString& key, const CORE::CString& defaultValue = CORE::CString::Empty );
    static Int64 GetInt64( const CORE::CDataNode& mapping, const CORE::CString& key, Int64 defaultValue );
    static Int32 GetInt32( const CORE::CDataNode& mapping, const CORE::CString& key, Int32 defaultValue );
    static Float32 GetFloat( const CORE::CDataNode& mapping, const CORE::CString& key, Float32 defaultValue );
    static bool GetBool( const CORE::CDataNode& mapping, const CORE::CString& key, bool defaultValue );

    /** Whether the mapping has a scalar member 'key' */
    static bool HasScalar( const CORE::CDataNode& mapping, const CORE::CString& key );

    /**
     *  The components of the mapping member 'key' ({x: 1, y: 2, z: 3}), named by the characters of 'components'
     *  ("xyz", "xyzw", "rgba"); a missing component keeps its value. False when the member is no mapping.
     */
    static bool GetComponents( const CORE::CDataNode& mapping, const CORE::CString& key, const char* components, Float32* values );

    /** The same, of a mapping node itself */
    static bool GetComponents( const CORE::CDataNode* mapping, const char* components, Float32* values );

    /** The items of a sequence node (none for NULL or another node) */
    static void GetItems( const CORE::CDataNode* sequence, TNodeList& items );

    /** The text of a scalar item of a sequence */
    static CORE::CString GetItemText( const CORE::CDataNode& item );

    /**
     *  The entries of a member that Unity writes either as a sequence of single-key mappings ("- _MainTex: ...") or
     *  as a mapping: the key and the node of the value (a scalar value is in 'scalars').
     */
    static void GetKeyedEntries( const CORE::CDataNode* member, CORE::CString::StringMap& scalars, GUCEF::vector< std::pair< CORE::CString, const CORE::CDataNode* > >& nodes );

    private:

    CUnityYamlReader( void );
    ~CUnityYamlReader();
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

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLREADER_H ? */
