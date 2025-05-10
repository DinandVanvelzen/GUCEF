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

#ifndef GUCEF_KAITAI_CKAITAISCHEMA_H
#define GUCEF_KAITAI_CKAITAISCHEMA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#include "gucefKAITAI_CKaitaiSchemaBaseField.h"
#define GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMANUMERICSCALARFIELD_H
#include "gucefKAITAI_CKaitaiSchemaNumericScalarField.h"
#define GUCEF_KAITAI_CKAITAISCHEMANUMERICSCALARFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMANUMERICSCALARFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMASTRINGSCALARFIELD_H
#include "gucefKAITAI_CKaitaiSchemaStringScalarField.h"
#define GUCEF_KAITAI_CKAITAISCHEMASTRINGSCALARFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMASTRINGSCALARFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMABINARYSCALARFIELD_H
#include "gucefKAITAI_CKaitaiSchemaBinaryScalarField.h"
#define GUCEF_KAITAI_CKAITAISCHEMABINARYSCALARFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMABINARYSCALARFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMASWITCHLOGIC_H
#include "gucefKAITAI_CKaitaiSchemaSwitchLogic.h"
#define GUCEF_KAITAI_CKAITAISCHEMASWITCHLOGIC_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMASWITCHLOGIC_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMASREPEATLOGIC_H
#include "gucefKAITAI_CKaitaiSchemaRepeatLogic.h"
#define GUCEF_KAITAI_CKAITAISCHEMASREPEATLOGIC_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMASREPEATLOGIC_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMACONDITIONALLOGIC_H
#include "gucefKAITAI_CKaitaiSchemaConditionalLogic.h"
#define GUCEF_KAITAI_CKAITAISCHEMACONDITIONALLOGIC_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMACONDITIONALLOGIC_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMALOGICINSTANCE_H
#include "gucefKAITAI_CKaitaiSchemaLogicInstance.h"
#define GUCEF_KAITAI_CKAITAISCHEMALOGICINSTANCE_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMALOGICINSTANCE_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAOPAQUEFIELD_H
#include "gucefKAITAI_CKaitaiSchemaOpaqueField.h"
#define GUCEF_KAITAI_CKAITAISCHEMAOPAQUEFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAOPAQUEFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMASUBSTREAMFIELD_H
#include "gucefKAITAI_CKaitaiSchemaSubstreamField.h"
#define GUCEF_KAITAI_CKAITAISCHEMASUBSTREAMFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMASUBSTREAMFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMADELIMITEDFIELD_H
#include "gucefKAITAI_CKaitaiSchemaDelimitedField.h"
#define GUCEF_KAITAI_CKAITAISCHEMADELIMITEDFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMADELIMITEDFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H
#include "gucefKAITAI_CKaitaiSchemaStructureField.h"
#define GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAENUMSCALARFIELD_H
#include "gucefKAITAI_CKaitaiSchemaEnumScalarField.h"
#define GUCEF_KAITAI_CKAITAISCHEMAENUMSCALARFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAENUMSCALARFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H
#include "gucefKAITAI_CKaitaiSchemaEnumDefinition.h"
#define GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMACONSTVALIDATIONSCALARFIELD_H
#include "gucefKAITAI_CKaitaiSchemaConstValidationScalarField.h"
#define GUCEF_KAITAI_CKAITAISCHEMACONSTVALIDATIONSCALARFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMACONSTVALIDATIONSCALARFIELD_H ? */

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


