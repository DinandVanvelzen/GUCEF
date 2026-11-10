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

#ifndef GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H
#define GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CRPCCOMPATIBILITYREPORT_H
#include "gucefCOMCORE_CRpcCompatibilityReport.h"
#define GUCEF_COMCORE_CRPCCOMPATIBILITYREPORT_H
#endif /* GUCEF_COMCORE_CRPCCOMPATIBILITYREPORT_H ? */

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
 *  The compatibility rules between two versions of a contract (gucefCOMCORE RPC design 3.9). Within one major
 *  version only additions (with since = the new minor), reserved names and numbers, documentation, deadlines, credits,
 *  deprecation, widening of integers of the same signedness, single <-> optional and public -> confidential are
 *  allowed; everything else is BREAKING. Between majors every change is allowed and reported as a warning.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcContractCompatibility
{
    public:

    enum EResult
    {
        RESULT_COMPATIBLE               = 0 ,
        RESULT_COMPATIBLE_WITH_WARNINGS = 1 ,
        RESULT_BREAKING                 = 2
    };
    typedef enum EResult TResult;

    static TResult Check( const CRpcContract& older, const CRpcContract& newer, CRpcCompatibilityReport& report );

    /** True when a value written as type from can be read as type to (same type or widening of the same signedness) */
    static bool IsWireCompatibleWidening( TRpcFieldType from, TRpcFieldType to );

    private:

    CRpcContractCompatibility( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCONTRACTCOMPATIBILITY_H ? */
