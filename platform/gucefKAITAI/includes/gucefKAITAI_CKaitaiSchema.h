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

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAFIELDDEFINITION_H
#include "gucefKAITAI_CKaitaiSchemaFieldDefinition.h"
#define GUCEF_KAITAI_CKAITAISCHEMAFIELDDEFINITION_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAFIELDDEFINITION_H ? */

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


class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchema : public CORE::CIDataNodeSerializable ,
                                              public CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >
{
    public:

    typedef std::map< CORE::CString, CKaitaiSchemaFieldDefinition >     TFieldMap;
    typedef std::map< CORE::CString, CKaitaiSchemaEnumDefinition >      TEnumMap;

    static const CORE::CString ClassTypeName;
    
    CORE::CString id;
    CORE::CString::StringSet imports;
    CORE::UInt8 endianess;
    TFieldMap fields;
    TEnumMap enums;

    CKaitaiSchema( void );
    CKaitaiSchema( const CKaitaiSchema& src );   
    CKaitaiSchema& operator=( const CKaitaiSchema& src ); 

    /*
     *  Utility function for loading a Kaitai schema
     */
    bool LoadSchemaFromString( const CORE::CString& schemaContent );

    /*
     *  Utility function for loading a Kaitai schema
     */
    bool LoadSchema( CORE::IOAccessPtr schemaResource );

    /*
     *  Utility function for loading a Kaitai schema
     */
    bool LoadSchema( const CORE::CUri& schemaResource ); 

    /*
     *  Utility function for loading a Kaitai schema using the VFS
     */
    bool LoadSchemaUsingVfs( const CORE::CString& schemaResourcePath );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool DeserializeMetaData( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );
    virtual bool DeserializeFieldSequence( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );
    virtual bool DeserializeEnumDefinitions( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMA_H ? */
