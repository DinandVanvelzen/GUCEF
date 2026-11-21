/*
 *  assetpluginUNREAL: Reads the tagged properties of an export into a data node tree
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H
#include "assetpluginUNREAL_CUnrealPropertyTag.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTAG_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H
#include "assetpluginUNREAL_CUnrealNativeStructs.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNATIVESTRUCTS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  Reads tagged properties (both tag layouts) into a CORE::CDataNode tree: one child per property, named as the property,
 *  with the attribute "type" (the type name tree as text) and "index" for a non-zero array index. Scalars are the value of
 *  the node; structs hold their properties as children (binary structs hold their components as attributes); arrays and
 *  sets are nodes of type array whose children are named by their position (an array of numbers of more than 256 elements is
 *  one binary value in the file's own element encoding, with the attributes packed, count, elementType and elementBytes);
 *  maps are arrays of nodes with the children
 *  "key" and "value". Object references hold the package index as value and the resolved path in the attribute "path".
 *
 *  Every tag is read inside its declared size: after the value the reader moves to the declared end whatever the value
 *  reader consumed. A value that cannot be read, or that was read with another size than the tag says, is reported once
 *  per class and property and the node gets the attribute "unread" or "mismatch"; properties of an unknown type are
 *  skipped by their size and listed.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPropertyReader
{
    public:

    /** Arrays of plain numbers with more elements than this are kept as one block of bytes (attributes packed, count, elementType) */
    static const UInt32 PackedArrayThreshold = 256;

    /** Reads the properties of the exports of an opened package; object references are resolved through it */
    explicit CUnrealPropertyReader( const CUnrealPackage& package );

    CUnrealPropertyReader( const CUnrealNameTable& names, const CUnrealVersionContext& versions, const CUnrealSoftObjectPath::TSoftObjectPathVector* softPaths, const CUnrealReaderLimits& limits );

    ~CUnrealPropertyReader();

    /**
     *  Reads the script part of an export (from OpenPropertyRange()): the class serialisation control byte of UE5 file
     *  version 1011 and later, then the tags up to the "None" terminator or the end of the range. The properties become
     *  children of out.
     */
    bool ReadExport( CUnrealArchiveReader& reader, CORE::CDataNode& out, const CORE::CString& className );

    /** Reads tags up to "None" or the end of the range into children of parent, for data that holds a bare tag list */
    bool ReadTagList( CUnrealArchiveReader& reader, CORE::CDataNode& parent );

    /** Problems of the values read (sizes that did not match, values that could not be read), once per class and property */
    const CORE::CString::StringVector& GetMismatches( void ) const;

    /** "Class.Property: Type" of the properties whose type is not known and that were skipped by size */
    const CORE::CString::StringVector& GetUnknownProperties( void ) const;

    UInt32 GetPropertyCount( void ) const;

    private:

    CUnrealPropertyReader( const CUnrealPropertyReader& src );            /**< not implemented */
    CUnrealPropertyReader& operator=( const CUnrealPropertyReader& src ); /**< not implemented */

    enum EKind
    {
        K_UNKNOWN = 0, K_BOOL, K_BYTE, K_INT8, K_INT16, K_INT32, K_INT64, K_UINT16, K_UINT32, K_UINT64, K_FLOAT, K_DOUBLE,
        K_STR, K_NAME, K_TEXT, K_OBJECT, K_LAZY, K_SOFTOBJECT, K_ENUM, K_STRUCT, K_ARRAY, K_SET, K_MAP, K_OPTIONAL,
        K_FIELDPATH, K_DELEGATE, K_MULTICAST
    };

    static EKind GetKind( const CORE::CString& typeName );

    static UInt32 MinElementBytes( EKind kind );

    bool ReadTags( CUnrealArchiveReader& reader, CORE::CDataNode& parent, UInt32 depth );

    bool ReadValue( CUnrealArchiveReader& reader, const CUnrealPropertyTag* tag, const CUnrealPropertyTypeName& type, EKind kind, bool byteIsName, bool nativeHint, CORE::CDataNode& node, UInt32 depth );

    bool ReadStruct( CUnrealArchiveReader& reader, const CORE::CString& structName, bool native, Int64 declaredSize, CORE::CDataNode& node, UInt32 depth );

    bool ReadContainer( CUnrealArchiveReader& reader, const CUnrealPropertyTag* tag, const CUnrealPropertyTypeName& type, EKind kind, bool nativeHint, CORE::CDataNode& node, UInt32 depth );

    bool ReadText( CUnrealArchiveReader& reader, CORE::CDataNode& node );

    bool ReadObjectIndex( CUnrealArchiveReader& reader, CORE::CDataNode& node );

    bool CountElements( CUnrealArchiveReader& reader, UInt64 count );

    void AddMismatch( const CORE::CString& property, const CORE::CString& text );

    private:

    const CUnrealPackage* m_package;
    const CUnrealNameTable* m_names;
    const CUnrealVersionContext* m_versions;
    const CUnrealSoftObjectPath::TSoftObjectPathVector* m_softPaths;
    CUnrealReaderLimits m_limits;
    CORE::CString m_className;
    CORE::CString::StringVector m_mismatches;
    CORE::CString::StringVector m_unknown;
    CORE::CString::StringSet m_reported;
    UInt64 m_elements;
    UInt32 m_propertyCount;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */
