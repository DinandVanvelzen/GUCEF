/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_REGISTRYHUBVIEW_H
#define GUCEF_LABLINK_REGISTRYHUBVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefLabLink_HubView.h"
#include "gucefLabLink_Registry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The view of a hub in the same process: reads its registry, which is thread safe */
class CRegistryHubView : public ILabHubView
{
    public:

    CRegistryHubView( const CNodeRegistry& registry, const GUCEF::CORE::CString& hubName );

    virtual ~CRegistryHubView() GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::CString GetHubName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::UInt64 GetHubTimeInUnixSecs( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetNodes( GUCEF::vector< SHubNode >& nodes ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetNode( const GUCEF::CORE::CString& machineId, SHubNode& node, bool& found ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetEndpoints( GUCEF::vector< SHubEndpoint >& endpoints ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A registry entry as the view shows it */
    static void ToHubNode( const SNodeEntry& entry, SHubNode& node );

    private:

    CRegistryHubView( const CRegistryHubView& src );
    CRegistryHubView& operator=( const CRegistryHubView& src );

    const CNodeRegistry& m_registry;
    GUCEF::CORE::CString m_hubName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_REGISTRYHUBVIEW_H ? */
