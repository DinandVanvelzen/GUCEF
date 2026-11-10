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

#include "gucefCOM_CMimeCodec.h"

#include "gucefCOM_CMimeComposer.h"

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

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

/** One entity of the message that is being built */
struct SNode
{
    enum EKind { KIND_TEXT, KIND_ATTACHMENT, KIND_MULTI };

    EKind kind;
    CORE::CString subtype;                  /**< plain or html for text, alternative, related or mixed for a multipart */
    CORE::CString text;                     /**< KIND_TEXT: the text with CR LF line ends, ending in a line break */
    const CEMailAttachment* attachment;
    GUCEF::vector< SNode* > children;

    SNode( EKind nodeKind ) : kind( nodeKind ), subtype(), text(), attachment( GUCEF_NULL ), children() {}
};

/*-------------------------------------------------------------------------*/

/** Owns the nodes of a message */
class CNodePool
{
    public:

    CNodePool( void ) : m_nodes() {}

    ~CNodePool()
    {
        for ( size_t i=0; i<m_nodes.size(); ++i )
            delete m_nodes[ i ];
    }

    SNode* Make( SNode::EKind kind )
    {
        SNode* node = new SNode( kind );
        m_nodes.push_back( node );
        return node;
    }

    private:

    CNodePool( const CNodePool& );
    CNodePool& operator=( const CNodePool& );

    GUCEF::vector< SNode* > m_nodes;
};

/*-------------------------------------------------------------------------*/

/** CR LF line ends for every kind of line end, ending in a line break unless empty */
CORE::CString
NormalizeLineEnds( const CORE::CString& text )
{
    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    GUCEF::vector< char > out;
    out.reserve( size + size / 20 + 2 );
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '\r' == p[ i ] )
        {
            if ( i + 1 < size && '\n' == p[ i + 1 ] )
                ++i;
            out.push_back( '\r' );
            out.push_back( '\n' );
        }
        else
        if ( '\n' == p[ i ] )
        {
            out.push_back( '\r' );
            out.push_back( '\n' );
        }
        else
            out.push_back( p[ i ] );
    }
    if ( !out.empty() && !( out.size() >= 2 && '\r' == out[ out.size() - 2 ] && '\n' == out[ out.size() - 1 ] ) )
    {
        out.push_back( '\r' );
        out.push_back( '\n' );
    }
    return out.empty() ? CORE::CString() : CORE::CString( &out[ 0 ], (UInt32) out.size() );
}

/*-------------------------------------------------------------------------*/

bool
ContainsText( const CORE::CString& haystack, const CORE::CString& needle )
{
    return haystack.HasSubstr( needle ) >= 0;
}

/*-------------------------------------------------------------------------*/

class CWriter
{
    public:

    CWriter( CORE::CDynamicBuffer& out, UInt32 maxSize ) : m_out( out ), m_maxSize( maxSize ), m_error() {}

    bool Header( const CORE::CString& name, const CORE::CString& value )
    {
        if ( !CMimeCodec::AppendHeaderField( m_out, name, value ) )
        {
            m_error = "A header could not be written";
            return false;
        }
        return Check();
    }

    void Raw( const CORE::CString& text )
    {
        if ( ByteLength( text ) > 0 )
            m_out.Append( text.C_String(), ByteLength( text ), true );
    }

    void Raw( const char* text )
    {
        m_out.Append( text, (UInt32) strlen( text ), true );
    }

    bool Check( void )
    {
        if ( m_out.GetDataSize() > m_maxSize )
        {
            m_error = "The message is larger than the limit";
            return false;
        }
        return true;
    }

    CORE::CDynamicBuffer& Buffer( void ) { return m_out; }

    const CORE::CString& GetError( void ) const { return m_error; }

    void SetError( const char* error ) { m_error = error; }

    private:

    CORE::CDynamicBuffer& m_out;
    UInt32 m_maxSize;
    CORE::CString m_error;
};

/*-------------------------------------------------------------------------*/

bool
WriteNode( CWriter& writer, const SNode& node, bool isTop );

/*-------------------------------------------------------------------------*/

