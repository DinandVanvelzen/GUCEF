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

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CIASSETIMPORTER_H
#include "AssetConverterLib_CIAssetImporter.h"
#define ASSETCONVERTERLIB_CIASSETIMPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETIMPORTER_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETEXPORTER_H
#include "AssetConverterLib_CIAssetExporter.h"
#define ASSETCONVERTERLIB_CIASSETEXPORTER_H
#endif /* ASSETCONVERTERLIB_CIASSETEXPORTER_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETCOOKER_H
#include "AssetConverterLib_CIAssetCooker.h"
#define ASSETCONVERTERLIB_CIASSETCOOKER_H
#endif /* ASSETCONVERTERLIB_CIASSETCOOKER_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#include "AssetConverterLib_CIAssetPostProcessor.h"
#define ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H
#endif /* ASSETCONVERTERLIB_CIASSETPOSTPROCESSOR_H ? */

#ifndef ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#include "AssetConverterLib_CIAssetValidator.h"
#define ASSETCONVERTERLIB_CIASSETVALIDATOR_H
#endif /* ASSETCONVERTERLIB_CIASSETVALIDATOR_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAssetConverterGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  The plugins of the converter by kind and name: importers and exporters by GetName(), cookers by GetTargetName(),
 *  post-processors and validators by GetName(). Plugins register here when they load and unregister when they unload;
 *  code (and tests) can register directly. The built-in post-processors and validators of the library are registered
 *  as built-in entries, which stay when the plugins are unloaded. Reach it through CAssetConverterGlobal.
 *
 *  Every kind is a CTGlobalONRegistry, so observers get its registration events and the entries of plugins go when
 *  the application shuts down. Lookups ignore letter case. Thread-safe.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPluginRegistry
{
    public:

    TAssetImporterRegistry& GetImporters( void );
    TAssetExporterRegistry& GetExporters( void );
    TAssetCookerRegistry& GetCookers( void );
    TAssetPostProcessorRegistry& GetPostProcessors( void );
    TAssetValidatorRegistry& GetValidators( void );

    /** Registers under the name of the plugin; false when the name is taken */
    bool RegisterImporter( const TAssetImporterPtr& importer );
    bool RegisterExporter( const TAssetExporterPtr& exporter );
    bool RegisterCooker( const TAssetCookerPtr& cooker );
    bool RegisterPostProcessor( const TAssetPostProcessorPtr& processor );
    bool RegisterValidator( const TAssetValidatorPtr& validator );

    bool UnregisterImporter( const CORE::CString& name );
    bool UnregisterExporter( const CORE::CString& name );
    bool UnregisterCooker( const CORE::CString& targetName );
    bool UnregisterPostProcessor( const CORE::CString& name );
    bool UnregisterValidator( const CORE::CString& name );

    bool TryGetImporter( const CORE::CString& name, TAssetImporterPtr& importer ) const;
    bool TryGetExporter( const CORE::CString& name, TAssetExporterPtr& exporter ) const;
    bool TryGetCooker( const CORE::CString& targetName, TAssetCookerPtr& cooker ) const;
    bool TryGetPostProcessor( const CORE::CString& name, TAssetPostProcessorPtr& processor ) const;
    bool TryGetValidator( const CORE::CString& name, TAssetValidatorPtr& validator ) const;

    /** The importers by name (the order in which 'auto' asks them) */
    void GetImporters( GUCEF::vector< TAssetImporterPtr >& importers ) const;

    private:
    friend class CAssetConverterGlobal;

    CAssetPluginRegistry( void );

    ~CAssetPluginRegistry();

    /** Registers the post-processors and validators of the library as built-in entries */
    void RegisterBuiltIns( void );

    CAssetPluginRegistry( const CAssetPluginRegistry& src );            /**< not implemented */
    CAssetPluginRegistry& operator=( const CAssetPluginRegistry& src ); /**< not implemented */

    private:

    TAssetImporterRegistry m_importers;
    TAssetExporterRegistry m_exporters;
    TAssetCookerRegistry m_cookers;
    TAssetPostProcessorRegistry m_postProcessors;
    TAssetValidatorRegistry m_validators;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */
