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
                                        TAssetValidatorPtr( GUCEF_NEW CNamingValidator() ) };
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

    return !importer.IsNULL() && m_importers.TryRegister( importer->GetName(), importer );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPluginRegistry::RegisterExporter( const TAssetExporterPtr& exporter )
{GUCEF_TRACE;

    return !exporter.IsNULL() && m_exporters.TryRegister( exporter->GetName(), exporter );
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

bool CAssetPluginRegistry::UnregisterImporter( const CORE::CString& name ) { return m_importers.TryUnregister( name ); }
bool CAssetPluginRegistry::UnregisterExporter( const CORE::CString& name ) { return m_exporters.TryUnregister( name ); }
bool CAssetPluginRegistry::UnregisterCooker( const CORE::CString& targetName ) { return m_cookers.TryUnregister( targetName ); }
bool CAssetPluginRegistry::UnregisterPostProcessor( const CORE::CString& name ) { return m_postProcessors.TryUnregister( name ); }
bool CAssetPluginRegistry::UnregisterValidator( const CORE::CString& name ) { return m_validators.TryUnregister( name ); }

/*-------------------------------------------------------------------------*/

bool CAssetPluginRegistry::TryGetImporter( const CORE::CString& name, TAssetImporterPtr& importer ) const { return TryGetFrom( m_importers, name, importer ); }
bool CAssetPluginRegistry::TryGetExporter( const CORE::CString& name, TAssetExporterPtr& exporter ) const { return TryGetFrom( m_exporters, name, exporter ); }
bool CAssetPluginRegistry::TryGetCooker( const CORE::CString& targetName, TAssetCookerPtr& cooker ) const { return TryGetFrom( m_cookers, targetName, cooker ); }
bool CAssetPluginRegistry::TryGetPostProcessor( const CORE::CString& name, TAssetPostProcessorPtr& processor ) const { return TryGetFrom( m_postProcessors, name, processor ); }
bool CAssetPluginRegistry::TryGetValidator( const CORE::CString& name, TAssetValidatorPtr& validator ) const { return TryGetFrom( m_validators, name, validator ); }

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
