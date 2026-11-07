/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMAILADDRESS_H
#include "gucefCOM_CMailAddress.h"
#define GUCEF_COM_CMAILADDRESS_H
#endif /* GUCEF_COM_CMAILADDRESS_H ? */

#include "matchprovidermail_CMailFiles.h"
#include "matchprovidermail_CMailIntroductions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CMailIntroductions::StateFileName = "introductions.state";
const char* const CMailIntroductions::BookFileName = "addressbook.state";

static const UInt32 g_maxChildren = 64;
static const UInt32 g_maxSettings = 64;
static const UInt32 g_maxInbound = 256;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
Lower( const CORE::CString& text )
{GUCEF_TRACE;

    return text.Lowercase();
}

/*-------------------------------------------------------------------------*/

static bool
IsUsableAddress( const CORE::CString& address )
{GUCEF_TRACE;

    if ( address.IsNULLOrEmpty() || address.Length() > CMailIntroRequest::MaxAddressBytes )
        return false;
    return COM::CMailAddress( address ).IsValid();
}

/*-------------------------------------------------------------------------*/

/** A name or a title for a person: controls become blanks, the ends are trimmed, it is cut on a character boundary */
static CORE::CString
CleanText( const CORE::CString& text, UInt32 maxBytes )
{GUCEF_TRACE;

    std::string clean;
    const UInt32 size = text.Length();
    clean.reserve( size < maxBytes + 4 ? size : maxBytes + 4 );
    for ( UInt32 i=0; i<size && clean.size() < maxBytes + 4; ++i )
    {
        const unsigned char c = (unsigned char) text[ i ];
        clean += ( c < 0x20 || 0x7F == c ) ? ' ' : (char) c;
    }
    size_t first = clean.find_first_not_of( ' ' );
    if ( std::string::npos == first )
        return CORE::CString::Empty;
    size_t last = clean.find_last_not_of( ' ' );
    clean = clean.substr( first, last - first + 1 );
    if ( clean.size() > maxBytes )
    {
        size_t cut = maxBytes;
        while ( cut > 0 && 0x80 == ( (unsigned char) clean[ cut ] & 0xC0 ) )
            --cut;
        clean.resize( cut );
    }
    return clean.c_str();
}

/*-------------------------------------------------------------------------*/

static bool
IsNonce( const CORE::CString& nonce )
{GUCEF_TRACE;

    return MULTIPLAY::CMatchSigning::IsHexOfLength( nonce, CMailIntroRequest::NonceHexSize );
}

/** Nonces are compared in time that does not depend on where they differ */
static bool
NonceEquals( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    if ( !IsNonce( a ) || !IsNonce( b ) )
        return false;
    return CORE::CSha256::ConstantTimeEquals( a.Lowercase().C_String(), b.Lowercase().C_String(), CMailIntroRequest::NonceHexSize );
}

/*-------------------------------------------------------------------------*/

