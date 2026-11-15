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

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEINFO_H
#define GUCEF_ASSETSTORE_CASSETSERVICEINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETLOCATIONINFO_H
#include "gucefASSETSTORE_CAssetLocationInfo.h"
#define GUCEF_ASSETSTORE_CASSETLOCATIONINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETLOCATIONINFO_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREINFO_H
#include "gucefASSETSTORE_CAssetStoreInfo.h"
#define GUCEF_ASSETSTORE_CASSETSTOREINFO_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREINFO_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#include "gucefASSETSTORE_CPrimaryAssetStoreSettings.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORESETTINGS_H ? */
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
 *  What an asset store service is and offers (section 4.2 of the service design, group Info): the implementation, its
 *  version, whether it runs in the process of the caller, its capabilities, the roles of the caller, the bounds of its
 *  lists, the catalogue revision, the state of the keyring, the storage locations with their state, the online stores, the
 *  policies of the sources, the converter and whether a renderer is attached.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetServiceInfo : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString implementationId;                             /**< "local", "remote" */
    CORE::CString version;
    bool inProcess;                                             /**< calls run in the process of the caller, without a network */
    CORE::CString::StringSet capabilities;                      /**< the method groups and features the service offers */
    CORE::CString::StringSet roles;                             /**< the roles of the caller */
    UInt32 maxPageEntries;                                      /**< the most entries of a page of a list */
    UInt64 catalogueRevision;
    CORE::CString keyringState;                                 /**< "none", "locked", "unlocked" */
    TAssetLocationInfoVector locations;
    TAssetStoreInfoVector stores;
    CPrimaryAssetStoreSettings::TSourcePolicyVector sources;
    CORE::CString converterName;                                /**< empty without a converter */
    CORE::CString converterProfile;
    bool hasRenderer;                                           /**< a render hook is attached: contact sheets can be made */

    CAssetServiceInfo( void );
    virtual ~CAssetServiceInfo() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    /** Whether the capabilities name the capability */
    bool HasCapability( const CORE::CString& capability ) const;

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

#endif /* GUCEF_ASSETSTORE_CASSETSERVICEINFO_H ? */
