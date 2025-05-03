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

#ifndef GUCEF_KAITAI_CKAITAISCHEMASWITCHFIELD_H
#include "gucefKAITAI_CKaitaiSchemaSwitchField.h"
#define GUCEF_KAITAI_CKAITAISCHEMASWITCHFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMASWITCHFIELD_H ? */

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

#ifndef GUCEF_KAITAI_CKAITAISCHEMAINSTANCEFIELD_H
#include "gucefKAITAI_CKaitaiSchemaInstanceField.h"
#define GUCEF_KAITAI_CKAITAISCHEMAINSTANCEFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAINSTANCEFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H
#include "gucefKAITAI_CKaitaiSchemaStructureField.h"
#define GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMASTRUCTUREFIELD_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H
#include "gucefKAITAI_CKaitaiSchemaEnumDefinition.h"
#define GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAENUMDEFINITION_H ? */

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
    
    CKaitaiSchemaStructureFieldPtr structure;   // the layout of the schema as an object hierarchy    

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

    virtual bool DeserializeTypesData( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );

    virtual bool DeserializeEnumsData( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );

    virtual bool DeserializeInstancesData( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );

    //virtual bool DeserializeFieldSequence( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );
    //virtual bool DeserializeEnumDefinitions( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );
    
    KaitaiSchemaElementType GetFieldTypeForTypeFieldString( const CORE::CString& typeFieldStr ) const;
    
    virtual CKaitaiSchemaBaseFieldPtr CreateFieldObjectForFieldTypeStr( const CORE::CString& typeFieldStr, CKaitaiSchemaMetaPtr schemaMeta ) const GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    bool HasUnresolvedImports( void ) const;

    bool ResolveImports( CORE::CStringSet& unresolvedImports );

    bool SetSchemaFamily( const CORE::CString& schemaFamily );

    CKaitaiSchemaBaseFieldPtr TryGetDefinedInstance( const CORE::CString& instanceName ) const;
    
    CKaitaiSchemaBaseFieldPtr TryGetDefinedEnum( const CORE::CString& enumName ) const;
    
    CKaitaiSchemaBaseFieldPtr TryGetDefinedType( const CORE::CString& typeName ) const;

    CKaitaiSchemaBaseFieldPtr TryGetDefinedImport( const CORE::CString& importName ) const;

    
    const TFieldTypeMap& GetDefinedEnums( void ) const;
    
    const TFieldTypeMap& GetDefinedTypes( void ) const;

    
    
    private:

    TFieldTypeMap m_instances;
    TFieldTypeMap m_enums;
    TFieldTypeMap m_types;
    TFieldTypeMap m_imports;
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
