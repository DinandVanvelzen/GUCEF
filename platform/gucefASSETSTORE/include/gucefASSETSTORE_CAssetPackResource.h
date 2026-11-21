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


#ifndef GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H
#define GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
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
 *  A resource of a pack index as the catalogue holds it: a row of the table pack_resources. It carries what a list of the
 *  resources of a group needs (name, kind, group, format, size, status, what is analysed and converted); the statistics, the
 *  references and the outputs stay in the document. The id is the id the store gives the resource (the asset id when the
 *  resource is an asset, which the formula of the store makes the same).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackResource : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString resourceId;
    CORE::CString packId;
    CORE::CString assetId;    /**< the asset the resource is when it is one, else empty */
    CORE::CString path;    /**< relative to the pack root, with / separators */
    CORE::CString name;    /**< as the pack spells it */
    CORE::CString title;    /**< display name derived from the name */
    CORE::CString kind;    /**< the kind of the registry of the converter */
    CORE::CString storeKind;    /**< the kind of the vocabulary of the store, when the name says it */
    CORE::CString groupId;    /**< the function group */
    CORE::CString subGroup;    /**< path of the sub-group, '/' for nesting */
    CORE::CString groupRule;    /**< which stage of the rules decided */
    CORE::CString format;
    UInt64 sizeBytes;
    CORE::CString digest;
    CORE::CString status;    /**< indexed, analysed, converted, failed or refused */
    bool analysed;
    bool converted;
    bool hasThumbnail;
    CORE::CString presentedUnder;    /**< for a prefab the group a view shows it in */

    CAssetPackResource( void );
    virtual ~CAssetPackResource() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKRESOURCE_H ? */
