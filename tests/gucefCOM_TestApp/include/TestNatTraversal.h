/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COM_TESTNATTRAVERSAL_H
#define GUCEF_COM_TESTNATTRAVERSAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The NAT-PMP / PCP port mapper against a scripted gateway on loopback */
void PerformNatPmpTests( void );

/** The STUN client against a scripted server on loopback */
void PerformStunTests( void );

/** The UPnP port mapper of the comcorepluginUPNP plugin */
void PerformUpnpPluginTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_TESTNATTRAVERSAL_H ? */
