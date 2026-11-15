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

#ifndef TESTTOOLSCONVERTERHOOK_H
#define TESTTOOLSCONVERTERHOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef TESTFAKECONVERTERHOOK_H
#include "TestFakeConverterHook.h"
#define TESTFAKECONVERTERHOOK_H
#endif /* TESTFAKECONVERTERHOOK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The converter of the tests of the asset tool set on the in-process service: every pipeline operation but
 *  render_sheet, answered with small documents. licence_check: packs whose id starts with "nml" withhold content,
 *  with "refused" refuse the conversion; convert_pack steps through its assets with progress and stops when asked to
 *  cancel; manifest_edit previews with a change token and applies only with it; read_resource serves the URI it is
 *  given; every document may carry the text of SetEcho() (for the redaction sweep).
 */
class CToolsTestConverterHook : public CTestConverterHook
{
    public:

    CToolsTestConverterHook( void );

    virtual ~CToolsTestConverterHook();

    void SetJobSteps( GUCEF::UInt32 steps, GUCEF::UInt32 stepDelayMs );

    /** A text the hook puts in its documents and messages */
    void SetEcho( const GUCEF::CORE::CString& text );

    GUCEF::UInt32 GetOperationCount( const GUCEF::CORE::CString& operation ) const;

    virtual bool SupportsPipelineOperation( const GUCEF::CORE::CString& operation ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RunPipelineOperation( const GUCEF::ASSETSTORE::CAssetPipelineRequest& request ,
                                                                       GUCEF::ASSETSTORE::CIAssetConversionObserver* observer ,
                                                                       GUCEF::ASSETSTORE::CAssetServiceDocument& result       ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    mutable GUCEF::MT::CMutex m_hookLock;
    GUCEF::UInt32 m_steps;
    GUCEF::UInt32 m_stepDelayMs;
    GUCEF::CORE::CString m_echo;
    GUCEF::UInt32 m_tokens;
    GUCEF::CORE::CString::StringSet m_openTokens;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::UInt32 > m_counts;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTTOOLSCONVERTERHOOK_H ? */
