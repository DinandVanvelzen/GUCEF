/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CDatabaseTransaction.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDatabaseTransaction::CDatabaseTransaction( const CIDatabaseClientPtr& client )
    : m_clientPtr( client )
    , m_client( m_clientPtr.GetPointerAlways() )
    , m_beginStatus( CORE::DATATABLE_STATUS_INVALID_ARGUMENT )
    , m_active( false )
{GUCEF_TRACE;

    Begin();
}

/*-------------------------------------------------------------------------*/

CDatabaseTransaction::CDatabaseTransaction( CIDatabaseClient& client )
    : m_clientPtr()
    , m_client( &client )
    , m_beginStatus( CORE::DATATABLE_STATUS_INVALID_ARGUMENT )
    , m_active( false )
{GUCEF_TRACE;

    Begin();
}

/*-------------------------------------------------------------------------*/

CDatabaseTransaction::~CDatabaseTransaction()
{GUCEF_TRACE;

    if ( m_active )
    {
        const CORE::TDataTableStatus status = Rollback();
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DatabaseTransaction: The rollback at the end of the scope failed with " <<
                CORE::DataTableStatusToString( status ) << ": " << m_client->GetLastErrorMessage() );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDatabaseTransaction::Begin( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
    {
        m_beginStatus = CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        return;
    }
    m_beginStatus = m_client->BeginTransaction();
    m_active = CORE::DATATABLE_STATUS_OK == m_beginStatus;
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseTransaction::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDatabaseTransaction::GetBeginStatus( void ) const
{GUCEF_TRACE;

    return m_beginStatus;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDatabaseTransaction::Commit( void )
{GUCEF_TRACE;

    if ( !m_active )
        return CORE::DATATABLE_STATUS_OK == m_beginStatus ? CORE::DATATABLE_STATUS_INVALID_ARGUMENT : m_beginStatus;
    m_active = false;
    return m_client->CommitTransaction();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDatabaseTransaction::Rollback( void )
{GUCEF_TRACE;

    if ( !m_active )
        return CORE::DATATABLE_STATUS_OK == m_beginStatus ? CORE::DATATABLE_STATUS_INVALID_ARGUMENT : m_beginStatus;
    m_active = false;
    return m_client->RollbackTransaction();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
