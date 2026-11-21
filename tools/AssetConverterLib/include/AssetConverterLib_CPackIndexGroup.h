/*
 *  AssetConverterLib: a function group summary of the pack index, with its sub-groups
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXGROUP_H
#define ASSETCONVERTERLIB_CPACKINDEXGROUP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXSUBGROUP_H
#include "AssetConverterLib_CPackIndexSubGroup.h"
#define ASSETCONVERTERLIB_CPACKINDEXSUBGROUP_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSUBGROUP_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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
 *  { id, name, count, bytes, subGroups[], presentation? }: sorted by the fixed order of the vocabulary, sub-groups by id.
 *  A sub-group list longer than MaxSubGroupsInManifest is truncated in the manifest (subGroupsTruncated) and complete in the shards.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexGroup
{
    public:

    typedef GUCEF::vector< CPackIndexGroup > TGroupVector;
    typedef GUCEF::vector< CPackIndexSubGroup > TSubGroupVector;

    static const UInt32 MaxSubGroupsInManifest = 200;

    CORE::CString id;
    CORE::CString name;
    UInt32 count;
    UInt64 bytes;
    TSubGroupVector subGroups;
    bool subGroupsTruncated;
    CORE::CString presentation;     /**< the default presentation id of the group (F17 item 4) */

    CPackIndexGroup( void );

    const CPackIndexSubGroup* FindSubGroup( const CORE::CString& subGroupId ) const;
    bool operator==( const CPackIndexGroup& other ) const;
    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXGROUP_H ? */

