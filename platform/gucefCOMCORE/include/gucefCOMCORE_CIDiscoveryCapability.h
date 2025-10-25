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

#ifndef GUCEF_COMCORE_CIDISCOVERYCAPABILITY_H
#define GUCEF_COMCORE_CIDISCOVERYCAPABILITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

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
class GUCEF_COMCORE_EXPORT_CPP CIDiscoveryCapability
{
    public:

    typedef GUCEF::set< CORE::CString > TStringSet;

    CIDiscoveryCapability( void );

    CIDiscoveryCapability( const CIDiscoveryCapability& src );

    virtual ~CIDiscoveryCapability();

    CIDiscoveryCapability& operator=( const CIDiscoveryCapability& src );
    
    /**
     *  Identifier of the application providing the relevant capabilities
     */
    virtual bool SetCapability( const CORE::CString& capability ) = 0;

    virtual bool GetCapability( CORE::CString& capability ) const = 0;

    /**
     *  Identifier of the communication method to be used by applications 
     *  interested in utilizing the provided capability
     *  This would determine whether the communication method utilized is compatible between client and server
     *  These identifiers are essentially the IDs of the possible contracts between clients and the server
     */
    virtual bool SetCapabilityAccessMethods( const TStringSet& capabilityAccessMethods ) = 0;

    virtual bool GetCapabilityAccessMethods( TStringSet& capabilityAccessMethods ) const = 0;

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIDISCOVERYCAPABILITIES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
