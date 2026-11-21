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


#ifndef GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H
#define GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFSFETCHPROGRESS_H
#include "gucefVFS_CVfsFetchProgress.h"
#define GUCEF_VFS_CVFSFETCHPROGRESS_H
#endif /* GUCEF_VFS_CVFSFETCHPROGRESS_H ? */

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

/**
 *  Makes a source of cancellation count as the cancel flag of a fetch progress for the length of a scope (see
 *  CVfsFetchProgress::SetCancelSource()), and takes it away at the end of the scope, before the source can go.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsFetchProgressCancelScope
{
    public:

    CVfsFetchProgressCancelScope( const CVfsFetchProgressPtr& progress, const CIVfsCancelSource* source );

    ~CVfsFetchProgressCancelScope();

    private:

    CVfsFetchProgressCancelScope( const CVfsFetchProgressCancelScope& src );                    /**< not implemented */
    CVfsFetchProgressCancelScope& operator=( const CVfsFetchProgressCancelScope& src );         /**< not implemented */

    CVfsFetchProgressPtr m_progress;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSFETCHPROGRESSCANCELSCOPE_H ? */
