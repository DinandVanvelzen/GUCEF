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

#ifndef GUCEF_CORE_CINIDATASTORECODEC_H
#define GUCEF_CORE_CINIDATASTORECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H ? */

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
 *  Data store codec providing an API to use the ini parser
 */
class GUCEF_CORE_PUBLIC_CPP CIniDataStoreCodec : public CDStoreCodec
{
    public:

    CIniDataStoreCodec( void );

    virtual ~CIniDataStoreCodec() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Whether or not the codec is currently valid
     *
     *      @return validity of the codec
     */
    virtual bool IsValid( void ) const;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec meta data
     *
     *      @param tree the data tree you wish to store
     *      @param filename path and name of the file you wish to store the data in.
     *      @return whether storing the tree was successful
     */
    virtual bool StoreDataTree( const CDataNode* tree   ,
                                const CString& filename ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec meta data
     *
     *      @param tree the data tree you wish to store
     *      @param file access to the resource to which the data will be written
     *      @return whether storing the tree was successful
     */
    virtual bool StoreDataTree( const CDataNode* tree   ,
                                CIOAccess* file         ) GUCEF_VIRTUAL_OVERRIDE;


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
                                const CString& filename ) GUCEF_VIRTUAL_OVERRIDE;

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
                                CIOAccess* file     ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the name of the codec
     *      The author of the codec can give an codec a name that the user can
     *      obtain using this member function.
     *
     *      @return name of the codec
     */
    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Get the name of the type that this codec implements
     *      support for
     *
     *      @return the name of the resource type supported by this codec
     */
    virtual CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsCodecTypeDataDriven( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the copyright of the codec
     *      The author of the codec can give an codec a copyright notice that the user can
     *      obtain using this member function.
     *
     *      @return copyright notice of the codec
     */
    virtual CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the codec version
     *
     *      @return the version of the codec
     */
    virtual TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    private:
    
    CIniDataStoreCodec( const CIniDataStoreCodec& src );             /**< no sense in an copied codec */
    CIniDataStoreCodec& operator=( const CIniDataStoreCodec& src );  /**< no sense in an copied codec */
    
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CINIDATASTORECODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 05-04-2005 :
        - Initial implementation

----------------------------------------------------------------------------*/
