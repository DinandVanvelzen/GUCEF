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

#ifndef GUCEF_COMCORE_CPORTMAPPINGRESULT_H
#define GUCEF_COMCORE_CPORTMAPPINGRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

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
 *  What a port mapper got from the gateway: the result data of its futures and the data of its mapping events
 */
class GUCEF_COMCORE_EXPORT_CPP CPortMappingResult : public CORE::CICloneable
{
    public:

    enum EProtocol
    {
        PROTOCOL_UDP = 0 ,
        PROTOCOL_TCP = 1
    };
    typedef enum EProtocol TProtocol;

    CPortMappingResult( void );

    CPortMappingResult( const CPortMappingResult& src );

    virtual ~CPortMappingResult();

    CPortMappingResult& operator=( const CPortMappingResult& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    TProtocol protocol;
    UInt16 internalPort;                /**< host byte order */
    CIPv4Address externalAddress;       /**< the public address of the gateway, the port is that of the mapping */
    UInt16 externalPort;                /**< host byte order, 0 when the result is only about the external address */
    UInt32 leaseInSecs;                 /**< the lease the gateway granted, 0 for a removed mapping */
    CORE::CString mapperName;           /**< the method that made the mapping, for example "natpmp" */

    /**
     *  Gets the result out of a ready future of a port mapper
     *
     *  @return false when the future is not ready, failed or has no port mapping result
     */
    static bool TryGetFromFuture( const CORE::CFutureResult& future, CPortMappingResult& result );

    static const char* ProtocolToString( TProtocol protocol );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CPORTMAPPINGRESULT_H ? */
