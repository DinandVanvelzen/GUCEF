/*
 *  AssetConverterLib: deterministic JSON text of data trees
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

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#define ASSETCONVERTERLIB_CCANONICALJSON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The deterministic writing of the converter: the same data tree always gives the same bytes, which the cache keys,
 *  the golden tests and the byte-identical intermediates rely on.
 *
 *  - An object node (GUCEF_DATATYPE_OBJECT) is a JSON object of its attributes and its children, sorted by name.
 *  - An array node (GUCEF_DATATYPE_ARRAY) is a JSON array of its children, in their order.
 *  - Any other node is its value: a value child, as CDataNode::AddValueAsChild() makes it.
 *  - Numbers are written with round-trip precision in their shortest form: a Float32 as the shortest text that reads
 *    back as the same Float32, a Float64 likewise. Integers as integers, booleans as true and false, an unset value or
 *    nil as null. Values that are not finite are written as null (JSON has no such numbers).
 *  - Four spaces per level, LF line ends, a LF at the end; strings escaped as JSON requires.
 *
 *  Reading goes through the JSON codec of the data store codecs (dstorepluginJSONPARSER), which makes scalar members
 *  attributes and object and array members children: the form written here.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CCanonicalJson
{
    public:

    /** The JSON text of the tree */
    static CORE::CString ToJson( const CORE::CDataNode& node );

    /** The JSON text of the tree, appended to the buffer */
    static void ToJson( const CORE::CDataNode& node, CORE::CDynamicBuffer& buffer );

    /** The shortest decimal text that reads back as the same Float32 */
    static CORE::CString FormatFloat32( Float32 value );

    /** The shortest decimal text that reads back as the same Float64 */
    static CORE::CString FormatFloat64( Float64 value );

    /** The JSON literal of a value */
    static CORE::CString FormatValue( const CORE::CVariant& value );

    /** A JSON string literal, with the quotes */
    static CORE::CString QuoteString( const CORE::CString& text );

    /** The SHA-256 of the JSON text, as 64 lower case hex digits */
    static CORE::CString Hash( const CORE::CDataNode& node );

    /** Writes the JSON text to a file of the VFS, replacing it; the folder is made when it is not there */
    static bool Store( const CORE::CString& vfsPath, const CORE::CDataNode& node );

    /** Reads a JSON file of the VFS into the tree: the members of the top object become those of 'node' */
    static bool Load( const CORE::CString& vfsPath, CORE::CDataNode& node );

    /** Reads JSON text into the tree, as Load() does */
    static bool Parse( const CORE::CString& text, CORE::CDataNode& node );

    private:

    static void Append( const CORE::CDataNode& node, UInt32 depth, CORE::CDynamicBuffer& buffer );

    CCanonicalJson( void );
    ~CCanonicalJson();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */
