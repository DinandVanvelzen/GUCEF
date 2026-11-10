/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
 *  Copyright (C) 2026.  Dinand Vanvelzen
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

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )
  #include <locale.h>
  #define GUCEF_VFSPLUGIN_LIBARCHIVE_HAVE_THREAD_LOCALE
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "vfspluginLIBARCHIVE_CUtf8LocaleScope.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef GUCEF_VFSPLUGIN_LIBARCHIVE_HAVE_THREAD_LOCALE

static locale_t
CreateUtf8Locale( void )
{GUCEF_TRACE;

    const char* candidates[] = { "C.UTF-8", "C.utf8", "en_US.UTF-8", "en_US.utf8" };
    for ( UInt32 i=0; i<sizeof( candidates ) / sizeof( candidates[ 0 ] ); ++i )
    {
        locale_t utf8Locale = ::newlocale( LC_CTYPE_MASK, candidates[ i ], (locale_t) 0 );
        if ( (locale_t) 0 != utf8Locale )
            return utf8Locale;
    }
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "vfspluginLIBARCHIVE: No UTF-8 locale is available, names which are not ASCII may not be handled" );
    return (locale_t) 0;
}

/*-------------------------------------------------------------------------*/

static locale_t
GetUtf8Locale( void )
{GUCEF_TRACE;

    // Created once and kept for the lifetime of the process, thread-safe initialization
    static locale_t utf8Locale = CreateUtf8Locale();
    return utf8Locale;
}

#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_HAVE_THREAD_LOCALE ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUtf8LocaleScope::CUtf8LocaleScope( void )
    : m_previousLocale( GUCEF_NULL )
    , m_switched( false )
{GUCEF_TRACE;

    #ifdef GUCEF_VFSPLUGIN_LIBARCHIVE_HAVE_THREAD_LOCALE
    locale_t utf8Locale = GetUtf8Locale();
    if ( (locale_t) 0 != utf8Locale )
    {
        m_previousLocale = (void*) ::uselocale( utf8Locale );
        m_switched = true;
    }
    #endif
}

/*-------------------------------------------------------------------------*/

CUtf8LocaleScope::~CUtf8LocaleScope()
{GUCEF_TRACE;

    #ifdef GUCEF_VFSPLUGIN_LIBARCHIVE_HAVE_THREAD_LOCALE
    if ( m_switched )
        ::uselocale( (locale_t) m_previousLocale );
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
