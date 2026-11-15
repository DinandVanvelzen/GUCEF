/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTMODELANALYZER_H
#define TESTMODELANALYZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H
#include "gucefASSETSTORE_CIAssetContentAnalyzer.h"
#define GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H ? */

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A content analyser of models for the tests, in the place of the analysers of the converter that come later: the
 *  test models are text such as "triangles=1200;vertices=900;rigged=1;animated=0;bones=40;animations=2", and an
 *  animation "length=2.5;fps=30". It can be told to fail the next analyses.
 */
class CTestModelAnalyzer : public GUCEF::ASSETSTORE::CIAssetContentAnalyzer
{
    public:

    CTestModelAnalyzer( void );
    virtual ~CTestModelAnalyzer();

    void FailNext( GUCEF::UInt32 count );
    GUCEF::UInt32 GetAnalysedCount( void ) const;

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CanAnalyze( const GUCEF::ASSETSTORE::CAssetRecord& asset ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus Analyze( const GUCEF::CORE::CString& vfsPath, GUCEF::ASSETSTORE::CAssetRecord& asset ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::UInt32 m_failNext;
    GUCEF::UInt32 m_analysed;
    mutable GUCEF::MT::CMutex m_lock;
};

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTMODELANALYZER_H ? */
