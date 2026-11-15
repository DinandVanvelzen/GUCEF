/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWENGINESELECTION_H
#define GUCEF_WEB_CWEBVIEWENGINESELECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_IWEBVIEWENGINE_H
#include "gucefWEB_IWebViewEngine.h"
#define GUCEF_WEB_IWEBVIEWENGINE_H
#endif /* GUCEF_WEB_IWEBVIEWENGINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The outcome of CWebViewGlobal::FindEngine(): the engine chosen (NULL when none qualified) and, for every engine that
 *  was considered or named in the order of preference, why it was or was not chosen. The report of a sign-in shows
 *  these lines: "WebKitGTK: not available, no display server", "WPE: not loaded".
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewEngineSelection
{
    public:

    enum EOutcome
    {
        OUTCOME_CHOSEN              = 0 ,
        OUTCOME_NOT_LOADED              ,   /**< named in the order of preference, but no plugin registered it */
        OUTCOME_NOT_AVAILABLE           ,   /**< loaded, but it cannot run here */
        OUTCOME_CAPABILITY_MISSING      ,
        OUTCOME_STALLED                 ,
        OUTCOME_NOT_TRIED                   /**< an engine before it was chosen */
    };
    typedef enum EOutcome TOutcome;

    struct GUCEF_WEB_PUBLIC_CPP SCandidate
    {
        CString engineName;
        TOutcome outcome;
        CString reason;                 /**< a sentence for the owner */
        UInt32 missingCapabilities;     /**< OUTCOME_CAPABILITY_MISSING: CWebViewCapabilities flags */

        SCandidate( void );
    };
    typedef struct SCandidate TCandidate;
    typedef GUCEF::vector< TCandidate > TCandidateVector;

    TWebViewEnginePtr engine;           /**< the engine chosen, NULL for none */
    TCandidateVector candidates;

    CWebViewEngineSelection( void );

    bool HasEngine( void ) const;

    /** One line per candidate */
    CString Describe( void ) const;

    static const char* OutcomeToString( TOutcome outcome );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWENGINESELECTION_H ? */
