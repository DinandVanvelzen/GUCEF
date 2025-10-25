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

#ifndef GUCEF_COMCORE_CIDISCOVERY_H
#define GUCEF_COMCORE_CIDISCOVERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* often used gucefCOMCORE macros */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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
 *  Interface class which serves as the base class for all discovery clients
 */
class GUCEF_COMCORE_EXPORT_CPP CIDiscovery
{
    public:

    typedef GUCEF::set< CIDiscoveryEntryPtr > TIDiscoveryEntrySet;

    CIDiscovery( void );

    CIDiscovery( const CIDiscovery& src );

    virtual ~CIDiscovery();

    CIDiscovery& operator=( const CIDiscovery& src );

    virtual bool FindCapability( const CORE::CString& capabilityId ,
                                 TIDiscoveryEntrySet& entries      ) const = 0;

    virtual bool FindApplication( const CORE::CString& applicationId ,
                                 TIDiscoveryEntrySet& entries        ) const = 0;

    virtual bool FindHost( const COMCORE::CHostAddress& host ,
                           TIDiscoveryEntrySet& entries      ) const = 0;

    virtual bool Register( CIDiscoveryEntryPtr entry ) = 0;

    virtual bool Unregister( CIDiscoveryEntryPtr entry ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIDISCOVERYCLIENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
