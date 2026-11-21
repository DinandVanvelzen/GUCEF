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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKWORKGATE_H
#include "AssetConverterLib_CPackWorkGate.h"
#define ASSETCONVERTERLIB_CPACKWORKGATE_H
#endif /* ASSETCONVERTERLIB_CPACKWORKGATE_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXIMAGEPROBE_H
#include "AssetConverterLib_CPackIndexImageProbe.h"
#define ASSETCONVERTERLIB_CPACKINDEXIMAGEPROBE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXIMAGEPROBE_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CPackIndexAnalyser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPackIndexAnalyser::AnalyserId = "header-probe";

/*-------------------------------------------------------------------------*/

CPackIndexAnalyser::CPackIndexAnalyser( void )
    : m_analysed( 0 )
    , m_reused( 0 )
    , m_failed( 0 )
    , m_probes( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32 CPackIndexAnalyser::GetAnalysedCount( void ) const { return m_analysed; }
UInt32 CPackIndexAnalyser::GetReusedCount( void ) const { return m_reused; }
UInt32 CPackIndexAnalyser::GetFailedCount( void ) const { return m_failed; }
UInt32 CPackIndexAnalyser::GetProbeCount( void ) const { return m_probes; }

/*-------------------------------------------------------------------------*/

bool
CPackIndexAnalyser::Analyse( CPackIndexDocument& document, const CORE::CString& packVfsRoot, const CPackRedoRequest& request, CIPackIndexObserver* observer, bool& changed )
{GUCEF_TRACE;

    m_analysed = 0;
    m_reused = 0;
    m_failed = 0;
    m_probes = 0;
    changed = false;
    bool anyFailed = false;
    const UInt32 total = (UInt32) document.resources.size();
    for ( size_t i=0; i<document.resources.size(); ++i )
    {
        CPackIndexResource& resource = document.resources[ i ];
        if ( 0 == ( i & 255 ) && GUCEF_NULL != observer )
        {
            if ( observer->IsCancelRequested() )
                return false;
            observer->OnProgress( "analysis", (UInt32) i, total );
        }
        const CORE::CString key = CPackWorkGate::MakeAnalyseKey( AnalyserId, Version, CORE::CString::Empty, resource.digest, resource.size );
        const CPackGateDecision decision = CPackWorkGate::DecideAnalyse( &resource, key, resource.size, request );
        if ( decision.IsFresh() )
        {
            ++m_reused;
            const CPackIndexStepStatus* step = resource.status.FindStep( CPackIndexStatus::StepAnalyse );
            anyFailed = anyFailed || ( GUCEF_NULL != step && CPackIndexStepStatus::Failed == step->state );
            continue;
        }

        CPackIndexStepStatus step;
        step.key = key;
        const bool picture = ( "texture" == resource.kind || "image" == resource.kind ) && CPackIndexImageProbe::IsSupported( resource.format );
        if ( picture )
        {
            ++m_probes;
            UInt32 width = 0, height = 0, channels = 0, bitDepth = 0;
            if ( !packVfsRoot.IsNULLOrEmpty() && CPackIndexImageProbe::Probe( CVfsPaths::Join( packVfsRoot, resource.path ), resource.format, width, height, channels, bitDepth ) )
            {
                resource.SetStat( "width", width );
                resource.SetStat( "height", height );
                if ( channels > 0 )
                    resource.SetStat( "channels", channels );
                if ( bitDepth > 0 )
                    resource.SetStat( "bitDepth", bitDepth );
                step.state = CPackIndexStepStatus::Fresh;
                if ( CPackIndexStatus::Indexed == resource.status.state || CPackIndexStatus::Failed == resource.status.state )
                    resource.status.state = CPackIndexStatus::Analysed;
                ++m_analysed;
            }
            else
            {
                step.state = CPackIndexStepStatus::Failed;
                step.reasons.push_back( "the header of the file cannot be read as " + resource.format );
                resource.status.state = CPackIndexStatus::Failed;
                ++m_failed;
                anyFailed = true;
            }
        }
        else
        {
            // nothing this analyser reads: the record says it looked, so it does not look again
            step.state = CPackIndexStepStatus::Skipped;
        }
        resource.status.SetStep( CPackIndexStatus::StepAnalyse, step );
        changed = true;
    }
    if ( !document.HasLevel( CPackIndexDocument::LevelAnalysis ) )
    {
        document.AddLevel( CPackIndexDocument::LevelAnalysis );
        changed = true;
    }
    document.state = anyFailed ? CPackIndexDocument::StateCompleteWithFailures : CPackIndexDocument::StateComplete;
    if ( anyFailed && document.stateReasons.empty() )
        document.stateReasons.push_back( "some files could not be analysed" );
    if ( !anyFailed )
        document.stateReasons.clear();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
