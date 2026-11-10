/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefCORE_CUri.h"

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

CUri::CUri( void )
    : m_scheme()
    , m_authorityUsername()
    , m_authorityPassword()
    , m_authorityHost()
    , m_authorityPort()
    , m_path()
    , m_query()
    , m_fragment()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUri::CUri( const CUri& src )
    : m_scheme( src.m_scheme )
    , m_authorityUsername( src.m_authorityUsername )
    , m_authorityPassword( src.m_authorityPassword )
    , m_authorityHost( src.m_authorityHost )
    , m_authorityPort( src.m_authorityPort )
    , m_path( src.m_path )
    , m_query( src.m_query )
    , m_fragment( src.m_fragment )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUri::CUri( const CString& uriInStringForm )
    : m_scheme()
    , m_authorityUsername()
    , m_authorityPassword()
    , m_authorityHost()
    , m_authorityPort()
    , m_path()
    , m_query()
    , m_fragment()
{GUCEF_TRACE;

    ParseUriInStringForm( uriInStringForm );
}

/*-------------------------------------------------------------------------*/

CUri::~CUri()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CUri& 
CUri::operator=( const CUri& src )
{GUCEF_TRACE;
    
    if ( this != &src )
    {
        Clear();

        m_scheme = src.m_scheme;
        m_authorityUsername = src.m_authorityUsername;
        m_authorityPassword = src.m_authorityPassword;
        m_authorityHost = src.m_authorityHost;
        m_authorityPort = src.m_authorityPort;
        m_path = src.m_path;
        m_query = src.m_query;
        m_fragment = src.m_fragment;
    }

    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CUri::operator==( const CUri& other ) const
{GUCEF_TRACE;

    return m_scheme == other.m_scheme &&
           m_authorityUsername == other.m_authorityUsername &&
           m_authorityPassword == other.m_authorityPassword &&
           m_authorityHost == other.m_authorityHost &&
           m_authorityPort == other.m_authorityPort &&
           m_path == other.m_path &&
           m_query == other.m_query &&
           m_fragment == other.m_fragment;        
}

/*-------------------------------------------------------------------------*/

bool
CUri::operator!=( const CUri& other ) const
{GUCEF_TRACE;

    return m_scheme != other.m_scheme ||
           m_authorityUsername != other.m_authorityUsername ||
           m_authorityPassword != other.m_authorityPassword ||
           m_authorityHost != other.m_authorityHost ||
           m_authorityPort != other.m_authorityPort ||
           m_path != other.m_path ||
           m_query != other.m_query ||
           m_fragment != other.m_fragment;        
}

/*-------------------------------------------------------------------------*/

bool
CUri::operator<( const CUri& other ) const
{GUCEF_TRACE;

    return m_scheme < other.m_scheme &&
           m_authorityUsername < other.m_authorityUsername &&
           m_authorityPassword < other.m_authorityPassword &&
           m_authorityHost < other.m_authorityHost &&
           m_authorityPort < other.m_authorityPort &&
           m_path < other.m_path &&
           m_query < other.m_query &&
           m_fragment < other.m_fragment;       
}

/*-------------------------------------------------------------------------*/

bool
CUri::operator==( const CString& other ) const
{GUCEF_TRACE;

    return ToUriInStringForm() == other;       
}

/*-------------------------------------------------------------------------*/

bool
CUri::operator!=( const CString& other ) const
{GUCEF_TRACE;

    return ToUriInStringForm() != other;       
}

/*-------------------------------------------------------------------------*/

bool
CUri::operator<( const CString& other ) const
{GUCEF_TRACE;

    return ToUriInStringForm() < other;       
}

/*-------------------------------------------------------------------------*/

CUri::operator CString() const
{GUCEF_TRACE;

    return ToUriInStringForm();
}

/*-------------------------------------------------------------------------*/

bool 
CUri::HasAuthority( void ) const
{GUCEF_TRACE;

    return !( m_authorityUsername.IsNULLOrEmpty() && 
              m_authorityPassword.IsNULLOrEmpty() &&
              m_authorityHost.IsNULLOrEmpty()     &&
              m_authorityPort.IsNULLOrEmpty()     );
}

/*-------------------------------------------------------------------------*/

CString
CUri::ToUriInStringForm( void ) const
{GUCEF_TRACE;

    if ( HasAuthority() )
    {
        CString uriStr = m_scheme.Lowercase() + "://" + GetAuthority() + '/' + m_path;
        if ( !m_query.IsNULLOrEmpty() )
            uriStr += '?' + m_query;
        if ( !m_fragment.IsNULLOrEmpty() )
            uriStr += '#' + m_fragment;
        return uriStr;
    }
    else
    {
        CString uriStr = m_scheme.Lowercase() + ':' + m_path;
        if ( !m_query.IsNULLOrEmpty() )
            uriStr += '?' + m_query;
        if ( !m_fragment.IsNULLOrEmpty() )
            uriStr += '#' + m_fragment;
        return uriStr;        
    }
}

/*-------------------------------------------------------------------------*/

void
CUri::SetScheme( const CString& scheme )
{GUCEF_TRACE;

    m_scheme = scheme;
}

/*-------------------------------------------------------------------------*/

const CString& 
CUri::GetScheme( void ) const
{GUCEF_TRACE;

    return m_scheme;
}

/*-------------------------------------------------------------------------*/

void
CUri::SetAuthority( const CString& authority )
{GUCEF_TRACE;

    m_authorityUsername = authority;
}

/*-------------------------------------------------------------------------*/

CString 
CUri::GetAuthority( void ) const
{GUCEF_TRACE;

    CString uriAuthorityStr;
    if ( !m_authorityPassword.IsNULLOrEmpty() )    
    {
        uriAuthorityStr = m_authorityUsername + ':' + m_authorityPassword + '@';
    }
    else
    if ( !m_authorityUsername.IsNULLOrEmpty() )
    {
        uriAuthorityStr = m_authorityUsername + '@';
    }

    uriAuthorityStr += m_authorityHost;

    if ( !m_authorityPort.IsNULLOrEmpty() )
        uriAuthorityStr += ':' + m_authorityPort; 

    return uriAuthorityStr;
}

/*-------------------------------------------------------------------------*/

CString 
CUri::GetAuthorityAndPath( void ) const
{GUCEF_TRACE;

    if ( HasAuthority() )
    {
        return GetAuthority() + '/' + m_path;
    }
    else
    {
        return m_path;
    }
}

/*-------------------------------------------------------------------------*/

void 
CUri::SetAuthorityUsername( const CString& username )
{GUCEF_TRACE;

    m_authorityUsername = username;
}

/*-------------------------------------------------------------------------*/

const CString&
CUri::GetAuthorityUsername( void ) const
{GUCEF_TRACE;

    return m_authorityUsername;
}

/*-------------------------------------------------------------------------*/

void
CUri::SetAuthorityPassword( const CString& password )
{GUCEF_TRACE;

    m_authorityPassword = password;
}

/*-------------------------------------------------------------------------*/

const CString& 
CUri::GetAuthorityPassword( void ) const
{GUCEF_TRACE;

    return m_authorityPassword;
}

/*-------------------------------------------------------------------------*/

void 
CUri::SetAuthorityHost( const CString& host )
{GUCEF_TRACE;

    m_authorityHost = host;
}

/*-------------------------------------------------------------------------*/

const CString& 
CUri::GetAuthorityHost( void ) const
{GUCEF_TRACE;

    return m_authorityHost;
}

/*-------------------------------------------------------------------------*/

void
CUri::SetAuthorityPort( const CString& port )
{GUCEF_TRACE;

    m_authorityPort = port;
}

/*-------------------------------------------------------------------------*/

const CString& 
CUri::GetAuthorityPort( void ) const
{GUCEF_TRACE;

    return m_authorityPort;
}

/*-------------------------------------------------------------------------*/

void
CUri::SetPath( const CString& path )
{GUCEF_TRACE;

    m_path = path;
}

/*-------------------------------------------------------------------------*/

const CString&
CUri::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

void 
CUri::SetQuery( const CString& query )
{GUCEF_TRACE;

    m_query = query;
}

/*-------------------------------------------------------------------------*/

const CString& 
CUri::GetQuery( void ) const
{GUCEF_TRACE;

    return m_query;
}

/*-------------------------------------------------------------------------*/

void 
CUri::SetFragment( const CString& fragment )
{GUCEF_TRACE;

    m_fragment = fragment;
}

/*-------------------------------------------------------------------------*/

const CString& 
CUri::GetFragment( void ) const
{GUCEF_TRACE;

    return m_fragment;
}

/*-------------------------------------------------------------------------*/

void
CUri::Clear( void )
{GUCEF_TRACE;

    m_scheme.Clear();
    m_authorityUsername.Clear();
    m_authorityPassword.Clear();
    m_authorityHost.Clear();
    m_authorityPort.Clear();
    m_path.Clear();
    m_query.Clear();
    m_fragment.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CUri::ParseUriInStringForm( const CString& uriInStringForm )
{GUCEF_TRACE;
    
    Clear();

    // the end of the scheme segment is signaled via a colon
    Int32 schemeDelimiter = uriInStringForm.HasChar( ':', true );
    if ( schemeDelimiter > 0 )
    {
        bool passedSanityCheck = true;
        
        m_scheme = uriInStringForm.SubstrToIndex( schemeDelimiter, true );
        
        // an Authority segment is signaled via a double slash
        Int32 authorityDelimiter = uriInStringForm.HasSubstr( "//", schemeDelimiter, true );
        if ( authorityDelimiter > 0 )
        {
            // we have an optional authority segment, lets parse it

            Int32 userPassDelimiter = uriInStringForm.HasChar( '@', authorityDelimiter+2, true );
            Int32 authoritySegmentEndDelimiter = uriInStringForm.HasChar( '/', authorityDelimiter+2, true );

            if ( authoritySegmentEndDelimiter < 0 ) // we should always end the authority segment in a slash unless there simply is no next segment
                authoritySegmentEndDelimiter = (Int32) uriInStringForm.Length();

            if ( userPassDelimiter > authorityDelimiter )
            {
                // we have a optional username and potentially a password prefix

                Int32 userVsPassDelimiter = uriInStringForm.HasChar( ':', authorityDelimiter, true );
                if ( userVsPassDelimiter > 0 && userVsPassDelimiter < userPassDelimiter )
                {
                    // we have both a username and a password

                    m_authorityUsername = uriInStringForm.SubstrFromRange( authorityDelimiter+2, userVsPassDelimiter );
                    m_authorityPassword = uriInStringForm.SubstrFromRange( userVsPassDelimiter+1, userPassDelimiter );
                }
                else
                {
                    // We only have a username
                    
                    m_authorityUsername = uriInStringForm.SubstrFromRange( authorityDelimiter+2, userPassDelimiter );
                }

                // now get the second half which is the hostname with an optional port
                
                Int32 portColonDelimiter = uriInStringForm.HasChar( ':', userPassDelimiter, true );
                if ( portColonDelimiter > 0 && portColonDelimiter < authoritySegmentEndDelimiter )
                {
                    // we have an optional port segment
                    m_authorityHost = uriInStringForm.SubstrFromRange( userPassDelimiter+1, portColonDelimiter );
                    m_authorityPort = uriInStringForm.SubstrFromRange( portColonDelimiter+1, authoritySegmentEndDelimiter );
                }
                else
                {
                    // We only have a host specified but no port
                    m_authorityHost = uriInStringForm.SubstrFromRange( userPassDelimiter+1, authoritySegmentEndDelimiter );
                }
            }
            else
            {
                // Just get the hostname with an optional port
                
                Int32 portColonDelimiter = uriInStringForm.HasChar( ':', authorityDelimiter+2, true );
                if ( portColonDelimiter > 0 && portColonDelimiter < authoritySegmentEndDelimiter )
                {
                    // we have an optional port segment
                    m_authorityHost = uriInStringForm.SubstrFromRange( authorityDelimiter+2, portColonDelimiter );
                    m_authorityPort = uriInStringForm.SubstrFromRange( portColonDelimiter+1, authoritySegmentEndDelimiter );
                }
                else
                {
                    // We only have a host specified but no port
                    m_authorityHost = uriInStringForm.SubstrFromRange( authorityDelimiter+2, authoritySegmentEndDelimiter );
                }                
            }

            // An empty authority, as in "file:///dir/file", is allowed. It leaves the path, with its leading slash,
            // which is how a uri without authority holds an absolute path.
            bool hasEmptyAuthority = m_authorityHost.IsNULLOrEmpty() && authoritySegmentEndDelimiter == authorityDelimiter+2;

            // Since we had an authority segment at minimum we should have a hostname
            if ( m_authorityHost.IsNULLOrEmpty() && !hasEmptyAuthority )
                passedSanityCheck = false;

            if ( hasEmptyAuthority )
                --authoritySegmentEndDelimiter; // the path keeps its leading slash, the code below skips one character

            Int32 queryDelimiter = uriInStringForm.HasChar( '?', authoritySegmentEndDelimiter+1, true );
            if ( queryDelimiter > 0 )
            {
                // we have additional segment(s) so we only want a subset of the remainder as the path
                m_path = uriInStringForm.SubstrFromRange( authoritySegmentEndDelimiter+1, queryDelimiter );

                // the query section itself can still have a fragment behind it as well
                Int32 fragmentDelimiter = uriInStringForm.HasChar( '#', queryDelimiter+1, true );
                if ( fragmentDelimiter > 0 )
                {
                    // We also have a fragment, so we only want a sub string as the query
                    m_query = uriInStringForm.SubstrFromRange( queryDelimiter+1, fragmentDelimiter );
                    m_fragment = uriInStringForm.SubstrToIndex( (UInt32) fragmentDelimiter+1, false );
                }
                else
                {
                    // we dont have a fragment segment so everything after the question mark is part of the query segment
                    m_query = uriInStringForm.SubstrToIndex( (UInt32) queryDelimiter+1, false );
                }
            }
            else
            {
                // we dont have a query segment so everything after the authority segment is part of the path
                m_path = uriInStringForm.SubstrToIndex( (UInt32) authoritySegmentEndDelimiter+1, false );
            }
        }
        else
        {
            // less common, especially with most uri's tending to be url's, is the absence of the authority segment
            // this is allowed in which case we transition straight into the path etc segments

            Int32 queryDelimiter = uriInStringForm.HasChar( '?', schemeDelimiter+1, true );
            if ( queryDelimiter > 0 )
            {
                // we have additional segment(s) so we only want a subset of the remainder as the path
                m_path = uriInStringForm.SubstrFromRange( schemeDelimiter+1, queryDelimiter );

                // the query section itself can still have a fragment behind it as well
                Int32 fragmentDelimiter = uriInStringForm.HasChar( '#', queryDelimiter+1, true );
                if ( fragmentDelimiter > 0 )
                {
                    // We also have a fragment, so we only want a sub string as the query
                    m_query = uriInStringForm.SubstrFromRange( queryDelimiter+1, fragmentDelimiter );
                    m_fragment = uriInStringForm.SubstrToIndex( (UInt32) fragmentDelimiter+1, false );
                }
                else
                {
                    // we dont have a fragment segment so everything after the question mark is part of the query segment
                    m_query = uriInStringForm.SubstrToIndex( (UInt32) queryDelimiter+1, false );
                }
            }
            else
            {
                // we dont have a query segment so everything after the scheme segment is part of the path
                m_path = uriInStringForm.SubstrToIndex( (UInt32) schemeDelimiter+1, false );
            }

            // Since we did not have an authority segment at minimum we should have a path
            if ( m_path.IsNULLOrEmpty() )
                passedSanityCheck = false;
        }

        return passedSanityCheck;
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

void
CUri::ReplaceSubstr( const CString& oldSubstr, const CString& newSubstr )
{GUCEF_TRACE;

    m_scheme = m_scheme.ReplaceSubstr( oldSubstr, newSubstr );
    m_authorityUsername = m_authorityUsername.ReplaceSubstr( oldSubstr, newSubstr );
    m_authorityPassword = m_authorityPassword.ReplaceSubstr( oldSubstr, newSubstr );
    m_authorityHost = m_authorityHost.ReplaceSubstr( oldSubstr, newSubstr );
    m_authorityPort = m_authorityPort.ReplaceSubstr( oldSubstr, newSubstr );
    m_path = m_path.ReplaceSubstr( oldSubstr, newSubstr );
    m_query = m_query.ReplaceSubstr( oldSubstr, newSubstr );
    m_fragment = m_fragment.ReplaceSubstr( oldSubstr, newSubstr );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
