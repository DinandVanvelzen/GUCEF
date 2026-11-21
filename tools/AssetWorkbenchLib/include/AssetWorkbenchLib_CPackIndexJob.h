/*
 *  AssetWorkbenchLib: builds or reads the pack index of a pack in the background for the resource view
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

#ifndef ASSETWORKBENCHLIB_CPACKINDEXJOB_H
#define ASSETWORKBENCHLIB_CPACKINDEXJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#include "AssetWorkbenchLib_CIAssetLoadJob.h"
#define ASSETWORKBENCHLIB_CIASSETLOADJOB_H
#endif /* ASSETWORKBENCHLIB_CIASSETLOADJOB_H ? */

#ifndef ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H
#include "AssetConverterLib_CIPackIndexObserver.h"
#define ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H
#endif /* ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H
#include "AssetConverterLib_CPackIndexRunResult.h"
#define ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXRUNRESULT_H ? */

#ifndef ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#include "AssetConverterLib_CPackRedoRequest.h"
#define ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#endif /* ASSETCONVERTERLIB_CPACKREDOREQUEST_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWorkbenchShell;

/**
 *  Gets the pack index of a pack that was opened as a request of the CAssetLoader, so that it runs in the background with the
 *  activity indicator after 300 ms, can be cancelled, and is dropped when another request came meanwhile (the cold start of F18).
 *
 *  Prepare() runs on a thread of a pool: it lists the pack, lets the recognisers say what it is, and asks CPackIndexUpdater for the
 *  index: read when it is there and fresh (the work gate says), built (structure level) and written when it is missing or stale. The
 *  place of the index is the user cache folder of the pack key. Step() runs on the thread of the renderer and hands the result to the
 *  shell, which shows it in the resource view. A folder that no recogniser knows is no pack: the job ends without a result.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CPackIndexJob : public CIAssetLoadJob, public ASSETCONVERTER::CIPackIndexObserver
{
    public:

    CPackIndexJob( CWorkbenchShell& shell, const CORE::CString& vfsFolder, const ASSETCONVERTER::CPackRedoRequest& redo );
    virtual ~CPackIndexJob() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReportsProgress( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Prepare( CAssetLoadControl& control ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EStepResult Step( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Discard( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetProblem( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnProgress( const CORE::CString& phase, UInt32 done, UInt32 total ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetFolder( void ) const;
    const ASSETCONVERTER::CPackIndexDocument& GetDocument( void ) const;
    const ASSETCONVERTER::CPackIndexRunResult& GetResult( void ) const;
    bool IsPack( void ) const;

    private:

    CPackIndexJob( const CPackIndexJob& src );
    CPackIndexJob& operator=( const CPackIndexJob& src );

    private:

    CWorkbenchShell& m_shell;
    CORE::CString m_folder;
    ASSETCONVERTER::CPackRedoRequest m_redo;
    ASSETCONVERTER::CPackIndexDocument m_document;
    ASSETCONVERTER::CPackIndexRunResult m_result;
    CAssetLoadControl* m_control;
    bool m_isPack;
    bool m_ok;
    CORE::CString m_problem;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CPACKINDEXJOB_H ? */

