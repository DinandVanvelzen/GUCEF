/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_GAMEGUI_TESTAPP_TESTLISTINGPROVIDER_H
#define GUCEF_GAMEGUI_TESTAPP_TESTLISTINGPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A listing provider whose listings the test sets: the source that feeds a CGameBrowser in the tests */
class CTestListingProvider : public GUCEF::MULTIPLAY::CIGameListingProvider
{
    public:

    CTestListingProvider( void ) : GUCEF::MULTIPLAY::CIGameListingProvider(), m_active( false ), m_refreshCount( 0 ) {}

    virtual bool Start( const GUCEF::MULTIPLAY::CGameListingFilter& filter ) GUCEF_VIRTUAL_OVERRIDE { SetFilter( filter ); m_active = true; return true; }
    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE { m_active = false; RemoveAllListings(); }
    virtual bool Refresh( void ) GUCEF_VIRTUAL_OVERRIDE { ++m_refreshCount; return m_active; }
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_active; }
    virtual const GUCEF::CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString providerName = "test-listings";
        return providerName;
    }
    virtual bool SaveConfig( GUCEF::CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CTestListingProvider";
        return classTypeName;
    }

    void Put( const GUCEF::MULTIPLAY::CGameListingEntry& listing ) { UpsertListing( listing ); }
    void Drop( const GUCEF::CORE::CString& listingId ) { RemoveListing( listingId ); }

    GUCEF::UInt32 GetRefreshCount( void ) const { return m_refreshCount; }

    private:

    bool m_active;
    GUCEF::UInt32 m_refreshCount;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::CORE::CTFactory< GUCEF::MULTIPLAY::CIGameListingProvider, CTestListingProvider, GUCEF::MT::CMutex > TTestListingProviderFactory;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_TESTAPP_TESTLISTINGPROVIDER_H ? */
