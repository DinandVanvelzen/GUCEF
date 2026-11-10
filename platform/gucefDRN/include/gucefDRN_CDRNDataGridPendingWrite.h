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


#ifndef GUCEF_DRN_CDRNDATAGRIDPENDINGWRITE_H
#define GUCEF_DRN_CDRNDATAGRIDPENDINGWRITE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITERESULT_H
#include "gucefCORE_CDataTableWriteResult.h"
#define GUCEF_CORE_CDATATABLEWRITERESULT_H
#endif /* GUCEF_CORE_CDATATABLEWRITERESULT_H ? */

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

/** One row a table write changes: the key and the new value, or the removal of the key */
class GUCEF_DRN_EXPORT_CPP CDRNRowChange
{
    public:

    CORE::CVariant key;
    CORE::CVariant value;
    bool remove;

    CDRNRowChange( void );
};

typedef GUCEF::vector< CDRNRowChange >  TDRNRowChangeVector;

/*-------------------------------------------------------------------------*/

/**
 *  An asynchronous table write on a replica of a grid that the replica may not write directly: the rows went to the
 *  publisher as proposals, and the write ends when the publisher (and the vote, if there is one) has answered for
 *  every proposal. The answers are matched to the rows by proposal number, which grows with every proposal sent.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataGridPendingWrite
{
    public:

    /** What the publisher said about one proposal */
    class GUCEF_DRN_EXPORT_CPP CAnswer
    {
        public:

        UInt32 proposalId;
        bool accepted;
        UInt32 voteOutcome;
        CORE::CString reason;

        CAnswer( void );
    };
    typedef GUCEF::vector< CAnswer >    TAnswerVector;

    /** The rows of the write, how they combine with the rows there are, and how long the answers are waited for (0 = without end) once the proposals went out */
    CDRNDataGridPendingWrite( const TDRNRowChangeVector& changes, CORE::TDataTableWriteMode mode, UInt32 timeoutMs );

    CORE::TDataTableWriteMode GetWriteMode( void ) const;

    /** The future that ends when the write has ended */
    CORE::CFutureResult GetFuture( void ) const;

    /** The rows to send, to be written to the grid by the table */
    const TDRNRowChangeVector& GetChanges( void ) const;

    /** Rows (indices into the changes) that went out as proposals, and so are waited for */
    const GUCEF::vector< UInt32 >& GetProposedRows( void ) const;

    /** Rows that are waited for, once the proposals went out */
    void SetProposedRows( const GUCEF::vector< UInt32 >& proposedRows );

    /** The result of the rows as far as it is known: the sent state of every row, and the answers for those that came in */
    const CORE::CDataTableWriteResult& GetSentResult( void ) const;
    void SetSentResult( const CORE::CDataTableWriteResult& sent );

    /** Starts the clock of the timeout, in microseconds of the caller's clock; 0 = no timeout */
    void Start( UInt64 nowInMicroseconds );

    bool HasStarted( void ) const;

    void AddAnswer( UInt32 proposalId, bool accepted, UInt32 voteOutcome, const CORE::CString& reason );

    /** An answer came in for every proposal */
    bool IsComplete( void ) const;

    bool IsOverdue( UInt64 nowInMicroseconds ) const;

    /** The result with the answers applied to the rows they belong to */
    void BuildResult( CORE::CDataTableWriteResult& result ) const;

    /** Ends the future successfully with the result, or when the write failed as a whole with the status text */
    bool Complete( void );
    bool Fail( const CORE::CString& statusText );

    private:

    CDRNDataGridPendingWrite( const CDRNDataGridPendingWrite& src );
    CDRNDataGridPendingWrite& operator=( const CDRNDataGridPendingWrite& src );

    TDRNRowChangeVector m_changes;
    CORE::TDataTableWriteMode m_mode;
    CORE::CDataTableWriteResult m_sent;
    GUCEF::vector< UInt32 > m_proposedRows;
    TAnswerVector m_answers;
    CORE::CFutureResultSource m_source;
    UInt32 m_timeoutMs;
    UInt64 m_deadline;
    bool m_started;
};

typedef CORE::CTSharedPtr< CDRNDataGridPendingWrite, MT::CMutex >   CDRNDataGridPendingWritePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATAGRIDPENDINGWRITE_H ? */
