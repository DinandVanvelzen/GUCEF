/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CDISCOVERYENTRY_H
#define GUCEF_COMCORE_CDISCOVERYENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIDISCOVERYENTRY_H
#include "gucefCOMCORE_CIDiscoveryEntry.h"
#define GUCEF_COMCORE_CIDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CIDISCOVERYENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The concrete discovery entry: what an advertiser publishes and what a browser finds
 */
class GUCEF_COMCORE_EXPORT_CPP CDiscoveryEntry : public CIDiscoveryEntry
{
    public:

    CDiscoveryEntry( void );

    CDiscoveryEntry( const CDiscoveryEntry& src );

    /** Copies all the data of any entry implementation */
    explicit CDiscoveryEntry( const CIDiscoveryEntry& src );

    virtual ~CDiscoveryEntry();

    CDiscoveryEntry& operator=( const CDiscoveryEntry& src );

    virtual bool SetApplicationId( const CORE::CString& appId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetApplicationId( CORE::CString& appId ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetApplicationInstanceId( const CORE::CString& appInstanceId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetApplicationInstanceId( CORE::CString& appInstanceId ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetHostInstanceId( const CORE::CString& hostInstanceId ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetHostInstanceId( CORE::CString& hostInstanceId ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetHost( const COMCORE::CHostAddress& host ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetHost( COMCORE::CHostAddress& host ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetCapabilities( const CORE::CString& capabilities ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetCapabilities( CORE::CString& capabilities ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetServiceType( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetServiceType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetInstanceName( const CORE::CString& instanceName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetInstanceName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetAttributes( const CORE::CValueList& attributes ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CValueList& GetAttributes( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Direct access for filling the attributes in place */
    CORE::CValueList& GetAttributes( void );

    virtual bool SetEndpoints( const CORE::CString::StringVector& endpoints ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString::StringVector& GetEndpoints( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Direct access for filling the endpoints in place */
    CORE::CString::StringVector& GetEndpoints( void );

    virtual bool SetTtlInSecs( UInt32 ttlInSecs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetTtlInSecs( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetLastSeen( const CORE::CDateTime& lastSeen ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CDateTime& GetLastSeen( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SetSourceClientName( const CORE::CString& clientName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetSourceClientName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Copies all the data of any entry implementation */
    void Assign( const CIDiscoveryEntry& src );

    /**
     *  Whether the advertised content is the same: everything except when it was last seen and by which client
     */
    bool HasSameContent( const CIDiscoveryEntry& other ) const;

    /** Whether the two value lists have the same keys with the same values */
    static bool AreAttributesEqual( const CORE::CValueList& a, const CORE::CValueList& b );

    private:

    CORE::CString m_applicationId;
    CORE::CString m_applicationInstanceId;
    CORE::CString m_hostInstanceId;
    CHostAddress m_host;
    CORE::CString m_capabilities;
    CORE::CString m_serviceType;
    CORE::CString m_instanceName;
    CORE::CValueList m_attributes;
    CORE::CString::StringVector m_endpoints;
    UInt32 m_ttlInSecs;
    CORE::CDateTime m_lastSeen;
    CORE::CString m_sourceClientName;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CDiscoveryEntry, MT::CMutex > CDiscoveryEntryPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDISCOVERYENTRY_H ? */
