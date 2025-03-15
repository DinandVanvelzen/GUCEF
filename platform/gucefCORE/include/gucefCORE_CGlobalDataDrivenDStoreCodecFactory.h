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

#ifndef GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H
#define GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H
#include "gucefCORE_CDataDrivenDStoreCodecFactory.h"         
#define GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

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
 *  Globally configurable version of the CDataDrivenDStoreCodecFactory
 */
class GUCEF_HIDDEN CGlobalDataDrivenDStoreCodecFactory : public CDataDrivenDStoreCodecFactory ,
                                                         public CGloballyConfigurable
{
    public:

    CGlobalDataDrivenDStoreCodecFactory( void );

    virtual ~CGlobalDataDrivenDStoreCodecFactory( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to store the given tree in the file
     *  given according to the method of the codec metadata
     *
     *  @param cfg the data tree you wish to store the config
     *  @return wheter storing all the config information to the provided tree was successfull
     */
    virtual bool SaveConfig( CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Attempts to load data from the given file to the
     *  root node given. The root data will be replaced
     *  and any children the node may already have will be deleted.
     *
     *  @param cfg node that is to act as root of the config data tree
     *  @return whether loading required/mandatory settings from the given config was successfull
     */
    virtual bool LoadConfig( const CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGlobalDataDrivenDStoreCodecFactory( const CGlobalDataDrivenDStoreCodecFactory& src );              /**< not implemented, not supported >*/
    CGlobalDataDrivenDStoreCodecFactory& operator=( const CGlobalDataDrivenDStoreCodecFactory& src );   /**< not implemented, not supported >*/

};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H ? */
