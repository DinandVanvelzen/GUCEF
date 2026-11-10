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

#ifndef GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H
#define GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

class CRpcContract;
class CRpcTypeDescriptor;
class CRpcServiceDescriptor;

/*-------------------------------------------------------------------------*/

/**
 *  A method of a service (gucefCOMCORE RPC design 3.3, 3.4). Immutable once its contract is built.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcMethodDescriptor
{
    public:

    CRpcMethodDescriptor( void );
    ~CRpcMethodDescriptor();

    UInt32 GetNumber( void ) const;
    const CORE::CString& GetName( void ) const;
    const CORE::CString& GetPath( void ) const;                 /**< "<service full name>/<Method>" */
    TRpcMethodKind GetKind( void ) const;
    const CORE::CString& GetRequestTypeName( void ) const;      /**< full name */
    const CORE::CString& GetResponseTypeName( void ) const;     /**< full name */
    const CRpcTypeDescriptor* GetRequestType( void ) const;
    const CRpcTypeDescriptor* GetResponseType( void ) const;
    UInt32 GetFlags( void ) const;                              /**< GUCEF_RPC_METHODFLAG_* */
    UInt32 GetDefaultDeadlineMs( void ) const;                  /**< 0 = none */
    TRpcProtectionClass GetProtection( void ) const;            /**< class of the provider's frames */
    TRpcProtectionClass GetRequestProtection( void ) const;     /**< class of the caller's frames */
    UInt32 GetInitialCredit( void ) const;                      /**< 0 = the session default */
    UInt32 GetSince( void ) const;
    const CORE::CString& GetDoc( void ) const;
    const CRpcServiceDescriptor* GetService( void ) const;

    bool IsIdempotent( void ) const;
    bool MayAnswerAsync( void ) const;
    bool IsResumableStream( void ) const;
    bool IsSensitive( void ) const;
    bool IsStreaming( void ) const;
    bool HasClientStream( void ) const;                         /**< CLIENT_STREAM, BIDI_STREAM */
    bool HasServerStream( void ) const;                         /**< SERVER_STREAM, BIDI_STREAM */

    private:
    friend class CRpcContract;

    CRpcMethodDescriptor( const CRpcMethodDescriptor& src );                /**< not implemented */
    CRpcMethodDescriptor& operator=( const CRpcMethodDescriptor& src );     /**< not implemented */

    UInt32 m_number;
    CORE::CString m_name;
    CORE::CString m_path;
    TRpcMethodKind m_kind;
    CORE::CString m_requestTypeName;
    CORE::CString m_responseTypeName;
    const CRpcTypeDescriptor* m_requestType;
    const CRpcTypeDescriptor* m_responseType;
    UInt32 m_flags;
    UInt32 m_defaultDeadlineMs;
    TRpcProtectionClass m_protection;
    TRpcProtectionClass m_requestProtection;
    UInt32 m_initialCredit;
    UInt32 m_since;
    CORE::CString m_doc;
    const CRpcServiceDescriptor* m_service;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H ? */
