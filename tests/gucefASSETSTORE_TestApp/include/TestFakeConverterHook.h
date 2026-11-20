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
        MODE_WAIT_FOR_CANCEL,
        MODE_FAIL_MIDWAY        /**< writes the output of one asset, then the storage "refuses" a write */
    };

    /** What the last conversion was asked */
    struct SLastRequest
    {
        bool cookOnly;
        bool noCook;
        bool reconvert;
        bool keepPartialResult;
        GUCEF::CORE::CString::StringVector assetGlobs;
        GUCEF::CORE::CString convertedVfsFolder;
        GUCEF::CORE::CString cookedVfsFolder;
        SLastRequest( void ) : cookOnly( false ), noCook( false ), reconvert( false ), keepPartialResult( false ), assetGlobs(), convertedVfsFolder(), cookedVfsFolder() {}
    };

    /** The models (asset names) that fail in a conversion: no output, the status failed in the report, the run FAILED */
    void SetFailingAssets( const GUCEF::CORE::CString::StringSet& assetNames );

    SLastRequest GetLastRequest( void ) const;

    CTestConverterHook( void );
    virtual ~CTestConverterHook();

    void SetMode( EMode mode );
    void SetVersion( GUCEF::UInt32 version );

    /** What BuildPackIndex() does: not carried out (the default, as a hook without the operation), builds a document, or fails */
    enum EIndexMode
    {
        INDEX_UNSUPPORTED = 0   ,
        INDEX_BUILD             ,
        INDEX_FAIL
    };
    void SetIndexMode( EIndexMode mode );
    GUCEF::UInt32 GetIndexCallCount( void ) const;      /**< calls of BuildPackIndex() */
    GUCEF::UInt32 GetIndexBuildCount( void ) const;     /**< documents it wrote: a call that found the document there builds nothing */
    GUCEF::UInt32 GetCallCount( void ) const;
    GUCEF::UInt32 GetStartedWaitCount( void ) const;

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetProfileName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CanConvert( const GUCEF::ASSETSTORE::CAssetRecord& asset ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ConvertPack( const GUCEF::ASSETSTORE::CAssetConversionRequest& request ,
                                                              GUCEF::ASSETSTORE::TAssetRecordVector& assets             ,
                                                              GUCEF::ASSETSTORE::CAssetConversionOutcome& outcome       ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConversionStale( const GUCEF::ASSETSTORE::CAssetConversionRecord& recorded, GUCEF::CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    /** With the mode INDEX_BUILD: the document of the pack from the files of the source folder, or from the assets of the request when it derives; reused when it is there already */
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus BuildPackIndex( const GUCEF::ASSETSTORE::CAssetPackIndexRequest& request ,
                                                                 GUCEF::ASSETSTORE::CAssetPackIndexOutcome& outcome       ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::ASSETSTORE::TAssetStoreStatus MakeIndexDocument( const GUCEF::ASSETSTORE::CAssetPackIndexRequest& request, GUCEF::ASSETSTORE::CAssetPackIndexOutcome& outcome, bool countBuild );

    EMode m_mode;
    GUCEF::UInt32 m_version;
    GUCEF::UInt32 m_calls;
    GUCEF::UInt32 m_waits;
    EIndexMode m_indexMode;
    GUCEF::UInt32 m_indexCalls;
    GUCEF::UInt32 m_indexBuilds;
    GUCEF::CORE::CString::StringSet m_failingAssets;
    SLastRequest m_lastRequest;
    mutable GUCEF::MT::CMutex m_lock;
};

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKECONVERTERHOOK_H ? */
