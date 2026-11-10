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

#ifndef GUCEF_CORE_CICODEC_H
#define GUCEF_CORE_CICODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_CORE_CCODECPARAMETERINFO_H
#include "gucefCORE_CCodecParameterInfo.h"
#define GUCEF_CORE_CCODECPARAMETERINFO_H
#endif /* GUCEF_CORE_CCODECPARAMETERINFO_H ? */

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

class CValueList;

/*-------------------------------------------------------------------------*/

/**
 *  Interface for codecs: turns bytes into other bytes and back.
 *  Codecs are found by family and type in the CCodecRegistry and are shared: Encode and Decode may be called
 *  from several threads at once.
 */
class GUCEF_CORE_PUBLIC_CPP CICodec : public CICloneable ,
                                      public CITypeNamed
{
    public:

    CICodec( void );

    CICodec( const CICodec& src );

    CICodec& operator=( const CICodec& src );

    virtual ~CICodec();

    virtual bool Encode( CIOAccess& source ,
                         CIOAccess& dest   ) = 0;

    virtual bool Decode( CIOAccess& source ,
                         CIOAccess& dest   ) = 0;

    /**
     *  Encodes with parameters (a key, a level). Parameters the codec does not know are ignored.
     *  The default ignores all of them and runs the two argument version.
     */
    virtual bool Encode( CIOAccess& source          ,
                         CIOAccess& dest            ,
                         const CValueList& params   );

    /**
     *  Decodes with parameters. Parameters the codec does not know are ignored.
     *  The default ignores all of them and runs the two argument version.
     */
    virtual bool Decode( CIOAccess& source          ,
                         CIOAccess& dest            ,
                         const CValueList& params   );

    /** Describes the parameters the codec takes. The default: none */
    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const;

    /**
     *  Makes a prepared, stateful object for many calls: a cipher with its key, a hash in progress.
     *  The default gives a CDefaultCodecInstance, which gathers the input and runs Encode / Decode with the parameters.
     */
    virtual bool CreateInstance( const CValueList& params       ,
                                 CICodecInstancePtr& instance   );

    /** What the codec can do: GUCEF_CODEC_CAPABILITY_*. The default: encode and decode */
    virtual UInt32 GetCapabilities( void ) const;

    virtual CString GetType( void ) const = 0;

    virtual CString GetFamilyName( void ) const = 0;

    /**
     *  The outcome of the last Encode / Decode with parameters that the calling thread made on this codec
     *  GUCEF_CODEC_STATUS_OK when that thread made none
     */
    TCodecStatus GetLastStatus( void ) const;

    /** What went wrong in the last Encode / Decode with parameters of the calling thread, empty when nothing did */
    CString GetLastErrorMessage( void ) const;

    protected:

    /** Records the outcome of a call for the calling thread, see GetLastStatus() */
    void SetLastStatus( TCodecStatus status                 ,
                        const char* message = GUCEF_NULL    ) const;

    private:

    struct SLastStatus
    {
        TCodecStatus status;
        CString message;
    };
    typedef GUCEF::map< UInt32, SLastStatus > TLastStatusMap;

    mutable TLastStatusMap m_lastStatus;    /**< per thread */
    MT::CMutex m_lastStatusLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CICODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 20-07-2005 :
        - Dinand: Added this class

-----------------------------------------------------------------------------*/
