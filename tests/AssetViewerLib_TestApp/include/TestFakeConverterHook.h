/*
 *  AssetViewerLib_TestApp: tests of the AssetViewerLib module
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

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#include "gucefASSETSTORE_CIAssetConverterHook.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */

namespace SOURCESTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A converter hook for the tests of the sources page, in the place of the converter pipeline (whose plugins the viewer
 *  tests do not load): it "converts" every model by copying its file to "<name>.gltf" below the converted folder of the
 *  request, so that the converted file of the catalogue opens in the views, or it refuses.
 */
class CFakeConverterHook : public GUCEF::ASSETSTORE::CIAssetConverterHook
{
    public:

    CFakeConverterHook( void );
    virtual ~CFakeConverterHook();

    void SetRefuse( bool refuse );
    GUCEF::UInt32 GetCallCount( void ) const;

    virtual GUCEF::CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::CORE::CString GetProfileName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CanConvert( const GUCEF::ASSETSTORE::CAssetRecord& asset ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual GUCEF::ASSETSTORE::TAssetStoreStatus ConvertPack( const GUCEF::ASSETSTORE::CAssetConversionRequest& request ,
                                                              GUCEF::ASSETSTORE::TAssetRecordVector& assets             ,
                                                              GUCEF::ASSETSTORE::CAssetConversionOutcome& outcome       ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConversionStale( const GUCEF::ASSETSTORE::CAssetConversionRecord& recorded, GUCEF::CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool m_refuse;
    GUCEF::UInt32 m_calls;
};

}; /* namespace SOURCESTEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTFAKECONVERTERHOOK_H ? */
