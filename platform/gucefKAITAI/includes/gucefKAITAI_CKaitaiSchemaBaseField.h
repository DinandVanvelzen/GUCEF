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

#ifndef GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#define GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H

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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAMETA_H
#include "gucefKAITAI_CKaitaiSchemaMeta.h"
#define GUCEF_KAITAI_CKAITAISCHEMAMETA_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAMETA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

// Enum to represent schema element types
enum KaitaiSchemaElementType 
{
    UnknownField = 0,
    
    Schema = 1,    
    BaseField = 2,

    NumericScalarField = 3,
    EnumScalarField    = 4,
    StringScalarField  = 5,
    BinaryScalarField  = 6,

    EnumDefinition = 7,
    
    ConditionalField,
    RepeatedField,
    SwitchField,
    InstanceField,
    SubstreamField,
    BitField,    
    OpaqueField,
    DelimitedField,
    StructureField
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * CKaitaiSchemaBaseField serves as the foundational class for all field types.
 * It includes shared properties like name, type, endianness, data type, and optional validation.
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaBaseField : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString ClassTypeName;

    typedef CORE::CTBasicSharedPtr< CKaitaiSchemaBaseField, MT::CMutex >     CKaitaiSchemaBaseFieldPtr;
    typedef CORE::CTSharedPtr< CKaitaiSchemaBaseField, MT::CMutex >          CKaitaiSchemaBaseFieldTypedPtr;
    
    CORE::CString id;             // Field id
    CORE::CString type;           // Field data type

    UInt8 gucefDataType;          // Custom data type for GUCEF    

    /**
     * Virtual validation method.
     * This can be overridden by derived classes for custom validation logic.
     * Default implementation assumes validation passes
     */
    virtual bool IsValid( void ) const;

    virtual bool IsFixedSize( void ) const;
    virtual Int32 GetFixedSizeIfAny( void ) const;
    virtual KaitaiSchemaElementType GetFieldType( void ) const;
    virtual const CORE::CString& GetSchemaFamily( void ) const;
    virtual const CORE::CString& GetSchemaId( void ) const;    
    virtual bool IsLittleEndian( void ) const;
    virtual bool IsBigEndian( void ) const;
    virtual CKaitaiSchemaMetaPtr GetSchemaMeta( void ) const;

    virtual void Clear( void );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CKaitaiSchemaBaseFieldPtr CloneAsFieldObject( void ) const;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    

    static CORE::UInt8 StringToEndianValue( const CORE::CString& endianTypeStr );
    static const CORE::CString& EndianValueToString( CORE::UInt8 endianIdValue );
    static CORE::UInt8 KaitaiBuildInTypeStringToGucefType( const CORE::CString& typeName );
    static CORE::UInt8 KaitaiFixedSizeValueStringToGucefType( Int32 sizeValue );
    static bool IsValidPossibleFieldName( const CORE::CString& testStr );
    CORE::UInt8 KaitaiFixedSizeValueStringToGucefType( const CORE::CString& sizeValue ) const;
    CORE::Int32 KaitaiFixedSizeValueStringToFixedSizeIfAny( const CORE::CString& sizeValue ) const;
    static CKaitaiSchemaBaseFieldPtr CreateDefaultFieldObjectForFieldType( KaitaiSchemaElementType fieldType, CKaitaiSchemaMetaPtr schemaMeta );
    static CKaitaiSchemaBaseFieldPtr CreateDefaultFieldObjectForBuildInFieldTypeName( const CORE::CString& typeName, CKaitaiSchemaMetaPtr schemaMeta );    
    virtual CKaitaiSchemaBaseFieldPtr CreateFieldObjectForFieldTypeStr( const CORE::CString& typeFieldStr, CKaitaiSchemaMetaPtr schemaMeta ) const;

    CKaitaiSchemaBaseField( KaitaiSchemaElementType fieldType, CKaitaiSchemaMetaPtr schemaMeta );
    CKaitaiSchemaBaseField( const CKaitaiSchemaBaseField& src );    
    CKaitaiSchemaBaseField& operator=( const CKaitaiSchemaBaseField& src ); 

    protected:

    KaitaiSchemaElementType m_fieldType; // The type of field (e.g., BasicField, StructureField, etc.)
    CKaitaiSchemaMetaPtr m_schemaMeta; // The schema meta information for the field, needed for context

    private:

    CKaitaiSchemaBaseField( void ); /**< Don't use, need params to correctly construct */
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaBaseField::CKaitaiSchemaBaseFieldPtr           CKaitaiSchemaBaseFieldPtr;
typedef CKaitaiSchemaBaseField::CKaitaiSchemaBaseFieldTypedPtr      CKaitaiSchemaBaseFieldTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H ? */
