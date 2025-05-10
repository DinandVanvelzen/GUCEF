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
    typedef std::pair< const CORE::CString, CKaitaiSchemaBaseFieldPtr >                                                           TFieldTypePair;
    typedef std::map< CORE::CString, CKaitaiSchemaBaseFieldPtr, std::less< CORE::CString >, gucef_allocator< TFieldTypePair > >   TFieldTypeMap;
    
    CORE::CString params;                  // Parameters for dynamic behavior

    virtual void Clear( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 GetFixedSizeIfAny( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeserializeInstancesData( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );
    virtual bool DeserializeTypesData( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );
    virtual bool DeserializeEnumsData( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );

    /**
     *  Attempts to detect what OOP construct the field is based on the various datanode shapes presented
     *  Once the correct object type is able to be determined the object is created and returned 
     *  with the data node deserialized into the construct. Returns a null shared ptr object on failure.
     */
    CKaitaiSchemaBaseFieldPtr CreateSchemaObjectForFieldDataNode( const CORE::CDataNode& fieldNode , 
                                                                  bool& totalSuccess               ,
                                                                  CKaitaiSchemaBaseFieldPtr parent ) const;

    CKaitaiSchemaStructureField( void );                                  /**< dont use this, use the other constructor */
    CKaitaiSchemaStructureField( CKaitaiSchemaBaseFieldPtr parent );
    CKaitaiSchemaStructureField( const CKaitaiSchemaStructureField& src );
    virtual ~CKaitaiSchemaStructureField();
    CKaitaiSchemaStructureField& operator=( const CKaitaiSchemaStructureField& src );

    const CKaitaiSchemaBaseFieldPtrVector& GetFields( void ) const;

    /**
     *  Attempt to find an element with the given name in the local scope
     *  This follows the local scope order or precedence wrt name resolution
     */
    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeElement( const CORE::CString& elementName ) const;

    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeInstance( const CORE::CString& instanceName ) const;

    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeField( const CORE::CString& fieldName ) const;

    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeEnum( const CORE::CString& enumName ) const;

    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeTypedef( const CORE::CString& typeName ) const;
    
    const TFieldTypeMap& GetDefinedEnums( void ) const;
    
    const TFieldTypeMap& GetDefinedTypes( void ) const;

    private:

    CKaitaiSchemaBaseFieldPtrVector m_fields;    /**< the sequence of fields within the structure */
    TFieldTypeMap m_instances;
    TFieldTypeMap m_enums;
    TFieldTypeMap m_types;
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
