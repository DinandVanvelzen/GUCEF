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

#ifndef GUCEF_COMCORE_CRPCCOMPATIBILITYREPORT_H
#define GUCEF_COMCORE_CRPCCOMPATIBILITYREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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
 *  The findings of CRpcContractCompatibility::Check(): one entry per finding with the path of the element, a rule id
 *  and whether it breaks compatibility within the major version.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcCompatibilityReport
{
    public:

    struct GUCEF_COMCORE_EXPORT_CPP SEntry
    {
        CORE::CString path;         /**< "types.Note.fields.price", "services.Notes.methods.Get" */
        CORE::CString ruleId;       /**< "REMOVED", "RENAMED", "TYPE_CHANGED", ... */
        CORE::CString message;
        bool isBreaking;

        SEntry( void );
    };
    typedef GUCEF::vector< SEntry > TEntryVector;

    CRpcCompatibilityReport( void );

    void Add( const CORE::CString& path, const CORE::CString& ruleId, const CORE::CString& message, bool isBreaking );
    void Clear( void );

    const TEntryVector& GetEntries( void ) const;
    UInt32 GetBreakingCount( void ) const;
    bool HasRule( const CORE::CString& ruleId ) const;

    /** One line per entry */
    CORE::CString ToString( void ) const;

    private:

    TEntryVector m_entries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCOMPATIBILITYREPORT_H ? */
