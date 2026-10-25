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

#ifndef GUCEF_AUTHGUI_TESTAPP_TESTACCOUNTFIXTURE_H
#define GUCEF_AUTHGUI_TESTAPP_TESTACCOUNTFIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#include "gucefAUTH_CLocalIdentityLinkStore.h"
#define GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H ? */

#include "gucefAUTHGUI_TestCommon.h"
#include "TestFakeLoginProvider.h"
#include "TestSigner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A profile in memory that tells when it changes, as a tool that has a profile of its own would have it */
class CTestProfile : public GUCEF::CORE::CObservingNotifier ,
                     public GUCEF::AUTH::IAccountProfile
{
    public:

    GUCEF::CORE::CString name;
    GUCEF::CORE::CString id;
    bool refuseNames;
    GUCEF::UInt32 renames;

    static const GUCEF::CORE::CEvent& ChangedEvent( void )
    {
        static const GUCEF::CORE::CEvent changedEvent( "TestProfile::ChangedEvent" );
        return changedEvent;
    }

    CTestProfile( void )
        : GUCEF::CORE::CObservingNotifier(), GUCEF::AUTH::IAccountProfile()
        , name( "Alice" ), id( "5f2b9a3e-1111-4222-8333-444455556666" ), refuseNames( false ), renames( 0 )
    {
        ChangedEvent().Initialize();
    }

    virtual ~CTestProfile() {}

    virtual GUCEF::CORE::CString GetDisplayName( void ) GUCEF_VIRTUAL_OVERRIDE { return name; }

    virtual bool SetDisplayName( const GUCEF::CORE::CString& displayName ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( refuseNames || displayName.IsNULLOrEmpty() )
            return false;
        name = displayName;
        ++renames;
        NotifyObservers( ChangedEvent() );
        return true;
    }

    virtual GUCEF::CORE::CString GetProfileId( void ) GUCEF_VIRTUAL_OVERRIDE { return id; }

    virtual GUCEF::CORE::CNotifier* GetProfileNotifier( void ) GUCEF_VIRTUAL_OVERRIDE { return this; }

    virtual const GUCEF::CORE::CEvent& GetProfileChangedEvent( void ) const GUCEF_VIRTUAL_OVERRIDE { return ChangedEvent(); }

    /** The name changes outside the page, as another window of the tool would do it */
    void ChangeElsewhere( const GUCEF::CORE::CString& newName )
    {
        name = newName;
        NotifyObservers( ChangedEvent() );
    }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CTestProfile";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  A settings form with the page "Account" (made by name from the page factory of the module) on the null driver, with a manager that
 *  has a profile key, stores in memory and fake providers, and a profile in memory: everything the page is tested with, without a
 *  network or a browser. The manager and the profile can be left out.
 */
class CAccountFixture
{
    public:

    TFakeLoginProviderFactory factory;      // the providers of the manager are made by it: it goes away after the manager
    GUCEF::GUI::TGuiContextPtr contextPtr;
    GUCEF::GUI::CGUIContext* context;
    CTestSigner signer;
    GUCEF::AUTH::CMemoryCredentialStore credentials;
    GUCEF::AUTH::CLocalIdentityLinkStore links;
    GUCEF::AUTH::CAuthManager manager;
    CTestProfile profile;
    GUCEF::GUI::CSettingsForm settings;
    GUCEF::AUTHGUI::CAccountSettingsForm* page;

    explicit CAccountFixture( GUCEF::UInt32 width = 1280, GUCEF::UInt32 height = 960, bool withManager = true, bool withProfile = true )
        : factory()
        , contextPtr( CreateSizedFormContext( width, height, true ) )
        , context( ContextOf( contextPtr ) )
        , signer( "profile" )
        , credentials()
        , links()
        , manager()
        , profile()
        , settings( context )
        , page( GUCEF_NULL )
    {
        if ( GUCEF_NULL == context || !signer.IsValid() )
            return;
        links.SetSigner( &signer );
        manager.SetSigner( &signer );
        manager.SetCredentialStore( &credentials );
        manager.SetLinkStore( &links );
        page = static_cast< GUCEF::AUTHGUI::CAccountSettingsForm* >( settings.AddPage( "Account" ) );
        if ( GUCEF_NULL == page )
            return;
        if ( withManager )
            page->SetAuthManager( &manager );
        if ( withProfile )
            page->SetProfile( &profile );
        settings.Build();
    }

    ~CAccountFixture()
    {
        if ( GUCEF_NULL != page )
        {
            page->SetAuthManager( GUCEF_NULL );
            page->SetProfile( GUCEF_NULL );
        }
        PumpFor( 10 );
    }

    bool IsReady( void ) const
    {
        return GUCEF_NULL != context && signer.IsValid() && GUCEF_NULL != page && GUCEF_NULL != settings.GetRootWidget();
    }

    /** A fake provider with the name, offered by the manager */
    CFakeLoginProvider* AddProvider( const GUCEF::CORE::CString& name )
    {
        GUCEF::AUTH::CIAuthProviderPtr product = factory.Create();
        CFakeLoginProvider* provider = static_cast< CFakeLoginProvider* >( product.GetPointerAlways() );
        provider->name = name;
        if ( !manager.AddProvider( product ) )
            return GUCEF_NULL;
        return provider;
    }

    /** Events of the providers reach the manager and the page, the page makes its content at the next frame */
    void Settle( void )
    {
        PumpFor( 30 );
        context->RenderFrame( 16 );
        context->FlushChanges();
        context->RenderFrame( 16 );
        context->FlushChanges();
    }

    /** Shows the settings with the page in front */
    bool Open( void )
    {
        if ( !settings.Open( "Account" ) )
            return false;
        Settle();
        return true;
    }

    private:

    CAccountFixture( const CAccountFixture& src );
    CAccountFixture& operator=( const CAccountFixture& src );
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_TESTAPP_TESTACCOUNTFIXTURE_H ? */
