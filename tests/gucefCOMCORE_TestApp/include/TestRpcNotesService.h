/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCNOTESSERVICE_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCNOTESSERVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The test implementation of gucef.sample.notes Notes (the contract of TestRpcSupport.h). Header only: gucefCOM_TestApp
 *  uses it as well.
 *      Get       unary: note noteId with title "note <id>"; id 404 -> NOT_FOUND; id 999 -> never answered (deadlines);
 *                with attachmentBytes the note carries an attachment of that many bytes (byte i is i * 31 + 7)
 *      Save      unary: revision 1 + the number of saves; savedAttachmentBytes is the size of the attachment saved last
 *      Watch     server stream: watchCount events, then the end (or held open when holdWatch)
 *      Import    client stream: counts the chunks and their bytes, answers on half close
 *      Edit      bidi stream: echoes every message, ends on half close
 *      Reindex   unary, mayAnswerAsync: full=false answers at once; full=true accepts (request id kind and durability
 *                from the metadata "id-kind" ("string" / "number") and "durable"), CompleteReindex() completes it
 */
class CRpcNotesTestService : public GUCEF::COMCORE::CRpcServiceImpl ,
                             public GUCEF::CORE::CObserver
{
    public:

    typedef GUCEF::COMCORE::CTRpcMethodHandler< CRpcNotesTestService > THandler;

    GUCEF::UInt32 watchCount;
    bool holdWatch;
    GUCEF::UInt32 calls;
    GUCEF::UInt32 saves;
    GUCEF::UInt32 lastHandlerThread;
    GUCEF::COMCORE::CRpcMetadata lastMetadata;
    GUCEF::UInt32 cancelledCalls;
    GUCEF::UInt64 importedChunks;
    GUCEF::UInt64 importedBytes;
    GUCEF::UInt32 reindexAccepted;
    GUCEF::UInt32 attachmentBytes;
    GUCEF::UInt32 savedAttachmentBytes;

    CRpcNotesTestService( void )
        : GUCEF::COMCORE::CRpcServiceImpl()
        , GUCEF::CORE::CObserver()
        , watchCount( 3 ), holdWatch( false ), calls( 0 ), saves( 0 ), lastHandlerThread( 0 ), lastMetadata(), cancelledCalls( 0 )
        , importedChunks( 0 ), importedBytes( 0 ), reindexAccepted( 0 ), attachmentBytes( 0 ), savedAttachmentBytes( 0 ), m_lock(), m_held(), m_reindex(), m_watching(), m_nextJob( 1 )
    {
        BindMethod( "Get", GUCEF_NEW THandler( this, &CRpcNotesTestService::OnGet ) );
        BindMethod( "Save", GUCEF_NEW THandler( this, &CRpcNotesTestService::OnSave ) );
        BindMethod( "Watch", GUCEF_NEW THandler( this, &CRpcNotesTestService::OnWatch ) );
        BindMethod( "Import", GUCEF_NEW THandler( this, &CRpcNotesTestService::OnImport ) );
        BindMethod( "Edit", GUCEF_NEW THandler( this, &CRpcNotesTestService::OnEdit ) );
        BindMethod( "Reindex", GUCEF_NEW THandler( this, &CRpcNotesTestService::OnReindex ) );
    }

    virtual ~CRpcNotesTestService()
    {
        UnsubscribeAllFromObserver();
    }

    /** Completes accepted Reindex calls with the given message (of any type: a newer provider's fields); how many */
    GUCEF::UInt32 CompleteReindexWith( const GUCEF::COMCORE::CRpcMessage& result )
    {
        GUCEF::vector< GUCEF::COMCORE::CRpcServerCallPtr > pending;
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            pending.swap( m_reindex );
        }
        for ( GUCEF::UInt32 i=0; i<pending.size(); ++i )
            pending[ i ]->CompleteResult( result );
        return (GUCEF::UInt32) pending.size();
    }

    /** Completes accepted Reindex calls; returns how many */
    GUCEF::UInt32 CompleteReindex( GUCEF::UInt64 indexed )
    {
        GUCEF::vector< GUCEF::COMCORE::CRpcServerCallPtr > pending;
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            pending.swap( m_reindex );
        }
        for ( GUCEF::UInt32 i=0; i<pending.size(); ++i )
        {
            GUCEF::COMCORE::CRpcMessage result = pending[ i ]->NewResponse();
            result.SetUInt64( 1, indexed );
            result.SetDuration( 2, 1500 );
            pending[ i ]->CompleteResult( result );
        }
        return (GUCEF::UInt32) pending.size();
    }

    /** Server streams held open (holdWatch), ended with the status */
    void FinishHeldStreams( const GUCEF::COMCORE::CRpcStatus& status )
    {
        GUCEF::vector< GUCEF::COMCORE::CRpcServerCallPtr > held;
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            held.swap( m_held );
        }
        for ( GUCEF::UInt32 i=0; i<held.size(); ++i )
            held[ i ]->Finish( status );
    }

    GUCEF::UInt32 GetHeldCount( void ) const { GUCEF::MT::CScopeMutex lock( m_lock ); return (GUCEF::UInt32) m_held.size(); }

    /** For tests whose handlers run on another thread than the test */
    GUCEF::UInt32 GetCancelledCalls( void ) const { GUCEF::MT::CScopeMutex lock( m_lock ); return cancelledCalls; }

    GUCEF::COMCORE::TRpcHandlerResult OnGet( GUCEF::COMCORE::CRpcServerCall& call )
    {
        Note( call );
        GUCEF::UInt64 id = call.GetRequest().GetUInt64( 1 );
        if ( 404 == id )
        {
            call.Fail( GUCEF::COMCORE::CRpcStatus( GUCEF::COMCORE::RPC_STATUS_NOT_FOUND, "no note 404" ) );
            return GUCEF::COMCORE::RPC_HANDLER_DONE;
        }
        if ( 999 == id )
        {
            Hold( call );
            return GUCEF::COMCORE::RPC_HANDLER_DEFERRED;
        }
        GUCEF::COMCORE::CRpcMessage note = call.NewResponse();
        note.SetUInt64( 1, id );
        note.SetString( 2, "note " + GUCEF::CORE::ToString( id ) );
        note.SetString( 3, "the secret body" );
        note.SetEnum( 4, 2 );
        note.Add( 5, GUCEF::CORE::CVariant( "a" ) );
        note.Add( 5, GUCEF::CORE::CVariant( "b" ) );
        note.SetTimestamp( 6, 1700000000000000LL );
        if ( GUCEF_NULL != call.GetContract()->FindType( "Note" ) && GUCEF_NULL != call.GetContract()->FindType( "Note" )->FindFieldByNumber( 8 ) )
            note.SetDecimal( 8, GUCEF::CORE::CDecimal::Parse( "12.5" ) );
        if ( attachmentBytes > 0 )
        {
            GUCEF::CORE::CDynamicBuffer attachment( attachmentBytes, true );
            for ( GUCEF::UInt32 i=0; i<attachmentBytes; ++i )
                static_cast< GUCEF::UInt8* >( attachment.GetBufferPtr() )[ i ] = (GUCEF::UInt8) ( i * 31 + 7 );
            attachment.SetDataSize( attachmentBytes );
            note.SetBytes( 10, attachment );
        }
        call.Reply( note );
        return GUCEF::COMCORE::RPC_HANDLER_DONE;
    }

    GUCEF::COMCORE::TRpcHandlerResult OnSave( GUCEF::COMCORE::CRpcServerCall& call )
    {
        Note( call );
        GUCEF::UInt32 revision = 0;
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            revision = ++saves;
        }
        GUCEF::COMCORE::CRpcMessage response = call.NewResponse();
        const GUCEF::COMCORE::CRpcMessage* note = call.GetRequest().GetMessage( 1 );
        response.SetUInt64( 1, GUCEF_NULL != note ? note->GetUInt64( 1 ) : 0 );
        GUCEF::CORE::CVariant attachment;
        if ( GUCEF_NULL != note && note->Get( 10, attachment ) )
            savedAttachmentBytes = attachment.ByteSize( false );
        response.SetUInt64( 2, revision );
        call.Reply( response );
        return GUCEF::COMCORE::RPC_HANDLER_DONE;
    }

    GUCEF::COMCORE::TRpcHandlerResult OnWatch( GUCEF::COMCORE::CRpcServerCall& call )
    {
        Note( call );
        GUCEF::UInt64 next = call.HasResumeFromIndex() ? call.GetResumeFromIndex() : 0;
        if ( !WriteEvents( call, next ) )
        {
            // out of credit: the rest follows on WritableEvent. Subscribed outside the service lock: subscribing takes
            // the lock the call's events are delivered under, and those handlers take the service lock
            {
                GUCEF::MT::CScopeMutex lock( m_lock );
                m_watching.push_back( SWatch( call.GetSharedPtr(), next ) );
            }
            typedef GUCEF::CORE::CTEventHandlerFunctor< CRpcNotesTestService > TEventCallback;
            TEventCallback writable( this, &CRpcNotesTestService::OnWritable );
            TEventCallback cancelled( this, &CRpcNotesTestService::OnCancelled );
            SubscribeTo( &call, GUCEF::COMCORE::CRpcServerCall::WritableEvent, writable );
            SubscribeTo( &call, GUCEF::COMCORE::CRpcServerCall::CancelledEvent, cancelled );
            return GUCEF::COMCORE::RPC_HANDLER_DEFERRED;
        }
        if ( holdWatch )
        {
            Hold( call );
            return GUCEF::COMCORE::RPC_HANDLER_DEFERRED;
        }
        call.Finish();
        return GUCEF::COMCORE::RPC_HANDLER_DONE;
    }

    GUCEF::COMCORE::TRpcHandlerResult OnImport( GUCEF::COMCORE::CRpcServerCall& call )
    {
        Note( call );
        Track( call );
        if ( call.HasRequest() )
            Count( call.GetRequest() );
        return GUCEF::COMCORE::RPC_HANDLER_DEFERRED;
    }

    GUCEF::COMCORE::TRpcHandlerResult OnEdit( GUCEF::COMCORE::CRpcServerCall& call )
    {
        Note( call );
        Track( call );
        if ( call.HasRequest() )
            call.Write( call.GetRequest() );
        return GUCEF::COMCORE::RPC_HANDLER_DEFERRED;
    }

    GUCEF::COMCORE::TRpcHandlerResult OnReindex( GUCEF::COMCORE::CRpcServerCall& call )
    {
        Note( call );
        if ( !call.GetRequest().GetBool( 1 ) )
        {
            GUCEF::COMCORE::CRpcMessage result = call.NewResponse();
            result.SetUInt64( 1, 7 );
            call.Reply( result );
            return GUCEF::COMCORE::RPC_HANDLER_DONE;
        }
        GUCEF::COMCORE::CRpcRequestId id;
        GUCEF::CORE::CString kind = call.GetMetadata().GetString( "id-kind" );
        GUCEF::UInt64 job = 0;
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            job = m_nextJob++;
        }
        if ( "string" == kind )
            id = GUCEF::COMCORE::CRpcRequestId( GUCEF::CORE::CString( "job-" ) + GUCEF::CORE::ToString( job ) );
        else if ( "number" == kind )
            id = GUCEF::COMCORE::CRpcRequestId( 1000 + job );
        bool durable = "true" == call.GetMetadata().GetString( "durable" );
        if ( !call.Accept( id, durable, 2000, 100 ) )
        {
            call.Fail( GUCEF::COMCORE::CRpcStatus( GUCEF::COMCORE::RPC_STATUS_INTERNAL, "accept refused" ) );
            return GUCEF::COMCORE::RPC_HANDLER_DONE;
        }
        GUCEF::MT::CScopeMutex lock( m_lock );
        ++reindexAccepted;
        m_reindex.push_back( call.GetSharedPtr() );
        return GUCEF::COMCORE::RPC_HANDLER_DEFERRED;
    }

    private:

    struct SWatch
    {
        GUCEF::COMCORE::CRpcServerCallPtr call;
        GUCEF::UInt64 next;
        SWatch( const GUCEF::COMCORE::CRpcServerCallPtr& c, GUCEF::UInt64 n ) : call( c ), next( n ) {}
    };

    /** Writes events from next on; false when the credit ran out first */
    bool WriteEvents( GUCEF::COMCORE::CRpcServerCall& call, GUCEF::UInt64& next )
    {
        for ( ; next<watchCount; ++next )
        {
            GUCEF::COMCORE::CRpcMessage event = call.NewResponse();
            event.GetMutableMessage( 1 )->SetUInt64( 1, next + 1 );
            event.GetMutableMessage( 1 )->SetString( 2, "event" );
            if ( !call.Write( event ) )
                return call.IsDone();
        }
        return true;
    }

    void OnWritable( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* )
    {
        GUCEF::COMCORE::CRpcServerCallPtr call;
        GUCEF::UInt64 next = 0;
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            for ( GUCEF::UInt32 i=0; i<m_watching.size(); ++i )
                if ( static_cast< GUCEF::CORE::CNotifier* >( m_watching[ i ].call.GetPointerAlways() ) == notifier )
                {
                    call = m_watching[ i ].call;
                    next = m_watching[ i ].next;
                    m_watching.erase( m_watching.begin() + i );
                    break;
                }
        }
        if ( call.IsNULL() )
            return;
        if ( !WriteEvents( *call, next ) )
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            m_watching.push_back( SWatch( call, next ) );
            return;
        }
        UnsubscribeFrom( *call.GetPointerAlways() );
        if ( holdWatch )
            Hold( *call );
        else
            call->Finish();
    }

    void Note( GUCEF::COMCORE::CRpcServerCall& call )
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        ++calls;
        lastHandlerThread = GUCEF::MT::GetCurrentTaskID();
        lastMetadata = call.GetMetadata();
    }

    void Hold( GUCEF::COMCORE::CRpcServerCall& call )
    {
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            m_held.push_back( call.GetSharedPtr() );
        }
        typedef GUCEF::CORE::CTEventHandlerFunctor< CRpcNotesTestService > TEventCallback;
        TEventCallback cancelled( this, &CRpcNotesTestService::OnCancelled );
        SubscribeTo( &call, GUCEF::COMCORE::CRpcServerCall::CancelledEvent, cancelled );
    }

    void Track( GUCEF::COMCORE::CRpcServerCall& call )
    {
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            m_held.push_back( call.GetSharedPtr() );
        }
        typedef GUCEF::CORE::CTEventHandlerFunctor< CRpcNotesTestService > TEventCallback;
        TEventCallback message( this, &CRpcNotesTestService::OnStreamMessage );
        TEventCallback halfClosed( this, &CRpcNotesTestService::OnHalfClosed );
        TEventCallback cancelled( this, &CRpcNotesTestService::OnCancelled );
        SubscribeTo( &call, GUCEF::COMCORE::CRpcServerCall::MessageEvent, message );
        SubscribeTo( &call, GUCEF::COMCORE::CRpcServerCall::HalfClosedEvent, halfClosed );
        SubscribeTo( &call, GUCEF::COMCORE::CRpcServerCall::CancelledEvent, cancelled );
    }

    void Count( const GUCEF::COMCORE::CRpcMessage& chunk )
    {
        GUCEF::CORE::CDynamicBuffer data;
        chunk.GetBytes( 1, data );
        GUCEF::MT::CScopeMutex lock( m_lock );
        ++importedChunks;
        importedBytes += data.GetDataSize();
    }

    GUCEF::COMCORE::CRpcServerCall* Find( GUCEF::CORE::CNotifier* notifier, GUCEF::COMCORE::CRpcServerCallPtr& held )
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        for ( GUCEF::UInt32 i=0; i<m_held.size(); ++i )
            if ( static_cast< GUCEF::CORE::CNotifier* >( m_held[ i ].GetPointerAlways() ) == notifier )
            {
                held = m_held[ i ];
                return held.GetPointerAlways();
            }
        return GUCEF_NULL;
    }

    void Release( GUCEF::COMCORE::CRpcServerCall* call )
    {
        GUCEF::MT::CScopeMutex lock( m_lock );
        for ( GUCEF::UInt32 i=0; i<m_held.size(); ++i )
            if ( m_held[ i ].GetPointerAlways() == call )
            {
                m_held.erase( m_held.begin() + i );
                return;
            }
    }

    void OnStreamMessage( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        GUCEF::COMCORE::CRpcServerCallPtr held;
        GUCEF::COMCORE::CRpcServerCall* call = Find( notifier, held );
        if ( GUCEF_NULL == call || GUCEF_NULL == eventData )
            return;
        const GUCEF::COMCORE::CRpcMessage& message = static_cast< GUCEF::COMCORE::CRpcStreamMessageEventData* >( eventData )->message;
        if ( "Import" == call->GetMethod().GetName() )
            Count( message );
        else
            call->Write( message );
    }

    void OnHalfClosed( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* )
    {
        GUCEF::COMCORE::CRpcServerCallPtr held;
        GUCEF::COMCORE::CRpcServerCall* call = Find( notifier, held );
        if ( GUCEF_NULL == call )
            return;
        if ( "Import" == call->GetMethod().GetName() )
        {
            GUCEF::COMCORE::CRpcMessage summary = call->NewResponse();
            {
                GUCEF::MT::CScopeMutex lock( m_lock );
                summary.SetUInt32( 1, (GUCEF::UInt32) importedChunks );
            }
            call->Reply( summary );
        }
        else
            call->Finish();
        Release( call );
    }

    void OnCancelled( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        GUCEF::COMCORE::CRpcServerCallPtr held;
        GUCEF::COMCORE::CRpcServerCall* call = Find( notifier, held );
        {
            GUCEF::MT::CScopeMutex lock( m_lock );
            ++cancelledCalls;
        }
        if ( GUCEF_NULL == call )
            return;
        GUCEF::COMCORE::CRpcStatus status( GUCEF::COMCORE::RPC_STATUS_CANCELLED, "cancelled" );
        if ( GUCEF_NULL != eventData )
            status = *static_cast< GUCEF::COMCORE::CRpcStatus* >( eventData );
        call->Fail( status );
        Release( call );
    }

    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::vector< GUCEF::COMCORE::CRpcServerCallPtr > m_held;
    GUCEF::vector< GUCEF::COMCORE::CRpcServerCallPtr > m_reindex;
    GUCEF::vector< SWatch > m_watching;
    GUCEF::UInt64 m_nextJob;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCNOTESSERVICE_H ? */
