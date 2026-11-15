/*
 *  AssetConverterLib: the global systems of the converter library
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

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H
#include "AssetConverterLib_CAssetConversionTaskConsumer.h"
#define ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H ? */


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

class CModule;
class CAssetPluginRegistry;

/*-------------------------------------------------------------------------*/

/**
 *  The global systems of the converter library, created on first use after the globals of the modules it uses
 *  (gucefCORE, gucefVFS, gucefIMAGE, gucefMATH, gucefGEOMETRY), and destroyed when the library unloads. It owns the
 *  plugin registry with the built-in post-processors and validators, and registers the task of the asset conversion
 *  with the task manager. The library is listed for plugin link-back, so importer, cooker and other converter
 *  plugins find it.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetConverterGlobal
{
    public:

    /** The global, created on first use; null after the library unloaded */
    static CAssetConverterGlobal* Instance( void );

    static CAssetConverterGlobal* InstanceIfCreated( void );

    CAssetPluginRegistry& GetPluginRegistry( void );

    /** The version of the converter library: raised whenever its output for the same input changes; part of every cache key */
    static UInt32 GetConverterVersion( void );

    private:
    friend class CModule;

    static void Deinstance( void );

    static void AllowCreation( void );

    CAssetConverterGlobal( void );

    ~CAssetConverterGlobal();

    void Initialize( void );

    CAssetConverterGlobal( const CAssetConverterGlobal& src );            /**< not implemented */
    CAssetConverterGlobal& operator=( const CAssetConverterGlobal& src ); /**< not implemented */

    private:

    CAssetPluginRegistry* m_pluginRegistry;
    TAssetConversionTaskConsumerFactory* m_taskFactory;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */
