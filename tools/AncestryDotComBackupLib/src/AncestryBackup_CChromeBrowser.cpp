/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#include "AncestryBackup_CChromeBrowser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CChromeBrowser::CChromeBrowser( void )
    : m_browser()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void CChromeBrowser::SetExecutablePath( const CORE::CString& path ) {GUCEF_TRACE; m_browser.SetExecutablePath( path ); }
void CChromeBrowser::SetDebugPort( UInt16 port ) {GUCEF_TRACE; m_browser.SetDebugPort( port ); }
void CChromeBrowser::SetProfileDir( const CORE::CString& dir ) {GUCEF_TRACE; m_browser.SetProfileDir( dir ); }
void CChromeBrowser::SetHeadless( bool headless ) {GUCEF_TRACE; m_browser.SetHeadless( headless ); }
void CChromeBrowser::AddArgument( const CORE::CString& argument ) {GUCEF_TRACE; m_browser.AddArgument( argument ); }
UInt16 CChromeBrowser::GetDebugPort( void ) const {GUCEF_TRACE; return m_browser.GetDebugPort(); }
CORE::CString CChromeBrowser::FindExecutable( void ) const {GUCEF_TRACE; return m_browser.FindExecutable(); }
void CChromeBrowser::SetAttachToRunning( bool attach ) {GUCEF_TRACE; m_browser.SetAttachToRunning( attach ); }
bool CChromeBrowser::IsAttached( void ) const {GUCEF_TRACE; return m_browser.IsAttached(); }
bool CChromeBrowser::IsListening( UInt16 port ) const {GUCEF_TRACE; return m_browser.IsListening( port ); }
bool CChromeBrowser::IsAnswering( UInt32 timeoutInMs ) const {GUCEF_TRACE; return m_browser.IsAnswering( timeoutInMs ); }
bool CChromeBrowser::EnsureRunning( const CORE::CString& startUrl, UInt32 timeoutInMs ) {GUCEF_TRACE; return m_browser.EnsureRunning( startUrl, timeoutInMs ); }
bool CChromeBrowser::WasLaunched( void ) const {GUCEF_TRACE; return m_browser.WasLaunched(); }
bool CChromeBrowser::Shutdown( void ) {GUCEF_TRACE; return m_browser.Shutdown(); }
bool CChromeBrowser::ConnectToBrowser( WEB::CChromeDevToolsClient& client, UInt32 timeoutInMs ) const {GUCEF_TRACE; return m_browser.ConnectToBrowser( client, timeoutInMs ); }

/*-------------------------------------------------------------------------*/

bool
CChromeBrowser::ParseDevToolsActivePort( const CORE::CString& fileContent, UInt16& port, CORE::CString& webSocketPath )
{GUCEF_TRACE;

    return WEB::CDevToolsBrowser::ParseDevToolsActivePort( fileContent, port, webSocketPath );
}

/*-------------------------------------------------------------------------*/

std::vector< CORE::CString >
CChromeBrowser::BuildArguments( const CORE::CString& startUrl ) const
{GUCEF_TRACE;

    const CORE::CString::StringVector args = m_browser.BuildArguments( startUrl );
    return std::vector< CORE::CString >( args.begin(), args.end() );
}

/*-------------------------------------------------------------------------*/

std::vector< CORE::CString >
CChromeBrowser::GetDefaultUserDataDirs( void )
{GUCEF_TRACE;

    const CORE::CString::StringVector dirs = WEB::CDevToolsBrowser::GetDefaultUserDataDirs();
    return std::vector< CORE::CString >( dirs.begin(), dirs.end() );
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::SetUserDataDirs( const std::vector< CORE::CString >& dirs )
{GUCEF_TRACE;

    m_browser.SetUserDataDirs( CORE::CString::StringVector( dirs.begin(), dirs.end() ) );
}

/*-------------------------------------------------------------------------*/

void
CChromeBrowser::BuildCleanEnvironment( const std::vector< std::pair< std::string, std::string > >& environment ,
                                       std::vector< std::string >& unsetNames                                  ,
                                       std::map< std::string, std::string >& overrides                         )
{GUCEF_TRACE;

    WEB::CDevToolsBrowser::TEnvironment converted;
    for ( size_t i=0; i<environment.size(); ++i )
        converted.push_back( WEB::CDevToolsBrowser::TNameValue( environment[ i ].first, environment[ i ].second ) );
    CORE::CString::StringVector names;
    WEB::CDevToolsBrowser::TStringMap restored;
    WEB::CDevToolsBrowser::BuildCleanEnvironment( converted, names, restored );

    unsetNames.clear();
    overrides.clear();
    for ( size_t i=0; i<names.size(); ++i )
        unsetNames.push_back( names[ i ].STL_String() );
    for ( WEB::CDevToolsBrowser::TStringMap::const_iterator r = restored.begin(); r != restored.end(); ++r )
        overrides[ r->first.STL_String() ] = r->second.STL_String();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
