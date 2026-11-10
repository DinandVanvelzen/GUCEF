/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTAUTHIDENTITY_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTAUTHIDENTITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the identity with linked accounts: the profile key as a signer, the "sso" provider (accounts, name, proofs,
 *  link and unlink, events), the private key in the credential store and its migration
 */
void
PerformAuthIdentityTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTAUTHIDENTITY_H ? */
