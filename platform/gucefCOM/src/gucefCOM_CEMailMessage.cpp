/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include <string.h>

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#include "gucefCOM_CEMailMessage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsValidContentType( const CORE::CString& type )
{
    const UInt32 size = type.ByteSize() > 0 ? type.ByteSize() - 1 : 0;
    const char* p = type.C_String();
    if ( size < 3 || size > 127 )
        return false;
    UInt32 slashes = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = p[ i ];
        if ( '/' == c )
        {
            if ( 0 == i || i + 1 == size )
                return false;
            ++slashes;
            continue;
        }
        if ( ( (UInt8) c ) <= 0x20 || ( (UInt8) c ) >= 0x7F || NULL != strchr( "()<>@,;:\\\"[]?=", c ) )
            return false;
    }
    return 1 == slashes;
}

/*-------------------------------------------------------------------------*/

bool
IsValidContentId( const CORE::CString& id )
{
    const UInt32 size = id.ByteSize() > 0 ? id.ByteSize() - 1 : 0;
    const char* p = id.C_String();
    if ( size > 200 )
        return false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c <= 0x20 || c >= 0x7F || '<' == c || '>' == c || '"' == c || '\\' == c )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IsReservedHeaderName( const CORE::CString& name )
{
    static const char* const reserved[] =
    {
        "from", "sender", "to", "cc", "bcc", "reply-to", "subject", "date", "message-id", "in-reply-to", "references",
        "mime-version", "content-type", "content-transfer-encoding", "content-disposition", "content-id", "content-length"
    };
    for ( size_t i=0; i<sizeof( reserved ) / sizeof( reserved[ 0 ] ); ++i )
        if ( name.Equals( reserved[ i ], false ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
IsWireSafeValue( const CORE::CString& value )
{
    const UInt32 size = value.ByteSize() > 0 ? value.ByteSize() - 1 : 0;
    const char* p = value.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( ( c < 0x20 && '\t' != c ) || c >= 0x7F )
            return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailMessage::CEMailMessage( void )
    : CORE::CICloneable()
    , from()
    , sender()
    , replyTo()
    , to()
    , cc()
    , bcc()
    , subject()
    , date()
    , hasDate( false )
    , messageId()
    , inReplyTo()
    , references()
    , textBody()
    , htmlBody()
    , attachments()
    , extraHeaders()
    , headers()
    , rawSize( 0 )
    , warnings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMessage::CEMailMessage( const CEMailMessage& src )
    : CORE::CICloneable( src )
    , from( src.from )
    , sender( src.sender )
    , replyTo( src.replyTo )
    , to( src.to )
    , cc( src.cc )
    , bcc( src.bcc )
    , subject( src.subject )
    , date( src.date )
    , hasDate( src.hasDate )
    , messageId( src.messageId )
    , inReplyTo( src.inReplyTo )
    , references( src.references )
    , textBody( src.textBody )
    , htmlBody( src.htmlBody )
    , attachments( src.attachments )
    , extraHeaders( src.extraHeaders )
    , headers( src.headers )
    , rawSize( src.rawSize )
    , warnings( src.warnings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMessage::~CEMailMessage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailMessage&
CEMailMessage::operator=( const CEMailMessage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        from = src.from;
        sender = src.sender;
        replyTo = src.replyTo;
        to = src.to;
        cc = src.cc;
        bcc = src.bcc;
        subject = src.subject;
        date = src.date;
        hasDate = src.hasDate;
        messageId = src.messageId;
        inReplyTo = src.inReplyTo;
        references = src.references;
        textBody = src.textBody;
        htmlBody = src.htmlBody;
        attachments = src.attachments;
        extraHeaders = src.extraHeaders;
        headers = src.headers;
        rawSize = src.rawSize;
        warnings = src.warnings;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEMailMessage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEMailMessage( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CEMailMessage::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CEMailMessage::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CEMailMessage";
}

/*-------------------------------------------------------------------------*/

void
CEMailMessage::GetEnvelopeRecipients( CEMailAddress::TEMailAddressVector& recipients ) const
{GUCEF_TRACE;

    const CEMailAddress::TEMailAddressVector* lists[ 3 ] = { &to, &cc, &bcc };
    for ( int l=0; l<3; ++l )
    {
        for ( size_t i=0; i<lists[ l ]->size(); ++i )
        {
            bool known = false;
            for ( size_t r=0; r<recipients.size() && !known; ++r )
                known = recipients[ r ].IsSameAddress( (*lists[ l ])[ i ] );
            if ( !known )
                recipients.push_back( (*lists[ l ])[ i ] );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CEMailMessage::Validate( CORE::CString& reason, bool allowUtf8Addresses ) const
{GUCEF_TRACE;

    reason.Clear();
    if ( from.empty() )
    {
        reason = "The message has no From address";
        return false;
    }
    if ( to.empty() && cc.empty() && bcc.empty() )
    {
        reason = "The message has no recipients";
        return false;
    }

    const CEMailAddress::TEMailAddressVector* lists[ 5 ] = { &from, &replyTo, &to, &cc, &bcc };
    const char* listNames[ 5 ] = { "From", "Reply-To", "To", "Cc", "Bcc" };
    for ( int l=0; l<5; ++l )
    {
        for ( size_t i=0; i<lists[ l ]->size(); ++i )
        {
            if ( !(*lists[ l ])[ i ].IsValid( allowUtf8Addresses ) )
            {
                reason = CORE::CString( "Address " ) + CORE::UInt32ToString( (UInt32) i + 1 ) + " of " + listNames[ l ] + " is not a valid address";
                return false;
            }
        }
    }
    if ( !sender.IsEmpty() && !sender.IsValid( allowUtf8Addresses ) )
    {
        reason = "Sender is not a valid address";
        return false;
    }

    if ( !messageId.IsNULLOrEmpty() && !CMimeCodec::IsValidMessageId( messageId ) )
    {
        reason = "The message id is not valid";
        return false;
    }
    for ( size_t i=0; i<inReplyTo.size(); ++i )
        if ( !CMimeCodec::IsValidMessageId( inReplyTo[ i ] ) )
        {
            reason = "An id in In-Reply-To is not valid";
            return false;
        }
    for ( size_t i=0; i<references.size(); ++i )
        if ( !CMimeCodec::IsValidMessageId( references[ i ] ) )
        {
            reason = "An id in References is not valid";
            return false;
        }

    for ( UInt32 i=0; i<extraHeaders.GetCount(); ++i )
    {
        const CEMailHeaders::TField& field = extraHeaders.GetField( i );
        if ( !CMimeCodec::IsValidHeaderName( field.name ) || CMimeCodec::HasLineBreakOrNul( field.value ) )
        {
            reason = "An extra header has an invalid name or value";
            return false;
        }
        if ( IsReservedHeaderName( field.name ) )
        {
            reason = "An extra header is a field that the composer writes itself";
            return false;
        }
        if ( !IsWireSafeValue( field.value ) )
        {
            reason = "An extra header value is not encoded (use AddText)";
            return false;
        }
    }

    for ( size_t i=0; i<attachments.size(); ++i )
    {
        const CEMailAttachment& attachment = attachments[ i ];
        if ( !IsValidContentType( attachment.contentType ) )
        {
            reason = CORE::CString( "Attachment " ) + CORE::UInt32ToString( (UInt32) i + 1 ) + " has an invalid content type";
            return false;
        }
        if ( CMimeCodec::HasLineBreakOrNul( attachment.fileName ) )
        {
            reason = CORE::CString( "Attachment " ) + CORE::UInt32ToString( (UInt32) i + 1 ) + " has a file name with a line break";
            return false;
        }
        if ( !IsValidContentId( attachment.contentId ) )
        {
            reason = CORE::CString( "Attachment " ) + CORE::UInt32ToString( (UInt32) i + 1 ) + " has an invalid content id";
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

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
