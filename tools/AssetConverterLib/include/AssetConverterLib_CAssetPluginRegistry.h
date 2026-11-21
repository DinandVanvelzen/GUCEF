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

#ifndef ASSETCONVERTERLIB_CASSETKINDREGISTRY_H
#include "AssetConverterLib_CAssetKindRegistry.h"
#define ASSETCONVERTERLIB_CASSETKINDREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETKINDREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#include "AssetConverterLib_CAssetPackListing.h"
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H
#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#include "AssetConverterLib_CAssetPackProfile.h"
#define ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPROFILE_H ? */


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

    /**
     *  Another name under which the cooker of a target is found (the cooked area that its output goes to is named differently
     *  from the cooker: "gucef-ogre3d" for "ogre"). The alias is resolved by TryGetCooker() and ResolveCookerName(); a name
     *  that a cooker is registered under is never an alias. False when the alias is empty or is a registered target name.
     */
    bool RegisterCookerAlias( const CORE::CString& alias, const CORE::CString& targetName );
    bool UnregisterCookerAlias( const CORE::CString& alias );

    /** The target name for a name that is a target name or an alias; the name as it is when it is neither */
    CORE::CString ResolveCookerName( const CORE::CString& name ) const;
    bool UnregisterPostProcessor( const CORE::CString& name );
    bool UnregisterValidator( const CORE::CString& name );

    bool TryGetImporter( const CORE::CString& name, TAssetImporterPtr& importer ) const;
    bool TryGetExporter( const CORE::CString& name, TAssetExporterPtr& exporter ) const;
    bool TryGetCooker( const CORE::CString& targetName, TAssetCookerPtr& cooker ) const;
    bool TryGetPostProcessor( const CORE::CString& name, TAssetPostProcessorPtr& processor ) const;
    bool TryGetValidator( const CORE::CString& name, TAssetValidatorPtr& validator ) const;

    /** The importers by name (the order in which 'auto' asks them) */
    void GetImporters( GUCEF::vector< TAssetImporterPtr >& importers ) const;

    /** The table of file kinds: the built-in extensions and the declarations of the importers and exporters that are registered */
    CAssetKindRegistry& GetKinds( void );
    const CAssetKindRegistry& GetKinds( void ) const;

    /**
     *  Asks the importers, in the order of their names, whether the files are a pack they know; the most sure recogniser fills the
     *  profile (the first of equal ones). False when none recognises it, with the profile cleared.
     */
    bool RecognisePack( const CAssetPackListing& listing, CAssetPackProfile& profile ) const;

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
    GUCEF::map< CORE::CString, CORE::CString > m_cookerAliases;     /**< lower case alias to target name */
    mutable MT::CMutex m_aliasLock;
    TAssetPostProcessorRegistry m_postProcessors;
    TAssetValidatorRegistry m_validators;
    CAssetKindRegistry m_kinds;
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
