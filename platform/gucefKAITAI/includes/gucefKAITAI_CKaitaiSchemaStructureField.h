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

#ifndef GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H
#define GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#include "gucefKAITAI_CKaitaiSchemaBaseField.h"
#define GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H ? */

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
 * CKaitaiSchemaStructureField represents fields that are themselves nested structures.
 * It allows for hierarchical schemas with subfields.
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaStructureField : public CKaitaiSchemaBaseField ,
                                                            public CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaStructureFieldPtr;  
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaStructureFieldTypedPtr;
    typedef std::vector< CKaitaiSchemaBaseFieldPtr, gucef_allocator< CKaitaiSchemaBaseFieldPtr > >             CKaitaiSchemaBaseFieldPtrVector;

    
    CORE::CString params;                  // Parameters for dynamic behavior

    virtual void Clear( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 GetFixedSizeIfAny( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to detect what OOP construct the field is based on the various datanode shapes presented
     *  Once the correct object type is able to be determined the object is created and returned 
     *  with the data node deserialized into the construct. Returns a null shared ptr object on failure.
     */
    CKaitaiSchemaBaseFieldPtr CreateSchemaObjectForFieldDataNode( const CORE::CDataNode& fieldNode, bool& totalSuccess  ) const;

    CKaitaiSchemaStructureField( void );                                  /**< dont use this, use the other constructor */
    CKaitaiSchemaStructureField( CKaitaiSchemaMetaPtr schemaMeta );
    CKaitaiSchemaStructureField( const CKaitaiSchemaStructureField& src );
    virtual ~CKaitaiSchemaStructureField();
    CKaitaiSchemaStructureField& operator=( const CKaitaiSchemaStructureField& src );

    const CKaitaiSchemaBaseFieldPtrVector& GetFields( void ) const;

    private:

    CKaitaiSchemaBaseFieldPtrVector fields;    /**< the sequence of fields within the structure */

};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaStructureField::CKaitaiSchemaStructureFieldPtr         CKaitaiSchemaStructureFieldPtr;
typedef CKaitaiSchemaStructureField::CKaitaiSchemaStructureFieldTypedPtr    CKaitaiSchemaStructureFieldTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H ? */
