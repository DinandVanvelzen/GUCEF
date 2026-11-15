/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CJobTrigger.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CJobTrigger::ClassTypeName = "GUCEF::CORE::CJobTrigger";

/*-------------------------------------------------------------------------*/

static CString
UtcMsToIso8601( Int64 utcMs )
{GUCEF_TRACE;

    CDateTime dt;
    dt.FromUnixEpochBasedTicksInMillisecs( (UInt64) ( utcMs < 0 ? 0 : utcMs ) );
    return dt.ToIso8601DateTimeString( true, true );
}

/*-------------------------------------------------------------------------*/

static bool
JobTriggerKindFromString( const CString& text, TJobTriggerKind& kind )
{GUCEF_TRACE;

    static const TJobTriggerKind kinds[] = { JOBTRIGGER_SCHEDULE, JOBTRIGGER_INTERVAL, JOBTRIGGER_ONCE_AT, JOBTRIGGER_MANUAL, JOBTRIGGER_WORK_SOURCE };
    for ( UInt32 i=0; i<sizeof(kinds)/sizeof(kinds[0]); ++i )
    {
        if ( text.Equals( JobTriggerKindToString( kinds[ i ] ), false ) )
        {
            kind = kinds[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CJobTrigger::CJobTrigger( void )
    : CIDataNodeSerializable()
    , m_kind( JOBTRIGGER_MANUAL )
    , m_schedule()
    , m_periodMs( 0 )
    , m_intervalFrom( JOBINTERVALFROM_START )
    , m_hasAlignment( false )
    , m_alignToUtcMs( 0 )
    , m_onceAtUtcMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobTrigger::~CJobTrigger()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobTrigger
CJobTrigger::Schedule( const CSchedule& schedule )
{GUCEF_TRACE;

    CJobTrigger trigger;
    trigger.m_kind = JOBTRIGGER_SCHEDULE;
    trigger.m_schedule = schedule;
    return trigger;
}

/*-------------------------------------------------------------------------*/

CJobTrigger
CJobTrigger::Interval( UInt64 periodMs, TJobIntervalFrom intervalFrom )
{GUCEF_TRACE;

    CJobTrigger trigger;
    trigger.m_kind = JOBTRIGGER_INTERVAL;
    trigger.m_periodMs = periodMs;
    trigger.m_intervalFrom = intervalFrom;
    return trigger;
}

/*-------------------------------------------------------------------------*/

CJobTrigger
CJobTrigger::OnceAt( Int64 utcMs )
{GUCEF_TRACE;

    CJobTrigger trigger;
    trigger.m_kind = JOBTRIGGER_ONCE_AT;
    trigger.m_onceAtUtcMs = utcMs;
    return trigger;
}

/*-------------------------------------------------------------------------*/

CJobTrigger
CJobTrigger::Manual( void )
{GUCEF_TRACE;

    return CJobTrigger();
}

/*-------------------------------------------------------------------------*/

CJobTrigger
CJobTrigger::WorkSource( void )
{GUCEF_TRACE;

    CJobTrigger trigger;
    trigger.m_kind = JOBTRIGGER_WORK_SOURCE;
    return trigger;
}

/*-------------------------------------------------------------------------*/

CString
CJobTrigger::GetDescription( void ) const
{GUCEF_TRACE;

    switch ( m_kind )
    {
        case JOBTRIGGER_SCHEDULE:
        {
            CString text;
            if ( m_schedule.HasCronSchedule() )
                text = "cron " + m_schedule.GetCronSchedule().ToCronString() + " UTC";
            if ( m_schedule.HasTimeRange() )
            {
                if ( !text.IsNULLOrEmpty() )
                    text += ", ";
                text += "from " + m_schedule.GetTimeRange().GetStart().ToIso8601DateTimeString( true, false ) +
                        " until " + m_schedule.GetTimeRange().GetEnd().ToIso8601DateTimeString( true, false );
            }
            return text.IsNULLOrEmpty() ? CString( "schedule" ) : text;
        }
        case JOBTRIGGER_INTERVAL:
        {
            CString text = "every " + ToString( m_periodMs ) + " ms";
            if ( JOBINTERVALFROM_END == m_intervalFrom )
                text += " after the end of the run";
            if ( m_hasAlignment )
                text += ", aligned to " + UtcMsToIso8601( m_alignToUtcMs );
            return text;
        }
        case JOBTRIGGER_ONCE_AT:        { return "once at " + UtcMsToIso8601( m_onceAtUtcMs ); }
        case JOBTRIGGER_MANUAL:         { return "manual"; }
        case JOBTRIGGER_WORK_SOURCE:    { return "work source"; }
        default:                        { return "unspecified"; }
    }
}

/*-------------------------------------------------------------------------*/

bool
CJobTrigger::IsValid( CString& error ) const
{GUCEF_TRACE;

    error.Clear();
    switch ( m_kind )
    {
        case JOBTRIGGER_SCHEDULE:
        {
            if ( !m_schedule.HasCronSchedule() && !m_schedule.HasTimeRange() )
            {
                error = "a schedule trigger needs a cron expression or a date time range";
                return false;
            }
            if ( !m_schedule.IsValid() )
            {
                error = "the schedule of the trigger is not valid";
                return false;
            }
            return true;
        }
        case JOBTRIGGER_INTERVAL:
        {
            if ( 0 == m_periodMs )
            {
                error = "an interval trigger needs a period above 0 ms";
                return false;
            }
            if ( JOBINTERVALFROM_START != m_intervalFrom && JOBINTERVALFROM_END != m_intervalFrom )
            {
                error = "an interval trigger is measured from START or END";
                return false;
            }
            return true;
        }
        case JOBTRIGGER_ONCE_AT:
        {
            if ( m_onceAtUtcMs <= 0 )
            {
                error = "a once-at trigger needs an instant (UTC ms since 1970)";
                return false;
            }
            return true;
        }
        case JOBTRIGGER_MANUAL:
        case JOBTRIGGER_WORK_SOURCE:
            return true;
        default:
        {
            error = "the trigger kind is not specified";
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CJobTrigger::operator==( const CJobTrigger& other ) const
{GUCEF_TRACE;

    return m_kind == other.m_kind                 &&
           m_schedule == other.m_schedule         &&
           m_periodMs == other.m_periodMs         &&
           m_intervalFrom == other.m_intervalFrom &&
           m_hasAlignment == other.m_hasAlignment &&
           m_alignToUtcMs == other.m_alignToUtcMs &&
           m_onceAtUtcMs == other.m_onceAtUtcMs;
}

/*-------------------------------------------------------------------------*/

bool
CJobTrigger::Serialize( CDataNode& domRootNode                        ,
                        const CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    domRootNode.SetNodeType( GUCEF_DATATYPE_OBJECT );
    totalSuccess = domRootNode.SetAttribute( "kind", JobTriggerKindToString( m_kind ) ) && totalSuccess;
    switch ( m_kind )
    {
        case JOBTRIGGER_SCHEDULE:
        {
            if ( m_schedule.HasCronSchedule() )
                totalSuccess = domRootNode.SetAttribute( "cron", m_schedule.GetCronSchedule().ToCronString() ) && totalSuccess;
            if ( m_schedule.HasTimeRange() )
            {
                totalSuccess = domRootNode.SetAttribute( "rangeStartUtc", m_schedule.GetTimeRange().GetStart().ToIso8601DateTimeString( true, true ) ) && totalSuccess;
                totalSuccess = domRootNode.SetAttribute( "rangeEndUtc", m_schedule.GetTimeRange().GetEnd().ToIso8601DateTimeString( true, true ) ) && totalSuccess;
            }
            break;
        }
        case JOBTRIGGER_INTERVAL:
        {
            totalSuccess = domRootNode.SetAttribute( "periodMs", m_periodMs ) && totalSuccess;
            totalSuccess = domRootNode.SetAttribute( "intervalFrom", JOBINTERVALFROM_END == m_intervalFrom ? "END" : "START" ) && totalSuccess;
            if ( m_hasAlignment )
                totalSuccess = domRootNode.SetAttribute( "alignToUtcMs", m_alignToUtcMs ) && totalSuccess;
            break;
        }
        case JOBTRIGGER_ONCE_AT:
        {
            totalSuccess = domRootNode.SetAttribute( "onceAtUtcMs", m_onceAtUtcMs ) && totalSuccess;
            break;
        }
        default:
            break;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CJobTrigger::Deserialize( const CDataNode& domRootNode                  ,
                          const CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CJobTrigger trigger;
    CString kindText = domRootNode.GetAttributeValueOrChildValueByName( "kind" ).AsString( CString::Empty, true );
    if ( !JobTriggerKindFromString( kindText, trigger.m_kind ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CJobTrigger:Deserialize: unknown trigger kind \"" << kindText << "\"" );
        return false;
    }

    switch ( trigger.m_kind )
    {
        case JOBTRIGGER_SCHEDULE:
        {
            CString cronText = domRootNode.GetAttributeValueOrChildValueByName( "cron" ).AsString( CString::Empty, true );
            if ( !cronText.IsNULLOrEmpty() )
            {
                CCronSchedule cron;
                if ( !cron.FromCronString( cronText ) )
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CJobTrigger:Deserialize: \"" << cronText << "\" is not a cron expression" );
                    return false;
                }
                trigger.m_schedule.SetCronSchedule( cron );
            }
            CString startText = domRootNode.GetAttributeValueOrChildValueByName( "rangeStartUtc" ).AsString( CString::Empty, true );
            CString endText = domRootNode.GetAttributeValueOrChildValueByName( "rangeEndUtc" ).AsString( CString::Empty, true );
            if ( !startText.IsNULLOrEmpty() || !endText.IsNULLOrEmpty() )
            {
                CDateTime start, end;
                if ( !start.FromIso8601DateTimeString( startText ) || !end.FromIso8601DateTimeString( endText ) )
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CJobTrigger:Deserialize: the range \"" << startText << "\" - \"" << endText << "\" is not two ISO 8601 instants" );
                    return false;
                }
                trigger.m_schedule.SetTimeRange( CDateTimeRange( start, end ) );
            }
            break;
        }
        case JOBTRIGGER_INTERVAL:
        {
            trigger.m_periodMs = domRootNode.GetAttributeValueOrChildValueByName( "periodMs" ).AsUInt64( 0, true );
            CString fromText = domRootNode.GetAttributeValueOrChildValueByName( "intervalFrom" ).AsString( "START", true );
            trigger.m_intervalFrom = fromText.Equals( "END", false ) ? JOBINTERVALFROM_END : JOBINTERVALFROM_START;
            CVariant align = domRootNode.GetAttributeValueOrChildValueByName( "alignToUtcMs" );
            if ( !align.IsNULLOrEmpty() )
            {
                trigger.m_hasAlignment = true;
                trigger.m_alignToUtcMs = align.AsInt64( 0, true );
            }
            break;
        }
        case JOBTRIGGER_ONCE_AT:
        {
            trigger.m_onceAtUtcMs = domRootNode.GetAttributeValueOrChildValueByName( "onceAtUtcMs" ).AsInt64( 0, true );
            break;
        }
        default:
            break;
    }

    *this = trigger;
    return true;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CJobTrigger::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CJobTrigger( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CJobTrigger::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CJobTrigger::SetKind( TJobTriggerKind kind )
{GUCEF_TRACE;

    m_kind = kind;
}

/*-------------------------------------------------------------------------*/

TJobTriggerKind
CJobTrigger::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

void
CJobTrigger::SetSchedule( const CSchedule& schedule )
{GUCEF_TRACE;

    m_schedule = schedule;
}

/*-------------------------------------------------------------------------*/

const CSchedule&
CJobTrigger::GetSchedule( void ) const
{GUCEF_TRACE;

    return m_schedule;
}

/*-------------------------------------------------------------------------*/

void
CJobTrigger::SetPeriodMs( UInt64 periodMs )
{GUCEF_TRACE;

    m_periodMs = periodMs;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobTrigger::GetPeriodMs( void ) const
{GUCEF_TRACE;

    return m_periodMs;
}

/*-------------------------------------------------------------------------*/

void
CJobTrigger::SetIntervalFrom( TJobIntervalFrom intervalFrom )
{GUCEF_TRACE;

    m_intervalFrom = intervalFrom;
}

/*-------------------------------------------------------------------------*/

TJobIntervalFrom
CJobTrigger::GetIntervalFrom( void ) const
{GUCEF_TRACE;

    return m_intervalFrom;
}

/*-------------------------------------------------------------------------*/

void
CJobTrigger::SetHasAlignment( bool hasAlignment )
{GUCEF_TRACE;

    m_hasAlignment = hasAlignment;
}

/*-------------------------------------------------------------------------*/

bool
CJobTrigger::GetHasAlignment( void ) const
{GUCEF_TRACE;

    return m_hasAlignment;
}

/*-------------------------------------------------------------------------*/

void
CJobTrigger::SetAlignToUtcMs( Int64 alignToUtcMs )
{GUCEF_TRACE;

    m_alignToUtcMs = alignToUtcMs;
}

/*-------------------------------------------------------------------------*/

Int64
CJobTrigger::GetAlignToUtcMs( void ) const
{GUCEF_TRACE;

    return m_alignToUtcMs;
}

/*-------------------------------------------------------------------------*/

void
CJobTrigger::SetOnceAtUtcMs( Int64 onceAtUtcMs )
{GUCEF_TRACE;

    m_onceAtUtcMs = onceAtUtcMs;
}

/*-------------------------------------------------------------------------*/

Int64
CJobTrigger::GetOnceAtUtcMs( void ) const
{GUCEF_TRACE;

    return m_onceAtUtcMs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
