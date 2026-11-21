/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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


#ifndef GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H
#define GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFSASYNCCALLGATE_H
#include "gucefVFS_CVfsAsyncCallGate.h"
#define GUCEF_VFS_CVFSASYNCCALLGATE_H
#endif /* GUCEF_VFS_CVFSASYNCCALLGATE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Enters the gate for the length of a scope when the door is open, and leaves it at the end */
class GUCEF_VFS_PUBLIC_CPP CVfsAsyncCallGateScope
{
    public:

    explicit CVfsAsyncCallGateScope( const CVfsAsyncCallGatePtr& gate );

    ~CVfsAsyncCallGateScope();

    /** Whether the door was open: only then the owner of the gate may be used */
    bool IsEntered( void ) const;

    private:

    CVfsAsyncCallGateScope( const CVfsAsyncCallGateScope& src );                    /**< not implemented */
    CVfsAsyncCallGateScope& operator=( const CVfsAsyncCallGateScope& src );         /**< not implemented */

    CVfsAsyncCallGatePtr m_gate;
    bool m_entered;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSASYNCCALLGATESCOPE_H ? */
