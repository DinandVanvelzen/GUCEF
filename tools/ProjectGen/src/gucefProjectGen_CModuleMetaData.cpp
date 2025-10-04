/*
 *  ProjectGen: GUCEF module providing the business logic to generate module/project files
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#include "gucefProjectGen_CModuleMetaData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CModuleMetaData::ClassTypeName = "GUCEF::PROJECTGEN::CModuleMetaData";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CModuleMetaData::CModuleMetaData( void ) 
    : CORE::CIDataNodeSerializable()
    , lastEditBy()
    , authors()
    , maintainers()
    , descriptionHeadline()
    , descriptionDetails()
    , license()
    , m_semver()
    , m_hasSemVer( false )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleMetaData::CModuleMetaData( const CModuleMetaData& src ) 
    : CORE::CIDataNodeSerializable( src )
    , lastEditBy( src.lastEditBy )
    , authors( src.authors )
    , maintainers( src.maintainers )
    , descriptionHeadline( src.descriptionHeadline )
    , descriptionDetails( src.descriptionDetails )
    , license( src.license )
    , m_semver( src.m_semver )
    , m_hasSemVer( src.m_hasSemVer )
{GUCEF_TRACE;

}

/*---------------------------------------------------------------------------*/

CModuleMetaData::~CModuleMetaData() 
{GUCEF_TRACE;

    Clear();
}

/*---------------------------------------------------------------------------*/

void
CModuleMetaData::Clear( void ) 
{GUCEF_TRACE;

    lastEditBy.Clear();
    authors.clear();
    maintainers.clear();    
    descriptionHeadline.Clear();
    descriptionDetails.Clear();
    license.Clear();
    m_semver.Clear();
    m_hasSemVer = false;
}

/*---------------------------------------------------------------------------*/

void
CModuleMetaData::SetSemVer( const CORE::CVersion& semVer )
{GUCEF_TRACE;

    m_semver = semVer;
    m_hasSemVer = true;
}

/*---------------------------------------------------------------------------*/

const CORE::CVersion&
CModuleMetaData::GetSemVer( void ) const
{GUCEF_TRACE;

    return m_semver;
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::HasSemVer( void ) const
{GUCEF_TRACE;

    return m_hasSemVer;
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::HasAnyMetaData( void ) const
{GUCEF_TRACE;

    return m_hasSemVer ||
           !lastEditBy.IsNULLOrEmpty() ||
           !authors.empty() ||
           !maintainers.empty() ||
           !descriptionHeadline.IsNULLOrEmpty() ||
           !descriptionDetails.IsNULLOrEmpty() ||
           !license.IsNULLOrEmpty();
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::Merge( const CModuleMetaData& moduleMetaDataToMergeIn ,
                        bool onConflictOriginalInfoStays               )
{GUCEF_TRACE;

    if ( !moduleMetaDataToMergeIn.lastEditBy.IsNULLOrEmpty() )
        lastEditBy = moduleMetaDataToMergeIn.lastEditBy;
    
    MergeStringSet( authors, moduleMetaDataToMergeIn.authors, false );

    MergeStringSet( maintainers, moduleMetaDataToMergeIn.maintainers, false );

    if ( descriptionHeadline.IsNULLOrEmpty() )
        descriptionHeadline = moduleMetaDataToMergeIn.descriptionHeadline;
    if ( descriptionDetails.IsNULLOrEmpty() )
        descriptionDetails = moduleMetaDataToMergeIn.descriptionDetails;
    if ( license.IsNULLOrEmpty() )
        license = moduleMetaDataToMergeIn.license;

    if ( ( !m_hasSemVer || !onConflictOriginalInfoStays ) && moduleMetaDataToMergeIn.m_hasSemVer )
    {
        m_semver = moduleMetaDataToMergeIn.m_semver;
        m_hasSemVer = true;
    }

    return true;
}

/*---------------------------------------------------------------------------*/

CModuleMetaData&
CModuleMetaData::operator=( const CModuleMetaData& src ) 
{GUCEF_TRACE;

    if ( &src != this )
    {
        lastEditBy = src.lastEditBy;
        authors = src.authors;
        maintainers = src.maintainers;        
        descriptionHeadline = src.descriptionHeadline;
        descriptionDetails = src.descriptionDetails;
        license = src.license;
        m_semver = src.m_semver;
        m_hasSemVer = src.m_hasSemVer;
    }
    return *this;
}

/*---------------------------------------------------------------------------*/

bool 
CModuleMetaData::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    if ( !lastEditBy.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "LastEditBy", lastEditBy, GUCEF_DATATYPE_STRING );
    }
    if ( !descriptionHeadline.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "DescriptionHeadline", descriptionHeadline, GUCEF_DATATYPE_STRING );
    }
    if ( !descriptionDetails.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "DescriptionDetails", descriptionDetails, GUCEF_DATATYPE_STRING );
    }
    if ( !license.IsNULLOrEmpty() )
    {
        domRootNode.AddChildWithValue( "License", license, GUCEF_DATATYPE_STRING );
    }
    if ( m_hasSemVer )
    {
        domRootNode.AddChildWithValue( "SemVer", m_semver.ToString(), GUCEF_DATATYPE_STRING );
    }

    domRootNode.AddAllValuesAsChildrenOfChild< CORE::CString::StringSet >( "Authors", authors, GUCEF_DATATYPE_ARRAY, "Author" );
    domRootNode.AddAllValuesAsChildrenOfChild< CORE::CString::StringSet >( "Maintainers", maintainers, GUCEF_DATATYPE_ARRAY, "Maintainer" );

    return true;
}

/*---------------------------------------------------------------------------*/

bool
CModuleMetaData::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* metaDataNode = domRootNode.Find( "ModuleMetaData" );
    if ( GUCEF_NULL != metaDataNode )
    {
        license = metaDataNode->GetAttributeValueOrChildValueByName( "License", license, false ).AsString( license, true );
        descriptionHeadline = metaDataNode->GetAttributeValueOrChildValueByName( "DescriptionHeadline", descriptionHeadline, false ).AsString( descriptionHeadline, true );
        descriptionDetails = metaDataNode->GetAttributeValueOrChildValueByName( "DescriptionDetails", descriptionDetails, false ).AsString( descriptionDetails, true );
        m_semver.FromString( metaDataNode->GetAttributeValueOrChildValueByName( "SemVer", m_semver.ToString(), false ).AsString( m_semver.ToString(), true ) );
        m_hasSemVer = metaDataNode->GetAttributeValueOrChildValueByName( "HasSemVer", !m_semver.IsAllZero() ).AsBool( !m_semver.IsAllZero(), true );

        metaDataNode->GetValuesOfChildByName( "Authors", authors );
        metaDataNode->GetValuesOfChildByName( "Maintainers", maintainers );
    }
    return true;
}

/*---------------------------------------------------------------------------*/

CORE::CICloneable* 
CModuleMetaData::Clone( void ) const 
{GUCEF_TRACE;

    return new CModuleMetaData( *this );
}

/*---------------------------------------------------------------------------*/

const CORE::CString& 
CModuleMetaData::GetClassTypeName( void ) const 
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
