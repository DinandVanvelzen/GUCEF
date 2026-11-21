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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSREQUEST_H
#define GUCEF_ASSETSTORE_CASSETSETTINGSREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
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
 *  The arguments of the operations of the settings (gucefASSETSTORE_RemoteService_Design.md 18.2): the values of Settings/Set, the keys of
 *  Settings/Reset, and who asks. The host of the remote service sets the caller from the device whatever the client sent ("device:<id>",
 *  the device name, remoteClient true, fullRead and mayChange for a device with the role owner and without the role mcp); an in-process
 *  caller without a host is the site administrator (the defaults: remoteClient false, fullRead and mayChange true).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingsRequest : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString::StringMap values;        /**< Settings/Set: the key and the new value as text */
    CORE::CString::StringVector keys;       /**< Settings/Reset: the keys that go back to the site value */
    CORE::CString caller;                   /**< "device:<id>", "admin", "mcp", ...: for the audit */
    CORE::CString callerName;               /**< the name of the device or of the caller, for the audit */
    bool remoteClient;                      /**< a client of the remote service: allowSettingsChanges applies to it */
    bool fullRead;                          /**< the caller may read the values of the settings that are owner-only-read */
    bool mayChange;                         /**< the caller has the role to change settings */

    CAssetSettingsRequest( void );
    virtual ~CAssetSettingsRequest() GUCEF_VIRTUAL_OVERRIDE;

    /** The caller as a client device of the host: what the host sets, whatever the client sent */
    void SetRemoteCaller( const CORE::CString& deviceId, const CORE::CString& deviceName, bool readsAll, bool changes );

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

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSREQUEST_H ? */
