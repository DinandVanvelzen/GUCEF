/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H
#define GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads a settings object strictly, for the settings and exposure files of the asset tools: every member that is
 *  read is named, a value must have the right form (a number for a number, true or false for a flag, a list of texts
 *  for a list) and lie in its range, and Finish() refuses members that were not named. The first problem is kept;
 *  later reads do nothing once there is one. A member that is missing keeps the value the caller had.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStrictConfigReader
{
    public:

    /** 'context' names the object in the problem texts ("AssetToolSet", "the exposure file") */
    CAssetStrictConfigReader( const CORE::CDataNode& node, const CORE::CString& context );

    /** The member may be there; nothing is read */
    void Allow( const CORE::CString& name );

    void ReadString( const CORE::CString& name, CORE::CString& value, UInt32 maxLength = 0 );

    /** A text that must be one of the values separated by '|' */
    void ReadChoice( const CORE::CString& name, const CORE::CString& choices, CORE::CString& value );

    void ReadBool( const CORE::CString& name, bool& value );

    void ReadUInt32( const CORE::CString& name, UInt32 minimum, UInt32 maximum, UInt32& value );

    void ReadUInt64( const CORE::CString& name, UInt64 minimum, UInt64 maximum, UInt64& value );

    void ReadStringList( const CORE::CString& name, CORE::CString::StringVector& values, UInt32 maxItems = 0 );

    /** An object member; GUCEF_NULL when it is not there (or not an object, which is a problem) */
    const CORE::CDataNode* ReadObject( const CORE::CString& name );

    /** An array member; GUCEF_NULL when it is not there (or not an array, which is a problem) */
    const CORE::CDataNode* ReadArray( const CORE::CString& name );

    /** Sets a problem of the caller's own */
    void Fail( const CORE::CString& problem );

    bool HasProblem( void ) const;

    /** Checks that no member was left that was not named; true when there is no problem */
    bool Finish( CORE::CString& problem );

    private:

    bool Has( const CORE::CString& name, const CORE::CVariant*& attribute, const CORE::CDataNode*& child );

    const CORE::CDataNode& m_node;
    CORE::CString m_context;
    CORE::CString::StringSet m_named;
    CORE::CString m_problem;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTRICTCONFIGREADER_H ? */
