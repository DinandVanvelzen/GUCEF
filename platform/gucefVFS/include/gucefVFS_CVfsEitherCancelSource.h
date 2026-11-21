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


#ifndef GUCEF_VFS_CVFSEITHERCANCELSOURCE_H
#define GUCEF_VFS_CVFSEITHERCANCELSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CIVFSCANCELSOURCE_H
#include "gucefVFS_CIVfsCancelSource.h"
#define GUCEF_VFS_CIVFSCANCELSOURCE_H
#endif /* GUCEF_VFS_CIVFSCANCELSOURCE_H ? */

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

/** Two sources of cancellation as one: asks to stop when either of them does. Neither is owned and both have to outlive it */
class GUCEF_VFS_PUBLIC_CPP CVfsEitherCancelSource : public CIVfsCancelSource
{
    public:

    CVfsEitherCancelSource( const CIVfsCancelSource& first, const CIVfsCancelSource& second );

    virtual ~CVfsEitherCancelSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CVfsEitherCancelSource( const CVfsEitherCancelSource& src );                    /**< not implemented */
    CVfsEitherCancelSource& operator=( const CVfsEitherCancelSource& src );         /**< not implemented */

    const CIVfsCancelSource& m_first;
    const CIVfsCancelSource& m_second;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSEITHERCANCELSOURCE_H ? */
