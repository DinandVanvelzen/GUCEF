/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_TESTAPP_TESTRECORDER_H
#define GUCEF_AUTH_TESTAPP_TESTRECORDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#include "TestPump.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Records the events of providers and of the manager, in order */
class CAuthRecorder : public GUCEF::CORE::CObserver
{
    public:

    enum EKind
    {
        KIND_SUCCEEDED, KIND_FAILED, KIND_ACTION, KIND_STATE, KIND_LINKED, KIND_UNLINKED, KIND_LOGGEDOUT, KIND_AUTOFINISHED, KIND_SETTINGS
    };

    struct SRecord
    {
        EKind kind;
        GUCEF::UInt32 threadId;
        GUCEF::AUTH::CAuthResult result;
        GUCEF::AUTH::CAuthFailure failure;
        GUCEF::AUTH::CAuthActionRequired action;
        GUCEF::AUTH::CAuthStateEventData state;
        GUCEF::AUTH::CAuthAccountEventData account;
    };

    GUCEF::vector< SRecord > records;

    CAuthRecorder( void ) : GUCEF::CORE::CObserver(), records() {}

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CAuthRecorder";
        return classTypeName;
    }

    GUCEF::UInt32 Count( EKind kind ) const
    {
        GUCEF::UInt32 count = 0;
        for ( size_t i=0; i<records.size(); ++i )
            if ( records[ i ].kind == kind )
                ++count;
        return count;
    }

    /** The last record of the kind, GUCEF_NULL when there is none */
    const SRecord* Last( EKind kind ) const
    {
        for ( size_t i=records.size(); i>0; --i )
            if ( records[ i-1 ].kind == kind )
                return &records[ i-1 ];
        return GUCEF_NULL;
    }

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        using namespace GUCEF;
        SRecord record;
        record.threadId = GUCEF::MT::GetCurrentTaskID();
        if ( AUTH::CIAuthProvider::LoginSucceededEvent == eventId || AUTH::CAuthManager::LoginSucceededEvent == eventId )
        {
            record.kind = KIND_SUCCEEDED;
            record.result = *static_cast< AUTH::CAuthResult* >( eventData );
        }
        else
        if ( AUTH::CIAuthProvider::LoginFailedEvent == eventId || AUTH::CAuthManager::LoginFailedEvent == eventId )
        {
            record.kind = KIND_FAILED;
            record.failure = *static_cast< AUTH::CAuthFailure* >( eventData );
        }
        else
        if ( AUTH::CIAuthProvider::ActionRequiredEvent == eventId || AUTH::CAuthManager::ActionRequiredEvent == eventId )
        {
            record.kind = KIND_ACTION;
            record.action = *static_cast< AUTH::CAuthActionRequired* >( eventData );
        }
        else
        if ( AUTH::CAuthManager::LoginStateChangedEvent == eventId )
        {
            record.kind = KIND_STATE;
            record.state = *static_cast< AUTH::CAuthStateEventData* >( eventData );
        }
        else
        if ( AUTH::CAuthManager::AutoLoginFinishedEvent == eventId )
        {
            record.kind = KIND_AUTOFINISHED;
            record.state = *static_cast< AUTH::CAuthStateEventData* >( eventData );
        }
        else
        if ( AUTH::CAuthManager::AccountLinkedEvent == eventId )
        {
            record.kind = KIND_LINKED;
            record.account = *static_cast< AUTH::CAuthAccountEventData* >( eventData );
        }
        else
        if ( AUTH::CAuthManager::AccountUnlinkedEvent == eventId )
        {
            record.kind = KIND_UNLINKED;
            record.account = *static_cast< AUTH::CAuthAccountEventData* >( eventData );
        }
        else
        if ( AUTH::CAuthManager::AccountLoggedOutEvent == eventId )
        {
            record.kind = KIND_LOGGEDOUT;
            record.account = *static_cast< AUTH::CAuthAccountEventData* >( eventData );
        }
        else
        if ( AUTH::CAuthManager::SettingsChangedEvent == eventId )
        {
            record.kind = KIND_SETTINGS;
        }
        else
        {
            return;
        }
        records.push_back( record );
    }
};

/*-------------------------------------------------------------------------*/

/** Pulses until the recorder has seen the event kind the given number of times; true if it did in time */
inline bool
PumpUntilRecorded( const CAuthRecorder& recorder, CAuthRecorder::EKind kind, GUCEF::UInt32 count, GUCEF::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs && recorder.Count( kind ) < count; ++waited )
        PumpFor( 1 );
    return recorder.Count( kind ) >= count;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTRECORDER_H ? */
