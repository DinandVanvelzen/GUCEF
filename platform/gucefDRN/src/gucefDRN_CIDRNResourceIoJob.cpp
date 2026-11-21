/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include "gucefDRN_CIDRNResourceIoJob.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIDRNResourceIoJob::CIDRNResourceIoJob( void )
    : m_stopRequested( false )
    , m_completionPulseGenerator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIDRNResourceIoJob::~CIDRNResourceIoJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIDRNResourceIoJob::RequestStop( void )
{GUCEF_TRACE;

    m_stopRequested = true;
}

/*-------------------------------------------------------------------------*/

bool
CIDRNResourceIoJob::IsStopRequested( void ) const
{GUCEF_TRACE;

    return m_stopRequested;
}

/*-------------------------------------------------------------------------*/

void
CIDRNResourceIoJob::SetCompletionPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    m_completionPulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CIDRNResourceIoJob::SignalCompleted( void )
{GUCEF_TRACE;

    if ( !m_completionPulseGenerator.IsNULL() )
        m_completionPulseGenerator->RequestImmediatePulseThroughDriver();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
