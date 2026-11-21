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

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H
#define GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROBERESULT_H
#include "gucefASSETSTORE_CAssetStoreProbeResult.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPROBERESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROBERESULT_H ? */

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
 *  The optional "test connection" of a store: a check of the shape of what the store answers (is the sign in good, does the
 *  library listing have the fields the plugin expects), the same check a probe tool of the store plugin runs. A provider offers
 *  one with CIAssetStoreProvider::GetProbe(); the settings of an application show a Test connection action for the stores that do.
 *
 *  A probe only reports shapes: its result holds no token, cookie, account id or other value of the store, and nothing of that is
 *  logged. Run() works on the calling thread and may take as long as the store takes to answer; the application runs it with
 *  CIAssetStoreProvider::RunProbeAsync(), on a thread of a pool.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetStoreProbe
{
    public:

    CIAssetStoreProbe( void );

    virtual ~CIAssetStoreProbe();

    /** What the probe checks, in a line for a person, for the tooltip of the action */
    virtual CORE::CString GetDescription( void ) const = 0;

    /** Runs the checks and adds them to the result; false when the probe could not run at all (the result then says why in its summary) */
    virtual bool Run( CAssetStoreProbeResult& result ) = 0;

    private:

    CIAssetStoreProbe( const CIAssetStoreProbe& src );              /**< not implemented */
    CIAssetStoreProbe& operator=( const CIAssetStoreProbe& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H ? */
