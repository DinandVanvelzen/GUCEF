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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROBERESULT_H
#define GUCEF_ASSETSTORE_CASSETSTOREPROBERESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the probe of a store found (CIAssetStoreProbe): a list of checks, each with the name of the check, its outcome as a
 *  short text and whether it passed. The texts describe the shape of what the store answered, never its values: no token,
 *  cookie, account id or address of a person is in a result, and a probe must not put one there. The result data of
 *  CIAssetStoreProvider::RunProbeAsync().
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreProbeResult : public CORE::CICloneable
{
    public:

    struct SCheck
    {
        CORE::CString name;         /**< what was checked, for example "library listing" */
        CORE::CString outcome;      /**< what was found, as a short text without values of the store */
        bool passed;

        SCheck( void );
        SCheck( const CORE::CString& checkName, const CORE::CString& checkOutcome, bool checkPassed );
    };
    typedef struct SCheck TCheck;
    typedef GUCEF::vector< TCheck > TCheckVector;

    TCheckVector checks;
    CORE::CString summary;          /**< one line about the whole probe, may be empty */

    CAssetStoreProbeResult( void );

    virtual ~CAssetStoreProbeResult() GUCEF_VIRTUAL_OVERRIDE;

    void Clear( void );

    void AddCheck( const CORE::CString& name, const CORE::CString& outcome, bool passed );

    /** Whether every check passed; false for a result without checks */
    bool IsAllPassed( void ) const;

    UInt32 GetPassedCount( void ) const;

    /** The lines a person reads: "name: outcome" with "ok" or "failed" at the start, one per check, and the summary last */
    void GetLines( CORE::CString::StringVector& lines ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROBERESULT_H ? */
