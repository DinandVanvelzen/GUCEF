/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_DRNUTILS_CDEVICECONTRACTS_H
#define GUCEF_DRNUTILS_CDEVICECONTRACTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The contract gucef.drnutils.devices 1.0 of the remote service kit: Pairing (only on the pairing listener, authenticated
 *  by the one-time code) and Devices (administration by admin devices), built in code with COMCORE::CRpcContractBuilder.
 *  The authoring source is platform/gucefDRNUTILS/rpc/gucef.drnutils.devices.json
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDeviceContracts
{
    public:

    static const char* ContractName;    /**< "gucef.drnutils.devices" */
    static const UInt32 Major = 1;
    static const char* PairingServiceName;          /**< "Pairing" */
    static const char* DevicesServiceName;          /**< "Devices" */
    static const char* BeginPath;                   /**< "gucef.drnutils.devices.Pairing/Begin" */
    static const char* CompletePath;                /**< ".../Complete" */
    static const char* ListPath;                    /**< "gucef.drnutils.devices.Devices/List" */
    static const char* RevokePath;
    static const char* SetRolesPath;
    static const char* RenamePath;
    static const char* OpenPairingWindowPath;
    static const char* ClosePairingWindowPath;
    static const char* ListPendingPairingsPath;
    static const char* ConfirmPairingPath;
    static const char* ListSessionsPath;
    static const char* CloseSessionPath;

    /** gucef.drnutils.devices 1.0, built once and registered in the contract registry of gucefCOMCORE; NULL when building failed (logged) */
    static COMCORE::CRpcContractPtr GetContract( void );

    /** The authoring form of the contract */
    static void GetContractSource( CORE::CDataNode& source );

    private:

    CDeviceContracts( void );       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICECONTRACTS_H ? */
