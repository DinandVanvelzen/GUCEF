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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETSHAREDECISION_H
#include "gucefASSETSTORE_CAssetShareDecision.h"
#define GUCEF_ASSETSTORE_CASSETSHAREDECISION_H
#endif /* GUCEF_ASSETSTORE_CASSETSHAREDECISION_H ? */

#include "gucefASSETSTORE_CAssetShareWarmJob.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetShareWarmJob::CAssetShareWarmJob( CAssetShareDecision& decision, const CORE::CString& servicePath )
    : DRN::CIDRNResourceIoJob()
    , m_decision( &decision )
    , m_servicePath( servicePath )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetShareWarmJob::~CAssetShareWarmJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetShareWarmJob::Execute( void )
{GUCEF_TRACE;

    m_decision->Warm( m_servicePath );
    m_decision->WarmDone( m_servicePath );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
