/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#define GUCEF_COMCORE_CRPCSTATUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

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
 *  The outcome of an RPC call or exchange (gucefCOMCORE RPC design 9): a code with the numbering of gRPC, a message
 *  for humans and details for programs. Also used as event data, hence cloneable.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcStatus : public CORE::CICloneable
{
    public:

    TRpcStatusCode code;
    CORE::CString message;
    CORE::CDataNode details;

    CRpcStatus( void );
    explicit CRpcStatus( TRpcStatusCode code, const CORE::CString& message = CORE::CString::Empty );
    CRpcStatus( const CRpcStatus& src );
    virtual ~CRpcStatus() GUCEF_VIRTUAL_OVERRIDE;
    CRpcStatus& operator=( const CRpcStatus& src );

    bool IsOk( void ) const;

    /** Sets code and message, clears the details */
    void Set( TRpcStatusCode code, const CORE::CString& message = CORE::CString::Empty );

    /** "OK" or "NAME: message" */
    CORE::CString ToString( void ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const CRpcStatus Ok;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */
