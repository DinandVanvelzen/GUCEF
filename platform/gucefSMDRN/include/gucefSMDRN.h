/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

/*
 *      This is the main include header for the gucefDRN library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with RAD networking functionality
 *      The facilities in this library are designed to make it easy to setup
 *      a networking infrastructure without having to worry about the details
 *      of low-level networking.
 */

#ifndef GUCEF_DRN_H
#define GUCEF_DRN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration      
 */
#ifndef GUCEF_DRN_CONFIG_H
#include "gucefDRN_config.h"
#define GUCEF_DRN_CONFIG_H
#endif /* GUCEF_DRN_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_DRN_ETYPES_H
#include "gucefDRN_ETypes.h"
#define GUCEF_DRN_ETYPES_H
#endif /* GUCEF_DRN_ETYPES_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNPACKETPROTECTION_H
#include "gucefDRN_CDRNPacketProtection.h"
#define GUCEF_DRN_CDRNPACKETPROTECTION_H
#endif /* GUCEF_DRN_CDRNPACKETPROTECTION_H ? */

#endif /* __cplusplus ? */
/*--------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 21-04-2007 :
       - Made the initial version of the library header.

-----------------------------------------------------------------------------*/
