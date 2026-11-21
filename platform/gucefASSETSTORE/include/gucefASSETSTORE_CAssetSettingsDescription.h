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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSDESCRIPTION_H
#define GUCEF_ASSETSTORE_CASSETSETTINGSDESCRIPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSTATE_H
#include "gucefASSETSTORE_CAssetSettingState.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGSTATE_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSTATE_H ? */

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
 *  The answer of Settings/Describe (gucefASSETSTORE_RemoteService_Design.md 18.2): the settings the caller may read with their values,
 *  whether the server lets clients change settings (its site setting allowSettingsChanges), whether the caller may change them (its role)
 *  and the two together, with the reason for people when not.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingsDescription : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< CAssetSettingState > TSettingStateVector;

    TAssetStoreStatus status;
    CORE::CString message;
    bool changesAllowed;                    /**< the server lets clients change settings (allowSettingsChanges) */
    bool callerMayChange;                   /**< the caller has the role to change settings */
    bool canChange;                         /**< a change of the caller would be carried out: both of the above, or the site administrator */
    CORE::CString changeRefusal;            /**< why a change of the caller would be refused, empty when it would not */
    bool persistent;                        /**< the overrides are kept in the state folder over a restart */
    UInt64 revision;                        /**< counts the changes since the start; a new value means the settings changed */
    TSettingStateVector entries;            /**< in the order of the registry */

    CAssetSettingsDescription( void );
    virtual ~CAssetSettingsDescription() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    /** The entry of the key, GUCEF_NULL when it is not described */
    const CAssetSettingState* Find( const CORE::CString& key ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Type check of the result of a future or of the data of an event without RTTI (CICloneable::CloneableAs()) */
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

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSDESCRIPTION_H ? */
