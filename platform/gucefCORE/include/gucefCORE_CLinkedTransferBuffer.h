/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CLINKEDTRANSFERBUFFER_H
#define GUCEF_CORE_CLINKEDTRANSFERBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H
#include "CTLinkedCloneableObj.h"
#define GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Utility class for passing a transfer buffer as a cloneable object by means of linking.
 *  Transfer buffers are typically the sort of thing where you have large buffers that you
 *  do not want to copy unless absolutely needed. This utility class provides a convenient
 *  way of passing along all information
 */
class GUCEF_CORE_PUBLIC_CPP CLinkedTransferBuffer : public CTLinkedCloneableObj< CDynamicBuffer >
{
    public:

    CLinkedTransferBuffer( void );

    CLinkedTransferBuffer( const CDynamicBuffer* buffer             ,
                           const Float32 transferProgress = 0       ,
                           const Int64 totalSize = -1               ,
                           const CICloneable* additionalData = GUCEF_NULL );

    CLinkedTransferBuffer( const CLinkedTransferBuffer& src );

    virtual ~CLinkedTransferBuffer();

    CLinkedTransferBuffer& operator=( const CLinkedTransferBuffer& src );

    virtual CICloneable* Clone( void ) const;

    void SetTransferProgress( const Float32 transferProgress );

    Float32 GetTransferProgress( void ) const;

    void SetTotalSize( const Int64 totalSize );

    // Returns the total size if known, -1 if unknown.
    Int64 GetTotalSize( void ) const;

    const CICloneable* GetAdditionalData( void ) const;

    private:

    Float32 m_transferProgress;
    CICloneable* m_additionalData;
    Int64 m_totalSize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CLINKEDTRANSFERBUFFER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-10-2006 :
        - Initial implementation

---------------------------------------------------------------------------*/
