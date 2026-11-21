/*
 *  AssetConverterLib: the analysis level of the pack index: statistics read from the headers of the files
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXANALYSER_H
#define ASSETCONVERTERLIB_CPACKINDEXANALYSER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#include "AssetConverterLib_CPackRedoRequest.h"
#define ASSETCONVERTERLIB_CPACKREDOREQUEST_H
#endif /* ASSETCONVERTERLIB_CPACKREDOREQUEST_H ? */

#ifndef ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H
#include "AssetConverterLib_CIPackIndexObserver.h"
#define ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H
#endif /* ASSETCONVERTERLIB_CIPACKINDEXOBSERVER_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Fills the statistics of the records that the headers of the files give: the width, height, channels and bit depth of the pictures
 *  (texture and image kinds), without decoding them. Each record keeps the key of the step (CPackWorkGate::MakeAnalyseKey()) in
 *  status.steps.analyse, so a second run does nothing for a record whose file did not change, a changed file is analysed again alone,
 *  and a record that failed stays failed until the analyser version changes or the person asks to retry. The model statistics, the
 *  bounds and the animation lengths come from the sidecars of the conversion (CPackIndexUpdater::RefreshOutputs()). The analysis
 *  runs on the thread that calls it; the workbench calls it from its background job.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexAnalyser
{
    public:

    static const UInt32 Version = 1;
    static const CORE::CString AnalyserId;          /**< "header-probe" */

    CPackIndexAnalyser( void );

    /**
     *  Analyses the records of the document that need it. 'packVfsRoot' is the folder the paths of the records are relative to; with an
     *  empty root nothing can be read and every record that needs work is marked failed. False when cancelled (the records done so far keep
     *  their results). 'changed' says whether any record was touched.
     */
    bool Analyse( CPackIndexDocument& document, const CORE::CString& packVfsRoot, const CPackRedoRequest& request, CIPackIndexObserver* observer, bool& changed );

    UInt32 GetAnalysedCount( void ) const;      /**< records that were analysed (a counter for the tests: the work done) */
    UInt32 GetReusedCount( void ) const;        /**< records whose stored result was valid */
    UInt32 GetFailedCount( void ) const;
    UInt32 GetProbeCount( void ) const;         /**< files whose header was read */

    private:

    UInt32 m_analysed;
    UInt32 m_reused;
    UInt32 m_failed;
    UInt32 m_probes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXANALYSER_H ? */

