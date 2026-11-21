/*
 *  AssetConverterLib: the importers, exporters, cookers, post-processors and validators by name
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef ASSETCONVERTERLIB_CNAMEHINTSPOSTPROCESSOR_H
#include "AssetConverterLib_CNameHintsPostProcessor.h"
#define ASSETCONVERTERLIB_CNAMEHINTSPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CNAMEHINTSPOSTPROCESSOR_H ? */

#ifndef ASSETCONVERTERLIB_CGENERATETANGENTSPOSTPROCESSOR_H
#include "AssetConverterLib_CGenerateTangentsPostProcessor.h"
#define ASSETCONVERTERLIB_CGENERATETANGENTSPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CGENERATETANGENTSPOSTPROCESSOR_H ? */

#ifndef ASSETCONVERTERLIB_CPRUNEEMPTYNODESPOSTPROCESSOR_H
#include "AssetConverterLib_CPruneEmptyNodesPostProcessor.h"
#define ASSETCONVERTERLIB_CPRUNEEMPTYNODESPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CPRUNEEMPTYNODESPOSTPROCESSOR_H ? */

#ifndef ASSETCONVERTERLIB_CMERGEIDENTICALMATERIALSPOSTPROCESSOR_H
#include "AssetConverterLib_CMergeIdenticalMaterialsPostProcessor.h"
#define ASSETCONVERTERLIB_CMERGEIDENTICALMATERIALSPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CMERGEIDENTICALMATERIALSPOSTPROCESSOR_H ? */

#ifndef ASSETCONVERTERLIB_CTEXTURESIZEVALIDATOR_H
#include "AssetConverterLib_CTextureSizeValidator.h"
#define ASSETCONVERTERLIB_CTEXTURESIZEVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CTEXTURESIZEVALIDATOR_H ? */

#ifndef ASSETCONVERTERLIB_CMISSINGTEXTURESVALIDATOR_H
#include "AssetConverterLib_CMissingTexturesValidator.h"
#define ASSETCONVERTERLIB_CMISSINGTEXTURESVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CMISSINGTEXTURESVALIDATOR_H ? */

#ifndef ASSETCONVERTERLIB_CBONELIMITVALIDATOR_H
#include "AssetConverterLib_CBoneLimitValidator.h"
#include "AssetConverterLib_CLayerPenetrationValidator.h"
#define ASSETCONVERTERLIB_CBONELIMITVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CBONELIMITVALIDATOR_H ? */

#ifndef ASSETCONVERTERLIB_CTRIANGLEBUDGETVALIDATOR_H
#include "AssetConverterLib_CTriangleBudgetValidator.h"
#define ASSETCONVERTERLIB_CTRIANGLEBUDGETVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CTRIANGLEBUDGETVALIDATOR_H ? */

#ifndef ASSETCONVERTERLIB_CNAMINGVALIDATOR_H
#include "AssetConverterLib_CNamingValidator.h"
#define ASSETCONVERTERLIB_CNAMINGVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CNAMINGVALIDATOR_H ? */

#include "AssetConverterLib_CAssetPluginRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

