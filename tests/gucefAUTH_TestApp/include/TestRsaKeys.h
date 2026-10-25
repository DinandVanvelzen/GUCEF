/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTRSAKEYS_H
#define GUCEF_AUTH_TESTAPP_TESTRSAKEYS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestRsaVectors.h"
#include "TestRsaSigner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The key a provider publishes at first */
inline const CTestRsaKey&
TestKeyA( void )
{
    static CTestRsaKey key( g_testKeyA_n, g_testKeyA_e, g_testKeyA_p, g_testKeyA_q, g_testKeyA_dp, g_testKeyA_dq, g_testKeyA_qinv );
    return key;
}

/** The key a provider rotates to */
inline const CTestRsaKey&
TestKeyB( void )
{
    static CTestRsaKey key( g_testKeyB_n, g_testKeyB_e, g_testKeyB_p, g_testKeyB_q, g_testKeyB_dp, g_testKeyB_dq, g_testKeyB_qinv );
    return key;
}

/** A key that is in no key set: tokens signed with it name a key id that is there */
inline const CTestRsaKey&
TestKeyC( void )
{
    static CTestRsaKey key( g_testKeyC_n, g_testKeyC_e, g_testKeyC_p, g_testKeyC_q, g_testKeyC_dp, g_testKeyC_dq, g_testKeyC_qinv );
    return key;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTRSAKEYS_H ? */
