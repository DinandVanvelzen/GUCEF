/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CJOBLANEDEFINITION_H
#define GUCEF_CORE_CJOBLANEDEFINITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CIJobThrottlePolicy;

/*-------------------------------------------------------------------------*/

/**
 *  A lane: a named serialisation and throttling channel (an external API, a disk, a provider) with a concurrency limit,
 *  an optional throttle policy and modes. At run time a lane also has an availability, a mode and an optional exclusive
 *  hold by an outside party (CJobScheduler::TryHoldLane).
 */
class GUCEF_CORE_PUBLIC_CPP CJobLaneDefinition
{
    public:

    /** Name of the mode the scheduler interprets: no run of the lane is dispatched */
    static const CString DisabledModeName;

    CJobLaneDefinition( void );

    void SetName( const CString& name );
    const CString& GetName( void ) const;

    /** Runs of the lane at once */
    void SetMaxConcurrent( UInt32 maxConcurrent );
    UInt32 GetMaxConcurrent( void ) const;

    /** Optional, not owned; must outlive the scheduler */
    void SetThrottlePolicy( CIJobThrottlePolicy* throttlePolicy );
    CIJobThrottlePolicy* GetThrottlePolicy( void ) const;

    /** Mode names the service gives a meaning to (for example scheduled, onDemand, both, disabled); the scheduler only interprets "disabled" */
    void SetDeclaredModes( const CString::StringVector& declaredModes );
    const CString::StringVector& GetDeclaredModes( void ) const;

    void SetInitialMode( const CString& initialMode );
    const CString& GetInitialMode( void ) const;
    private:

    CString m_name;
    UInt32 m_maxConcurrent;
    CIJobThrottlePolicy* m_throttlePolicy;
    CString::StringVector m_declaredModes;
    CString m_initialMode;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBLANEDEFINITION_H ? */
