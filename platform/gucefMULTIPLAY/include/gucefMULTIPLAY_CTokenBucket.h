/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CTOKENBUCKET_H
#define GUCEF_MULTIPLAY_CTOKENBUCKET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A rate limit as a token bucket: Take() succeeds while there is a token. Tokens come back at the rate, up to the
 *  burst, which the bucket starts with. A rate of 0 is no limit.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CTokenBucket
{
    public:

    /** No limit */
    CTokenBucket( void );

    CTokenBucket( UInt32 perSecond, UInt32 burst );

    /** A new rate, the bucket starts full again */
    void SetRate( UInt32 perSecond, UInt32 burst );

    /** Takes a token, false when there is none now */
    bool Take( UInt64 nowMs );

    private:

    UInt32 m_perSecond;
    UInt32 m_burst;
    UInt64 m_milliTokens;
    UInt64 m_lastMs;
    bool m_started;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CTOKENBUCKET_H ? */
