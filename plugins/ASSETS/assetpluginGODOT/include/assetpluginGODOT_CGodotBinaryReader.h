/*
 *  assetpluginGODOT: the reader of the binary resource format of Godot
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

#ifndef GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H
#define GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#include "assetpluginGODOT_macros.h"
#define GUCEF_ASSETPLUGIN_GODOT_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_GODOT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GODOT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the binary resource files of Godot 4 (.res, .mesh and the binary form of .tres: a file that starts with
 *  "RSRC") into the same tree as CGodotTextReader makes of the text form, so the rest of the importer reads both alike:
 *
 *  - a section "gd_resource" whose attributes are the type of the main resource, the format (3, as in text files) and
 *    the uid of the file as text ("uid://...");
 *  - a section "ext_resource" per external resource with the attributes type, path, uid and id (the index in the
 *    table of the file as text), and a section "sub_resource" per internal resource but the last with the attributes
 *    type and id (the name of the resource without "local://");
 *  - a section "resource" with the properties of the last internal resource, the main resource of the file;
 *  - the values as CGodotTextReader gives them: a reference to an internal resource is a SubResource with its id, one
 *    to an external resource an ExtResource with its index.
 *
 *  Supported are the files of Godot 4 (format versions 3 to 6, 32 and 64 bit reals) in the byte order of the host that
 *  are not compressed (a compressed file is reported). The variants of resources are read: nil, bool, integers, floats, strings, names, vectors,
 *  rectangles, planes, quaternions, bases, transforms, projections, colours, node paths, dictionaries, arrays and the
 *  packed arrays; a type of value that this reader does not know ends the read with a problem.
 */
class GUCEF_ASSETPLUGIN_GODOT_EXPORT_CPP CGodotBinaryReader
{
    public:

    CGodotBinaryReader( void );

    ~CGodotBinaryReader();

    /** Whether the bytes are the start of a binary resource, compressed ("RSCC", which Parse() does not read) or not ("RSRC") */
    static bool IsBinaryResource( const void* data, UInt32 size );

    /** The text of a uid of Godot ("uid://" and the digits of the base 34 of Godot) */
    static CORE::CString UidToText( UInt64 uid );

    /** The type and the uid text of the main resource from the first bytes of a binary resource file (a few hundred are enough) */
    static bool ReadHeader( const void* data, UInt32 size, CORE::CString& type, CORE::CString& uid );

    bool Parse( const void* data, UInt32 size, CORE::CDataNode& result );

    /** Reads a file of the VFS and parses it */
    bool ParseFile( const CORE::CString& vfsPath, CORE::CDataNode& result );

    const CORE::CString& GetProblem( void ) const;

    private:

    CGodotBinaryReader( const CGodotBinaryReader& src );            /**< not implemented */
    CGodotBinaryReader& operator=( const CGodotBinaryReader& src ); /**< not implemented */

    bool Fail( const CORE::CString& message );
    bool Need( UInt32 bytes );
    bool ReadU32( UInt32& value );
    bool ReadU64( UInt64& value );
    bool ReadReal( Float64& value );
    bool ReadString( CORE::CString& text );
    bool ReadVariant( CORE::CDataNode& node );
    bool ReadNumbers( CORE::CDataNode& node, const char* type, UInt32 count, bool integers );
    bool ReadPacked( CORE::CDataNode& node, const char* type, const char* element, UInt32 components, UInt32 elementSize, bool pad );
    bool ReadProperties( CORE::CDataNode& properties, CORE::CString& type );

    private:

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_pos;
    bool m_real64;
    GUCEF::vector< CORE::CString > m_strings;
    GUCEF::vector< CORE::CString > m_internalIds;
    CORE::CString m_problem;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GODOT */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_GODOT_CGODOTBINARYREADER_H ? */