template < class TRegistry, class TPtr >
bool
TryGetFrom( const TRegistry& registry, const CORE::CString& name, TPtr& result )
{
    typename TRegistry::TRegisteredObjPtr found;
    if ( !registry.TryLookup( name, found, false ) || found.IsNULL() )
        return false;
    result = TPtr( found );
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetPluginRegistry::CAssetPluginRegistry( void )
    : m_importers()
    , m_exporters()
    , m_cookers()
    , m_postProcessors()
    , m_validators()
    , m_kinds()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetPluginRegistry::~CAssetPluginRegistry()
{GUCEF_TRACE;

    m_validators.UnregisterAll();
    m_postProcessors.UnregisterAll();
    m_cookers.UnregisterAll();
    m_exporters.UnregisterAll();
    m_importers.UnregisterAll();
}

/*-------------------------------------------------------------------------*/

void
CAssetPluginRegistry::RegisterBuiltIns( void )
{GUCEF_TRACE;

    TAssetPostProcessorPtr processors[] = { TAssetPostProcessorPtr( GUCEF_NEW CNameHintsPostProcessor() ),
                                            TAssetPostProcessorPtr( GUCEF_NEW CGenerateTangentsPostProcessor() ),
                                            TAssetPostProcessorPtr( GUCEF_NEW CPruneEmptyNodesPostProcessor() ),
                                            TAssetPostProcessorPtr( GUCEF_NEW CMergeIdenticalMaterialsPostProcessor() ) };
    for ( UInt32 i=0; i<sizeof( processors ) / sizeof( processors[ 0 ] ); ++i )
        m_postProcessors.TryRegisterBuiltIn( processors[ i ]->GetName(), processors[ i ] );

    TAssetValidatorPtr validators[] = { TAssetValidatorPtr( GUCEF_NEW CTextureSizeValidator() ),
                                        TAssetValidatorPtr( GUCEF_NEW CMissingTexturesValidator() ),
                                        TAssetValidatorPtr( GUCEF_NEW CBoneLimitValidator() ),
                                        TAssetValidatorPtr( GUCEF_NEW CTriangleBudgetValidator() ),
                                        TAssetValidatorPtr( GUCEF_NEW CNamingValidator() ),
                                        TAssetValidatorPtr( GUCEF_NEW CLayerPenetrationValidator() ) };
    for ( UInt32 i=0; i<sizeof( validators ) / sizeof( validators[ 0 ] ); ++i )
        m_validators.TryRegisterBuiltIn( validators[ i ]->GetName(), validators[ i ] );
}

/*-------------------------------------------------------------------------*/

TAssetImporterRegistry& CAssetPluginRegistry::GetImporters( void ) { return m_importers; }
TAssetExporterRegistry& CAssetPluginRegistry::GetExporters( void ) { return m_exporters; }
TAssetCookerRegistry& CAssetPluginRegistry::GetCookers( void ) { return m_cookers; }
TAssetPostProcessorRegistry& CAssetPluginRegistry::GetPostProcessors( void ) { return m_postProcessors; }
TAssetValidatorRegistry& CAssetPluginRegistry::GetValidators( void ) { return m_validators; }

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::RegisterImporter( const TAssetImporterPtr& importer )
{GUCEF_TRACE;

    if ( importer.IsNULL() || !m_importers.TryRegister( importer->GetName(), importer ) )
        return false;
    TAssetFileKindVector kinds;
    importer->GetFileKinds( kinds );
    m_kinds.RegisterFileKinds( kinds, "importer:" + importer->GetName().Lowercase() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::RegisterExporter( const TAssetExporterPtr& exporter )
{GUCEF_TRACE;

    if ( exporter.IsNULL() || !m_exporters.TryRegister( exporter->GetName(), exporter ) )
        return false;
    TAssetFileKindVector kinds;
    exporter->GetFileKinds( kinds );
    m_kinds.RegisterFileKinds( kinds, "exporter:" + exporter->GetName().Lowercase() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::RegisterCooker( const TAssetCookerPtr& cooker )
{GUCEF_TRACE;

    return !cooker.IsNULL() && m_cookers.TryRegister( cooker->GetTargetName(), cooker );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::RegisterPostProcessor( const TAssetPostProcessorPtr& processor )
{GUCEF_TRACE;

    return !processor.IsNULL() && m_postProcessors.TryRegister( processor->GetName(), processor );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::RegisterValidator( const TAssetValidatorPtr& validator )
{GUCEF_TRACE;

    return !validator.IsNULL() && m_validators.TryRegister( validator->GetName(), validator );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::UnregisterImporter( const CORE::CString& name )
{GUCEF_TRACE;

    m_kinds.UnregisterSource( "importer:" + name.Lowercase() );
    return m_importers.TryUnregister( name );
}

bool
CAssetPluginRegistry::UnregisterExporter( const CORE::CString& name )
{GUCEF_TRACE;

    m_kinds.UnregisterSource( "exporter:" + name.Lowercase() );
    return m_exporters.TryUnregister( name );
}

bool CAssetPluginRegistry::UnregisterCooker( const CORE::CString& targetName ) { return m_cookers.TryUnregister( targetName ); }

bool
CAssetPluginRegistry::RegisterCookerAlias( const CORE::CString& alias, const CORE::CString& targetName )
{GUCEF_TRACE;

    if ( alias.IsNULLOrEmpty() || targetName.IsNULLOrEmpty() )
        return false;
    TAssetCookerPtr existing;
    if ( TryGetFrom( m_cookers, alias, existing ) )
        return false;
    MT::CScopeMutex lock( m_aliasLock );
    m_cookerAliases[ alias.Lowercase() ] = targetName;
    return true;
}

bool
CAssetPluginRegistry::UnregisterCookerAlias( const CORE::CString& alias )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_aliasLock );
    return 0 != m_cookerAliases.erase( alias.Lowercase() );
}

CORE::CString
CAssetPluginRegistry::ResolveCookerName( const CORE::CString& name ) const
{GUCEF_TRACE;

    TAssetCookerPtr existing;
    if ( TryGetFrom( m_cookers, name, existing ) )
        return name;
    MT::CScopeMutex lock( m_aliasLock );
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator i = m_cookerAliases.find( name.Lowercase() );
    return i == m_cookerAliases.end() ? name : (*i).second;
}
bool CAssetPluginRegistry::UnregisterPostProcessor( const CORE::CString& name ) { return m_postProcessors.TryUnregister( name ); }
bool CAssetPluginRegistry::UnregisterValidator( const CORE::CString& name ) { return m_validators.TryUnregister( name ); }

/*-------------------------------------------------------------------------*/

bool CAssetPluginRegistry::TryGetImporter( const CORE::CString& name, TAssetImporterPtr& importer ) const { return TryGetFrom( m_importers, name, importer ); }
bool CAssetPluginRegistry::TryGetExporter( const CORE::CString& name, TAssetExporterPtr& exporter ) const { return TryGetFrom( m_exporters, name, exporter ); }

bool
CAssetPluginRegistry::TryGetCooker( const CORE::CString& targetName, TAssetCookerPtr& cooker ) const
{GUCEF_TRACE;

    return TryGetFrom( m_cookers, targetName, cooker ) || TryGetFrom( m_cookers, ResolveCookerName( targetName ), cooker );
}

bool CAssetPluginRegistry::TryGetPostProcessor( const CORE::CString& name, TAssetPostProcessorPtr& processor ) const { return TryGetFrom( m_postProcessors, name, processor ); }
bool CAssetPluginRegistry::TryGetValidator( const CORE::CString& name, TAssetValidatorPtr& validator ) const { return TryGetFrom( m_validators, name, validator ); }

/*-------------------------------------------------------------------------*/

CAssetKindRegistry&
CAssetPluginRegistry::GetKinds( void )
{GUCEF_TRACE;

    return m_kinds;
}

/*-------------------------------------------------------------------------*/

const CAssetKindRegistry&
CAssetPluginRegistry::GetKinds( void ) const
{GUCEF_TRACE;

    return m_kinds;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::RecognisePack( const CAssetPackListing& listing, CAssetPackProfile& profile ) const
{GUCEF_TRACE;

    profile.Clear();
    GUCEF::vector< TAssetImporterPtr > importers;
    GetImporters( importers );
    for ( size_t i=0; i<importers.size(); ++i )
    {
        CAssetPackProfile candidate;
        if ( importers[ i ]->RecognisePack( listing, candidate ) && candidate.IsRecognised() )
        {
            if ( candidate.recognisedBy.IsNULLOrEmpty() )
                candidate.recognisedBy = importers[ i ]->GetName();
            if ( !profile.IsRecognised() || candidate.confidence > profile.confidence )
                profile = candidate;
        }
    }
    return profile.IsRecognised();
}

/*-------------------------------------------------------------------------*/

void
CAssetPluginRegistry::GetImporters( GUCEF::vector< TAssetImporterPtr >& importers ) const
{GUCEF_TRACE;

    importers.clear();
    CORE::CString::StringVector names;
    m_importers.GetRegisteredObjNames( names );
    for ( size_t i=0; i<names.size(); ++i )
    {
        TAssetImporterPtr importer;
        if ( TryGetImporter( names[ i ], importer ) )
            importers.push_back( importer );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
