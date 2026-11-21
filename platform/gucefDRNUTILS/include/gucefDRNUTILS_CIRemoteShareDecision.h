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

#ifndef GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H
#define GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICERECORD_H
#include "gucefDRNUTILS_CDeviceRecord.h"
#define GUCEF_DRNUTILS_CDEVICERECORD_H
#endif /* GUCEF_DRNUTILS_CDEVICERECORD_H ? */

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
 *  What a product decides about a share of its remote service (CRemoteServiceHost::RegisterShare()), beyond what the host does itself.
 *  The host (CDeviceDrnAccessPolicy) has resolved the member to its device in the device registry at this call, found it ACTIVE with one of
 *  the roles of the share, and the share rules of the facility have said the name is shared: 'device' is that record, with the roles it has now
 *  (not those of its token). Names are relative to the share and have passed the resource name rules of the facility (no "..", no absolute
 *  or drive forms, no control characters).
 *
 *  The hooks are called on the thread of the DRN node, for every request of a member: they must not block, wait for another thread or do
 *  input or output; a decision that needs slow data keeps it primed in memory and refuses (retryably) while it has not got it. The host
 *  holds the share's registration while a hook runs, so an object is not deleted under a call: UnregisterShare() returns after the calls under way.
 *
 *  Defaults: browsing and listing allowed, reading refused (a product has to say what may be read).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CIRemoteShareDecision
{
    public:

    CIRemoteShareDecision( void );
    virtual ~CIRemoteShareDecision();

    /** Whether the device may read the bytes of the file; when not, 'refusal' may say why in plain words (shown to the device) */
    virtual bool MayRead( const CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, CORE::CString& refusal );

    /**
     *  Whether the device may read the bytes of the file, and when it may not, whether that is temporary: 'retryLater' set means the
     *  device is not refused but told to ask again (BUSY), which a decision uses for data it has not got yet and is getting (it still must
     *  not wait for it here). The default sets 'retryLater' to false and asks MayRead(), so a decision that does not override this behaves as before.
     */
    virtual bool CheckRead( const CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, CORE::CString& refusal, bool& retryLater );

    /** Whether the device may see the name, and its size and time (STAT, and LIST of a directory; the root is the empty name) */
    virtual bool MayBrowse( const CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name );

    /** Whether the entry is left out of listings and is not found by name (hidden entries) */
    virtual bool MayListEntry( const CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, bool isDirectory );

    private:

    CIRemoteShareDecision( const CIRemoteShareDecision& src );                  /**< not implemented */
    CIRemoteShareDecision& operator=( const CIRemoteShareDecision& src );       /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H ? */
