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

#ifndef GUCEF_DRN_CDRNRESOURCEIOTASKDATA_H
#define GUCEF_DRN_CDRNRESOURCEIOTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#include "gucefDRN_CIDRNResourceIoJob.h"
#define GUCEF_DRN_CIDRNRESOURCEIOJOB_H
#endif /* GUCEF_DRN_CIDRNRESOURCEIOJOB_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The task data of a resource transfer job: a shared reference to the job */
class GUCEF_DRN_EXPORT_CPP CDRNResourceIoTaskData : public CORE::CICloneable
{
    public:

    CIDRNResourceIoJobPtr job;

    CDRNResourceIoTaskData( void );
    explicit CDRNResourceIoTaskData( const CIDRNResourceIoJobPtr& job );
    CDRNResourceIoTaskData( const CDRNResourceIoTaskData& src );
    virtual ~CDRNResourceIoTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDRNResourceIoTaskData& operator=( const CDRNResourceIoTaskData& src );    /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNRESOURCEIOTASKDATA_H ? */
