/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
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

#ifndef GUCEF_AUTHGUI_TESTAPP_TESTOPENID2BACKEND_H
#define GUCEF_AUTHGUI_TESTAPP_TESTOPENID2BACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The profile of an OpenID 2.0 provider that exists in the tests only, "testopenid": what a plugin of a backend brings, here
 *  to test the generic OpenID 2.0 support of the module without any real backend. An identity is
 *  <origin of the endpoint>/openid/id/<1 to 32 characters of 0-9 and a-z> (claimed id and identity the same), the subject
 *  of the account is that text. A test changes what it says about the local account and the realm with the members.
 */
class CTestOpenId2Profile : public GUCEF::AUTH::IOpenId2ProviderProfile
{
    public:

    static const GUCEF::CORE::CString ProviderName;         /**< "testopenid": the name of the descriptor, the provider and the account namespace */
    static const GUCEF::CORE::CString NameField;            /**< "openid.test_name": the field of an assertion that has the name of the person, when the profile reads it */

    bool relayAllowed;                                      /**< IsRelayAllowed() */
    bool readsName;                                         /**< GetDisplayName() reads the field NameField */
    bool hintInstalled;                                     /**< what GetDetectedAccountHint() says: something was detected */
    bool hintRunning;
    GUCEF::CORE::CString hintName;
    mutable GUCEF::UInt32 hintLookups;                      /**< how often the hint was asked for */
    GUCEF::UInt32 forgets;                                  /**< how often ForgetDetectedAccountHint() was called */

    CTestOpenId2Profile( void );

    virtual ~CTestOpenId2Profile();

    /** Back to the defaults: relay allowed, no name, nothing detected */
    void Reset( void );

    virtual const GUCEF::CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ExtractSubject( const GUCEF::CORE::CString& endpoint ,
                                 const GUCEF::CORE::CString& claimedId ,
                                 const GUCEF::CORE::CString& identity  ,
                                 GUCEF::CORE::CString& subject         ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsRelayAllowed( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetDisplayName( const GUCEF::AUTH::COpenId2Assertion& assertion, GUCEF::CORE::CString& displayName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetDetectedAccountHint( GUCEF::CORE::CString& displayName, bool& isRunning ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ForgetDetectedAccountHint( void ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/** The descriptor of "testopenid": OpenID 2.0 in the browser, no client id, display name "TestOpenId" */
GUCEF::AUTH::CAuthProviderDescriptor
MakeTestOpenId2Descriptor( void );

/*-------------------------------------------------------------------------*/

/**
 *  The test backend as a plugin registers a backend: the descriptor in the registry of CAuthGlobal, the profile, and the
 *  provider in the factory (a plain CDescriptorAuthProvider). The destructor takes it all away again; the manager of the
 *  process lets go of the provider first.
 */
class CTestOpenId2Backend
{
    public:

    CTestOpenId2Backend( void );

    ~CTestOpenId2Backend();

    CTestOpenId2Profile& GetProfile( void );

    private:

    CTestOpenId2Backend( const CTestOpenId2Backend& src );
    CTestOpenId2Backend& operator=( const CTestOpenId2Backend& src );

    private:

    CTestOpenId2Profile m_profile;
};

/*-------------------------------------------------------------------------*/

/** The profile of the backend that the main of the test application registered */
CTestOpenId2Profile&
TestOpenId2Profile( void );

/** The backend that the main of the test application made, GUCEF_NULL outside of it */
void
SetTestOpenId2Backend( CTestOpenId2Backend* backend );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_TESTAPP_TESTOPENID2BACKEND_H ? */
