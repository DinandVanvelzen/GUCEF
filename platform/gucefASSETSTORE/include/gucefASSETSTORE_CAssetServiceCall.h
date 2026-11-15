/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETSERVICECALL_H
#define GUCEF_ASSETSTORE_CASSETSERVICECALL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One call of the asset store service as a value: the operation (a name of CAssetServiceOperations), the command id of a
 *  mutation (empty for a read) and the arguments as a DataNode object, each argument by the name of the parameter of the
 *  method. The asynchronous forms of CIAssetStoreService make one and hand it to CIAssetStoreService::ExecuteAsync(): an
 *  in-process implementation carries it out with CIAssetStoreService::Dispatch() on a pool thread, a remote one sends it.
 *  Secrets never go into a call: the operations that carry one (answers of sign in prompts, the keyring code) and the
 *  bytes of uploads have asynchronous forms of their own.
 *
 *  Serialised as an object with "operation", "commandId" and the child "args".
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetServiceCall : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString operation;
    CORE::CString commandId;                     /**< of a mutation: a repeat with the same id gets the first outcome */
    CORE::CDataNode args;                        /**< an object, the arguments by name */

    CAssetServiceCall( void );

    explicit CAssetServiceCall( const CORE::CString& operation, const CORE::CString& commandId = CORE::CString::Empty );

    virtual ~CAssetServiceCall() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    void SetText( const CORE::CString& name, const CORE::CString& value );
    CORE::CString GetText( const CORE::CString& name ) const;

    void SetUInt32( const CORE::CString& name, UInt32 value );
    UInt32 GetUInt32( const CORE::CString& name, UInt32 defaultValue = 0 ) const;

    void SetUInt64( const CORE::CString& name, UInt64 value );
    UInt64 GetUInt64( const CORE::CString& name, UInt64 defaultValue = 0 ) const;

    void SetBool( const CORE::CString& name, bool value );
    bool GetBool( const CORE::CString& name, bool defaultValue = false ) const;

    void SetTexts( const CORE::CString& name, const CORE::CString::StringVector& values );
    CORE::CString::StringVector GetTexts( const CORE::CString& name ) const;

    /** A record as the child with the name */
    bool SetRecord( const CORE::CString& name, const CORE::CIDataNodeSerializable& record );

    /** The record of the child with the name; false when there is none or it cannot be read */
    bool GetRecord( const CORE::CString& name, CORE::CIDataNodeSerializable& record ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSERVICECALL_H ? */
