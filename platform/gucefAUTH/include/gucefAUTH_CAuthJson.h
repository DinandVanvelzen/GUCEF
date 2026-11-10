/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CAUTHJSON_H
#define GUCEF_AUTH_CAUTHJSON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  JSON in, with the limits that text from the network needs: a size limit, a nesting limit and, for tokens and key
 *  sets, a refusal of duplicate member names. The parsing itself is the "json" data store codec of gucefCORE, which
 *  comes from dstorepluginJSONPARSER: without it nothing parses and IsAvailable() says so.
 *
 *  A member of an object that is a string, a number, a boolean or null is an attribute of the node, a member that is
 *  an object or an array is a child node of that name; the values of an array are the values of its children.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthJson
{
    public:

    static const UInt32 DefaultMaxDepth = 12;

    /** Whether the "json" data store codec is there */
    static bool IsAvailable( void );

    /**
     *  Parses the text into the document node. False for text that is not JSON, longer than maxSize bytes, nested deeper
     *  than maxDepth or with a NUL byte. With strictMembers a member name that occurs twice in one object, or that is
     *  written with an escape, is refused as well: that is how tokens and key sets are read, there is then one meaning
     *  only for every name.
     */
    static bool Parse( const void* data             ,
                       UInt32 size                  ,
                       UInt32 maxSize               ,
                       UInt32 maxDepth              ,
                       bool strictMembers           ,
                       CORE::CDataNode& document    );

    static bool Parse( const CORE::CString& text    ,
                       UInt32 maxSize               ,
                       UInt32 maxDepth              ,
                       bool strictMembers           ,
                       CORE::CDataNode& document    );

    /** Whether the text passes the checks of Parse() that do not need the codec: size, nesting, NUL, strict member names */
    static bool CheckStructure( const void* data, UInt32 size, UInt32 maxSize, UInt32 maxDepth, bool strictMembers );

    /** The node that holds the members of a parsed document: the root itself, or the one node a codec wraps it in */
    static const CORE::CDataNode& GetDocument( const CORE::CDataNode& root );

    /** The member as text: only for a member that is a JSON string */
    static bool GetString( const CORE::CDataNode& object, const CORE::CString& name, CORE::CString& value );

    /** The member as a number: a JSON number, not a string that looks like one. Fractions are cut off */
    static bool GetInt64( const CORE::CDataNode& object, const CORE::CString& name, Int64& value );

    static bool GetBool( const CORE::CDataNode& object, const CORE::CString& name, bool& value );

    /** The member as a list of texts: an array of strings, or a single string. False when it is neither or an entry is not a string */
    static bool GetStringList( const CORE::CDataNode& object, const CORE::CString& name, CORE::CString::StringVector& values );

    /**
     *  The scalar members of the object as text, the members of nested objects with names like "picture.data.url".
     *  Arrays of strings become one entry with the values joined by a space. At most maxEntries entries.
     */
    static void Flatten( const CORE::CDataNode& object, CORE::CString::StringMap& values, UInt32 maxEntries = 256 );

    private:

    CAuthJson( void );

    static void FlattenInto( const CORE::CDataNode& object, const CORE::CString& prefix, UInt32 depth, CORE::CString::StringMap& values, UInt32 maxEntries );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHJSON_H ? */
