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

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#define GUCEF_COMCORE_CRPCCONTRACT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CRPCENUMDESCRIPTOR_H
#include "gucefCOMCORE_CRpcEnumDescriptor.h"
#define GUCEF_COMCORE_CRPCENUMDESCRIPTOR_H
#endif /* GUCEF_COMCORE_CRPCENUMDESCRIPTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCTYPEDESCRIPTOR_H
#include "gucefCOMCORE_CRpcTypeDescriptor.h"
#define GUCEF_COMCORE_CRPCTYPEDESCRIPTOR_H
#endif /* GUCEF_COMCORE_CRPCTYPEDESCRIPTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H
#include "gucefCOMCORE_CRpcServiceDescriptor.h"
#define GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H
#endif /* GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTHASH_H
#include "gucefCOMCORE_CRpcContractHash.h"
#define GUCEF_COMCORE_CRPCCONTRACTHASH_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTHASH_H ? */

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
typedef CORE::CTSharedPtr< CRpcContract, MT::CMutex >   CRpcContractPtr;
typedef GUCEF::vector< CRpcContractPtr >                TRpcContractPtrVector;

class CRpcMessage;

/*-------------------------------------------------------------------------*/

/**
 *  A named, versioned RPC contract: enums, message types and services (gucefCOMCORE RPC design 3).
 *
 *  Authored as JSON (a CDataNode in the format of design 3.5) or built in code with CRpcContractBuilder. A contract
 *  is immutable once built: descriptors are reached through the contract and stay valid while a CRpcContractPtr
 *  to it exists. Its identity on the wire is the SHA-256 of its canonical schema encoding (3.8), computed on first
 *  use. Thread safe once built.
 *
 *  Decode limits for a schema from a peer or a file (3.8): 1 MB, 4096 types, 4096 fields per type, 1024 methods per
 *  service, 256 services.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcContract : public CORE::CTSharedObjCreator< CRpcContract, MT::CMutex >
{
    public:

    typedef GUCEF::vector< CRpcEnumDescriptor* >      TEnumVector;
    typedef GUCEF::vector< CRpcTypeDescriptor* >      TTypeVector;
    typedef GUCEF::vector< CRpcServiceDescriptor* >   TServiceVector;

    static const UInt8 SchemaEncodingVersion = 1;
    static const UInt32 MaxSchemaBytes = 1024 * 1024;
    static const UInt32 MaxTypes = 4096;
    static const UInt32 MaxFieldsPerType = 4096;
    static const UInt32 MaxMethodsPerService = 1024;
    static const UInt32 MaxServices = 256;
    static const UInt32 MaxNameLength = 128;

    /**
     *  Loads and validates a contract in the JSON authoring format. Every error names the JSON path, for example
     *  "services[0].methods[3].request: unknown type ..."; NULL on error
     */
    static CRpcContractPtr LoadFromDataNode( const CORE::CDataNode& source, CORE::CString& error );

    /** From canonical schema bytes (and optionally the documentation message); NULL on error */
    static CRpcContractPtr FromSchema( const void* schema, UInt32 size, CORE::CString& error,
                                       const void* docs = GUCEF_NULL, UInt32 docsSize = 0 );

    virtual ~CRpcContract() GUCEF_VIRTUAL_OVERRIDE;

    /** Writes the JSON authoring format (round trips with LoadFromDataNode) */
    bool SaveToDataNode( CORE::CDataNode& target ) const;

    const CORE::CString& GetName( void ) const;
    UInt32 GetMajor( void ) const;
    UInt32 GetMinor( void ) const;
    const CORE::CString& GetDoc( void ) const;

    /** "gucef.sample.notes 1.2" */
    CORE::CString GetDisplayName( void ) const;

    /** The canonical schema encoding: "GUCR", u8 version, the ContractSchema message */
    const CORE::CDynamicBuffer& GetSchema( void ) const;
    const CRpcContractHash& GetHash( void ) const;

    /** The documentation as a gucef.rpc.ContractDocs message; empty when there is none */
    const CORE::CDynamicBuffer& GetDocs( void ) const;
    bool HasDocs( void ) const;

    UInt32 GetEnumCount( void ) const;
    const CRpcEnumDescriptor& GetEnumAt( UInt32 index ) const;
    UInt32 GetTypeCount( void ) const;
    const CRpcTypeDescriptor& GetTypeAt( UInt32 index ) const;
    UInt32 GetServiceCount( void ) const;
    const CRpcServiceDescriptor& GetServiceAt( UInt32 index ) const;

    /** By full name, or by the name within the contract */
    const CRpcTypeDescriptor* FindType( const CORE::CString& name ) const;
    const CRpcEnumDescriptor* FindEnum( const CORE::CString& name ) const;
    const CRpcServiceDescriptor* FindService( const CORE::CString& name ) const;
    const CRpcServiceDescriptor* FindServiceByNumber( UInt32 number ) const;

    /** "<service>/<Method>", the service by full name or by its name within the contract */
    const CRpcMethodDescriptor* FindMethod( const CORE::CString& methodPath ) const;

    /** A new empty message of the type with the given name; unbound when there is no such type */
    CRpcMessage NewMessage( const CORE::CString& typeName ) const;

    /** Name rules of design 3.1 */
    static bool IsValidContractName( const CORE::CString& name );
    static bool IsValidTypeName( const CORE::CString& name );      /**< also service, enum and method names */
    static bool IsValidFieldName( const CORE::CString& name );
    static bool IsValidEnumValueName( const CORE::CString& name );

    private:
    friend class CORE::CTSharedObjCreator< CRpcContract, MT::CMutex >;
    friend class CRpcContractBuilder;
    friend class CRpcBuiltInContracts;

    CRpcContract( void );
    CRpcContract( const CRpcContract& src );                /**< not implemented */
    CRpcContract& operator=( const CRpcContract& src );     /**< not implemented */

    bool LoadFromDataNodeImpl( const CORE::CDataNode& source, CORE::CString& error );
    bool LoadFromSchemaMessage( const CRpcMessage& schema, CORE::CString& error );
    bool LoadDocsMessage( const CRpcMessage& docs );
    bool Finalize( CORE::CString& error );
    CORE::CString ResolveTypeName( const CORE::CString& ref ) const;
    bool BuildSchema( const CRpcContract& meta, CORE::CString& error );
    void BuildDocs( void );
    void Clear( void );

    CORE::CString m_name;
    UInt32 m_major;
    UInt32 m_minor;
    CORE::CString m_doc;
    TEnumVector m_enums;
    TTypeVector m_types;
    TServiceVector m_services;
    CORE::CDynamicBuffer m_schema;
    CORE::CDynamicBuffer m_docs;
    CRpcContractHash m_hash;
    bool m_isBuiltIn;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */
