/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNENDPOINTLIST_H
#define GUCEF_DRN_CDRNENDPOINTLIST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_CDRNENDPOINT_H
#include "gucefDRN_CDRNEndpoint.h"
#define GUCEF_DRN_CDRNENDPOINT_H
#endif /* GUCEF_DRN_CDRNENDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A prioritised list of endpoints: the first is tried first.
 *  As a string the endpoints are separated by ';', as a data node every endpoint is a child "Endpoint" with the
 *  URI as its value.
 */
class GUCEF_DRN_EXPORT_CPP CDRNEndpointList : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    typedef GUCEF::vector< CDRNEndpoint > TEndpointVector;

    CDRNEndpointList( void );

    /** A list with one endpoint */
    explicit CDRNEndpointList( const CDRNEndpoint& endpoint );

    CDRNEndpointList( const CDRNEndpointList& src );

    virtual ~CDRNEndpointList();

    CDRNEndpointList& operator=( const CDRNEndpointList& src );

    bool operator==( const CDRNEndpointList& other ) const;

    void Add( const CDRNEndpoint& endpoint );

    void Clear( void );

    UInt32 GetCount( void ) const;

    bool IsEmpty( void ) const;

    const CDRNEndpoint& Get( UInt32 index ) const;

    const TEndpointVector& GetEndpoints( void ) const;

    /** Parses ';' separated URIs, invalid ones are skipped. Returns false if any was invalid */
    bool FromString( const CORE::CString& endpoints );

    CORE::CString ToString( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    TEndpointVector m_endpoints;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNENDPOINTLIST_H ? */
