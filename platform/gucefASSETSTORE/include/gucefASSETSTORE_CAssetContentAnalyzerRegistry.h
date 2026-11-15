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

#ifndef GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#define GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H
#include "gucefASSETSTORE_CIAssetContentAnalyzer.h"
#define GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H ? */
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

class CAssetStoreGlobal;

/**
 *  The content analysers of the ingest pipeline by name (section 7.3), reached through CAssetStoreGlobal. The built
 *  in texture analyser is registered when the module sets up; plugins register theirs when they load and unregister
 *  them before they unload. The analysers of an asset are tried in the order of registration. Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetContentAnalyzerRegistry
{
    public:

    /** False when the name is taken or the analyser is null */
    bool Register( const TAssetContentAnalyzerPtr& analyzer );

    /** False when there is no analyser of the name */
    bool Unregister( const CORE::CString& name );

    bool IsRegistered( const CORE::CString& name ) const;

    /** The analysers that want to look at the asset, in the order of registration */
    void GetAnalyzersFor( const CAssetRecord& asset, TAssetContentAnalyzerPtrVector& analyzers ) const;

    void GetNames( CORE::CString::StringVector& names ) const;

    private:

    friend class CAssetStoreGlobal;

    CAssetContentAnalyzerRegistry( void );
    ~CAssetContentAnalyzerRegistry();

    CAssetContentAnalyzerRegistry( const CAssetContentAnalyzerRegistry& src );              /**< not implemented */
    CAssetContentAnalyzerRegistry& operator=( const CAssetContentAnalyzerRegistry& src );   /**< not implemented */

    private:

    TAssetContentAnalyzerPtrVector m_analyzers;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H ? */
