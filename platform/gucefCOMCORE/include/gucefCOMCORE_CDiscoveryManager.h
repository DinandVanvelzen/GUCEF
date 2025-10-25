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

#ifndef GUCEF_COMCORE_CDISCOVERYMANAGER_H
#define GUCEF_COMCORE_CDISCOVERYMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERYENTRY_H
#include "gucefCOMCORE_CIDiscoveryEntry.h"
#define GUCEF_COMCORE_CIDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CIDISCOVERYENTRY_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#include "gucefCOMCORE_CIDiscoveryClient.h"
#define GUCEF_COMCORE_CIDISCOVERYCLIENT_H
#endif /* GUCEF_COMCORE_CIDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COMCORE_CIDISCOVERY_H
#include "gucefCOMCORE_CIDiscovery.h"
#define GUCEF_COMCORE_CIDISCOVERY_H
#endif /* GUCEF_COMCORE_CIDISCOVERY_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* often used gucefCOMCORE macros */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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
 *  Interface class which serves as the base class for all discovery clients
 */
class GUCEF_COMCORE_EXPORT_CPP CDiscoveryManager : public virtual CORE::CGloballyConfigurable ,
                                                   public virtual CIDiscovery
{
    public:

    typedef GUCEF::set< CORE::CString > TStringSet;
    typedef GUCEF::set< CIDiscoveryClientPtr > TDiscoveryClientSet;

    bool RegisterDiscoveryClient( CIDiscoveryClientPtr discoveryClient );

    bool UnregisterDiscoveryClient( const CORE::CString& discoveryClientName );

    virtual bool FindCapability( const CORE::CString& capabilityId ,
                                 TIDiscoveryEntrySet& entries      ) const;

    virtual bool FindApplication( const CORE::CString& applicationId ,
                                 TIDiscoveryEntrySet& entries        ) const;

    virtual bool FindHost( const COMCORE::CHostAddress& host ,
                           TIDiscoveryEntrySet& entries      ) const;

    virtual bool Register( CIDiscoveryEntryPtr entry );

    virtual bool Unregister( CIDiscoveryEntryPtr entry );

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param tree the data tree you wish to store
     *      @return wheter storing the tree was successfull
     */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot );

    virtual const CString& GetClassTypeName( void ) const;

    private:

    CDiscoveryManager( const CDiscoveryManager& src );            /** not implemented */
    CDiscoveryManager& operator=( const CDiscoveryManager& src ); /** not implemented */

    private:
    friend class CComCoreGlobal;

    CDiscoveryManager( void );
    ~CDiscoveryManager();

    private:

    TDiscoveryClientSet m_clients;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CDISCOVERYMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
