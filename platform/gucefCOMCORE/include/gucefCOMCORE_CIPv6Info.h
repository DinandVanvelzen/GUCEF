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

#ifndef GUCEF_COMCORE_CIPV6INFO_H
#define GUCEF_COMCORE_CIPV6INFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"     
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"     
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIPV6ADDRESS_H
#include "gucefCOMCORE_CIPv6Address.h"     
#define GUCEF_COMCORE_CIPV6ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV6ADDRESS_H ? */

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

// @TODO: label as IPv4
class GUCEF_COMCORE_EXPORT_CPP CIPv6Info
{
    public:

    typedef GUCEF::vector< CIPv6Info, gucef_allocator< CIPv6Info > >                       TIPv6InfoVector;
    typedef GUCEF::set< CIPv6Info, std::less< CIPv6Info >, gucef_allocator< CIPv6Info > >  TIPv6InfoSet;

    CIPv6Address ip;
    CIPv6Address subnet;

    bool TrySetFromCIDRNotationString( const CString& cidrIpv6Str );

    /**
     *  This operator is only implemented to facilitate ordering
     *  in STL containers. The return value has no real meaning
     *  except that of a binary data compare.
     */
    bool operator<( const CIPv6Info& other ) const;

    void Clear( void );
    
    CIPv6Info( void );
    CIPv6Info( const CString& ipAsString, const CString& subnetAsString, bool resolveDns = true );
    CIPv6Info( const CIPv6Info& src );
    ~CIPv6Info();
    CIPv6Info& operator=( const CIPv6Info& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIPV6INFO_H ? */
