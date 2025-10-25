/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CHTTPCODECLINKS_H
#define GUCEF_WEB_CHTTPCODECLINKS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_WEB_CHTTPMIMETYPES_H
#include "gucefWEB_CHttpMimeTypes.h"
#define GUCEF_WEB_CHTTPMIMETYPES_H
#endif /* GUCEF_WEB_CHTTPMIMETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class for linking common MIME types and encoding types to codecs
 *  This class can be made threadsafe by providing a lock
 */
class GUCEF_WEB_PUBLIC_CPP CHttpCodecLinks : public MT::CILockable
{    
    public:
    
    typedef CORE::CDStoreCodecRegistry::TDStoreCodecPtr  TMimeTypeCodecPtr;
    typedef CORE::CCodecRegistry::TICodecPtr             TEncodingCodecPtr;
    
    CHttpCodecLinks( void );
    
    CHttpCodecLinks( const CHttpCodecLinks& src );
    
    virtual ~CHttpCodecLinks();
    
    CHttpCodecLinks& operator=( const CHttpCodecLinks& src );

    virtual bool InitMimeCodecLinks( void );

    virtual bool InitEncodingCodecLinks( void );

    virtual bool RemoveMimeCodecLinks( void );

    virtual bool RemoveEncodingCodecLinks( void );

    virtual TMimeTypeCodecPtr GetSerializationCodec( const CORE::CString& mimeType ) const;

    virtual TMimeTypeCodecPtr GetDeserializationCodec( const CORE::CString& mimeType ) const;

    virtual void GetSupportedSerializationMimeTypes( CORE::CString::StringVector& reps ) const;

    virtual void GetSupportedDeserializationMimeTypes( CORE::CString::StringVector& reps ) const;

    virtual TEncodingCodecPtr GetEncodingCodec( const CORE::CString& encodingType ) const;

    virtual const CString& GetClassTypeName( void ) const;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    typedef GUCEF::map< CORE::CString, TMimeTypeCodecPtr > TStringToMimeTypeCodecMap;
    typedef GUCEF::map< CORE::CString, TEncodingCodecPtr > TStringToEncodingCodecMap;

    TStringToMimeTypeCodecMap m_serializeRepToCodecMap;
    TStringToMimeTypeCodecMap m_deserializeRepToCodecMap;
    TStringToEncodingCodecMap m_encodingRepToCodecMap;
    CORE::CString::StringVector m_deserializationReps;
    CORE::CString::StringVector m_serializationReps;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPCODECLINKS_H ? */
   