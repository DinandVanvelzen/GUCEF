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

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

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

/**
 *  What a proof is bound to: the challenge of the accepting side and who is talking to whom
 */
class GUCEF_DRN_EXPORT_CPP CDRNAuthContext
{
    public:

    CORE::CString serviceName;
    CORE::CString initiatorNodeId;
    CORE::CString acceptorNodeId;
    UInt8 nonce[ DRN_CHALLENGE_NONCE_SIZE ];
    UInt8 transcriptHash[ DRN_HANDSHAKE_HASH_SIZE ];    /**< over HELLO and CHALLENGE of this connection, the keys are bound to it too */
    UInt8 policyHash[ DRN_HANDSHAKE_HASH_SIZE ];        /**< of the security policy of this node: what a token must be issued for */
    bool requirePolicyHash;                             /**< a token without a policy hash is refused */

    CDRNAuthContext( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  What the accepting side learned from a proof it accepted
 */
class GUCEF_DRN_EXPORT_CPP CDRNAuthResult
{
    public:

    CORE::CString userId;
    CORE::CString role;
    CORE::CString sessionName;
    CORE::CDynamicBuffer publicKey; /**< the user's public key when the proof carried one, checked against the ban list */
    CORE::CString::StringVector accounts;   /**< the accounts at identity providers the proof showed to be the user's, as "provider:subject", checked against the ban list; empty for authenticators that know none */
    CORE::CString refusalText;      /**< why a proof was not accepted */

    CDRNAuthResult( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Proves (connecting side) and checks (accepting side) the identity of a peer during the link handshake.
 *  Both sides use the same method; the proof never contains a secret.
 *  Settings come in through LoadConfig(), created by name through the factory of CDRNGlobal.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNAuthenticator : public CORE::CIConfigurable
{
    public:

    CIDRNAuthenticator( void );

    virtual ~CIDRNAuthenticator();

    /** The name of the method as sent in CHALLENGE and AUTH */
    virtual CORE::CString GetMethodName( void ) const = 0;

    /** Connecting side: makes the proof for a challenge */
    virtual bool CreateProof( const CDRNAuthContext& context, CORE::CDynamicBuffer& proof ) = 0;

    /** Accepting side: checks a proof */
    virtual bool VerifyProof( const CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, CDRNAuthResult& result ) = 0;

    private:

    CIDRNAuthenticator( const CIDRNAuthenticator& src );
    CIDRNAuthenticator& operator=( const CIDRNAuthenticator& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CIDRNAuthenticator, MT::CMutex >                         CIDRNAuthenticatorPtr;
typedef CORE::CTAbstractFactory< CORE::CString, CIDRNAuthenticator, MT::CMutex >    CDRNAuthenticatorFactory;

/*-------------------------------------------------------------------------*/

/**
 *  Which authenticator to use and its settings
 */
class GUCEF_DRN_EXPORT_CPP CDRNAuthSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    CORE::CString authenticatorType;        /**< "anonymous" (default), "shared-secret", "token" or a plugin's */
    CORE::CDataNode authenticatorConfig;    /**< handed to the authenticator's LoadConfig() */

    CDRNAuthSettings( void );
    CDRNAuthSettings( const CDRNAuthSettings& src );
    virtual ~CDRNAuthSettings();
    CDRNAuthSettings& operator=( const CDRNAuthSettings& src );

    /** Sets one setting of the authenticator, for example "secret" */
    void SetSetting( const CORE::CString& name, const CORE::CVariant& value );

    /** Creates and configures the authenticator, NULL if the type is unknown or its settings are not valid */
    CIDRNAuthenticatorPtr CreateAuthenticator( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */
