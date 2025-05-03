///*
// *  gucefKAITAI: Platform module supporting the Kaitai binary description format
// *
// *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
// *
// *  Licensed under the Apache License, Version 2.0 (the "License");
// *  you may not use this file except in compliance with the License.
// *  You may obtain a copy of the License at
// *
// *    http://www.apache.org/licenses/LICENSE-2.0
// *
// *  Unless required by applicable law or agreed to in writing, software
// *  distributed under the License is distributed on an "AS IS" BASIS,
// *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// *  See the License for the specific language governing permissions and
// *  limitations under the License.
// */
//
//#ifndef GUCEF_KAITAI_CKAITAISCHEMAFIELDDEFINITION_H
//#define GUCEF_KAITAI_CKAITAISCHEMAFIELDDEFINITION_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
//#include "gucefKAITAI_CKaitaiSchemaBaseField.h"
//#define GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
//#endif /* GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H ? */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//namespace GUCEF {
//namespace KAITAI {
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      TYPES                                                              //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//// Enum to represent field types
//enum KaitaiSchemaFieldType 
//{
//    BaseField,
//    BasicField,
//    ConditionalField,
//    RepeatedField,
//    SwitchField,
//    InstanceField,
//    SubstreamField,
//    BitField,
//    EnumField,
//    OpaqueField,
//    DelimitedField,
//    StructureField
//};
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      CLASSES                                                            //
////                                                                         //
////-------------------------------------------------------------------------*/
//
///**
// * CKaitaiSchemaBaseField serves as the foundational class for all field types.
// * It includes shared properties like name, type, endianness, data type, and optional validation.
// */
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaBaseField : public CORE::CIDataNodeSerializable
//{
//    public:
//
//    static const CORE::CString ClassTypeName;
//    
//    CORE::CString name;           // Field name
//    CORE::CString type;           // Field data type
//    bool isLittleEndian;          // Byte order (true = little-endian, false = big-endian)
//    UInt8 gucefDataType;          // Custom data type for GUCEF
//    CORE::CString validExpression; // Validation expression (optional)
//
//    // Virtual methods for customization in derived classes
//    virtual int GetSize() const = 0;
//
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return BaseField;
//    }
//
//    /**
//     * Virtual validation method.
//     * This can be overridden by derived classes for custom validation logic.
//     */
//    virtual bool Validate() const {
//        return true; // Default implementation assumes validation passes
//    }
//
//
//    //CORE::CString id;
//    //CORE::CString type;
//    //CORE::UInt8 endianess;
//    //CORE::UInt32 fieldTypeId;
//    //bool hasSize;
//    //CKaitaiSchemaFieldProperty size;
//    //bool isEnum;
//    //CORE::CString enumId;
//    //CORE::CString documentation;
//    //bool hasContents;
//    //CORE::CDynamicBuffer contents;
//
//    CKaitaiSchemaBaseField( void );
//    CKaitaiSchemaBaseField( const CKaitaiSchemaBaseField& src );    
//    CKaitaiSchemaBaseField& operator=( const CKaitaiSchemaBaseField& src ); 
//
//    static CORE::UInt8 StringToEndianValue( const CORE::CString& str );
//    static const CORE::CString& EndianValueToString( CORE::UInt8 value );
//
//    static CORE::UInt8 KaitaiTypeStringToGucefType( const CORE::CString& str );
//
//    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
//    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
//    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
//    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
//};
//
///*-------------------------------------------------------------------------*/
//
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaBasicField : public CKaitaiSchemaBaseField ,
//                                                        public CORE::CTSharedObjCreator< CKaitaiSchemaBasicField, MT::CMutex >
//{
//    public:
//
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaBasicField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaBasicFieldPtr;  
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaBasicField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaBasicFieldTypedPtr;  
//
//    CORE::CString contents; // Contents of the field
//    virtual int GetSize() const {
//        return 0;
//    }
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return BasicField;
//    }
//};
//
//typedef CKaitaiSchemaBasicField::CKaitaiSchemaBasicFieldPtr         CKaitaiSchemaBasicFieldPtr;
//typedef CKaitaiSchemaBasicField::CKaitaiSchemaBasicFieldTypedPtr    CKaitaiSchemaBasicFieldTypedPtr;
//
///*-------------------------------------------------------------------------*/
//
///**
// * CKaitaiSchemaStructureField represents fields that are themselves nested structures.
// * It allows for hierarchical schemas with subfields.
// */
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaStructureField : public CKaitaiSchemaBaseField ,
//                                                            public CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >
//{
//    public:
//
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaStructureFieldPtr;  
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaStructureFieldTypedPtr;
//
//    std::vector<CKaitaiSchemaBaseField*> nestedFields; // Collection of fields within the structure
//    CORE::CString params;                  // Parameters for dynamic behavior
//
//    CKaitaiSchemaStructureField() {}
//    virtual ~CKaitaiSchemaStructureField() {
//        for (std::vector<CKaitaiSchemaBaseField*>::iterator it = nestedFields.begin(); it != nestedFields.end(); ++it) {
//            delete *it;
//        }
//    }
//
//    virtual int GetSize() const {
//        int totalSize = 0;
//        for (std::vector<CKaitaiSchemaBaseField*>::const_iterator it = nestedFields.begin(); it != nestedFields.end(); ++it) {
//            totalSize += (*it)->GetSize();
//        }
//        return totalSize; // Sum of all nested fields' sizes
//    }
//
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return StructureField;
//    }
//};
//
//typedef CKaitaiSchemaStructureField::CKaitaiSchemaStructureFieldPtr         CKaitaiSchemaStructureFieldPtr;
//typedef CKaitaiSchemaStructureField::CKaitaiSchemaStructureFieldTypedPtr    CKaitaiSchemaStructureFieldTypedPtr;
//
///*-------------------------------------------------------------------------*/
//
//// SubstreamField class with processing capabilities
///**
// * CSubstreamField represents substreams and supports custom processing routines.
// * Useful for handling compressed, encrypted, or obfuscated data.
// */
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaSubstreamField : public CKaitaiSchemaBaseField ,
//                                                            public CORE::CTSharedObjCreator< CKaitaiSchemaSubstreamField, MT::CMutex >
//{
//    public:
//
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaSubstreamField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaSubstreamFieldPtr;
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaSubstreamField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaSubstreamFieldTypedPtr;
//    
//    CORE::CString substream;         // Reference to the substream
//    CORE::CString processingRoutine; // Hook for custom processing logic (e.g., XOR, compression)
//
//    virtual int GetSize() const {
//        return -1; // Substreams may have variable sizes
//    }
//
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return SubstreamField;
//    }
//};
//
//typedef CKaitaiSchemaSubstreamField::CKaitaiSchemaSubstreamFieldPtr         CKaitaiSchemaSubstreamFieldPtr;
//typedef CKaitaiSchemaSubstreamField::CKaitaiSchemaSubstreamFieldTypedPtr    CKaitaiSchemaSubstreamFieldTypedPtr;
//
///*-------------------------------------------------------------------------*/
//
//// DelimitedField class with delimiter support
///**
// * CDelimitedField represents fields with variable-length data that terminate upon reaching a specific delimiter.
// */
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaDelimitedField : public CKaitaiSchemaBaseField ,
//                                                            public CORE::CTSharedObjCreator< CKaitaiSchemaDelimitedField, MT::CMutex >
//{
//    public:
//
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaDelimitedField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaDelimitedFieldPtr;
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaDelimitedField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaDelimitedFieldTypedPtr;
//    
//    
//    CORE::CString delimiter; // Delimiter marking the end of the field
//
//    virtual int GetSize() const {
//        return -1; // Variable length based on the delimiter
//    }
//
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return DelimitedField;
//    }
//};
//
//typedef CKaitaiSchemaDelimitedField::CKaitaiSchemaDelimitedFieldPtr         CKaitaiSchemaDelimitedFieldPtr;
//typedef CKaitaiSchemaDelimitedField::CKaitaiSchemaDelimitedFieldTypedPtr    CKaitaiSchemaDelimitedFieldTypedPtr;
//
///*-------------------------------------------------------------------------*/
//
//// OpaqueField class with external processing support
///**
// * COpaqueField represents fields that are processed externally using custom logic.
// * This class can integrate external libraries or hooks.
// */
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaOpaqueField : public CKaitaiSchemaBaseField ,
//                                                         public CORE::CTSharedObjCreator< CKaitaiSchemaOpaqueField, MT::CMutex >
//{
//    public:
//
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaOpaqueField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaOpaqueFieldPtr;
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaOpaqueField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaOpaqueFieldTypedPtr;
//    
//    CORE::CString externalProcessor; // Name of the external processing routine
//
//    virtual int GetSize() const {
//        return -1; // Size determined externally
//    }
//
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return OpaqueField;
//    }
//};
//
//typedef CKaitaiSchemaOpaqueField::CKaitaiSchemaOpaqueFieldPtr         CKaitaiSchemaOpaqueFieldPtr;
//typedef CKaitaiSchemaOpaqueField::CKaitaiSchemaOpaqueFieldTypedPtr    CKaitaiSchemaOpaqueFieldTypedPtr;
//
///*-------------------------------------------------------------------------*/
//
//// SwitchField class with advanced switching logic
///**
// * CSwitchField supports advanced type switching based on dynamic conditions.
// * Nested switches and multi-condition cases are supported.
// */
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaSwitchField : public CKaitaiSchemaBaseField ,
//                                                         public CORE::CTSharedObjCreator< CKaitaiSchemaSwitchField, MT::CMutex >
//{
//    public:
//
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaSwitchField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaSwitchFieldPtr;
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaSwitchField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaSwitchFieldTypedPtr;
//    
//    CORE::CString switchOn;                              // Expression to determine type
//    std::map<CORE::CString, CORE::CString> cases;        // Mapping of cases to types
//    CORE::CString defaultCase;                           // Default case for unmatched conditions
//
//    virtual int GetSize() const {
//        return -1; // Size depends on selected type
//    }
//
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return SwitchField;
//    }
//};
//
//typedef CKaitaiSchemaSwitchField::CKaitaiSchemaSwitchFieldPtr         CKaitaiSchemaSwitchFieldPtr;
//typedef CKaitaiSchemaSwitchField::CKaitaiSchemaSwitchFieldTypedPtr    CKaitaiSchemaSwitchFieldTypedPtr;
//
///*-------------------------------------------------------------------------*/
//
//// InstanceField class with advanced computation
///**
// * CInstanceField represents computed fields derived from other schema data.
// * Can support complex expressions or dependencies.
// */
//class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaInstanceField : public CKaitaiSchemaBaseField ,
//                                                           public CORE::CTSharedObjCreator< CKaitaiSchemaInstanceField, MT::CMutex >
//{
//    public:
//
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaInstanceField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaInstanceFieldPtr;
//    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaInstanceField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaInstanceFieldTypedPtr;
//    
//    CORE::CString expression; // Logic for deriving the value
//
//    virtual int GetSize() const {
//        return 0; // Computed fields take no space
//    }
//
//    virtual KaitaiSchemaFieldType GetFieldType() const {
//        return InstanceField;
//    }
//
//    /**
//     * Computes the field value using the provided expression.
//     */
//    virtual CORE::CString ComputeValue() const {
//        return "DerivedValue"; // Placeholder: Replace with actual computation logic
//    }
//};
//
//typedef CKaitaiSchemaInstanceField::CKaitaiSchemaInstanceFieldPtr         CKaitaiSchemaInstanceFieldPtr;
//typedef CKaitaiSchemaInstanceField::CKaitaiSchemaInstanceFieldTypedPtr    CKaitaiSchemaInstanceFieldTypedPtr;
//
///*-------------------------------------------------------------------------*/
//
//// Meta class for schema metadata
///**
// * CSchemaMeta stores metadata about the schema, such as author, version, and other details.
// */
//class CSchemaMeta {
//public:
//    CORE::CString author;
//    CORE::CString version;
//    CORE::CString description;
//
//    CSchemaMeta() {}
//};
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//}; /* namespace KAITAI */
//}; /* namespace GUCEF */
//
///*-------------------------------------------------------------------------*/
//
//#endif /* GUCEF_KAITAI_CKAITAISCHEMAFIELDDEFINITION_H ? */