bool
TextNodeContainsBoundary( const SNode& node, const CORE::CString& marker )
{
    if ( SNode::KIND_TEXT == node.kind && ContainsText( node.text, marker ) )
        return true;
    for ( size_t i=0; i<node.children.size(); ++i )
        if ( TextNodeContainsBoundary( *node.children[ i ], marker ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
WriteTextNode( CWriter& writer, const SNode& node )
{
    const char* p = node.text.C_String();
    const UInt32 size = ByteLength( node.text );

    UInt32 nonAscii = 0;
    UInt32 controls = 0;
    UInt32 lineLength = 0;
    UInt32 maxLine = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c >= 0x80 ) ++nonAscii;
        else
        if ( ( c < 0x20 && '\t' != c && '\r' != c && '\n' != c ) || 0x7F == c ) ++controls;
        if ( '\n' == c ) lineLength = 0;
        else if ( ++lineLength > maxLine ) maxLine = lineLength;
    }

    const char* encoding = "7bit";
    if ( controls > 0 || nonAscii * 3 > size )
        encoding = "base64";
    else
    if ( nonAscii > 0 || maxLine > 998 )
        encoding = "quoted-printable";

    if ( !writer.Header( "Content-Type", "text/" + node.subtype + "; charset=utf-8" ) )
        return false;
    if ( !writer.Header( "Content-Transfer-Encoding", encoding ) )
        return false;
    writer.Raw( "\r\n" );

    if ( 0 == strcmp( encoding, "base64" ) )
        CMimeCodec::AppendBase64Wrapped( writer.Buffer(), p, size );
    else
    if ( 0 == strcmp( encoding, "quoted-printable" ) )
        CMimeCodec::AppendQuotedPrintable( writer.Buffer(), p, size );
    else
        writer.Raw( node.text );
    return writer.Check();
}

/*-------------------------------------------------------------------------*/

bool
WriteAttachmentNode( CWriter& writer, const SNode& node )
{
    const CEMailAttachment& attachment = *node.attachment;
    CORE::CString type = attachment.contentType;
    if ( !attachment.fileName.IsNULLOrEmpty() )
        type += "; " + CMimeCodec::FormatParameter( "name", attachment.fileName );
    if ( !writer.Header( "Content-Type", type ) )
        return false;
    if ( !writer.Header( "Content-Transfer-Encoding", "base64" ) )
        return false;

    CORE::CString disposition = attachment.isInline ? "inline" : "attachment";
    if ( !attachment.fileName.IsNULLOrEmpty() )
        disposition += "; " + CMimeCodec::FormatParameter( "filename", attachment.fileName );
    if ( !writer.Header( "Content-Disposition", disposition ) )
        return false;
    if ( !attachment.contentId.IsNULLOrEmpty() && !writer.Header( "Content-ID", "<" + attachment.contentId + ">" ) )
        return false;
    writer.Raw( "\r\n" );

    CMimeCodec::AppendBase64Wrapped( writer.Buffer(), attachment.data.GetConstBufferPtr(), attachment.data.GetDataSize() );
    return writer.Check();
}

/*-------------------------------------------------------------------------*/

bool
WriteMultiNode( CWriter& writer, const SNode& node )
{
    // a boundary that none of the text parts has in it
    CORE::CString boundary;
    for ( int attempt=0; attempt<5; ++attempt )
    {
        boundary = CMimeCodec::GenerateBoundary();
        if ( !TextNodeContainsBoundary( node, "--" + boundary ) )
            break;
        boundary.Clear();
    }
    if ( boundary.IsNULLOrEmpty() )
    {
        writer.SetError( "No boundary could be found that is not in the text" );
        return false;
    }

    if ( !writer.Header( "Content-Type", "multipart/" + node.subtype + "; boundary=" + boundary ) )
        return false;
    writer.Raw( "\r\n" );
    for ( size_t i=0; i<node.children.size(); ++i )
    {
        writer.Raw( "--" + boundary + "\r\n" );
        if ( !WriteNode( writer, *node.children[ i ], false ) )
            return false;
    }
    writer.Raw( "--" + boundary + "--\r\n" );
    return writer.Check();
}

/*-------------------------------------------------------------------------*/

bool
WriteNode( CWriter& writer, const SNode& node, bool isTop )
{
    switch ( node.kind )
    {
        case SNode::KIND_TEXT: return WriteTextNode( writer, node );
        case SNode::KIND_ATTACHMENT: return WriteAttachmentNode( writer, node );
        default: return WriteMultiNode( writer, node );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMimeComposer::CMimeComposer( void )
    : m_options()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMimeComposer::CMimeComposer( const CMimeComposeOptions& options )
    : m_options( options )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMimeComposer::SetOptions( const CMimeComposeOptions& options )
{GUCEF_TRACE;

    m_options = options;
}

/*-------------------------------------------------------------------------*/

const CMimeComposeOptions&
CMimeComposer::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

bool
CMimeComposer::Compose( const CEMailMessage& message, CORE::CDynamicBuffer& out, CORE::CString& error ) const
{GUCEF_TRACE;

    error.Clear();
    if ( !message.Validate( error, m_options.allowUtf8Addresses ) )
        return false;

    // written into a buffer of our own so that a failure leaves 'out' as it was
    CORE::CDynamicBuffer result;
    result.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    CWriter writer( result, m_options.maxSize );

    CORE::CString value;
    CORE::CDateTime date = message.hasDate ? message.date : CORE::CDateTime::NowUTCDateTime();
    if ( !writer.Header( "Date", CMimeCodec::FormatDate( date ) ) )
    {
        error = writer.GetError();
        return false;
    }

    if ( !CEMailAddress::FormatList( message.from, value, m_options.allowUtf8Addresses ) || !writer.Header( "From", value ) )
    {
        error = "The From header could not be written";
        return false;
    }
    if ( !message.sender.IsEmpty() )
    {
        if ( !message.sender.Format( value, m_options.allowUtf8Addresses ) || !writer.Header( "Sender", value ) )
        {
            error = "The Sender header could not be written";
            return false;
        }
    }
    if ( !message.to.empty() && ( !CEMailAddress::FormatList( message.to, value, m_options.allowUtf8Addresses ) || !writer.Header( "To", value ) ) )
    {
        error = "The To header could not be written";
        return false;
    }
    if ( !message.cc.empty() && ( !CEMailAddress::FormatList( message.cc, value, m_options.allowUtf8Addresses ) || !writer.Header( "Cc", value ) ) )
    {
        error = "The Cc header could not be written";
        return false;
    }
    if ( !message.replyTo.empty() && ( !CEMailAddress::FormatList( message.replyTo, value, m_options.allowUtf8Addresses ) || !writer.Header( "Reply-To", value ) ) )
    {
        error = "The Reply-To header could not be written";
        return false;
    }

    CORE::CString messageId = message.messageId;
    if ( messageId.IsNULLOrEmpty() )
        messageId = CMimeCodec::GenerateMessageId( message.from[ 0 ].domain );
    if ( !writer.Header( "Message-ID", messageId ) )
    {
        error = writer.GetError();
        return false;
    }
    if ( !message.inReplyTo.empty() )
    {
        value.Clear();
        for ( size_t i=0; i<message.inReplyTo.size(); ++i )
            value += ( i > 0 ? " " : "" ) + message.inReplyTo[ i ];
        if ( !writer.Header( "In-Reply-To", value ) )
        {
            error = writer.GetError();
            return false;
        }
    }
    if ( !message.references.empty() )
    {
        value.Clear();
        for ( size_t i=0; i<message.references.size(); ++i )
            value += ( i > 0 ? " " : "" ) + message.references[ i ];
        if ( !writer.Header( "References", value ) )
        {
            error = writer.GetError();
            return false;
        }
    }
    if ( !writer.Header( "Subject", CMimeCodec::EncodeHeaderText( message.subject ) ) )
    {
        error = writer.GetError();
        return false;
    }
    if ( !writer.Header( "MIME-Version", "1.0" ) )
    {
        error = writer.GetError();
        return false;
    }
    for ( UInt32 i=0; i<message.extraHeaders.GetCount(); ++i )
    {
        const CEMailHeaders::TField& field = message.extraHeaders.GetField( i );
        if ( !writer.Header( field.name, field.value ) )
        {
            error = writer.GetError();
            return false;
        }
    }

    // the structure
    CNodePool pool;
    SNode* textNode = GUCEF_NULL;
    SNode* htmlNode = GUCEF_NULL;
    if ( !message.textBody.IsNULLOrEmpty() || message.htmlBody.IsNULLOrEmpty() )
    {
        textNode = pool.Make( SNode::KIND_TEXT );
        textNode->subtype = "plain";
        textNode->text = NormalizeLineEnds( message.textBody );
    }
    if ( !message.htmlBody.IsNULLOrEmpty() )
    {
        htmlNode = pool.Make( SNode::KIND_TEXT );
        htmlNode->subtype = "html";
        htmlNode->text = NormalizeLineEnds( message.htmlBody );
    }

    // inline content that is referenced by content id belongs with the HTML
    GUCEF::vector< const CEMailAttachment* > related;
    GUCEF::vector< const CEMailAttachment* > regular;
    for ( size_t i=0; i<message.attachments.size(); ++i )
    {
        if ( GUCEF_NULL != htmlNode && message.attachments[ i ].isInline && !message.attachments[ i ].contentId.IsNULLOrEmpty() )
            related.push_back( &message.attachments[ i ] );
        else
            regular.push_back( &message.attachments[ i ] );
    }

    SNode* body = textNode;
    if ( GUCEF_NULL != htmlNode )
    {
        SNode* htmlPart = htmlNode;
        if ( !related.empty() )
        {
            SNode* relatedNode = pool.Make( SNode::KIND_MULTI );
            relatedNode->subtype = "related";
            relatedNode->children.push_back( htmlNode );
            for ( size_t i=0; i<related.size(); ++i )
            {
                SNode* attachmentNode = pool.Make( SNode::KIND_ATTACHMENT );
                attachmentNode->attachment = related[ i ];
                relatedNode->children.push_back( attachmentNode );
            }
            htmlPart = relatedNode;
        }
        if ( GUCEF_NULL != textNode )
        {
            body = pool.Make( SNode::KIND_MULTI );
            body->subtype = "alternative";
            body->children.push_back( textNode );
            body->children.push_back( htmlPart );
        }
        else
            body = htmlPart;
    }

    SNode* root = body;
    if ( !regular.empty() )
    {
        root = pool.Make( SNode::KIND_MULTI );
        root->subtype = "mixed";
        root->children.push_back( body );
        for ( size_t i=0; i<regular.size(); ++i )
        {
            SNode* attachmentNode = pool.Make( SNode::KIND_ATTACHMENT );
            attachmentNode->attachment = regular[ i ];
            root->children.push_back( attachmentNode );
        }
    }

    if ( !WriteNode( writer, *root, true ) )
    {
        error = writer.GetError().IsNULLOrEmpty() ? CORE::CString( "The message could not be written" ) : writer.GetError();
        return false;
    }

    out.Append( result, true );
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
