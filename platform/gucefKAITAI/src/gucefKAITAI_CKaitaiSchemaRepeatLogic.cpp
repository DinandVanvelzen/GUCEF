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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefKAITAI_CKaitaiSchemaRepeatLogic.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CKaitaiSchemaRepeatLogic::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaRepeatLogic";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaRepeatLogic::CKaitaiSchemaRepeatLogic( void )
    : CKaitaiSchemaBaseField( RepeatLogic, CKaitaiSchemaBaseFieldPtr() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaRepeatLogic, MT::CMutex >( this )
    , m_repeatExpression()
    , m_repeatType( UnknownRepeat )
    , m_repeatedField()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaRepeatLogic::CKaitaiSchemaRepeatLogic( CKaitaiSchemaBaseFieldPtr parent )
    : CKaitaiSchemaBaseField( RepeatLogic, parent )
    , CORE::CTSharedObjCreator< CKaitaiSchemaRepeatLogic, MT::CMutex >( this )
    , m_repeatExpression()
    , m_repeatType( UnknownRepeat )
    , m_repeatedField()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaRepeatLogic::CKaitaiSchemaRepeatLogic( const CKaitaiSchemaRepeatLogic& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaRepeatLogic, MT::CMutex >( this )
    , m_repeatExpression( src.m_repeatExpression )
    , m_repeatType( src.m_repeatType )
    , m_repeatedField()
{GUCEF_TRACE;

    *this = src; // use the assignment operator to do the work
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaRepeatLogic::~CKaitaiSchemaRepeatLogic()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CKaitaiSchemaRepeatLogic::Clear( void )
{GUCEF_TRACE;

    CKaitaiSchemaBaseField::Clear();
    m_repeatExpression.Clear();
    m_repeatType = UnknownRepeat;
    if ( !m_repeatedField.IsNULL() )
        m_repeatedField->Clear();
    m_repeatedField.Unlink();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaRepeatLogic& 
CKaitaiSchemaRepeatLogic::operator=( const CKaitaiSchemaRepeatLogic& src )
{GUCEF_TRACE;

    if GUCEF_PREDICT_TRUE( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );
        m_repeatExpression = src.m_repeatExpression;
        m_repeatType = src.m_repeatType;

        if ( !m_repeatedField.IsNULL() )
        {
            m_repeatedField = src.m_repeatedField->CloneAsFieldObject();
        }
        else
        {
            if ( !m_repeatedField.IsNULL() )
                m_repeatedField->Clear();
            m_repeatedField.Unlink();
        }

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaRepeatLogic::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaRepeatLogic( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaRepeatLogic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

RepeatType 
CKaitaiSchemaRepeatLogic::GetRepeatType( void ) const
{GUCEF_TRACE;

    return m_repeatType;
}

/*-------------------------------------------------------------------------*/
    
const CORE::CString& 
CKaitaiSchemaRepeatLogic::GetRepeatExpression( void ) const
{GUCEF_TRACE;

    return m_repeatExpression;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaRepeatLogic::GetRepeatedField( void ) const
{GUCEF_TRACE;

    return m_repeatedField;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRepeatLogic::Serialize( CORE::CDataNode& domRootNode                        , 
                                     const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRepeatLogic::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    
    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString();
    if GUCEF_PREDICT_FALSE( id.IsNULLOrEmpty() )
        return false; 

    CORE::CString repeatStyle = domRootNode.GetAttributeValueOrChildValueByName( "repeat" ).AsString();
    if GUCEF_PREDICT_FALSE( repeatStyle.IsNULLOrEmpty() )
        return false;
        
    if ( "eos" == repeatStyle )
    {
        m_repeatType = RepeatUntilEndOfStream;
    }
    else
    if ( "expr" == repeatStyle )
    {
        m_repeatType = RepeatUpToGivenCount;
        m_repeatExpression = domRootNode.GetAttributeValueOrChildValueByName( "repeat-expr" ).AsString();
        if GUCEF_PREDICT_FALSE( m_repeatExpression.IsNULLOrEmpty() )
            return false;
    }
    else
    if ( "until" == repeatStyle )
    {
        m_repeatType = RepeatUntilExpressionFalse;
        m_repeatExpression = domRootNode.GetAttributeValueOrChildValueByName( "repeat-until" ).AsString();
        if GUCEF_PREDICT_FALSE( m_repeatExpression.IsNULLOrEmpty() )
            return false;
    }

    // @TODO: we should be able to deserialize the repeated field here

    if ( m_repeatType != UnknownRepeat )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRepeatLogic:Deserialize: Successfully deserialized repeat logic for field with id \"" + id + 
            "\" and repeat expression \"" + m_repeatExpression + " and repeat type " + CORE::ToString( m_repeatType ) );
    
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRepeatLogic:Deserialize: Failed to deserialize repeat logic for field with id \"" + id + 
            "\" and repeat expression \"" + m_repeatExpression + " and repeat type " + CORE::ToString( m_repeatType ) );
    
        return false;
    }    
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/