class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchema : public CKaitaiSchemaBaseField ,
                                              public CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;
    static const CORE::CString SchemaFileExtension;

    typedef std::pair< const CORE::CString, CKaitaiSchemaBaseFieldPtr >                                                           TFieldTypePair;
    typedef std::map< CORE::CString, CKaitaiSchemaBaseFieldPtr, std::less< CORE::CString >, gucef_allocator< TFieldTypePair > >   TFieldTypeMap;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >::TBasicSharedPtrType                                   CKaitaiSchemaPtr;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >::TSharedPtrType                                        CKaitaiSchemaTypedPtr;   

    CKaitaiSchema( void );
    CKaitaiSchema( const CORE::CString& schemaFamily );
    CKaitaiSchema( const CKaitaiSchema& src );   
    virtual ~CKaitaiSchema() GUCEF_VIRTUAL_OVERRIDE;
    CKaitaiSchema& operator=( const CKaitaiSchema& src );    

    /*
     *  Utility function for loading a Kaitai schema
     */
    bool LoadSchemaFromString( const CORE::CString& schemaContent  ,
                               CORE::CStringSet& unresolvedImports );

    /*
     *  Utility function for loading a Kaitai schema
     */
    bool LoadSchema( CORE::IOAccessPtr schemaResource    , 
                     CORE::CStringSet& unresolvedImports );

    /*
     *  Utility function for loading a Kaitai schema
     */
    bool LoadSchema( const CORE::CUri& schemaResource    ,
                     CORE::CStringSet& unresolvedImports ); 

    /*
     *  Utility function for loading a Kaitai schema using the VFS
     */
    bool LoadSchemaUsingVfs( const CORE::CString& schemaResourcePath ,
                             CORE::CStringSet& unresolvedImports     );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsValid( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  , 
                              const CORE::CDataNodeSerializableSettings& settings ,
                              CORE::CStringSet& unresolvedImports                 );
    
    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  , 
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool DeserializeMetaData( const CORE::CDataNode& domRootNode                  , 
                                      const CORE::CDataNodeSerializableSettings& settings ,
                                      CORE::CStringSet& unresolvedImports                 );

    void Clear( void );

    bool HasUnresolvedImports( void ) const;

    bool ResolveImports( CORE::CStringSet& unresolvedImports );

    bool SetSchemaFamily( const CORE::CString& schemaFamily );

    /**
     *  Attempt to find an element with the given name in the local scope
     *  This follows the local scope order or precedence wrt name resolution
     */
    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeElement( const CORE::CString& elementName ) const;

    /**
     *  Attempt to find an 'instance' with the name given within the local scope
     */
    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeInstance( const CORE::CString& instanceName ) const;

    /**
     *  Attempt to find an 'field' with the name given within the local scope
     */
    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeField( const CORE::CString& fieldName ) const;

    /**
     *  Attempt to find an 'enum' with the name given within the local scope
     */
    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeEnum( const CORE::CString& enumName ) const;

    /**
     *  Attempt to find an 'type definition' with the name given within the local scope
     */
    CKaitaiSchemaBaseFieldPtr TryGetLocalScopeTypedef( const CORE::CString& typeName ) const;

    CKaitaiSchemaBaseFieldPtr TryGetDefinedImport( const CORE::CString& importName ) const;

    CKaitaiSchemaBaseFieldPtr TryGetReferencedFullyQualifiedElement( const CORE::CString& fullyQualifiedName );    

    CKaitaiSchemaStructureFieldPtr GetRootStructure( void ) const;

    virtual const CORE::CString& GetSchemaFamily( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetSchemaId( void ) const GUCEF_VIRTUAL_OVERRIDE;    
    virtual bool IsLittleEndian( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsBigEndian( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CKaitaiSchemaMetaPtr GetSchemaMeta( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CKaitaiSchemaBaseFieldPtr GetParent( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CKaitaiSchemaBaseFieldPtr GetRootParent( void ) const GUCEF_VIRTUAL_OVERRIDE;    
    private:

    CKaitaiSchemaStructureFieldPtr m_structure;   // the layout of the schema as an object hierarchy 
    TFieldTypeMap m_imports;
    CKaitaiSchemaMetaPtr m_schemaMeta;
    CORE::CDataNode m_schemaDocument;
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchema::CKaitaiSchemaPtr         CKaitaiSchemaPtr;
typedef CKaitaiSchema::CKaitaiSchemaTypedPtr    CKaitaiSchemaTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMA_H ? */
