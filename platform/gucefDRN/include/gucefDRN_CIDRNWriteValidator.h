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

#ifndef GUCEF_DRN_CIDRNWRITEVALIDATOR_H
#define GUCEF_DRN_CIDRNWRITEVALIDATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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

class CDRNDataGrid;

/**
 *  Checks a write that another member proposed to a grid with HOST or PROPOSAL_VOTE authority, on the sequencer,
 *  before it is applied or put to a vote. A refusal goes back to the proposer with the reason.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNWriteValidator
{
    public:

    CIDRNWriteValidator( void );

    virtual ~CIDRNWriteValidator();

    virtual bool ValidateWrite( UInt32 proposerPeerId            ,
                                const CDRNDataGrid& grid         ,
                                const CORE::CVariant& key        ,
                                const CORE::CVariant& value      ,
                                CORE::CString& refusalReason     ) = 0;

    private:

    CIDRNWriteValidator( const CIDRNWriteValidator& src );
    CIDRNWriteValidator& operator=( const CIDRNWriteValidator& src );
};

/*-------------------------------------------------------------------------*/

/** The default: every proposal is accepted */
class GUCEF_DRN_EXPORT_CPP CDRNDefaultWriteValidator : public CIDRNWriteValidator
{
    public:

    CDRNDefaultWriteValidator( void );

    virtual ~CDRNDefaultWriteValidator();

    virtual bool ValidateWrite( UInt32 proposerPeerId            ,
                                const CDRNDataGrid& grid         ,
                                const CORE::CVariant& key        ,
                                const CORE::CVariant& value      ,
                                CORE::CString& refusalReason     ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNWRITEVALIDATOR_H ? */
