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

#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#define GUCEF_ASSETSTORE_CASSETLICENCE_H

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
 *  The licence of a pack (section 9.2, table licences): its class, name, address and text, and its restriction
 *  flags. The flags are named; NoMachineLearning ("noMachineLearning") means no training on the assets and no use as
 *  reference by learning systems: packs with it are left out of every learning based feature (see
 *  CPrimaryAssetStore::GetPacksAllowingUse()). Classes and flags are set by the licence map of the vocabulary and by
 *  the metadata normalisers.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetLicence : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString NoMachineLearning;       /**< "noMachineLearning": no training on the assets, no use as reference by learning systems */
    static const CORE::CString AttributionRequired;     /**< "attributionRequired" */
    static const CORE::CString NoCommercialUse;         /**< "noCommercialUse" */
    static const CORE::CString NoRedistribution;        /**< "noRedistribution": the files themselves may not be passed on */
    static const CORE::CString EditorialOnly;           /**< "editorialOnly" */

    CORE::CString licenceId;
    CORE::CString licenceClass;                  /**< "public-domain", "attribution", "royalty-free", ... "unknown" */
    CORE::CString name;
    CORE::CString url;
    CORE::CString text;
    CORE::CString::StringSet restrictions;       /**< restriction flags */

    CAssetLicence( void );
    virtual ~CAssetLicence() GUCEF_VIRTUAL_OVERRIDE;

    bool HasRestriction( const CORE::CString& flag ) const;

    /** Whether the licence allows machine learning use: no NoMachineLearning flag */
    bool AllowsMachineLearning( void ) const;

    /** A stable id from the name, the address and the text: "lic-" and 16 hexadecimal characters */
    CORE::CString MakeLicenceId( void ) const;

    /** Every member back to its default */
    void Clear( void );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

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

#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */
