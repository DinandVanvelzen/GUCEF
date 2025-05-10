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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_KAITAI_CKAITAIGLOBAL_H
#include "gucefKAITAI_CKaitaiGlobal.h"
#define GUCEF_KAITAI_CKAITAIGLOBAL_H
#endif /* GUCEF_KAITAI_CKAITAIGLOBAL_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CKaitaiSchemaRegistry.h"
#define GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H ? */

#include "gucefKAITAI_CKaitaiSchemaMeta.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CKaitaiSchemaMeta::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaMeta";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaMeta::CKaitaiSchemaMeta( void )
    : CORE::CIDataNodeSerializable()
    , CORE::CTSharedObjCreator< CKaitaiSchemaMeta, MT::CMutex >( this )
    , m_schemaId()
    , m_schemaFamily()
    , m_isLittleEndian( true )
    , m_imports()
    , m_opaqueTypesSupported( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaMeta::CKaitaiSchemaMeta( const CORE::CString& schemaFamily )
    : CORE::CIDataNodeSerializable()
    , CORE::CTSharedObjCreator< CKaitaiSchemaMeta, MT::CMutex >( this )
    , m_schemaId()
    , m_schemaFamily( schemaFamily )
    , m_isLittleEndian( true )
    , m_imports()
    , m_opaqueTypesSupported( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaMeta::CKaitaiSchemaMeta( const CKaitaiSchemaMeta& src )
    : CORE::CIDataNodeSerializable( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaMeta, MT::CMutex >( this )
    , m_schemaId( src.m_schemaId )
    , m_schemaFamily( src.m_schemaFamily )
    , m_isLittleEndian( src.m_isLittleEndian )
    , m_imports( src.m_imports )
    , m_opaqueTypesSupported( src.m_opaqueTypesSupported )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaMeta::~CKaitaiSchemaMeta()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaMeta& 
CKaitaiSchemaMeta::operator=( const CKaitaiSchemaMeta& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIDataNodeSerializable::operator=( src );
        m_schemaId = src.m_schemaId;
        m_schemaFamily = src.m_schemaFamily;
        m_isLittleEndian = src.m_isLittleEndian;
        m_opaqueTypesSupported = src.m_opaqueTypesSupported;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CKaitaiSchemaMeta::Clear( void )
{GUCEF_TRACE;

    m_schemaId.Clear();
    m_schemaFamily.Clear();
    m_isLittleEndian = true;
    m_imports.clear();
    m_opaqueTypesSupported = false;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaMeta::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaMeta( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaMeta::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchemaMeta::Serialize( CORE::CDataNode& domRootNode                        ,  
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaMeta::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    const CORE::CDataNode* metaNode = domRootNode.Find( "meta" );
    if ( GUCEF_NULL != metaNode )
    {
        m_schemaId = metaNode->GetAttributeValueOrChildValueByName( "id", m_schemaId, true ).AsString( m_schemaId, true );

        const CORE::CString& endianCurrentStr = CKaitaiSchemaBaseField::EndianValueToString( m_isLittleEndian ? GUCEF_BYTEORDER_LITTLE_ENDIAN : GUCEF_BYTEORDER_BIG_ENDIAN );
        CORE::CString endianStr = metaNode->GetAttributeValueOrChildValueByName( "endian", endianCurrentStr, true ).AsString( endianCurrentStr, true );
        m_isLittleEndian = CKaitaiSchemaBaseField::StringToEndianValue( endianStr ) == GUCEF_BYTEORDER_LITTLE_ENDIAN;

        CORE::CDataNode::TVariantVector importsList;
        if ( metaNode->GetValuesOfChildByName( "imports", importsList ) )
        {
            m_imports = CORE::ToStringVector( importsList );
        }

        m_opaqueTypesSupported = metaNode->GetAttributeValueOrChildValueByName( "opaque-types" ).AsBool( false );


        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchemaMeta::AreOpaqueTypesEnabled( void ) const
{GUCEF_TRACE;

    return m_opaqueTypesSupported;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaMeta::GetSchemaFamily( void ) const
{GUCEF_TRACE;

    return m_schemaFamily;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaMeta::GetSchemaId( void ) const
{GUCEF_TRACE;

    return m_schemaId;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaMeta::IsLittleEndian( void ) const
{GUCEF_TRACE;

    return m_isLittleEndian;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaMeta::IsBigEndian( void ) const
{GUCEF_TRACE;

    return !m_isLittleEndian;
}

/*-------------------------------------------------------------------------*/

const CORE::CStringVector& 
CKaitaiSchemaMeta::GetImports( void ) const
{GUCEF_TRACE;

    return m_imports;
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaMeta::SetSchemaFamily( const CORE::CString& schemaFamily )
{GUCEF_TRACE;

    m_schemaFamily = schemaFamily;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
