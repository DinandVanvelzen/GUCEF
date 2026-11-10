/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTCODECPLUGINS_H
#define GUCEF_CORE_TESTAPP_TESTCODECPLUGINS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the codecs provided by codec plugins through the CORE codec registry:
 *  - compression codecs (codecspluginZLIB): round trips, partial writes, corrupt input
 *  - checksum codecs (codecspluginZLIB): known answer tests
 *  Tests for a plugin are skipped if the plugin is not available.
 */
void
PerformCodecPluginTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTCODECPLUGINS_H ? */
