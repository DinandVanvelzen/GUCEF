/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CDSTORECODEC_H
#define GUCEF_CORE_CDSTORECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"     /* abstract base class for cloneable objects */
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"        /* globally used generic data structures */
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"        /* framework string implementation */
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

class CDataNode;
class CIOAccess;

/*-------------------------------------------------------------------------*/

/**
 *      Abstract base class for an codec for storing an data tree
 *      and reading from a resource to a data tree.
 *      A codec is meta data that converts data from one type to another.
 */
class GUCEF_CORE_PUBLIC_CPP CDStoreCodec : public CICloneable
{
    public:

    CDStoreCodec( void );

    virtual ~CDStoreCodec();

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec meta data
     *
     *      @param tree the data tree you wish to store
     *      @param filename path and name of the file you wish to store the data in.
     *      @return whether storing the tree was successful
     */
    virtual bool StoreDataTree( const CDataNode* tree   ,
                                const CString& filename );

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec meta data
     *
     *      @param tree the data tree you wish to store
     *      @param file access to the resource to which the data will be written
     *      @return whether storing the tree was successful
     */
    virtual bool StoreDataTree( const CDataNode* tree   ,
                                CIOAccess* file         ) = 0;


    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @param filename path and name of the file from which the data tree information should be loaded
     *      @return whether building the tree from the given file was successful.
     */
    virtual bool BuildDataTree( CDataNode* treeroot     ,
                                const CString& filename );

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @param file media independent access to the data source from which the data tree information should be loaded
     *      @return whether building the tree from the given file was successful.
     */
    virtual bool BuildDataTree( CDataNode* treeroot ,
                                CIOAccess* file     ) = 0;

    /**
     *      Returns the name of the codec
     *      The author of the codec can give an codec a name that the user can
     *      obtain using this member function.
     *
     *      @return name of the codec
     */
    virtual CString GetName( void ) const = 0;

    /**
     *      Get the name of the type that this codec implements
     *      support for
     *
     *      @return the name of the resource type supported by this codec
     */
    virtual CString GetTypeName( void ) const = 0;

    /**
     *  Returns whether this codec type is data driven
     *  Data driven codecs require a data map or schema to be able to perform their transformations
     */
    virtual bool IsCodecTypeDataDriven( void ) const = 0;

    /**
     *      Returns the copyright of the codec
     *      The author of the codec can give an codec a copyright notice that the user can
     *      obtain using this member function.
     *
     *      @return copyright notice of the codec
     */
    virtual CString GetCopyright( void ) const = 0;

    /**
     *      Returns the codec version
     *
     *      @return the version of the codec
     */
    virtual TVersion GetVersion( void ) const = 0;

    private:
    CDStoreCodec( const CDStoreCodec& src );             /**< no sense in an copied codec */
    CDStoreCodec& operator=( const CDStoreCodec& src );  /**< no sense in an copied codec */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDSTORECODEC_H ? */
