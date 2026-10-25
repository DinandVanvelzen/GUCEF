/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#define GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#include "gucefAUTH_CAuthProviderDescriptor.h"
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The provider descriptors known to the process. CAuthGlobal fills it with the built in ones (RegisterBuiltIns());
 *  a plugin adds the descriptors of its backend when it loads (and takes them away when it unloads, UnregisterDescriptor());
 *  an application adds its own, or changes a built in one, from its configuration: LoadConfig() takes a node whose
 *  children are descriptors (see CAuthProviderDescriptor::Serialize()) and replaces those of the same name.
 *  Can be used from several threads.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthProviderRegistry : public CORE::CIConfigurable
{
    public:

    static const CORE::CString GoogleName;                  /**< "google" */
    static const CORE::CString MicrosoftName;               /**< "microsoft" */
    static const CORE::CString FacebookName;                /**< "facebook" */
    static const CORE::CString AmazonName;                  /**< "amazon" */
    static const CORE::CString AwsCognitoName;              /**< "aws-cognito" */
    static const CORE::CString AwsIdentityCenterName;       /**< "aws-identitycenter" */
    static const CORE::CString EpicName;                    /**< "epic" */

    CAuthProviderRegistry( void );

    virtual ~CAuthProviderRegistry();

    /** Adds the descriptors of google, microsoft, facebook, amazon, aws-cognito, aws-identitycenter and epic. The descriptors of backends come from their plugins */
    void RegisterBuiltIns( void );

    /** Adds or replaces; false when the descriptor is not well formed */
    bool RegisterDescriptor( const CAuthProviderDescriptor& descriptor );

    bool UnregisterDescriptor( const CORE::CString& name );

    bool GetDescriptor( const CORE::CString& name, CAuthProviderDescriptor& descriptor ) const;

    bool HasDescriptor( const CORE::CString& name ) const;

    void GetNames( CORE::CString::StringVector& names ) const;

    UInt32 GetCount( void ) const;

    void Clear( void );

    /** One child per descriptor, named by it */
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Every child that is a well formed descriptor is added or replaces the one of its name */
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAuthProviderRegistry( const CAuthProviderRegistry& src );
    CAuthProviderRegistry& operator=( const CAuthProviderRegistry& src );

    private:

    typedef GUCEF::map< CORE::CString, CAuthProviderDescriptor > TDescriptorMap;

    mutable MT::CMutex m_lock;
    TDescriptorMap m_descriptors;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H ? */
