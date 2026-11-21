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

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGSREGISTRY_H
#define GUCEF_ASSETSTORE_CASSETSETTINGSREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H
#include "gucefASSETSTORE_CAssetSettingDescriptor.h"
#define GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H
#endif /* GUCEF_ASSETSTORE_CASSETSETTINGDESCRIPTOR_H ? */

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
 *  The settings of a service that a client may read and an administrator may change while it runs (gucefASSETSTORE_RemoteService_Design.md
 *  18.1): a CAssetSettingDescriptor per key, in the order they were registered. The registry is filled before it is used and read only
 *  afterwards, so it needs no lock.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetSettingsRegistry
{
    public:

    typedef GUCEF::vector< CAssetSettingDescriptor > TDescriptorVector;

    CAssetSettingsRegistry( void );
    virtual ~CAssetSettingsRegistry();

    /** Adds a descriptor; false with the reason for one that is not valid or whose key is taken */
    bool Register( const CAssetSettingDescriptor& descriptor, CORE::CString& reason );

    /** The descriptor of the key, GUCEF_NULL when the key is not registered */
    const CAssetSettingDescriptor* Find( const CORE::CString& key ) const;

    const TDescriptorVector& GetDescriptors( void ) const;

    UInt32 GetCount( void ) const;

    /** Checks a value of a key (CAssetSettingDescriptor::Normalize()); a key that is not registered is "not a setting of this service" */
    bool Normalize( const CORE::CString& key, const CORE::CString& text, CORE::CString& normalized, CORE::CString& problem ) const;

    void Clear( void );

    private:

    TDescriptorVector m_descriptors;
    GUCEF::map< CORE::CString, UInt32 > m_index;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSETTINGSREGISTRY_H ? */
