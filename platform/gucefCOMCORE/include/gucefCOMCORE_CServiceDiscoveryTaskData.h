/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H
#define GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H
#include "gucefCOMCORE_CServiceFinderSettings.h"
#define GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H
#endif /* GUCEF_COMCORE_CSERVICEFINDERSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The task of CServiceFinder::FindOnceAsync(), the description of the work that CServiceDiscoveryTaskConsumer carries
 *  out, and the result data of its future: the settings of the look on the way in, the candidates and whether the
 *  minimum was reached on the way out
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceDiscoveryTaskData : public CORE::CICloneable
{
    public:

    typedef GUCEF::vector< CServiceCandidate > TServiceCandidateVector;

    CServiceFinderSettings settings;
    TServiceCandidateVector candidates;
    bool found;
    CORE::CFutureResultSource source;

    CServiceDiscoveryTaskData( void );
    CServiceDiscoveryTaskData( const CServiceDiscoveryTaskData& src );
    virtual ~CServiceDiscoveryTaskData() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    private:

    CServiceDiscoveryTaskData& operator=( const CServiceDiscoveryTaskData& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSERVICEDISCOVERYTASKDATA_H ? */
