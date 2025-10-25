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

#ifndef GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#define GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERRESOURCE_H
#include "gucefWEB_CDefaultHTTPServerResource.h"
#define GUCEF_WEB_CDEFAULTHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERRESOURCE_H ? */

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

class GUCEF_WEB_PUBLIC_CPP CCodecBasedHTTPServerResource : public CDefaultHTTPServerResource ,
                                                           public CORE::CTSGNotifier
{    
    public:
    
    CCodecBasedHTTPServerResource( void );
    
    CCodecBasedHTTPServerResource( const CCodecBasedHTTPServerResource& src );
    
    virtual ~CCodecBasedHTTPServerResource();
    
    CCodecBasedHTTPServerResource& operator=( const CCodecBasedHTTPServerResource& src );

    /**
     *  Serializes the resource into a stream according to the representation.
     */
    virtual bool Serialize( const CString& resourcePath         ,
                            CORE::CDynamicBuffer& outputBuffer  ,
                            const CORE::CString& representation ,
                            const CString& params               ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Serialize( const CString& resourcePath         ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CString& params               );

    /**
     *  Deserialize the resource from the given stream with given resource representation.
     *  @param isDeltaUpdateOnly Signals whether we are trying to deserialize a full resource in one go or just apply a delta update
     */
    virtual TDeserializeState Deserialize( const CString& resourcePath             ,
                                           const CORE::CDynamicBuffer& inputBuffer ,
                                           const CString& representation           ,
                                           bool isDeltaUpdateOnly                  ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Deserialize the resource from the given data tree 
     *  @param isDeltaUpdateOnly Signals whether we are trying to deserialize a full resource in one go or just apply a delta update
     */
    virtual TDeserializeState Deserialize( const CString& resourcePath   ,
                                           const CORE::CDataNode& input  ,
                                           const CString& representation ,
                                           bool isDeltaUpdateOnly        );

    /**
     *  Create a new (contained) resource
     *
     *  @param transactionID The unique identifier of the create operation. If a resource was created earlier using the same transactionId, this operation should not create a new one but return the originally created resource instead.
     *  @param inputBuffer The buffer containing the serialized representation of the resource to create.
     *  @param representation The representation of the resource to create
     *  @param resourceOutput The created IResource. The representation of this resource shall match the representation of the resource to create.
     *  @param supportedRepresentationsOutput In case the representation is not supported for creation, this returns the list of supported representations.
     *  @return The status of the operation. See TCreateState.
     */
    virtual TCreateState CreateResource( const CString& resourcePath                   ,
                                         const CString& transactionID                  ,
                                         const CORE::CDynamicBuffer& inputBuffer       ,
                                         const CString& representation                 ,
                                         const CString& params                         ,
                                         THTTPServerResourcePtr& resourceOutput        ,
                                         TStringVector& supportedRepresentationsOutput ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Create a new (contained) resource
     *
     *  @param transactionID The unique identifier of the create operation. If a resource was created earlier using the same transactionId, this operation should not create a new one but return the originally created resource instead.
     *  @param input The deserialized input with a DataNode based representation of the resource to create.
     *  @param representation The representation of the resource to create
     *  @param resourceOutput The created IResource. The representation of this resource shall match the representation of the resource to create.
     *  @param supportedRepresentationsOutput In case the representation is not supported for creation, this returns the list of supported representations.
     *  @return The status of the operation. See TCreateState.
     */
    virtual TCreateState CreateResource( const CString& resourcePath                   ,
                                         const CString& transactionID                  ,
                                         const CORE::CDataNode& input                  ,
                                         const CString& representation                 ,
                                         const CString& params                         ,
                                         THTTPServerResourcePtr& resourceOutput        ,
                                         TStringVector& supportedRepresentationsOutput );

    virtual bool IsCreateSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsSerializeSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsDeserializeSupported( bool deltaOnly ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetIsCreateSupported( bool isSupported );
    virtual void SetIsDeserializeSupported( bool isSupported, bool deltaOnly );

    private:

    typedef CORE::CTEventHandlerFunctor< CCodecBasedHTTPServerResource >            TEventCallback;

    protected:

    typedef GUCEF::map< CORE::CString, CORE::CDStoreCodecRegistry::TDStoreCodecPtr >  TStringToCodecMap;    

    virtual void OnGlobalMimeCodecsChanged( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData );

    bool InitCodecLinks( void );

    bool m_allowCreate;
    bool m_allowDeserialize;
    bool m_allowSerialize;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H ? */
