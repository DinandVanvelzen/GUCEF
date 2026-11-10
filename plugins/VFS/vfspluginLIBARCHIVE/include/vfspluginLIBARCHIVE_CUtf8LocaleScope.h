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

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_CUTF8LOCALESCOPE_H
#define GUCEF_VFSPLUGIN_LIBARCHIVE_CUTF8LOCALESCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#include "vfspluginLIBARCHIVE_macros.h"
#define GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  libarchive converts names using the character set of the current locale. Applications that never call
 *  setlocale() run in the "C" locale in which names that are not ASCII cannot be converted, which makes libarchive
 *  fail on them. Rather than changing the locale of the whole application this switches only the calling thread
 *  to a UTF-8 locale for the lifetime of the object. On platforms without per thread locales it does nothing.
 */
class CUtf8LocaleScope
{
    public:

    CUtf8LocaleScope( void );

    ~CUtf8LocaleScope();

    private:

    CUtf8LocaleScope( const CUtf8LocaleScope& src );            /**< not supported */
    CUtf8LocaleScope& operator=( const CUtf8LocaleScope& src ); /**< not supported */

    void* m_previousLocale;
    bool m_switched;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_CUTF8LOCALESCOPE_H ? */
