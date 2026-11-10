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

#ifndef GUCEF_AUTHGUI_TESTAPP_TESTLOGINFIXTURE_H
#define GUCEF_AUTHGUI_TESTAPP_TESTLOGINFIXTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#include "gucefAUTH_CLocalIdentityLinkStore.h"
#define GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H ? */

#include "TestSigner.h"
#include "gucefAUTHGUI_TestCommon.h"
#include "TestFakeLoginProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A login form on the null driver with a manager that has a profile key (the signer), stores in memory and fake providers:
 *  everything the form is tested with, without a network or a browser
 */
class CLoginFixture
{
    public:

    TFakeLoginProviderFactory factory;      // the providers of the manager are made by it: it goes away after the manager
    GUCEF::GUI::TGuiContextPtr contextPtr;
    GUCEF::GUI::CGUIContext* context;
    CTestSigner signer;
    GUCEF::AUTH::CMemoryCredentialStore credentials;
    GUCEF::AUTH::CLocalIdentityLinkStore links;
    GUCEF::AUTH::CAuthManager manager;
    GUCEF::AUTHGUI::CLoginForm form;

    explicit CLoginFixture( GUCEF::UInt32 width = 1280, GUCEF::UInt32 height = 960 )
        : factory()
        , contextPtr( CreateSizedFormContext( width, height, true ) )
        , context( ContextOf( contextPtr ) )
        , signer( "profile" )
        , credentials()
        , links()
        , manager()
        , form( context )
    {
        if ( GUCEF_NULL == context || !signer.IsValid() )
            return;
        links.SetSigner( &signer );
        manager.SetSigner( &signer );
        manager.SetCredentialStore( &credentials );
        manager.SetLinkStore( &links );
        form.SetAuthManager( &manager );
    }

    ~CLoginFixture()
    {
        form.SetAuthManager( GUCEF_NULL );
        PumpFor( 10 );
    }

    bool IsReady( void ) const
    {
        return GUCEF_NULL != context && signer.IsValid();
    }

    GUCEF::AUTHGUI::CLoginForm& Form( void ) { return form; }

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

    /** Events of the providers reach the manager and the form, the form makes its content at the next frame */
    void Settle( void )
    {
        PumpFor( 30 );
        context->RenderFrame( 16 );
        context->FlushChanges();
        context->RenderFrame( 16 );
        context->FlushChanges();
    }

    private:

    CLoginFixture( const CLoginFixture& src );
    CLoginFixture& operator=( const CLoginFixture& src );
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHGUI_TESTAPP_TESTLOGINFIXTURE_H ? */
