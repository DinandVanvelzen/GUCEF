/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H
#define GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */
#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#include "gucefASSETSTORE_CAssetRecord.h"
#define GUCEF_ASSETSTORE_CASSETRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Fills the typed properties of assets from their content during the ingest (section 7.3): textures through
 *  gucefIMAGE (the built in CAssetTextureAnalyzer), models through the importers of the converter and audio through
 *  gucefAUDIO, those from plugins that register with CAssetContentAnalyzerRegistry.
 *
 *  Analyze() is called on the threads of the analysis tasks of the ingest, at once for several assets: an analyser is
 *  thread safe. It reads the file at the VFS path and sets the properties of the kind of the asset (setting their
 *  isSet), and may refine the kind and the role. OK when it analysed the asset, UNSUPPORTED when it cannot read this
 *  file after all (the next analyser is tried); any other status fails the analysis step of the ingest, which can be
 *  resumed: assets that were analysed are not analysed again.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetContentAnalyzer
{
    public:

    CIAssetContentAnalyzer( void );
    virtual ~CIAssetContentAnalyzer();

    /** The name the analyser is registered with: "texture", "model", "audio", ... */
    virtual CORE::CString GetName( void ) const = 0;

    virtual CORE::CString GetVersion( void ) const = 0;

    /** Whether the analyser wants to look at the asset: by its kind and format */
    virtual bool CanAnalyze( const CAssetRecord& asset ) const = 0;

    virtual TAssetStoreStatus Analyze( const CORE::CString& vfsPath, CAssetRecord& asset ) = 0;

    private:

    CIAssetContentAnalyzer( const CIAssetContentAnalyzer& src );              /**< not implemented */
    CIAssetContentAnalyzer& operator=( const CIAssetContentAnalyzer& src );   /**< not implemented */
};

typedef CORE::CTSharedPtr< CIAssetContentAnalyzer, MT::CMutex > TAssetContentAnalyzerPtr;
typedef GUCEF::vector< TAssetContentAnalyzerPtr > TAssetContentAnalyzerPtrVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H ? */
