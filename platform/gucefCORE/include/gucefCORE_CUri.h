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

#ifndef GUCEF_CORE_CURI_H
#define GUCEF_CORE_CURI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  class that represents a Universal Resource Identifier
 */
class GUCEF_CORE_PUBLIC_CPP CUri
{
    public:

    typedef GUCEF::vector< CUri >    UriVector;
    typedef GUCEF::set< CUri >       UriSet;

    CUri( void );

    CUri( const CUri& src );

    CUri( const CString& uriInStringForm );

    ~CUri();

    CUri& operator=( const CUri& src );

    bool operator==( const CUri& other ) const;

    bool operator!=( const CUri& other ) const;

    bool operator<( const CUri& other ) const;

    bool operator==( const CString& other ) const;

    bool operator!=( const CString& other ) const;

    bool operator<( const CString& other ) const;    

    operator CString () const;

    void SetScheme( const CString& scheme );

    const CString& GetScheme( void ) const;

    void SetAuthority( const CString& authority );

    CString GetAuthority( void ) const;

    bool HasAuthority( void ) const;

    /**
     *  The authority and path segments are distinct sections in a URI with different meaning
     *  However due to the widespread usage or URLs most folks are used to something starting with ://
     *  Technically you should have <scheme>:<path> if you dont have an applicable authority segment
     *  However since most folks would not practically make that distinction and would typically type <scheme>://<path>
     *  that would cause the first path segment up to '/' to be interpreted as an 'authority'
     *  Based on the usecase, knowing your scheme, you can cover for this common mistake by using this member function
     */
    CString GetAuthorityAndPath( void ) const;

    void SetAuthorityUsername( const CString& username );

    const CString& GetAuthorityUsername( void ) const;

    void SetAuthorityPassword( const CString& password );

    const CString& GetAuthorityPassword( void ) const;

    void SetAuthorityHost( const CString& host );

    const CString& GetAuthorityHost( void ) const;

    void SetAuthorityPort( const CString& port );

    const CString& GetAuthorityPort( void ) const;

    void SetPath( const CString& path );

    const CString& GetPath( void ) const;

    void SetQuery( const CString& query );

    const CString& GetQuery( void ) const;

    void SetFragment( const CString& fragment );

    const CString& GetFragment( void ) const;

    void Clear( void );

    bool ParseUriInStringForm( const CString& uriInStringForm );

    CString ToUriInStringForm( void ) const;

    void ReplaceSubstr( const CString& oldSubstr, const CString& newSubstr );

    protected:

    CString m_scheme;
    CString m_authorityUsername;
    CString m_authorityPassword;
    CString m_authorityHost;
    CString m_authorityPort;
    CString m_path;
    CString m_query;
    CString m_fragment;
};

/*-------------------------------------------------------------------------*/

inline CString ToString( const CUri& uri ) { return uri.ToUriInStringForm(); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CURI_H ? */
