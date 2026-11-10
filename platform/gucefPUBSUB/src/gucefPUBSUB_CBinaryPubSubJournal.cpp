/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include "gucefPUBSUB_CBinaryPubSubJournal.h"    

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CBinaryPubSubJournal::JournalType = "binary";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBinaryPubSubJournal::CBinaryPubSubJournal( void )
    : CIPubSubJournal()
    , m_journal()
    , m_config()
{GUCEF_TRACE;
    
}

/*-------------------------------------------------------------------------*/

CBinaryPubSubJournal::CBinaryPubSubJournal( const CPubSubJournalConfig& config )
    : CIPubSubJournal()
    , m_journal()   
    , m_config()
{GUCEF_TRACE;
    
    LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

CBinaryPubSubJournal::~CBinaryPubSubJournal() 
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CBinaryPubSubJournal::ReadJournalEntry( UInt64& eventTimestamp   ,
                                       TPubSubActionType& action ,
                                       UInt64& msgActionId       )
{GUCEF_TRACE;

    if ( !m_journal.IsNULL() )
    {
        if ( m_journal->ReadValue( eventTimestamp ) )
            if ( m_journal->ReadValue( action ) )
                if ( m_journal->ReadValue( msgActionId ) )
                    return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryPubSubJournal::AddJournalEntry( UInt64 eventTimestamp    ,
                                       TPubSubActionType action ,
                                       UInt64 msgActionId       )
{GUCEF_TRACE;

    if ( !m_journal.IsNULL() )
    {
        if ( m_journal->WriteValue( eventTimestamp ) )
            if ( m_journal->WriteValue( action ) )
                if ( m_journal->WriteValue( msgActionId ) )
                    return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void 
CBinaryPubSubJournal::SetJournalResource( CORE::IOAccessPtr journal )
{GUCEF_TRACE;

    m_journal = journal;
}

/*-------------------------------------------------------------------------*/

bool
CBinaryPubSubJournal::LoadConfig( const CPubSubJournalConfig& config )
{GUCEF_TRACE;

    m_journal.Unlink();

    m_config = config;

    if ( m_config.useJournal && m_config.journalType == JournalType )
    {
        CORE::CUriResourceAccessorPtr access = CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().Create( m_config.journalResource.GetScheme() );        
        if ( !access.IsNULL() )
        {
            if ( access->GetResourceAccess( m_config.journalResource, m_journal, CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_APPEND ) )
            {
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BinaryPubSubJournal:LoadConfig: Failed to get resource access for journal resource " << CORE::CRedactor::ForUrl().RedactUrl( m_config.journalResource.ToUriInStringForm() ) );
                return false;
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "BinaryPubSubJournal:LoadConfig: Failed to create resource accessor for journal resource " << CORE::CRedactor::ForUrl().RedactUrl( m_config.journalResource.ToUriInStringForm() ) );
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
