/*
 *  assetpluginUNREAL: The type of a property as a tree of names
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H

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
 *  The type of a tagged property, for example StructProperty(LinearColor(/Script/CoreUObject)) or
 *  MapProperty(NameProperty,StructProperty(Foo(/Script/Bar))). It is held as the pre-order list of the complete type name
 *  layout: each node has a name and the number of parameters that follow its own subtree. The classic layout of the tag
 *  fields is turned into the same list.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPropertyTypeName
{
    public:

    struct SNode
    {
        CORE::CString name;
        UInt32 innerCount;

        SNode( void ) : name(), innerCount( 0 ) {}
    };

    typedef GUCEF::vector< SNode > TNodeVector;

    CUnrealPropertyTypeName( void );

    void Clear( void );

    /** Reads the nodes of the complete type name layout; the number of nodes is bounded by the reader limits */
    bool Read( CUnrealArchiveReader& reader, const CUnrealNameTable& names, UInt32 maxNodes );

    /** A single node without parameters */
    void SetSimple( const CORE::CString& name );

    /** Appends a node; used to build the list of a classic tag */
    void AddNode( const CORE::CString& name, UInt32 innerCount );

    bool IsEmpty( void ) const;

    /** The name of the first node: the kind of the property, for example "StructProperty" */
    const CORE::CString& GetName( void ) const;

    UInt32 GetParameterCount( void ) const;

    /** The name of the first node of parameter i, empty when there is no such parameter */
    const CORE::CString& GetParameterName( UInt32 i ) const;

    /** The subtree of parameter i; empty when there is no such parameter */
    CUnrealPropertyTypeName GetParameter( UInt32 i ) const;

    /** The text form with parentheses */
    CORE::CString ToString( void ) const;

    const TNodeVector& GetNodes( void ) const;

    private:

    UInt32 FindParameterStart( UInt32 i ) const;

    TNodeVector m_nodes;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYTYPENAME_H ? */
