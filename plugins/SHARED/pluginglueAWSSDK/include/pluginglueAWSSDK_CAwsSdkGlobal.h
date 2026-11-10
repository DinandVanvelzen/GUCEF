/*
 *  pluginglueAWSSDK: Library to support multiple AWS SDK based plugins that share overlap
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_PLUGINGLUE_AWSSDK_CAWSSDKGLOBAL_H
#define GUCEF_PLUGINGLUE_AWSSDK_CAWSSDKGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_PLUGINGLUE_AWSSDK_MACROS_H
#include "pluginglueAWSSDK_macros.h"
#define GUCEF_PLUGINGLUE_AWSSDK_MACROS_H
#endif /* GUCEF_PLUGINGLUE_AWSSDK_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_AWSSDK_CCREDENTIALSPROVIDERADAPTER_H
#include "pluginglueAWSSDK_CCredentialsProviderAdapter.h"
#define GUCEF_PLUGINGLUE_AWSSDK_CCREDENTIALSPROVIDERADAPTER_H
#endif /* GUCEF_PLUGINGLUE_AWSSDK_CCREDENTIALSPROVIDERADAPTER_H ? */ 

#ifndef GUCEF_PLUGINGLUE_AWSSDK_CCLIENTCONFIGURATION_H
#include "pluginglueAWSSDK_CClientConfiguration.h"
#define GUCEF_PLUGINGLUE_AWSSDK_CCLIENTCONFIGURATION_H
#endif /* GUCEF_PLUGINGLUE_AWSSDK_CCLIENTCONFIGURATION_H ? */

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#include <aws/core/Aws.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace AWSSDK {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_PLUGINGLUE_AWSSDK_EXPORT_CPP CAwsSdkGlobal : public CORE::CObservingNotifier    ,
                                                         public CORE::CGloballyConfigurable
{
    public:

    typedef std::shared_ptr< CCredentialsProviderAdapter > CredentialsProviderPtr;
    typedef std::shared_ptr< CClientConfiguration > CClientConfigurationPtr;

    static const CORE::CEvent AwsSdkInitializedEvent;

    static void RegisterEvents( void );

    static CAwsSdkGlobal* Instance( void );

    Aws::SDKOptions& GetAwsSdkOptions( void );

    CredentialsProviderPtr GetCredentialsProvider( void );

    const Aws::Client::ClientConfiguration& GetDefaultAwsClientConfig( void );
    
    virtual bool SaveConfig( CORE::CDataNode& tree ) const;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot );

    virtual const CORE::CString& GetClassTypeName( void ) const;
    
    private:
    friend class CModule;

    static void Deinstance( void );

    static void ShutdownAtProcessExit( void );

    private:

    CAwsSdkGlobal( void );

    CAwsSdkGlobal( const CAwsSdkGlobal& src );

    virtual ~CAwsSdkGlobal();

    void InitializeBeforeConfig( void );
    void InitializeAfterConfig( void );

    private:

    Aws::SDKOptions m_awsSdkOptions;
    CredentialsProviderPtr m_credsProvider;
    CClientConfigurationPtr m_defaultClientConfig;
    bool m_isAwsApiInitialized;
    
    static MT::CMutex g_dataLock;
    static CAwsSdkGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSDK */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_AWSSDK_CAWSSDKGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 04-05-2005 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/
