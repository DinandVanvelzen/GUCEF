/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#ifndef GUCEF_KAITAI_CKAITAISCHEMAMETA_H
#define GUCEF_KAITAI_CKAITAISCHEMAMETA_H

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

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  CKaitaiSchemaMeta is a class that represents the metadata of a Kaitai schema.
 *  It includes properties such as schema ID, endianness, and imported schemas.
 *  This information is needed by various schema field concepts due to the way
 *  types, sizes etc can all reference other types
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaMeta : public CORE::CIDataNodeSerializable,
                                                  public CORE::CTSharedObjCreator< CKaitaiSchemaMeta, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaMeta, MT::CMutex >::TBasicSharedPtrType      CKaitaiSchemaMetaPtr;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaMeta, MT::CMutex >::TSharedPtrType           CKaitaiSchemaMetaTypedPtr;

    CKaitaiSchemaMeta( void ); /**< dont use, need schema family to construct */    
    CKaitaiSchemaMeta( const CORE::CString& schemaFamily ); 
    CKaitaiSchemaMeta( const CKaitaiSchemaMeta& src );   
    virtual ~CKaitaiSchemaMeta() GUCEF_VIRTUAL_OVERRIDE;
    CKaitaiSchemaMeta& operator=( const CKaitaiSchemaMeta& src ); 

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        , 
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  , 
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetSchemaFamily( void ) const;

    const CORE::CString& GetSchemaId( void ) const;

    bool IsLittleEndian( void ) const;

    bool IsBigEndian( void ) const;

    const CORE::CStringVector& GetImports( void ) const;

    void Clear( void );

    private:
    friend class CKaitaiSchema;
    

    void SetSchemaFamily( const CORE::CString& schemaFamily );

    private:

    CORE::CString m_schemaId;
    CORE::CString m_schemaFamily;
    bool m_isLittleEndian;                        // True if the schema is little-endian, false if big-endian
    CORE::CStringVector m_imports;
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaMeta::CKaitaiSchemaMetaPtr         CKaitaiSchemaMetaPtr;
typedef CKaitaiSchemaMeta::CKaitaiSchemaMetaTypedPtr    CKaitaiSchemaMetaTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMAMETA_H ? */
