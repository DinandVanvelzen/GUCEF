/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#ifndef GUCEF_KAITAI_CKAITAIDSTORECODEC_H
#define GUCEF_KAITAI_CKAITAIDSTORECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#include "gucefCORE_CDataDrivenDStoreCodec.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODEC_H ? */

#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  class implementing the data storage codec for Kaitai
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiDStoreCodec : public CORE::CDStoreCodec
{
    public:

    static const CORE::CString CodecTypeName; /**< codec type name */

    CKaitaiDStoreCodec( void );
    
    virtual ~CKaitaiDStoreCodec() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param tree the data tree you wish to store
     *      @param file access to the recource to which the data will be written
     *      @return wheter storing the tree was successfull
     */
    virtual bool StoreDataTree( const CORE::CDataNode* tree   ,
                                CORE::CIOAccess* file         ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @param file media independant access to the data source from which the data tree information should be loaded
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool BuildDataTree( CORE::CDataNode* treeroot ,
                                CORE::CIOAccess* file     ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the codec type
     *      a dat file codec might return "dat" and an xml codec "xml" ect.
     *
     *      @return the codec type
     */
    virtual CORE::CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Returns whether this codec type is data driven
     *  Data driven codecs require a data map or schema to be able to perform their transformations
     */
    virtual bool IsCodecTypeDataDriven( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the name of the codec
     *      The author of the codec can give an codec a name that the user can
     *      obtain using this member function.
     *
     *      @return name of the codec
     */
    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the copyright of the codec
     *      The author of the codec can give an codec a copyright notice that the user can
     *      obtain using this member function.
     *
     *      @return copyright notice of the codec
     */
    virtual CORE::CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Returns the codec version
     *
     *      @return the version of the codec
     */
    virtual CORE::TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    
    CKaitaiDStoreCodec( const CKaitaiDStoreCodec& src ); /**< don't copy plugin objects */
    CKaitaiDStoreCodec& operator=( const CKaitaiDStoreCodec& src ); /**< don't copy plugin objects */


    private:
    UInt32 _id;                       /**< codec id, typicly used by manager classes */

};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CKaitaiDStoreCodec, MT::CMutex > TKaitaiDStoreCodecPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAIDSTORECODEC_H ? */
