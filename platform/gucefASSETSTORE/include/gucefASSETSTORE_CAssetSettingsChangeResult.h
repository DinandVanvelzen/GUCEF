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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSCHANGERESULT_H
#define GUCEF_ASSETSTORE_CASSETSETTINGSCHANGERESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGCHANGE_H
#include "gucefASSETSTORE_CAssetSettingChange.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGCHANGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGCHANGE_H ? */

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
 *  The outcome of Settings/Set and Settings/Reset (gucefASSETSTORE_RemoteService_Design.md 18.2), also the data of
 *  CIAssetStoreService::SettingsChangedEvent: the status, a message for people, the reason per key that was not valid (nothing was
 *  changed then), the changes, whether nothing changed, who changed them and the revision of the settings after the call.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingsChangeResult : public CORE::CIDataNodeSerializable
{
    public:

    typedef GUCEF::vector< CAssetSettingChange > TSettingChangeVector;

    TAssetStoreStatus status;
    CORE::CString message;                  /**< for people, never holds a secret */
    CORE::CString operation;                /**< "set" or "reset" */
    CORE::CString::StringMap errors;        /**< the key and why it is not valid: nothing was changed */
    TSettingChangeVector changes;           /**< per key of the call, also the unchanged ones */
    bool noChange;                          /**< the call changed nothing: every value was the value already */
    bool replayed;                          /**< the stored outcome of an earlier call with the same command id */
    CORE::CString caller;                   /**< who changed the settings */
    CORE::CString callerName;
    UInt64 revision;                        /**< of the settings after the call */

    CAssetSettingsChangeResult( void );
    virtual ~CAssetSettingsChangeResult() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    /** The change of the key, GUCEF_NULL when the call did not have it */
    const CAssetSettingChange* Find( const CORE::CString& key ) const;

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

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSCHANGERESULT_H ? */
