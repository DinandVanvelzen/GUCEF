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

#ifndef GUCEF_ASSETSTORE_CASSETSHAREWARMJOB_H
#define GUCEF_ASSETSTORE_CASSETSHAREWARMJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#include "gucefDRN_CIDRNResourceIoJob.h"
#define GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#endif /* GUCEF_DRN_CIDRNRESOURCEIOJOB_H ? */

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

class CAssetShareDecision;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The warming of the restrictions of the pack of a path of a share (CAssetShareDecision::Warm()), as a job of the pool of the resource transfers */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetShareWarmJob : public DRN::CIDRNResourceIoJob
{
    public:

    CAssetShareWarmJob( CAssetShareDecision& decision, const CORE::CString& servicePath );

    virtual ~CAssetShareWarmJob() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Execute( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetShareDecision* m_decision;
    CORE::CString m_servicePath;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSHAREWARMJOB_H ? */