static void
LogRefusal( const char* where, TMailIntroResult result )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "MailIntroductions: " << where << " was not taken: " << MailIntroResultToString( result ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      STORED STATE                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The drafts and the requests of other players as they are written to the state file */
class CMailIntroState : public CORE::CIDataNodeSerializable
{
    public:

    GUCEF::vector< CMailIntroDraft > drafts;
    GUCEF::vector< CMailInboundIntro > inbound;

    CMailIntroState( void ) : CORE::CIDataNodeSerializable(), drafts(), inbound() {}

    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDataNode* draftsNode = domRootNode.AddChild( "drafts", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<drafts.size(); ++i )
        {
            const CMailIntroDraft& draft = drafts[ i ];
            CORE::CDataNode* node = draftsNode->AddChild( "draft", GUCEF_DATATYPE_OBJECT );
            node->SetAttribute( "matchId", draft.matchId );
            node->SetAttribute( "title", draft.title );
            node->SetAttribute( "creatorName", draft.creatorName );
            node->SetAttribute( "creatorAddress", draft.creatorAddress );
            node->SetAttribute( "createdAt", draft.createdAt );
            CORE::CDataNode* settingsNode = node->AddChild( "settings", GUCEF_DATATYPE_OBJECT );
            for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = draft.settings.begin(); s != draft.settings.end(); ++s )
                settingsNode->SetAttribute( (*s).first, (*s).second );
            CORE::CDataNode* contactsNode = node->AddChild( "contacts", GUCEF_DATATYPE_ARRAY );
            for ( size_t c=0; c<draft.contacts.size(); ++c )
            {
                const CMailIntroContact& contact = draft.contacts[ c ];
                CORE::CDataNode* cn = contactsNode->AddChild( "contact", GUCEF_DATATYPE_OBJECT );
                cn->SetAttribute( "name", contact.name );
                cn->SetAttribute( "address", contact.address );
                cn->SetAttribute( "nonce", contact.nonce );
                cn->SetAttribute( "state", CORE::CString( MailIntroStateToString( contact.state ) ) );
                cn->SetAttribute( "source", (UInt32) contact.source );
                cn->SetAttribute( "publicKey", contact.publicKey );
                cn->SetAttribute( "displayName", contact.displayName );
                cn->SetAttribute( "codeKey", contact.codeKey );
                cn->SetAttribute( "pendingKey", contact.pendingKey );
                cn->SetAttribute( "pendingName", contact.pendingName );
                cn->SetAttribute( "requestedAt", contact.requestedAt );
                cn->SetAttribute( "expiresAt", contact.expiresAt );
                cn->SetAttribute( "lastSentAt", contact.lastSentAt );
                cn->SetAttribute( "sendCount", contact.sendCount );
            }
        }
        CORE::CDataNode* inboundNode = domRootNode.AddChild( "inbound", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<inbound.size(); ++i )
        {
            CORE::CDataNode* node = inboundNode->AddChild( "intro", GUCEF_DATATYPE_OBJECT );
            inbound[ i ].request.Serialize( *node->AddChild( "request", GUCEF_DATATYPE_OBJECT ), settings );
            node->SetAttribute( "fromAddress", inbound[ i ].fromAddress );
            node->SetAttribute( "receivedAt", inbound[ i ].receivedAt );
            node->SetAttribute( "status", (UInt32) inbound[ i ].status );
            node->SetAttribute( "keyChanged", inbound[ i ].keyChanged );
            node->SetAttribute( "knownKey", inbound[ i ].knownKey );
            node->SetAttribute( "lastReplyAt", inbound[ i ].lastReplyAt );
        }
        return true;
    }

    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE
    {
        drafts.clear();
        inbound.clear();
        const CORE::CDataNode* draftsNode = domRootNode.FindChild( "drafts" );
        const CORE::CDataNode* inboundNode = domRootNode.FindChild( "inbound" );
        if ( GUCEF_NULL == draftsNode || GUCEF_NULL == inboundNode )
            return false;
        for ( CORE::CDataNode::const_iterator i = draftsNode->ConstBegin(); i != draftsNode->ConstEnd(); ++i )
        {
            if ( drafts.size() >= g_maxChildren )
                return false;
            CMailIntroDraft draft;
            draft.matchId = (*i)->GetAttributeValueOrChildValueByName( "matchId" ).AsString( CORE::CString::Empty, true );
            draft.title = (*i)->GetAttributeValueOrChildValueByName( "title" ).AsString( CORE::CString::Empty, true );
            draft.creatorName = (*i)->GetAttributeValueOrChildValueByName( "creatorName" ).AsString( CORE::CString::Empty, true );
            draft.creatorAddress = (*i)->GetAttributeValueOrChildValueByName( "creatorAddress" ).AsString( CORE::CString::Empty, true );
            draft.createdAt = (*i)->GetAttributeValueOrChildValueByName( "createdAt" ).AsInt64( 0, true );
            if ( !MULTIPLAY::CMatchId::IsValid( draft.matchId ) || !IsUsableAddress( draft.creatorAddress ) )
                return false;
            const CORE::CDataNode* settingsNode = (*i)->FindChild( "settings" );
            if ( GUCEF_NULL != settingsNode )
            {
                CORE::CDataNode::TAttributeMap::const_iterator a = settingsNode->AttributeBegin();
                while ( a != settingsNode->AttributeEnd() && draft.settings.size() < g_maxSettings )
                {
                    draft.settings[ (*a).first ] = (*a).second.AsString( CORE::CString::Empty, true );
                    ++a;
                }
            }
            const CORE::CDataNode* contactsNode = (*i)->FindChild( "contacts" );
            if ( GUCEF_NULL == contactsNode )
                return false;
            for ( CORE::CDataNode::const_iterator c = contactsNode->ConstBegin(); c != contactsNode->ConstEnd(); ++c )
            {
                if ( draft.contacts.size() >= CMailIntroRequest::MaxParties )
                    return false;
                CMailIntroContact contact;
                contact.name = (*c)->GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
                contact.address = (*c)->GetAttributeValueOrChildValueByName( "address" ).AsString( CORE::CString::Empty, true );
                contact.nonce = (*c)->GetAttributeValueOrChildValueByName( "nonce" ).AsString( CORE::CString::Empty, true );
                const CORE::CString stateText = (*c)->GetAttributeValueOrChildValueByName( "state" ).AsString( CORE::CString::Empty, true );
                contact.state = MAILINTRO_NOT_ASKED;
                for ( int s=0; s<=(int) MAILINTRO_REFUSED; ++s )
                    if ( stateText == MailIntroStateToString( (TMailIntroState) s ) )
                        contact.state = (TMailIntroState) s;
                const UInt32 source = (*c)->GetAttributeValueOrChildValueByName( "source" ).AsUInt32( 0, true );
                contact.source = source <= (UInt32) MAILINTROSOURCE_CODE ? (TMailIntroSource) source : MAILINTROSOURCE_NONE;
                contact.publicKey = (*c)->GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
                contact.displayName = (*c)->GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true );
                contact.codeKey = (*c)->GetAttributeValueOrChildValueByName( "codeKey" ).AsString( CORE::CString::Empty, true );
                contact.pendingKey = (*c)->GetAttributeValueOrChildValueByName( "pendingKey" ).AsString( CORE::CString::Empty, true );
                contact.pendingName = (*c)->GetAttributeValueOrChildValueByName( "pendingName" ).AsString( CORE::CString::Empty, true );
                contact.requestedAt = (*c)->GetAttributeValueOrChildValueByName( "requestedAt" ).AsInt64( 0, true );
                contact.expiresAt = (*c)->GetAttributeValueOrChildValueByName( "expiresAt" ).AsInt64( 0, true );
                contact.lastSentAt = (*c)->GetAttributeValueOrChildValueByName( "lastSentAt" ).AsInt64( 0, true );
                contact.sendCount = (*c)->GetAttributeValueOrChildValueByName( "sendCount" ).AsUInt32( 0, true );
                if ( !IsUsableAddress( contact.address ) || ( !contact.nonce.IsNULLOrEmpty() && !IsNonce( contact.nonce ) ) )
                    return false;
                draft.contacts.push_back( contact );
            }
            drafts.push_back( draft );
        }
        for ( CORE::CDataNode::const_iterator i = inboundNode->ConstBegin(); i != inboundNode->ConstEnd(); ++i )
        {
            if ( inbound.size() >= g_maxInbound )
                return false;
            CMailInboundIntro intro;
            const CORE::CDataNode* requestNode = (*i)->FindChild( "request" );
            CORE::CString reason;
            if ( GUCEF_NULL == requestNode || !intro.request.Deserialize( *requestNode, settings ) || !intro.request.IsWellFormed( reason ) )
                return false;
            intro.fromAddress = (*i)->GetAttributeValueOrChildValueByName( "fromAddress" ).AsString( CORE::CString::Empty, true );
            intro.receivedAt = (*i)->GetAttributeValueOrChildValueByName( "receivedAt" ).AsInt64( 0, true );
            const UInt32 status = (*i)->GetAttributeValueOrChildValueByName( "status" ).AsUInt32( 0, true );
            intro.status = status <= (UInt32) CMailInboundIntro::STATUS_CANCELLED ? (CMailInboundIntro::EStatus) status : CMailInboundIntro::STATUS_PENDING;
            intro.keyChanged = (*i)->GetAttributeValueOrChildValueByName( "keyChanged" ).AsBool( false, true );
            intro.knownKey = (*i)->GetAttributeValueOrChildValueByName( "knownKey" ).AsString( CORE::CString::Empty, true );
            intro.lastReplyAt = (*i)->GetAttributeValueOrChildValueByName( "lastReplyAt" ).AsInt64( 0, true );
            if ( !IsUsableAddress( intro.fromAddress ) )
                return false;
            inbound.push_back( intro );
        }
        return true;
    }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CMailIntroState( *this ); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailIntroState";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      SMALL CLASSES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailIntroContactInput::CMailIntroContactInput( void )
    : name()
    , address()
    , playerCode()
{GUCEF_TRACE;

}

