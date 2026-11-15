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

#ifndef TESTFAKECONVERTERHOOK_H
#define TESTFAKECONVERTERHOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#include "gucefASSETSTORE_CIAssetConverterHook.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A converter hook for the tests of the store, in the place of the converter pipeline: it "converts" every model of a
 *  pack into a small file "<name>.gltf" below the converted folder of the request, writes a report there, and
 *  behaves as the mode says: convert, refuse, fail, not run (an IO error), or wait until it is asked to cancel. The
 *  "version" is what the record of a conversion keeps ("importer:test/<version>"); another version makes the
 *  conversions it made earlier stale.
 */
class CTestConverterHook : public GUCEF::ASSETSTORE::CIAssetConverterHook
{
    public:

    enum EMode
    {
        MODE_CONVERT = 0    ,
        MODE_REFUSE         ,
        MODE_FAIL           ,
        MODE_IO_ERROR       ,
        MODE_WAIT_FOR_CANCEL
    };

    CTestConverterHook( void );
    virtual ~CTestConverterHook();

    void SetMode( EMode mode );
    void SetVersion( GUCEF::UInt32 version );
    GUCEF::UInt32 GetCallCount( void ) const;
    GUCEF::UInt32 GetStartedWaitCount( void ) const;

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetProfileName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CanConvert( const GUCEF::ASSETSTORE::CAssetRecord& asset ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ConvertPack( const GUCEF::ASSETSTORE::CAssetConversionRequest& request ,
                                                              GUCEF::ASSETSTORE::TAssetRecordVector& assets             ,
                                                              GUCEF::ASSETSTORE::CAssetConversionOutcome& outcome       ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConversionStale( const GUCEF::ASSETSTORE::CAssetConversionRecord& recorded, GUCEF::CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    EMode m_mode;
    GUCEF::UInt32 m_version;
    GUCEF::UInt32 m_calls;
    GUCEF::UInt32 m_waits;
    mutable GUCEF::MT::CMutex m_lock;
};

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKECONVERTERHOOK_H ? */
