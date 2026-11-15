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

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#define GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

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
 *  What a converter hook reports to and asks of the job it works for (an ingest, or the conversion of a pack that is
 *  in the store already): whether the job was asked to cancel, and how far the conversion is. Both are called on the
 *  thread that runs the hook, which is the thread of the job.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CIAssetConversionObserver
{
    public:

    CIAssetConversionObserver( void );
    virtual ~CIAssetConversionObserver();

    /** Whether the person or the application asked for the job to stop; the hook ends as soon as it can with ASSETSTORE_STATUS_CANCELLED */
    virtual bool IsCancelRequested( void ) const = 0;

    /** 'done' of 'total' assets are converted; 'assetPath' is the one that just finished (relative to the source folder of the request) */
    virtual void OnConversionProgress( UInt32 done, UInt32 total, const CORE::CString& assetPath ) = 0;

    private:

    CIAssetConversionObserver( const CIAssetConversionObserver& src );              /**< not implemented */
    CIAssetConversionObserver& operator=( const CIAssetConversionObserver& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H ? */
