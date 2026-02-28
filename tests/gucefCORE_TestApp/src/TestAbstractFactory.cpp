/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#include "CTAbstractFactoryWithParam.h"
#define GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORYWITHPARAM_H
#include "CTFactoryWithParam.h"
#define GUCEF_CORE_CTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H
#include "TestAbstractFactory.h"
#define GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_TESTAPP_TESTABSTRACTFACTORY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Base interface for testing CTAbstractFactory.
 *  Represents a generic shape with a type name query.
 */
class ITestShape
{
    public:
    virtual ~ITestShape() {}
    virtual CString GetShapeType( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete shape: circle (no-param construction)
 */
class CTestCircle : public ITestShape
{
    public:
    CTestCircle() {}
    virtual ~CTestCircle() {}
    virtual CString GetShapeType( void ) const { return "Circle"; }
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete shape: square (no-param construction)
 */
class CTestSquare : public ITestShape
{
    public:
    CTestSquare() {}
    virtual ~CTestSquare() {}
    virtual CString GetShapeType( void ) const { return "Square"; }
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete shape: triangle (no-param construction)
 */
class CTestTriangle : public ITestShape
{
    public:
    CTestTriangle() {}
    virtual ~CTestTriangle() {}
    virtual CString GetShapeType( void ) const { return "Triangle"; }
};

/*-------------------------------------------------------------------------*/

/*
 *  Base interface for testing CTAbstractFactoryWithParam.
 *  Represents a colored shape where the color is passed at construction time.
 */
class ITestColoredShape
{
    public:
    virtual ~ITestColoredShape() {}
    virtual CString GetShapeType( void ) const = 0;
    virtual CString GetColor( void ) const = 0;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete colored shape: colored circle (param = color string)
 */
class CTestColoredCircle : public ITestColoredShape
{
    public:
    explicit CTestColoredCircle( const CString& color ) : m_color( color ) {}
    virtual ~CTestColoredCircle() {}
    virtual CString GetShapeType( void ) const { return "ColoredCircle"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete colored shape: colored triangle (param = color string)
 */
class CTestColoredTriangle : public ITestColoredShape
{
    public:
    explicit CTestColoredTriangle( const CString& color ) : m_color( color ) {}
    virtual ~CTestColoredTriangle() {}
    virtual CString GetShapeType( void ) const { return "ColoredTriangle"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------*/

/*
 *  Concrete colored shape: colored square (param = color string)
 */
class CTestColoredSquare : public ITestColoredShape
{
    public:
    explicit CTestColoredSquare( const CString& color ) : m_color( color ) {}
    virtual ~CTestColoredSquare() {}
    virtual CString GetShapeType( void ) const { return "ColoredSquare"; }
    virtual CString GetColor( void ) const { return m_color; }
    private:
    CString m_color;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAbstractFactoryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AbstractFactory" );

    /*
     *  Type aliases for readability
     */
    typedef CTAbstractFactory< CString, ITestShape, MT::CMutex >            TShapeFactory;
    typedef CTFactory< ITestShape, CTestCircle, MT::CMutex >                TCircleFactory;
    typedef CTFactory< ITestShape, CTestSquare, MT::CMutex >                TSquareFactory;
    typedef CTFactory< ITestShape, CTestTriangle, MT::CMutex >              TTriangleFactory;
    typedef TShapeFactory::TProductPtr                                       TShapePtr;

    typedef CTAbstractFactoryWithParam< CString, ITestColoredShape, CString, MT::CMutex >            TColoredShapeFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestColoredCircle, CString, MT::CMutex >        TColoredCircleFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestColoredSquare, CString, MT::CMutex >        TColoredSquareFactory;
    typedef CTFactoryWithParam< ITestColoredShape, CTestColoredTriangle, CString, MT::CMutex >      TColoredTriangleFactory;
    typedef TColoredShapeFactory::TProductPtr                                                        TColoredShapePtr;

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_DefaultConstruct" )
        try
        {
            TShapeFactory factory;
            GUCEF_TESTFW_ASSERT_TRUE( true ); // construction succeeded
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreateUnregisteredType_ReturnsNull" )
        try
        {
            TShapeFactory factory;
            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsConstructible_UnregisteredType_ReturnsFalse" )
        try
        {
            TShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_RegisterAndCreate" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "Circle" );
            }

            factory.UnregisterConcreteFactory( "Circle" ); 
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsConstructible_RegisteredType_ReturnsTrue" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_MultipleTypes_CreateCorrectConcrete" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );

            TShapePtr circle = factory.Create( "Circle" );
            TShapePtr square = factory.Create( "Square" );

            GUCEF_TESTFW_ASSERT_FALSE( circle.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( square.IsNULL() );
            if ( !circle.IsNULL() && !square.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetShapeType() == "Circle" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetShapeType() == "Square" );
            }

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreateUsingClassTypeName" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            // CreateUsingClassTypeName uses the concrete class's type name,
            // not the selection key. For CTFactory this is ToString<CTestCircle>()
            // which maps via the reverse lookup maintained by CTAbstractFactory.
            CString className = circleFactory.GetConcreteClassTypeName();
            TShapePtr ptr = factory.CreateUsingClassTypeName( className );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "Circle" );
            }

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsClassConstructible_RegisteredClass_ReturnsTrue" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsClassConstructible( className ) );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IsClassConstructible_UnregisteredClass_ReturnsFalse" )
        try
        {
            TShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsClassConstructible( "CTestCircle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_GetConcreteClassTypeNameForSelectionCriterea" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( className.IsNULLOrEmpty() );
            GUCEF_TESTFW_ASSERT_TRUE( className == circleFactory.GetConcreteClassTypeName() );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_GetConcreteClassTypeNameForSelectionCriterea_Unregistered" )
        try
        {
            TShapeFactory factory;
            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "Circle" );
            GUCEF_TESTFW_ASSERT_TRUE( className.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_GetSelectionCritereaForConcreteClassTypeName" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            CString selectionKey = factory.GetSelectionCritereaForConcreteClassTypeName( className );
            GUCEF_TESTFW_ASSERT_TRUE( selectionKey == "Circle" );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ObtainKeySet_Empty" )
        try
        {
            TShapeFactory factory;
            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ObtainKeySet_WithRegistrations" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 2 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Circle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Square" ) != keys.end() );

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterConcreteFactory" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );

            factory.UnregisterConcreteFactory( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );

            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_UnregisterAllConcreteFactories" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            TTriangleFactory triangleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );
            factory.RegisterConcreteFactory( "Triangle", &triangleFactory );

            CStringSet keysBefore;
            factory.ObtainKeySet( keysBefore );
            GUCEF_TESTFW_ASSERT_TRUE( keysBefore.size() == 3 );

            factory.UnregisterAllConcreteFactories();

            CStringSet keysAfter;
            factory.ObtainKeySet( keysAfter );
            GUCEF_TESTFW_ASSERT_TRUE( keysAfter.empty() );

            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Square" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Triangle" ) );

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
            factory.UnregisterConcreteFactory( "Triangle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ObtainKeySet_AfterUnregisterAll_IsEmpty" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.UnregisterAllConcreteFactories();

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ReRegisterAfterUnregister" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;

            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.UnregisterConcreteFactory( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Circle" ) );

            // Re-register with a different concrete type under the same key
            factory.RegisterConcreteFactory( "Circle", &squareFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );

            TShapePtr ptr = factory.Create( "Circle" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            // Now "Circle" key maps to CTestSquare
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "Square" );
            }

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_CreateUnknownClassTypeName_ReturnsNull" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );

            TShapePtr ptr = factory.CreateUsingClassTypeName( "NonExistentClass" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );

            factory.UnregisterConcreteFactory( "Circle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_ThreeTypes_ObtainKeySet" )
        try
        {
            TShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            TTriangleFactory triangleFactory;
            factory.RegisterConcreteFactory( "Circle", &circleFactory );
            factory.RegisterConcreteFactory( "Square", &squareFactory );
            factory.RegisterConcreteFactory( "Triangle", &triangleFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 3 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Circle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Square" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "Triangle" ) != keys.end() );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Circle" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Square" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "Triangle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "Hexagon" ) );

            factory.UnregisterConcreteFactory( "Circle" );
            factory.UnregisterConcreteFactory( "Square" );
            factory.UnregisterConcreteFactory( "Triangle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactory_IntKeySelector_RegisterAndCreate" )
        try
        {
            // Use integer keys instead of strings
            typedef CTAbstractFactory< Int32, ITestShape, MT::CMutex >  TIntKeyShapeFactory;
            typedef TIntKeyShapeFactory::TProductPtr                     TIntShapePtr;

            TIntKeyShapeFactory factory;
            TCircleFactory circleFactory;
            TSquareFactory squareFactory;
            factory.RegisterConcreteFactory( 1, &circleFactory );
            factory.RegisterConcreteFactory( 2, &squareFactory );

            TIntShapePtr circle = factory.Create( 1 );
            TIntShapePtr square = factory.Create( 2 );
            TIntShapePtr unknown = factory.Create( 99 );

            GUCEF_TESTFW_ASSERT_FALSE( circle.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( square.IsNULL() );
            GUCEF_TESTFW_ASSERT_TRUE( unknown.IsNULL() );
            if ( !circle.IsNULL() && !square.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetShapeType() == "Circle" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetShapeType() == "Square" );
            }

            factory.UnregisterConcreteFactory( 1 );
            factory.UnregisterConcreteFactory( 2 );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_DefaultConstruct" )
        try
        {
            TColoredShapeFactory factory;
            GUCEF_TESTFW_ASSERT_TRUE( true ); // construction succeeded
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreateUnregisteredType_ReturnsNull" )
        try
        {
            TColoredShapeFactory factory;
            TColoredShapePtr ptr = factory.Create( "ColoredCircle", "Red" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsConstructible_UnregisteredType_ReturnsFalse" )
        try
        {
            TColoredShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredCircle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_RegisterAndCreate" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            TColoredShapePtr ptr = factory.Create( "ColoredCircle", "Blue" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Blue" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsConstructible_RegisteredType_ReturnsTrue" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredCircle" ) );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsClassConstructible_RegisteredClass_ReturnsTrue" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsClassConstructible( className ) );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IsClassConstructible_UnregisteredClass_ReturnsFalse" )
        try
        {
            TColoredShapeFactory factory;
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsClassConstructible( "CTestColoredCircle" ) );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_MultipleTypes_CreateCorrectConcrete" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );

            TColoredShapePtr circle = factory.Create( "ColoredCircle", "Red" );
            TColoredShapePtr square = factory.Create( "ColoredSquare", "Green" );

            GUCEF_TESTFW_ASSERT_FALSE( circle.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( square.IsNULL() );
            if ( !circle.IsNULL() && !square.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetShapeType() == "ColoredSquare" );
                GUCEF_TESTFW_ASSERT_TRUE( circle->GetColor() == "Red" );
                GUCEF_TESTFW_ASSERT_TRUE( square->GetColor() == "Green" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            factory.UnregisterConcreteFactory( "ColoredSquare" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_DifferentParamValues_CreateIndependentObjects" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            TColoredShapePtr red    = factory.Create( "ColoredCircle", "Red" );
            TColoredShapePtr yellow = factory.Create( "ColoredCircle", "Yellow" );
            TColoredShapePtr blue   = factory.Create( "ColoredCircle", "Blue" );

            GUCEF_TESTFW_ASSERT_FALSE( red.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( yellow.IsNULL() );
            GUCEF_TESTFW_ASSERT_FALSE( blue.IsNULL() );
            if ( !red.IsNULL() && !yellow.IsNULL() && !blue.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( red->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( yellow->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( blue->GetShapeType() == "ColoredCircle" );
            }
            if ( !red.IsNULL() && !yellow.IsNULL() && !blue.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( red->GetColor() == "Red" );
                GUCEF_TESTFW_ASSERT_TRUE( yellow->GetColor() == "Yellow" );
                GUCEF_TESTFW_ASSERT_TRUE( blue->GetColor() == "Blue" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreateUsingClassTypeName" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            TColoredShapePtr ptr = factory.CreateUsingClassTypeName( className, "Purple" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "ColoredCircle" );
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Purple" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ObtainKeySet_WithRegistrations" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 2 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredCircle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredSquare" ) != keys.end() );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            factory.UnregisterConcreteFactory( "ColoredSquare" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterConcreteFactory" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredCircle" ) );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredCircle" ) );

            TColoredShapePtr ptr = factory.Create( "ColoredCircle", "Red" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_UnregisterAllConcreteFactories" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );

            factory.UnregisterAllConcreteFactories();

            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredCircle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredSquare" ) );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_CreateUnknownClassTypeName_ReturnsNull" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            TColoredShapePtr ptr = factory.CreateUsingClassTypeName( "NonExistentClass", "Red" );
            GUCEF_TESTFW_ASSERT_TRUE( ptr.IsNULL() );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_GetConcreteClassTypeNameForSelectionCriterea" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "ColoredCircle" );
            GUCEF_TESTFW_ASSERT_FALSE( className.IsNULLOrEmpty() );
            GUCEF_TESTFW_ASSERT_TRUE( className == circleFactory.GetConcreteClassTypeName() );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_GetSelectionCritereaForConcreteClassTypeName" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );

            CString className = circleFactory.GetConcreteClassTypeName();
            CString selectionKey = factory.GetSelectionCritereaForConcreteClassTypeName( className );
            GUCEF_TESTFW_ASSERT_TRUE( selectionKey == "ColoredCircle" );

            factory.UnregisterConcreteFactory( "ColoredCircle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_GetConcreteClassTypeNameForSelectionCriterea_Unregistered" )
        try
        {
            TColoredShapeFactory factory;
            CString className = factory.GetConcreteClassTypeNameForSelectionCriterea( "ColoredCircle" );
            GUCEF_TESTFW_ASSERT_TRUE( className.IsNULLOrEmpty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ObtainKeySet_Empty" )
        try
        {
            TColoredShapeFactory factory;
            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ObtainKeySet_AfterUnregisterAll_IsEmpty" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.UnregisterAllConcreteFactories();

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.empty() );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ReRegisterAfterUnregister" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;

            factory.RegisterConcreteFactory( "MyShape", &circleFactory );
            factory.UnregisterConcreteFactory( "MyShape" );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "MyShape" ) );

            // Re-register with a different concrete type under the same key
            factory.RegisterConcreteFactory( "MyShape", &squareFactory );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "MyShape" ) );

            TColoredShapePtr ptr = factory.Create( "MyShape", "Teal" );
            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            // Now "MyShape" key maps to CTestColoredSquare
            if ( !ptr.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetShapeType() == "ColoredSquare" );
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Teal" );
            }

            factory.UnregisterConcreteFactory( "MyShape" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_ThreeTypes_ObtainKeySet" )
        try
        {
            TColoredShapeFactory factory;
            TColoredCircleFactory circleFactory;
            TColoredSquareFactory squareFactory;
            TColoredTriangleFactory triangleFactory;
            factory.RegisterConcreteFactory( "ColoredCircle", &circleFactory );
            factory.RegisterConcreteFactory( "ColoredSquare", &squareFactory );
            factory.RegisterConcreteFactory( "ColoredTriangle", &triangleFactory );

            CStringSet keys;
            factory.ObtainKeySet( keys );
            GUCEF_TESTFW_ASSERT_TRUE( keys.size() == 3 );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredCircle" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredSquare" ) != keys.end() );
            GUCEF_TESTFW_ASSERT_TRUE( keys.find( "ColoredTriangle" ) != keys.end() );

            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredCircle" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredSquare" ) );
            GUCEF_TESTFW_ASSERT_TRUE( factory.IsConstructible( "ColoredTriangle" ) );
            GUCEF_TESTFW_ASSERT_FALSE( factory.IsConstructible( "ColoredHexagon" ) );

            TColoredShapePtr tri = factory.Create( "ColoredTriangle", "Gold" );
            GUCEF_TESTFW_ASSERT_FALSE( tri.IsNULL() );
            if ( !tri.IsNULL() )
            {
                GUCEF_TESTFW_ASSERT_TRUE( tri->GetShapeType() == "ColoredTriangle" );
                GUCEF_TESTFW_ASSERT_TRUE( tri->GetColor() == "Gold" );
            }

            factory.UnregisterConcreteFactory( "ColoredCircle" );
            factory.UnregisterConcreteFactory( "ColoredSquare" );
            factory.UnregisterConcreteFactory( "ColoredTriangle" );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

    GUCEF_TESTFW_TESTCASE( "CTAbstractFactoryWithParam_IntKeySelector" )
        try
        {
            typedef CTAbstractFactoryWithParam< Int32, ITestColoredShape, CString, MT::CMutex >        TIntColoredShapeFactory;
            typedef CTFactoryWithParam< ITestColoredShape, CTestColoredCircle, CString, MT::CMutex >   TIntColoredCircleFactory;
            typedef TIntColoredShapeFactory::TProductPtr                                               TIntColoredShapePtr;

            TIntColoredShapeFactory factory;
            TIntColoredCircleFactory circleFactory;
            factory.RegisterConcreteFactory( 10, &circleFactory );

            TIntColoredShapePtr ptr = factory.Create( 10, "Orange" );
            TIntColoredShapePtr missing = factory.Create( 99, "Orange" );

            GUCEF_TESTFW_ASSERT_FALSE( ptr.IsNULL() );
            GUCEF_TESTFW_ASSERT_TRUE( missing.IsNULL() );
            if ( !ptr.IsNULL() )
                GUCEF_TESTFW_ASSERT_TRUE( ptr->GetColor() == "Orange" );

            factory.UnregisterConcreteFactory( 10 );
        }
        catch( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    /*---------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------*/
