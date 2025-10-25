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

#ifndef GUCEF_COMCORE_CIPIV4NFO_H
#define GUCEF_COMCORE_CIPIV4NFO_H

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
class GUCEF_COMCORE_EXPORT_CPP CIPv4Info
{
    public:

    typedef GUCEF::vector< CIPv4Info, gucef_allocator< CIPv4Info > >                       TIPInfoVector;
    typedef GUCEF::set< CIPv4Info, std::less< CIPv4Info >, gucef_allocator< CIPv4Info > >    TIPv4InfoSet;

    CIPv4Address ip;
    CIPv4Address subnet;

    bool TrySetFromCIDRNotationString( const CString& cidrIpv4Str );

    /**
     *  This operator is only implemented to facilitate ordering
     *  in STL containers. The return value has no real meaning
     *  except that of a binary data compare.
     */
    bool operator<( const CIPv4Info& other ) const;

    void Clear( void );
    
    CIPv4Info( void );
    CIPv4Info( const CString& ipAsString, const CString& subnetAsString, bool resolveDns = true );
    CIPv4Info( const CIPv4Info& src );
    ~CIPv4Info();
    CIPv4Info& operator=( const CIPv4Info& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIPIV4NFO_H ? */