CMailIntroContactInput::CMailIntroContactInput( const CORE::CString& contactName, const CORE::CString& contactAddress, const CORE::CString& code )
    : name( contactName )
    , address( contactAddress )
    , playerCode( code )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailIntroContact::CMailIntroContact( void )
    : name()
    , address()
    , nonce()
    , state( MAILINTRO_NOT_ASKED )
    , source( MAILINTROSOURCE_NONE )
    , publicKey()
    , displayName()
    , codeKey()
    , pendingKey()
    , pendingName()
    , requestedAt( 0 )
    , expiresAt( 0 )
    , lastSentAt( 0 )
    , sendCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailIntroDraft::CMailIntroDraft( void )
    : matchId()
    , title()
    , creatorName()
    , creatorAddress()
    , createdAt( 0 )
    , settings()
    , contacts()
{GUCEF_TRACE;

}

CMailIntroContact*
CMailIntroDraft::FindContact( const CORE::CString& address )
{GUCEF_TRACE;

    const CORE::CString wanted = Lower( address );
    for ( size_t i=0; i<contacts.size(); ++i )
        if ( contacts[ i ].address == wanted )
            return &contacts[ i ];
    return GUCEF_NULL;
}

const CMailIntroContact*
CMailIntroDraft::FindContact( const CORE::CString& address ) const
{GUCEF_TRACE;

    const CORE::CString wanted = Lower( address );
    for ( size_t i=0; i<contacts.size(); ++i )
        if ( contacts[ i ].address == wanted )
            return &contacts[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CMailIntroSummary::CMailIntroSummary( void )
    : total( 0 )
    , notAsked( 0 )
    , waiting( 0 )
    , introduced( 0 )
    , declined( 0 )
    , expired( 0 )
    , keyChanged( 0 )
    , refused( 0 )
    , allIntroduced( false )
    , startable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboundIntro::CMailInboundIntro( void )
    : request()
    , fromAddress()
    , receivedAt( 0 )
    , status( STATUS_PENDING )
    , keyChanged( false )
    , knownKey()
    , lastReplyAt( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailIntroLimits::CMailIntroLimits( void )
    : expiryMs( 14LL * 24 * 60 * 60 * 1000 )
    , resendIntervalMs( 60LL * 60 * 1000 )
    , maxPerSender( 3 )
    , maxPending( 32 )
    , maxDrafts( 16 )
    , maxFutureSkewMs( 24LL * 60 * 60 * 1000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CMailIntroductions                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailIntroductions::CMailIntroductions( void )
    : m_identity()
    , m_directory()
    , m_limits()
    , m_book()
    , m_drafts()
    , m_inbound()
{GUCEF_TRACE;

}

CMailIntroductions::~CMailIntroductions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CMailIntroductions::Configure( const MULTIPLAY::CPlayerIdentity& identity, const CORE::CString& directory, const CMailIntroLimits& limits )
{GUCEF_TRACE;

    m_identity = identity;
    m_directory = directory;
    m_limits = limits;
}

/*-------------------------------------------------------------------------*/

CMailAddressBook& CMailIntroductions::GetBook( void ) { return m_book; }
const CMailAddressBook& CMailIntroductions::GetBook( void ) const { return m_book; }

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::IsStatePersistent( void ) const
{GUCEF_TRACE;

    return !m_directory.IsNULLOrEmpty();
}

CORE::CString
CMailIntroductions::GetPath( const char* name ) const
{GUCEF_TRACE;

    return CORE::CombinePath( m_directory, name );
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::Load( void )
{GUCEF_TRACE;

    m_drafts.clear();
    m_inbound.clear();
    if ( !IsStatePersistent() )
        return true;
    CMailIntroState state;
    if ( CMailFiles::ReadRecord( GetPath( StateFileName ), state, MaxFileBytes ) )
    {
        m_drafts = state.drafts;
        m_inbound = state.inbound;
    }
    else
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "MailIntroductions: There is no usable state of the introductions, it starts empty" );
    CMailAddressBook book;
    if ( book.LoadFromFile( GetPath( BookFileName ) ) )
        m_book = book;
    else
        m_book = CMailAddressBook();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::Save( void ) const
{GUCEF_TRACE;

    if ( !IsStatePersistent() )
        return true;
    CMailIntroState state;
    state.drafts = m_drafts;
    state.inbound = m_inbound;
    const bool saved = CMailFiles::WriteRecord( GetPath( StateFileName ), state );
    return m_book.SaveToFile( GetPath( BookFileName ) ) && saved;
}

void
CMailIntroductions::ChangedState( void )
{GUCEF_TRACE;

    if ( !Save() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailIntroductions: The state of the introductions could not be written" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailIntroductions::MakeNonce( void )
{GUCEF_TRACE;

    UInt8 random[ 16 ];
    if ( !CORE::CRandom::GetRandomBytes( random, sizeof( random ) ) )
        for ( int i=0; i<16; ++i )
            random[ i ] = (UInt8) CORE::CRandom::GetRandomUInt32( 0, 255 );
    static const char digits[] = "0123456789abcdef";
    CORE::CString hex;
    for ( UInt32 i=0; i<sizeof( random ); ++i )
    {
        hex += digits[ random[ i ] >> 4 ];
        hex += digits[ random[ i ] & 0x0F ];
    }
    return hex;
}

/*-------------------------------------------------------------------------*/

CMailIntroDraft*
CMailIntroductions::FindDraftPtr( const CORE::CString& matchId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_drafts.size(); ++i )
        if ( m_drafts[ i ].matchId == matchId )
            return &m_drafts[ i ];
    return GUCEF_NULL;
}

const CMailIntroDraft*
CMailIntroductions::FindDraftPtr( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_drafts.size(); ++i )
        if ( m_drafts[ i ].matchId == matchId )
            return &m_drafts[ i ];
    return GUCEF_NULL;
}

CMailInboundIntro*
CMailIntroductions::FindInboundPtr( const CORE::CString& matchId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_inbound.size(); ++i )
        if ( m_inbound[ i ].request.matchId == matchId )
            return &m_inbound[ i ];
    return GUCEF_NULL;
}

const CMailInboundIntro*
CMailIntroductions::FindInboundPtr( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_inbound.size(); ++i )
        if ( m_inbound[ i ].request.matchId == matchId )
            return &m_inbound[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE CREATOR                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CMailIntroductions::CheckContact( const CMailIntroDraft& draft, const CMailIntroContactInput& input, CORE::CString::StringSet& keys, CMailIntroContact& contact, CORE::CString& reason ) const
{GUCEF_TRACE;

    contact = CMailIntroContact();
    contact.name = CleanText( input.name, CMailIntroRequest::MaxNameBytes );
    contact.address = Lower( input.address );
    if ( contact.name.IsNULLOrEmpty() )
    {
        reason = "Every player needs a name";
        return false;
    }
    if ( !IsUsableAddress( input.address ) )
    {
        reason = "A mail address is not valid";
        return false;
    }
    if ( contact.address == draft.creatorAddress || NULL != draft.FindContact( contact.address ) )
    {
        reason = "A mail address is used twice";
        return false;
    }
    const CORE::CString code = input.playerCode.Lowercase();
    if ( !code.IsNULLOrEmpty() )
    {
        if ( !MULTIPLAY::CMatchSigning::IsHexOfLength( code, MULTIPLAY::CMatchSigning::PublicKeyHexSize ) )
        {
            reason = "A player code has to be 64 hex digits";
            return false;
        }
        if ( !keys.insert( code ).second )
        {
            reason = "A player code is used twice or is your own";
            return false;
        }
        contact.state = MAILINTRO_INTRODUCED;
        contact.source = MAILINTROSOURCE_CODE;
        contact.codeKey = code;
        contact.publicKey = code;
        contact.displayName = contact.name;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::BeginDraft( const CORE::CString& matchId, const CORE::CString& title, const CORE::CString& creatorName, const CORE::CString& creatorAddress,
                                const GUCEF::map< CORE::CString, CORE::CString >& settings, const GUCEF::vector< CMailIntroContactInput >& contacts, Int64 now, CORE::CString& reason )
{GUCEF_TRACE;

    reason.Clear();
    if ( !MULTIPLAY::CMatchId::IsValid( matchId ) || NULL != FindDraftPtr( matchId ) )
    {
        reason = "The id of the game is not valid or is in use";
        return false;
    }
    if ( m_drafts.size() >= m_limits.maxDrafts )
    {
        reason = "There are too many games that wait for introductions";
        return false;
    }
    const CORE::CString cleanTitle = CleanText( title, CMailIntroRequest::MaxTitleBytes );
    if ( cleanTitle.IsNULLOrEmpty() )
    {
        reason = "Give the game a name";
        return false;
    }
    if ( !IsUsableAddress( creatorAddress ) || !m_identity.HasKeyPair() )
    {
        reason = "The mailbox or the identity of the player is not usable";
        return false;
    }
    if ( contacts.empty() || contacts.size() > MaxContacts )
    {
        reason = "A game has one to five invited players";
        return false;
    }
    // the settings go into the request and into the record: they have to fit both
    if ( settings.size() > CMailIntroRequest::MaxSettings )
    {
        reason = "The game has too many settings";
        return false;
    }
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator s = settings.begin(); s != settings.end(); ++s )
        if ( (*s).first.IsNULLOrEmpty() || (*s).first.Length() > CMailIntroRequest::MaxSettingKeyBytes || (*s).second.Length() > CMailIntroRequest::MaxSettingValueBytes )
        {
            reason = "A setting of the game is not valid";
            return false;
        }

    CMailIntroDraft draft;
    draft.matchId = matchId;
    draft.title = cleanTitle;
    draft.creatorName = CleanText( creatorName, CMailIntroRequest::MaxNameBytes );
    draft.creatorAddress = Lower( creatorAddress );
    draft.createdAt = now;
    draft.settings = settings;
    CORE::CString::StringSet keys;
    keys.insert( m_identity.publicKey.Lowercase() );
    for ( size_t i=0; i<contacts.size(); ++i )
    {
        CMailIntroContact contact;
        if ( !CheckContact( draft, contacts[ i ], keys, contact, reason ) )
            return false;
        draft.contacts.push_back( contact );
    }
    m_drafts.push_back( draft );
    ChangedState();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::FindDraft( const CORE::CString& matchId, CMailIntroDraft& draft ) const
{GUCEF_TRACE;

    const CMailIntroDraft* found = FindDraftPtr( matchId );
    if ( GUCEF_NULL == found )
        return false;
    draft = *found;
    return true;
}

void
CMailIntroductions::GetDrafts( GUCEF::vector< CMailIntroDraft >& drafts ) const
{GUCEF_TRACE;

    drafts = m_drafts;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::GetSummary( const CORE::CString& matchId, CMailIntroSummary& summary ) const
{GUCEF_TRACE;

    summary = CMailIntroSummary();
    const CMailIntroDraft* draft = FindDraftPtr( matchId );
    if ( GUCEF_NULL == draft )
        return false;
    summary.total = (UInt32) draft->contacts.size();
    for ( size_t i=0; i<draft->contacts.size(); ++i )
    {
        switch ( draft->contacts[ i ].state )
        {
            case MAILINTRO_NOT_ASKED:   ++summary.notAsked; break;
            case MAILINTRO_WAITING:     ++summary.waiting; break;
            case MAILINTRO_INTRODUCED:  ++summary.introduced; break;
            case MAILINTRO_DECLINED:    ++summary.declined; break;
            case MAILINTRO_EXPIRED:     ++summary.expired; break;
            case MAILINTRO_KEY_CHANGED: ++summary.keyChanged; break;
            case MAILINTRO_REFUSED:     ++summary.refused; break;
        }
    }
    summary.allIntroduced = summary.total > 0 && summary.introduced == summary.total;
    summary.startable = summary.introduced > 0 && 0 == summary.waiting && 0 == summary.notAsked && 0 == summary.keyChanged;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::RemoveDraft( const CORE::CString& matchId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_drafts.size(); ++i )
        if ( m_drafts[ i ].matchId == matchId )
        {
            m_drafts.erase( m_drafts.begin() + i );
            ChangedState();
            return true;
        }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::RemoveContact( const CORE::CString& matchId, const CORE::CString& address )
{GUCEF_TRACE;

    CMailIntroDraft* draft = FindDraftPtr( matchId );
    if ( GUCEF_NULL == draft || draft->contacts.size() < 2 )
        return false;
    const CORE::CString lower = Lower( address );
    for ( size_t i=0; i<draft->contacts.size(); ++i )
        if ( Lower( draft->contacts[ i ].address ) == lower )
        {
            draft->contacts.erase( draft->contacts.begin() + i );
            ChangedState();
            return true;
        }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::AddContact( const CORE::CString& matchId, const CMailIntroContactInput& input, CORE::CString& reason )
{GUCEF_TRACE;

    reason.Clear();
    CMailIntroDraft* draft = FindDraftPtr( matchId );
    if ( GUCEF_NULL == draft )
    {
        reason = "The game does not wait for introductions";
        return false;
    }
    if ( draft->contacts.size() >= MaxContacts )
    {
        reason = "A game has one to five invited players";
        return false;
    }
    CORE::CString::StringSet keys;
    keys.insert( m_identity.publicKey.Lowercase() );
    for ( size_t i=0; i<draft->contacts.size(); ++i )
    {
        if ( !draft->contacts[ i ].publicKey.IsNULLOrEmpty() )
            keys.insert( draft->contacts[ i ].publicKey.Lowercase() );
        if ( !draft->contacts[ i ].codeKey.IsNULLOrEmpty() )
            keys.insert( draft->contacts[ i ].codeKey.Lowercase() );
    }
    CMailIntroContact contact;
    if ( !CheckContact( *draft, input, keys, contact, reason ) )
        return false;
    draft->contacts.push_back( contact );
    ChangedState();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::SetDraftSetting( const CORE::CString& matchId, const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    CMailIntroDraft* draft = FindDraftPtr( matchId );
    if ( GUCEF_NULL == draft || key.IsNULLOrEmpty() || key.Length() > MULTIPLAY::CMatchInfo::MaxSettingKeyLength || value.Length() > MULTIPLAY::CMatchInfo::MaxSettingValueLength )
        return false;
    draft->settings[ key ] = value;
    ChangedState();
    return true;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::PrepareRequest( const CORE::CString& matchId, const CORE::CString& address, bool force, Int64 now, CMailIntroRequest& request )
{GUCEF_TRACE;

    CMailIntroDraft* draft = FindDraftPtr( matchId );
    if ( GUCEF_NULL == draft )
        return MAILINTRORESULT_NO_DRAFT;
    CMailIntroContact* contact = draft->FindContact( address );
    if ( GUCEF_NULL == contact )
        return MAILINTRORESULT_NO_CONTACT;

    bool newNonce = false;
    switch ( contact->state )
    {
        case MAILINTRO_NOT_ASKED:
            newNonce = true;
            break;
        case MAILINTRO_INTRODUCED:
            if ( !force )
                return MAILINTRORESULT_DUPLICATE;
            newNonce = true;
            break;
        case MAILINTRO_KEY_CHANGED:
            if ( !force )
                return MAILINTRORESULT_NOT_WAITING;
            newNonce = true;
            break;
        case MAILINTRO_WAITING:
            newNonce = now >= contact->expiresAt;
            break;
        default:
            newNonce = true;
            break;
    }
    if ( contact->sendCount > 0 && now < contact->lastSentAt + m_limits.resendIntervalMs )
        return MAILINTRORESULT_RATE_LIMITED;

    const CORE::CString nonce = newNonce ? MakeNonce() : contact->nonce;
    CMailIntroRequest made;
    made.matchId = draft->matchId;
    made.nonce = nonce;
    made.creatorName = draft->creatorName;
    made.creatorAddress = draft->creatorAddress;
    made.inviteeName = contact->name;
    made.inviteeAddress = contact->address;
    made.title = draft->title;
    made.settings = draft->settings;
    made.parties.push_back( CMailIntroParty( draft->creatorName, draft->creatorAddress ) );
    for ( size_t i=0; i<draft->contacts.size(); ++i )
        made.parties.push_back( CMailIntroParty( draft->contacts[ i ].name, draft->contacts[ i ].address ) );
    made.createdAt = now;
    made.expiresAt = now + m_limits.expiryMs;
    if ( !made.Sign( m_identity ) )
        return MAILINTRORESULT_BAD_STATEMENT;

    contact->nonce = nonce;
    contact->state = MAILINTRO_WAITING;
    if ( newNonce )
    {
        contact->publicKey.Clear();
        contact->pendingKey.Clear();
        contact->pendingName.Clear();
        contact->source = MAILINTROSOURCE_NONE;
    }
    if ( 0 == contact->requestedAt || newNonce )
        contact->requestedAt = now;
    contact->expiresAt = made.expiresAt;
    contact->lastSentAt = now;
    ++contact->sendCount;
    request = made;
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::HandleReply( const CMailIntroReply& reply, const CORE::CString& fromAddress, Int64 now, CORE::CString* matchIdOut )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !reply.IsWellFormed( reason ) )
    {
        LogRefusal( "A reply", MAILINTRORESULT_BAD_STATEMENT );
        return MAILINTRORESULT_BAD_STATEMENT;
    }
    CMailIntroDraft* draft = FindDraftPtr( reply.matchId );
    if ( GUCEF_NULL == draft )
        return MAILINTRORESULT_NO_DRAFT;
    if ( GUCEF_NULL != matchIdOut )
        *matchIdOut = draft->matchId;
    CMailIntroContact* contact = draft->FindContact( fromAddress );
    if ( GUCEF_NULL == contact )
        return MAILINTRORESULT_NO_CONTACT;
    if ( Lower( reply.inviteeAddress ) != contact->address || Lower( reply.creatorAddress ) != draft->creatorAddress )
        return MAILINTRORESULT_BAD_ADDRESS;

    // an answer to a nonce that is used up: the same answer again is nothing, another one is a conflict
    switch ( contact->state )
    {
        case MAILINTRO_NOT_ASKED:
            return MAILINTRORESULT_NOT_WAITING;
        case MAILINTRO_EXPIRED:
            return MAILINTRORESULT_EXPIRED;
        case MAILINTRO_INTRODUCED:
        case MAILINTRO_DECLINED:
        case MAILINTRO_KEY_CHANGED:
        case MAILINTRO_REFUSED:
        {
            if ( !NonceEquals( reply.nonce, contact->nonce ) )
                return MAILINTRORESULT_BAD_NONCE;
            const CORE::CString& usedKey = MAILINTRO_INTRODUCED == contact->state ? contact->publicKey : contact->pendingKey;
            const bool same = MAILINTRO_DECLINED == contact->state ? !reply.accepted : ( reply.accepted && reply.publicKey.Lowercase() == usedKey );
            return same ? MAILINTRORESULT_DUPLICATE : MAILINTRORESULT_CONFLICT;
        }
        default:
            break;
    }

    // the contact is waiting
    if ( now >= contact->expiresAt )
    {
        contact->state = MAILINTRO_EXPIRED;
        ChangedState();
        return MAILINTRORESULT_EXPIRED;
    }
    if ( !NonceEquals( reply.nonce, contact->nonce ) )
        return MAILINTRORESULT_BAD_NONCE;

    if ( !reply.accepted )
    {
        contact->state = MAILINTRO_DECLINED;
        ChangedState();
        return MAILINTRORESULT_DECLINED;
    }
    if ( !reply.Verify( reason ) )
        return MAILINTRORESULT_BAD_SIGNATURE;

    const CORE::CString key = reply.publicKey.Lowercase();
    if ( key == m_identity.publicKey.Lowercase() )
        return MAILINTRORESULT_BAD_KEY;
    for ( size_t i=0; i<draft->contacts.size(); ++i )
    {
        const CMailIntroContact& other = draft->contacts[ i ];
        if ( &other != contact && ( ( MAILINTRO_INTRODUCED == other.state && other.publicKey == key ) || ( MAILINTRO_KEY_CHANGED == other.state && other.pendingKey == key ) ) )
            return MAILINTRORESULT_BAD_KEY;
    }
    if ( !contact->codeKey.IsNULLOrEmpty() && contact->codeKey != key )
        return MAILINTRORESULT_BAD_KEY;

    const CORE::CString name = CleanText( reply.displayName, CMailIntroRequest::MaxNameBytes );
    if ( CMailAddressBook::CHECK_DIFFERENT == m_book.Check( contact->address, key ) )
    {
        contact->state = MAILINTRO_KEY_CHANGED;
        contact->pendingKey = key;
        contact->pendingName = name;
        ChangedState();
        return MAILINTRORESULT_KEY_CHANGED;
    }
    contact->state = MAILINTRO_INTRODUCED;
    contact->source = MAILINTROSOURCE_REPLY;
    contact->publicKey = key;
    contact->displayName = name.IsNULLOrEmpty() ? contact->name : name;
    m_book.Remember( contact->address, key, contact->displayName, now );
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::ConfirmKeyChange( const CORE::CString& matchId, const CORE::CString& address, bool accept, Int64 now )
{GUCEF_TRACE;

    CMailIntroDraft* draft = FindDraftPtr( matchId );
    if ( GUCEF_NULL == draft )
        return MAILINTRORESULT_NO_DRAFT;
    CMailIntroContact* contact = draft->FindContact( address );
    if ( GUCEF_NULL == contact )
        return MAILINTRORESULT_NO_CONTACT;
    if ( MAILINTRO_KEY_CHANGED != contact->state )
        return MAILINTRORESULT_NOT_WAITING;
    if ( accept )
    {
        contact->state = MAILINTRO_INTRODUCED;
        contact->source = MAILINTROSOURCE_REPLY;
        contact->publicKey = contact->pendingKey;
        contact->displayName = contact->pendingName.IsNULLOrEmpty() ? contact->name : contact->pendingName;
        contact->pendingKey.Clear();
        contact->pendingName.Clear();
        m_book.Remember( contact->address, contact->publicKey, contact->displayName, now );
    }
    else
        contact->state = MAILINTRO_REFUSED;
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CMailIntroductions::ExpireDue( Int64 now, GUCEF::vector< CORE::CString >& changed )
{GUCEF_TRACE;

    bool any = false;
    for ( size_t d=0; d<m_drafts.size(); ++d )
    {
        bool draftChanged = false;
        for ( size_t c=0; c<m_drafts[ d ].contacts.size(); ++c )
        {
            CMailIntroContact& contact = m_drafts[ d ].contacts[ c ];
            if ( MAILINTRO_WAITING == contact.state && now >= contact.expiresAt )
            {
                contact.state = MAILINTRO_EXPIRED;
                draftChanged = true;
            }
        }
        if ( draftChanged )
        {
            changed.push_back( m_drafts[ d ].matchId );
            any = true;
        }
    }
    // a game that was never started is not kept for ever
    for ( size_t d=m_drafts.size(); d>0; --d )
        if ( m_drafts[ d - 1 ].createdAt + 4 * m_limits.expiryMs < now )
        {
            m_drafts.erase( m_drafts.begin() + ( d - 1 ) );
            any = true;
        }
    if ( any )
        ChangedState();
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::GetIntroduced( const CORE::CString& matchId, GUCEF::vector< CMailIntroContact >& contacts ) const
{GUCEF_TRACE;

    contacts.clear();
    const CMailIntroDraft* draft = FindDraftPtr( matchId );
    if ( GUCEF_NULL == draft )
        return false;
    for ( size_t i=0; i<draft->contacts.size(); ++i )
        if ( MAILINTRO_INTRODUCED == draft->contacts[ i ].state )
            contacts.push_back( draft->contacts[ i ] );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE INVITEE                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::HandleRequest( const CMailIntroRequest& request, const CORE::CString& fromAddress, const CORE::CString& ownAddress, bool matchKnown, Int64 now )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( !request.IsWellFormed( reason ) )
        return MAILINTRORESULT_BAD_STATEMENT;
    if ( !request.Verify( reason ) )
        return MAILINTRORESULT_BAD_SIGNATURE;
    const CORE::CString from = Lower( fromAddress );
    if ( from != Lower( request.creatorAddress ) )
        return MAILINTRORESULT_BAD_ADDRESS;
    if ( Lower( request.inviteeAddress ) != Lower( ownAddress ) )
        return MAILINTRORESULT_NOT_FOR_ME;
    if ( request.creatorPublicKey.Lowercase() == m_identity.publicKey.Lowercase() )
        return MAILINTRORESULT_BAD_KEY;
    if ( m_book.IsBlocked( from ) )
        return MAILINTRORESULT_BLOCKED;
    if ( matchKnown )
        return MAILINTRORESULT_KNOWN_MATCH;
    if ( now >= request.expiresAt )
        return MAILINTRORESULT_EXPIRED;
    if ( request.createdAt > now + m_limits.maxFutureSkewMs )
        return MAILINTRORESULT_BAD_STATEMENT;

    CMailInboundIntro* existing = FindInboundPtr( request.matchId );
    if ( GUCEF_NULL != existing )
    {
        if ( existing->request.creatorPublicKey.Lowercase() != request.creatorPublicKey.Lowercase() || !NonceEquals( existing->request.nonce, request.nonce ) )
            return MAILINTRORESULT_CONFLICT;
        if ( CMailInboundIntro::STATUS_PENDING == existing->status )
        {
            if ( request.expiresAt > existing->request.expiresAt )
            {
                existing->request = request;
                ChangedState();
            }
            return MAILINTRORESULT_DUPLICATE;
        }
        if ( CMailInboundIntro::STATUS_ACCEPTED == existing->status )
        {
            if ( now < existing->lastReplyAt + m_limits.resendIntervalMs )
                return MAILINTRORESULT_RATE_LIMITED;
            if ( request.expiresAt > existing->request.expiresAt )
                existing->request = request;
            return MAILINTRORESULT_RESEND_REPLY;
        }
        return MAILINTRORESULT_DUPLICATE;
    }

    UInt32 pending = 0, fromSender = 0;
    for ( size_t i=0; i<m_inbound.size(); ++i )
        if ( CMailInboundIntro::STATUS_PENDING == m_inbound[ i ].status )
        {
            ++pending;
            if ( m_inbound[ i ].fromAddress == from )
                ++fromSender;
        }
    if ( fromSender >= m_limits.maxPerSender || pending >= m_limits.maxPending || m_inbound.size() >= g_maxInbound )
        return MAILINTRORESULT_LIMIT;

    CMailInboundIntro intro;
    intro.request = request;
    intro.fromAddress = from;
    intro.receivedAt = now;
    if ( CMailAddressBook::CHECK_DIFFERENT == m_book.Check( from, request.creatorPublicKey ) )
    {
        CMailAddressEntry entry;
        intro.keyChanged = true;
        if ( m_book.Find( from, entry ) )
            intro.knownKey = entry.publicKey;
    }
    m_inbound.push_back( intro );
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CMailIntroductions::GetPending( GUCEF::vector< CMailInboundIntro >& pending ) const
{GUCEF_TRACE;

    pending = m_inbound;
}

bool
CMailIntroductions::FindInbound( const CORE::CString& matchId, CMailInboundIntro& intro ) const
{GUCEF_TRACE;

    const CMailInboundIntro* found = FindInboundPtr( matchId );
    if ( GUCEF_NULL == found )
        return false;
    intro = *found;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailIntroductions::MakeReply( const CMailInboundIntro& intro, const CORE::CString& displayName, Int64 now, CMailIntroReply& reply ) const
{GUCEF_TRACE;

    reply = CMailIntroReply();
    reply.matchId = intro.request.matchId;
    reply.nonce = intro.request.nonce;
    reply.accepted = true;
    reply.displayName = CleanText( displayName.IsNULLOrEmpty() ? m_identity.displayName : displayName, CMailIntroRequest::MaxNameBytes );
    reply.inviteeAddress = Lower( intro.request.inviteeAddress );
    reply.creatorAddress = Lower( intro.request.creatorAddress );
    reply.createdAt = now;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::AcceptRequest( const CORE::CString& matchId, bool confirmKeyChange, const CORE::CString& displayName, Int64 now, CMailIntroReply& reply )
{GUCEF_TRACE;

    CMailInboundIntro* intro = FindInboundPtr( matchId );
    if ( GUCEF_NULL == intro )
        return MAILINTRORESULT_NO_REQUEST;
    if ( CMailInboundIntro::STATUS_PENDING != intro->status )
        return MAILINTRORESULT_DUPLICATE;
    if ( now >= intro->request.expiresAt )
        return MAILINTRORESULT_EXPIRED;
    if ( intro->keyChanged && !confirmKeyChange )
        return MAILINTRORESULT_NEEDS_CONFIRMATION;
    CMailIntroReply made;
    MakeReply( *intro, displayName, now, made );
    if ( !made.Sign( m_identity ) )
        return MAILINTRORESULT_BAD_STATEMENT;
    reply = made;
    intro->status = CMailInboundIntro::STATUS_ACCEPTED;
    intro->lastReplyAt = now;
    m_book.Remember( intro->fromAddress, intro->request.creatorPublicKey, intro->request.creatorName, now );
    intro->keyChanged = false;
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::DeclineRequest( const CORE::CString& matchId, Int64 now, CMailIntroReply& reply )
{GUCEF_TRACE;

    CMailInboundIntro* intro = FindInboundPtr( matchId );
    if ( GUCEF_NULL == intro )
        return MAILINTRORESULT_NO_REQUEST;
    if ( CMailInboundIntro::STATUS_PENDING != intro->status )
        return MAILINTRORESULT_DUPLICATE;
    reply = CMailIntroReply();
    reply.matchId = intro->request.matchId;
    reply.nonce = intro->request.nonce;
    reply.accepted = false;
    reply.inviteeAddress = Lower( intro->request.inviteeAddress );
    reply.creatorAddress = Lower( intro->request.creatorAddress );
    reply.createdAt = now;
    intro->status = CMailInboundIntro::STATUS_DECLINED;
    intro->lastReplyAt = now;
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::RepeatReply( const CORE::CString& matchId, const CORE::CString& displayName, Int64 now, CMailIntroReply& reply )
{GUCEF_TRACE;

    CMailInboundIntro* intro = FindInboundPtr( matchId );
    if ( GUCEF_NULL == intro )
        return MAILINTRORESULT_NO_REQUEST;
    if ( CMailInboundIntro::STATUS_ACCEPTED != intro->status )
        return MAILINTRORESULT_DUPLICATE;
    if ( now < intro->lastReplyAt + m_limits.resendIntervalMs )
        return MAILINTRORESULT_RATE_LIMITED;
    CMailIntroReply made;
    MakeReply( *intro, displayName, now, made );
    if ( !made.Sign( m_identity ) )
        return MAILINTRORESULT_BAD_STATEMENT;
    reply = made;
    intro->lastReplyAt = now;
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

TMailIntroResult
CMailIntroductions::CancelInbound( const CORE::CString& matchId, const MULTIPLAY::CMatchProposal& cancel )
{GUCEF_TRACE;

    CMailInboundIntro* intro = FindInboundPtr( matchId );
    if ( GUCEF_NULL == intro )
        return MAILINTRORESULT_NO_REQUEST;
    if ( CMailInboundIntro::STATUS_CANCELLED == intro->status )
        return MAILINTRORESULT_DUPLICATE;

    // the creator named in the request is the only one who cancels, and the signature has the match id as context
    MULTIPLAY::CMatchInfo creatorOnly;
    creatorOnly.matchId = matchId;
    creatorOnly.creatorUserId = MULTIPLAY::CPlayerIdentity::MakeUserIdFromHex( intro->request.creatorPublicKey );
    MULTIPLAY::CMatchParticipant creator;
    creator.userId = creatorOnly.creatorUserId;
    creator.publicKey = intro->request.creatorPublicKey;
    creatorOnly.participants.push_back( creator );
    if ( MULTIPLAY::MATCHDECISION_CANCEL != cancel.kind || cancel.matchId != matchId || cancel.proposerUserId != creatorOnly.creatorUserId || !cancel.VerifySignature( creatorOnly ) )
        return MAILINTRORESULT_BAD_SIGNATURE;
    intro->status = CMailInboundIntro::STATUS_CANCELLED;
    ChangedState();
    return MAILINTRORESULT_OK;
}

/*-------------------------------------------------------------------------*/

void
CMailIntroductions::ForgetInbound( const CORE::CString& matchId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_inbound.size(); ++i )
        if ( m_inbound[ i ].request.matchId == matchId )
        {
            m_inbound.erase( m_inbound.begin() + i );
            ChangedState();
            return;
        }
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::IsTableAnnounced( const CORE::CString& matchId, const GUCEF::vector< CORE::CString >& addresses ) const
{GUCEF_TRACE;

    const CMailInboundIntro* intro = FindInboundPtr( matchId );
    if ( GUCEF_NULL == intro )
        return false;
    for ( size_t a=0; a<addresses.size(); ++a )
    {
        bool known = false;
        for ( size_t p=0; p<intro->request.parties.size() && !known; ++p )
            known = Lower( intro->request.parties[ p ].address ) == Lower( addresses[ a ] );
        if ( !known )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::IsTableAnnounced( const CORE::CString& matchId, const GUCEF::vector< CORE::CString >& addresses, const GUCEF::map< CORE::CString, CORE::CString >& settings ) const
{GUCEF_TRACE;

    if ( !IsTableAnnounced( matchId, addresses ) )
        return false;
    const CMailInboundIntro* intro = FindInboundPtr( matchId );
    return GUCEF_NULL != intro && ( intro->request.settings.empty() || intro->request.settings == settings );
}

/*-------------------------------------------------------------------------*/

bool
CMailIntroductions::IsTableAnnounced( const CORE::CString& matchId, const GUCEF::vector< CORE::CString >& addresses, const GUCEF::map< CORE::CString, CORE::CString >& settings, const CORE::CString& reducibleKey ) const
{GUCEF_TRACE;

    if ( reducibleKey.IsNULLOrEmpty() )
        return IsTableAnnounced( matchId, addresses, settings );
    if ( !IsTableAnnounced( matchId, addresses ) )
        return false;
    const CMailInboundIntro* intro = FindInboundPtr( matchId );
    if ( GUCEF_NULL == intro )
        return false;
    const GUCEF::map< CORE::CString, CORE::CString >& announced = intro->request.settings;
    if ( announced.empty() )
        return true;
    if ( announced.size() != settings.size() )
        return false;

    // every setting is as announced, except the reducible one, which has to be a number that is not higher than announced and at least one
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator a = announced.begin(); a != announced.end(); ++a )
    {
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator found = settings.find( (*a).first );
        if ( found == settings.end() )
            return false;
        if ( (*a).first != reducibleKey )
        {
            if ( (*found).second != (*a).second )
                return false;
            continue;
        }
        const CORE::CString& announcedText = (*a).second;
        const CORE::CString& recordText = (*found).second;
        if ( announcedText.IsNULLOrEmpty() || recordText.IsNULLOrEmpty() || recordText.Length() > 9 || announcedText.Length() > 9 )
            return false;
        for ( UInt32 c=0; c<recordText.Length(); ++c )
            if ( recordText[ c ] < '0' || recordText[ c ] > '9' )
                return false;
        for ( UInt32 c=0; c<announcedText.Length(); ++c )
            if ( announcedText[ c ] < '0' || announcedText[ c ] > '9' )
                return false;
        const UInt32 recordValue = CORE::StringToUInt32( recordText.C_String(), 0 );
        if ( recordValue < 1 || recordValue > CORE::StringToUInt32( announcedText.C_String(), 0 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMailIntroductions::PurgeExpired( Int64 now )
{GUCEF_TRACE;

    UInt32 removed = 0;
    for ( size_t i=m_inbound.size(); i>0; --i )
    {
        const CMailInboundIntro& intro = m_inbound[ i - 1 ];
        const bool pendingExpired = CMailInboundIntro::STATUS_PENDING == intro.status && now >= intro.request.expiresAt;
        const bool answeredOld = CMailInboundIntro::STATUS_PENDING != intro.status && now >= intro.request.expiresAt + 7LL * 24 * 60 * 60 * 1000;
        if ( pendingExpired || answeredOld )
        {
            m_inbound.erase( m_inbound.begin() + ( i - 1 ) );
            ++removed;
        }
    }
    if ( removed > 0 )
        ChangedState();
    return removed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
