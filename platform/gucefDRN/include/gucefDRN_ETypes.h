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

#ifndef GUCEF_DRN_ETYPES_H
#define GUCEF_DRN_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Maximum and minimal values for the simple types which we are about to
 *      define.
 */
#define GUCEFDRN_INT8MAX     GUCEFCORE_INT8MAX
#define GUCEFDRN_INT8MIN     GUCEFCORE_INT8MIN
#define GUCEFDRN_UINT8MAX    GUCEFCORE_UINT8MAX
#define GUCEFDRN_UINT8MIN    GUCEFCORE_UINT8MIN
#define GUCEFDRN_INT16MAX    GUCEFCORE_INT16MAX
#define GUCEFDRN_INT16MIN    GUCEFCORE_INT16MIN
#define GUCEFDRN_UINT16MAX   GUCEFCORE_UINT16MAX
#define GUCEFDRN_UINT16MIN   GUCEFCORE_UINT16MIN
#define GUCEFDRN_INT32MAX    GUCEFCORE_INT32MAX
#define GUCEFDRN_INT32MIN    GUCEFCORE_INT32MIN
#define GUCEFDRN_UINT32MAX   GUCEFCORE_UINT32MAX
#define GUCEFDRN_UINT32MIN   GUCEFCORE_UINT32MIN
#define GUCEFDRN_FLOAT32MAX  GUCEFCORE_FLOAT32MAX
#define GUCEFDRN_FLOAT32MIN  GUCEFCORE_FLOAT32MIN

/*--------------------------------------------------------------------------*/

/*
 *  When using C++ we should use namespacing
 */
#ifdef __cplusplus
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::UInt8 UInt8;    /* 1 byte, unsigned */
typedef CORE::Int8 Int8;      /* 1 byte, signed */

typedef CORE::UInt16 UInt16;   /* 2 bytes, unsigned */
typedef CORE::Int16 Int16;     /* 2 bytes, signed */

typedef CORE::UInt32 UInt32;   /* 4 bytes, unsigned */
typedef CORE::Int32  Int32;    /* 4 bytes, signed */

typedef CORE::Int64 Int64;     /* 8 bytes, signed */
typedef CORE::UInt64 UInt64;   /* 8 bytes, unsigned */

typedef CORE::Float32 Float32;  /* 4 bytes, signed, decimal */
typedef CORE::Float64 Float64;  /* 8 bytes, signed, decimal */

typedef CORE::TBuildinDataType TBuildinDataType;

/** The protection class of a replicated item or an RPC method (gucefDRN item protection) */
enum TDRNProtectionClass
{
    DRN_PROTECTION_CONFIDENTIAL     = 0,    /**< sealed: every packet that carries it is encrypted and authenticated (default) */
    DRN_PROTECTION_PUBLIC           = 1     /**< readable by an observer of the link; altering it is detected (mac) or not (none) */
};

/** How the messages of PUBLIC items travel on a protected link (CDRNSecurityPolicy::publicItemProtection) */
enum TDRNPublicItemProtection
{
    DRN_PUBLICPROTECTION_MAC        = 0,    /**< in MAC packets: clear body, authenticated with the link's AEAD (default) */
    DRN_PUBLICPROTECTION_NONE       = 1     /**< lane 3 messages in PLAIN packets without any protection, the rest in MAC packets */
};

/** Why a stream replica missed entries (CDRNStreamGapInfo) */
enum TDRNStreamGapReason
{
    DRN_STREAMGAP_TRIMMED           = 1,    /**< entries arrived above the expected one: the serving side no longer had those between */
    DRN_STREAMGAP_BEYOND_RETENTION  = 2,    /**< a (re)subscription started above the sequence asked for: the serving side trimmed them */
    DRN_STREAMGAP_EPOCH_CHANGED     = 3     /**< the stream has another epoch than the one resumed from: the numbering restarted */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* __cplusplus ? */

#endif /* GUCEF_DRN_ETYPES_H ? */