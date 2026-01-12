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

#ifndef GUCEF_CORE_CTBASICSHAREDPTR_H
#define GUCEF_CORE_CTBASICSHAREDPTR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"       /* module macro's */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_SFINAE_UTILS_H
#include "gucefCORE_SFINAE_utils.h"
#define GUCEF_CORE_SFINAE_UTILS_H
#endif /* GUCEF_CORE_SFINAE_UTILS_H ? */

#ifndef GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
#include "CTDynamicDestructorBase.h"
#define GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H
#endif /* GUCEF_CORE_CTDYNAMICDESTRUCTORBASE_H ? */

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#include "ExceptionClassMacros.h"
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */

#ifndef GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#include "ExceptionThrowMacros.h"
#define GUCEF_CORE_EXCEPTIONTHROWMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONTHROWMACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class LockType >
class TBasicSharedPtrSharedData : public MT::CILockable
{
    public:

    typedef LockType                       TLockType;

    Int32 m_refCounter;                        /**< shared reference counter */
    LockType m_lock;                           /**< shared lock, if any */
    bool m_hasIndependentLifeCycle;            /**< depending on if the shared data is part of a larger object it may not have an independent lifecycle */
    CIDynamicVoidDestructor* m_voidDestructor; /**< mandatory external object destructor */
    void* m_originalAddressAsCreated;          /**< original address of the instantiated memory, without offsets applied. To be used with destructor */

    TBasicSharedPtrSharedData( bool hasIndependentLifeCycle );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    /**
     *  Non-default move constructor that also takes the original address and destructor
     *  Allows you to move the pieces that can be moved while still keeping the correct memory references
     *  for the non-independent lifecycle management via the shared data and dynamic destructor which are incorporated into the object memory footprint
     */
    TBasicSharedPtrSharedData( TBasicSharedPtrSharedData&& src           ,
                               void* originalAddressAsCreated            ,
                               CIDynamicVoidDestructor* voidedDestructor ) GUCEF_NOEXCEPT;

    #endif

    virtual ~TBasicSharedPtrSharedData() GUCEF_VIRTUAL_OVERRIDE;

    bool InitSharedData( void* originalAddressAsCreated, CIDynamicVoidDestructor* voidedDestructor );
    bool InitSharedDataDestructor( CIDynamicVoidDestructor* voidedDestructor );
    bool InitSharedDataOrgAddr( void* originalAddressAsCreated );

    void OverrideSharedData( void* originalAddressAsCreated, CIDynamicVoidDestructor* voidedDestructor );

    /**
     *  Actual locking behavior depends on the LockType passed to the template
     */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Actual locking behavior depends on the LockType passed to the template
     */
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Templated implementation of a shared pointer.
 *
 *  A shared pointer adds some safety to pointers that get passed
 *  around where objects keep a reference to object instances independent
 *  of each other. In such situations it can become unclear who is responsible
 *  for destroying the referenced object instance.
 *  The shared pointer will reference count the pointer and ensure that even
 *  though the original owner has no knowledge of other classes referencing the
 *  object it can safely manage object destruction as desired.
 *
 *  Note that the usage of an external destructor for the shared object by means of the
 *  CTSharedObjectDestructor template is mandatory. By passing in a pointer to such a class
 *  you basically create a callback link for cleanup purposes.
 *
 *  Note that this shared pointer implementation's thread safety is dependent on the LockType
 *  template param. A NoLock could be used as the LockType for implementations where the
 *  shared ptr is guaranteed to never cross thread boundaries resulting is a minor optimization.
 */
template< typename T, class LockType >
class CTBasicSharedPtr : public MT::CILockable ,
                         public CICloneable
{
    public:

    typedef T                              TContainedType;
    typedef LockType                       TLockType;
    typedef CTDynamicDestructorBase< T >   TTypedDestructor;

    protected:

    mutable TBasicSharedPtrSharedData< LockType >* m_shared;    /**< holds data shared across multiple instances of the shared pointer */
    T* m_ptr;                                                   /**< the pointer that we wish to reference count */

    public:

    /**
     *  Constructor that allows you to create an 'uninitialized' pointer.
     *  You will have to assign a valid shared pointer at a later time
     *  to initialize the shared pointer.
     *
     *  Note that if you attempt to use the shared pointer before it
     *  is initialized an exception will be thrown
     *
     */
    CTBasicSharedPtr( void );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    CTBasicSharedPtr( CTBasicSharedPtr&& src ) GUCEF_NOEXCEPT;

    #endif

    CTBasicSharedPtr( T* ptr                                    ,
                      CIDynamicVoidDestructor* voidedDestructor ,
                      void* originalAddressAsCreated            );

    // inlined copy constructor, has to be inlined in class definition for now due to VC6 limitations
    template< class Derived >
    CTBasicSharedPtr( const CTBasicSharedPtr< Derived, LockType >& src )
        : m_shared( GUCEF_NULL )
        , m_ptr( GUCEF_NULL )
    {GUCEF_TRACE;

        InitializeUsingRelatedType( src );
    }

    CTBasicSharedPtr( const CTBasicSharedPtr& src );

    CTBasicSharedPtr( const TBasicSharedPtrSharedData< LockType >* shared ,
                      const T* ptr                                        );

    virtual ~CTBasicSharedPtr() GUCEF_VIRTUAL_OVERRIDE;

    const TBasicSharedPtrSharedData< LockType >* GetSharedData( void ) const
    {GUCEF_TRACE;

        return m_shared;
    }

    template< class RelatedClass >
    bool InitializeUsingRelatedType( const CTBasicSharedPtr< RelatedClass, LockType >& other )
    {GUCEF_TRACE;

        Unlink();

        MT::CObjectScopeLock lockOther( other );
        MT::CObjectScopeLock lock( this );

        // The static cast below is performed as a compile time validation
        // of the type passed.
        T* relatedClassAsT = static_cast< T* >( const_cast< RelatedClass* >( other.GetPointerAlways() ) );
        if ( GUCEF_NULL != relatedClassAsT )
        {
            // We only allow the creation of copies if a destructor has been set
            // init your shared pointer creator correctly first. this is to prevent creating dangling pointers
            const TBasicSharedPtrSharedData< LockType >* otherSharedData = other.GetSharedData();
            if ( GUCEF_NULL != otherSharedData->m_voidDestructor )
            {
                m_ptr = relatedClassAsT;
                m_shared = const_cast< TBasicSharedPtrSharedData< LockType >* >( otherSharedData );
                ++(m_shared->m_refCounter);
                return true;
            }
        }
        return false;
    }

    /**
     *  If the given shared pointer is the last remaining reference to the underlying raw pointer
     *  you can use this member function to relinquish ownership. This allows you to revert back to a raw pointer
     */
    bool RelinquishOwnership( T*& rawPtr );

    // implemented inline as a workaround for VC6 issues
    template< class Derived >
    CTBasicSharedPtr& operator=( const CTBasicSharedPtr< Derived, LockType >& src )
    {
        if ( &reinterpret_cast< const CTBasicSharedPtr& >( src ) != this )
        {
            InitializeUsingRelatedType( src );
        }
        return *this;
    }

    // implemented inline as a workaround for VC6 issues
    // The dummy param is a VC6 hack for templated member functions
    template< class Derived >
    CTBasicSharedPtr< Derived, LockType > StaticCast( Derived* dummyForCppNameMangling = GUCEF_NULL )
    {
        // We use the initialization function of the derived type's
        // shared pointer which will have knowledge of both types.
        CTBasicSharedPtr< Derived, LockType > retVal;
        retVal.InitializeUsingRelatedType( *this );
        return retVal;
    }

    CTBasicSharedPtr& operator=( const CTBasicSharedPtr& src );

    CTBasicSharedPtr& operator=( int nullValue );

    inline bool operator==( Int32 nullValue ) const;

    inline bool operator==( Int64 nullValue ) const;

    inline bool operator==( const void* other ) const;

    inline bool operator==( const CTBasicSharedPtr& other ) const;

    inline bool operator!=( Int32 other ) const;

    inline bool operator!=( Int64 other ) const;

    inline bool operator!=( const void* other ) const;

    inline bool operator!=( const CTBasicSharedPtr& other ) const;

    inline bool operator<( const CTBasicSharedPtr& other ) const;

    inline bool operator<( const void* other ) const;

    /**
     *  operator that implements '(*mySharedPtr)'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline T& operator*( void );

    /**
     *  operator that implements '(*mySharedPtr)'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline const T& operator*( void ) const;

    /**
     *  operator that emulates 'myPointer->'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline T* operator->( void );

    /**
     *  operator that emulates 'myPointer->'
     *
     *  @throws ENotInitialized if the pointer is not initialized
     */
    inline const T* operator->( void ) const;

    /**
     *  Conversion operator to bool to facilitate easy ! etc checks against the
     *  pointer being NULL as some people like to do versus an explicit NULL == check.
     */
    inline operator bool() const;

    inline T* GetPointer( void );

    inline const T* GetPointer( void ) const;

    inline T* GetPointerAlways( void );

    inline const T* GetPointerAlways( void ) const;

    inline bool IsNULL( void ) const;

    void Unlink( void );

    UInt32 GetReferenceCount( void ) const;

    CIDynamicVoidDestructor* GetDestructor( void ) const;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    GUCEF_DEFINE_INLINED_MSGEXCEPTION( ENotInitialized );

    protected:

    /**
     *  Can be used be decending implementations for late initialization.
     */
    void Initialize( T* ptr                                    ,
                     CIDynamicVoidDestructor* voidedDestructor ,
                     void* originalAddressAsCreated            );

    /**
     *  Simply sets the object destructor pointer overwriting whatever
     *  the values used to be.
     *  The pointer has to be valid.
     */
    void OverrideDestructor( CIDynamicVoidDestructor* voidedDestructor );

    /**
     *  Actual locking behaviour depends on the LockType passed to the template
     */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Actual locking behaviour depends on the LockType passed to the template
     */
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Creates a clone of the basic shared pointer
     *  Note that this increases the reference count like any copy would
     */
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  Templated base class to facilitate creation of shared pointers associated with an existing object.
 *  This reduces overhead by incorporating the shared data into the memory footprint of the to-be-shared object.
 *  Thus avoiding an extra memory allocation for the shared data and its associated penalties.
 */
template< typename T, class LockType, typename TypeWhenAllocated = T >
class CTBasicSharedPtrCreator
{
    public:

    typedef T                                   TBasicSharedPtrContainedType;
    typedef TypeWhenAllocated                   TBasicSharedPtrTypeWhenAllocatedype;
    typedef LockType                            TBasicSharedPtrLockType;
    typedef CTBasicSharedPtr< T, LockType >     TBasicSharedPtrType;
    typedef void                                IsBasicSharedPtrCreatorTag;

    /**
     *  Adding this member function is the entire point of this class
     *  We can use the shared data incorporated into the object footprint thus avoiding an extra memory allocation
     *  Plus it allows us to later create additional shared pointers to the same object that share the same shared data from an independent location
     *
     *  Note that you will receive null-d pointers if you attempt to use this function before properly initializing the shared ptr creator
     */
    virtual CTBasicSharedPtr< T, LockType > CreateBasicSharedPtr( T* dummyForCppNameMangling = GUCEF_NULL ) const;

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    /**
     *  Non-default move constructor that also takes the original address and destructor
     *  Allows you to move the pieces that can be moved while still keeping the correct memory references
     *  for the non-independent lifecycle management via the shared data and dynamic destructor which are incorporated into the object memory footprint
     *
     *  Note that it is not possible to provide a default move constructor as the original address (and potentially also the destructor) would not be valid
     */
    CTBasicSharedPtrCreator( CTBasicSharedPtrCreator&& src               ,
                             TypeWhenAllocated* originalAddressAsCreated ,
                             CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT;

    #endif

    /**
     *  Default constructor creates an uninitialized shared ptr creator
     *  You must use InitializeSharedPtrCreatorData() soon thereafter as part of your construction process
     *  Failure to do so will result in exceptions being thrown when attempting to create shared pointers
     */
    CTBasicSharedPtrCreator( void  );

    /**
     *  If you pass null for originalAddressAsCreated you must use InitializeSharedPtrCreatorData() soon thereafter as
     *  part of your construction process
     *  You must also call InitializeSharedPtrCreatorDataDestructor() to set the destructor
     */
    CTBasicSharedPtrCreator( TypeWhenAllocated* originalAddressAsCreated );

    /**
     *  If you pass null for originalAddressAsCreated you must use InitializeSharedPtrCreatorData() soon thereafter as
     *  part of your construction process
     */
    CTBasicSharedPtrCreator( TypeWhenAllocated* originalAddressAsCreated, CIDynamicVoidDestructor* voidDestructor );

    virtual ~CTBasicSharedPtrCreator();

    /**
     *  Use these functions to initialize the shared ptr creator data if you passed null
     *  for originalAddressAsCreated in the constructor. Useful if you dont know which base class
     *  is the templated shared ptr creator using template parameters at the time of construction.
     *
     *  Note that only a singular invocation of one of these functions is allowed
     *  Note that you can only use one of these functions, not a combination which initialized the same data member
     */
    bool InitializeSharedPtrCreatorData( TypeWhenAllocated* originalAddressAsCreated, CIDynamicVoidDestructor* voidDestructor );

    /**
     *  Use these functions to initialize the shared ptr creator data if you passed null
     *  for originalAddressAsCreated in the constructor. Useful if you dont know which base class
     *  is the templated shared ptr creator using template parameters at the time of construction.
     *
     *  Note that only a singular invocation of one of these functions is allowed
     *  Note that you can only use one of these functions, not a combination which initialized the same data member
     */
    bool InitializeSharedPtrCreatorDataDestructor( CIDynamicVoidDestructor* voidedDestructor );

    /**
     *  Use these functions to initialize the shared ptr creator data if you passed null
     *  for originalAddressAsCreated in the constructor. Useful if you dont know which base class
     *  is the templated shared ptr creator using template parameters at the time of construction.
     *
     *  Note that only a singular invocation of one of these functions is allowed
     *  Note that you can only use one of these functions, not a combination which initialized the same data member
     */
    bool InitializeSharedPtrCreatorDataOrgAddr( TypeWhenAllocated* originalAddressAsCreated );

    protected:

    void OverrideSharedPtrCreatorData( TypeWhenAllocated* originalAddressAsCreated, CIDynamicVoidDestructor* voidDestructor );

    const TBasicSharedPtrSharedData< LockType >& GetBasicSharedPtrData( void ) const;

    private:

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CTBasicSharedPtrCreator( CTBasicSharedPtrCreator&& src ) GUCEF_NOEXCEPT GUCEF_DELETED_MEMBER;    /**< dont use, not possible due to memory address dependencies */
    #endif
    CTBasicSharedPtrCreator( const CTBasicSharedPtrCreator& src ) GUCEF_DELETED_MEMBER;              /**< dont use, not possible due to memory address dependencies */
    CTBasicSharedPtrCreator& operator=( const CTBasicSharedPtrCreator& src ) GUCEF_DELETED_MEMBER;   /**< dont use, non sensical */

    TBasicSharedPtrSharedData< LockType > m_shared;
};

/*-------------------------------------------------------------------------*/

/**
 *  Templated helper class to facilitate creation of shared pointers associated with an existing object.
 *  If T does not itself inherit from CTBasicSharedPtrCreator then this class will help add that functionality
 *  and thus the benefits of integrated shared pointer creation to the enlarged T footprint as a single object.
 */
template< typename T, class LockType, typename TypeWhenAllocated = T >
class CTBasicSharedPtrCreatorExpansion : public T,
                                         public CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >
{
    public:

    typedef T                                   TBasicSharedPtrContainedType;
    typedef TypeWhenAllocated                   TBasicSharedPtrTypeWhenAllocatedType;
    typedef LockType                            TBasicSharedPtrLockType;
    typedef CTBasicSharedPtr< T, LockType >     TBasicSharedPtrType;
    typedef void                                IsBasicSharedPtrCreatorTag;

    CTBasicSharedPtrCreatorExpansion( void );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    /**
     *  Non-default move constructor that also takes the original address and destructor
     *  Allows you to move the pieces that can be moved while still keeping the correct memory references
     *  for the non-independent lifecycle management via the shared data and dynamic destructor which are incorporated into the object memory footprint
     */
    CTBasicSharedPtrCreatorExpansion( CTBasicSharedPtrCreatorExpansion&& src      ,
                                      TypeWhenAllocated* originalAddressAsCreated ,
                                      CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT;
    #endif

    CTBasicSharedPtrCreatorExpansion( TypeWhenAllocated* originalAddressAsCreated          ,
                                      CIDynamicVoidDestructor* voidDestructor = GUCEF_NULL );

    virtual CTBasicSharedPtr< T, LockType > CreateBasicSharedPtr( T* dummyForCppNameMangling = GUCEF_NULL ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual ~CTBasicSharedPtrCreatorExpansion() GUCEF_VIRTUAL_OVERRIDE;

    private:
    CTBasicSharedPtrCreatorExpansion( const CTBasicSharedPtrCreatorExpansion& src ) GUCEF_DELETED_MEMBER;              /**< dont use */
    CTBasicSharedPtrCreatorExpansion& operator=( const CTBasicSharedPtrCreatorExpansion& src ) GUCEF_DELETED_MEMBER;   /**< dont use */
};

/*-------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows for checking for the existence of the CreateBasicSharedPtr or compatible
 *  member function
 *           CTBasicSharedPtr< T, LockType > T::CreateBasicSharedPtr() const
 */
template < class ClassType, typename SharedPtrType, typename ContainedType >
struct TypeHasMemberFunctionCreateBasicSharedPtr
{
    // For the compile time comparison.
    typedef char    yes[1];
    typedef yes     no[2];

    template <typename U, U u> struct reallyHas;

    template < typename TestClass, typename ShrdPtrType, typename T > static yes& test( reallyHas< ShrdPtrType (TestClass::*)( T* dummyForCppNameMangling ) const, &TestClass::CreateBasicSharedPtr >* /*unused*/ ) { static yes result; return result; }
    template < typename TestClass, typename ShrdPtrType, typename T > static no&  test( ... ) { static no result; return result; }

    // The constant used as a return value for the test.
    enum { value = sizeof( test<ClassType, SharedPtrType, ContainedType>(0) ) == sizeof( yes ) };
};

/*-------------------------------------------------------------------------*/

/**
 *  C++98 compatible SFINAE template helper
 *  Allows for checking for the 'trait' (typedef tag in C++98) 
 *           CTBasicSharedPtr< T, LockType > T::CreateBasicSharedPtr() const
 */
template < class T >
struct HasIsBasicSharedPtrCreatorTag
{
    typedef char yes[1];
    typedef char no[2];

    template <typename U> static yes& test( const typename U::IsBasicSharedPtrCreatorTag* );
    template <typename>   static no&  test( ... );

    // The constant used as a return value for the test.
    enum { value = sizeof( test< T >( 0 ) ) == sizeof( yes ) };
};

/*-------------------------------------------------------------------------*/

/**
 * Helper accessor class that conditionally inherits from the CTBasicSharedPtrCreator in one way or another
 * Resolved the issue of multiple inheritance diamond problems when the user class already inherits from CTBasicSharedPtrCreator
 * without mandating virtual inheritance.
 */
template< typename T, class LockType, typename hasCreateBasicSharedPtr = typename BoolToType< HasIsBasicSharedPtrCreatorTag< T >::value >::type >
class CTBasicSharedPtrCreatorAccessor;

/**
 * Specialization of the helper accessor class that conditionally inherits from the CTBasicSharedPtrCreator in one way or another
 * Resolved the issue of multiple inheritance diamond problems when the user class already inherits from CTBasicSharedPtrCreator
 * without mandating virtual inheritance.
 * This specialization is used when the user class does NOT already inherit from CTBasicSharedPtrCreator
 */
template< typename T, class LockType >
class CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false > : public CTBasicSharedPtrCreatorExpansion< T, LockType, CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false > >
{
    public:

    typedef T                                                                                       TBasicSharedPtrContainedType;
    typedef LockType                                                                                TBasicSharedPtrLockType;
    typedef CTBasicSharedPtr< T, LockType >                                                         TBasicSharedPtrType;
    typedef CTBasicSharedPtrCreator< T, LockType >                                                  TBasicSharedPtrCreatorType;
    typedef CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false >                             TBasicSharedPtrCreatorAccessorType;
    typedef CTBasicSharedPtrCreatorExpansion< T, LockType, TBasicSharedPtrCreatorAccessorType >     TBasicSharedPtrCreatorAccessorBaseType;
    typedef TBasicSharedPtrCreatorAccessorType                                                      TypeWhenAllocated;

    /**
     *  Default constructor creates an uninitialized shared ptr creator, meaning you must use InitializeSharedPtrCreatorData() soon thereafter
     */
    CTBasicSharedPtrCreatorAccessor( void );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    /**
     *  Non-default move constructor that also takes the original address and destructor
     *  Allows you to move the pieces that can be moved while still keeping the correct memory references
     *  for the non-independent lifecycle management via the shared data and dynamic destructor which are incorporated into the object memory footprint
     */
    CTBasicSharedPtrCreatorAccessor( CTBasicSharedPtrCreatorAccessor&& src       ,
                                     TypeWhenAllocated* originalAddressAsCreated ,
                                     CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT;
    #endif

    /**
     *  Constructor of accessor template class which includes passing the original address of this class's instance or of a derived class
     *  Its important the correct pointer is passed here so that the destructor can do its job correctly
     *  Typically this would be 'this' from the most derived class. If you do not derived from this class but rather use it directly
     *  the more typical usage would be to pass the address used for a placement new operation's address for an object of this class to be the value passed here
     */
    CTBasicSharedPtrCreatorAccessor( TypeWhenAllocated* originalAddressAsCreated          ,
                                     CIDynamicVoidDestructor* voidDestructor = GUCEF_NULL );

    virtual ~CTBasicSharedPtrCreatorAccessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual CTBasicSharedPtr< T, LockType > CreateBasicSharedPtr( T* dummyForCppNameMangling = GUCEF_NULL ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTBasicSharedPtrCreatorAccessor( const CTBasicSharedPtrCreatorAccessor& src ) GUCEF_DELETED_MEMBER;              /**< dont use, nonsensical copy */
    CTBasicSharedPtrCreatorAccessor& operator=( const CTBasicSharedPtrCreatorAccessor& src ) GUCEF_DELETED_MEMBER;   /**< dont use, nonsensical copy */
};

/**
 * Specialization of the helper accessor class that conditionally inherits from the CTBasicSharedPtrCreator in one way or another
 * Resolved the issue of multiple inheritance diamond problems when the user class already inherits from CTBasicSharedPtrCreator
 * without mandating virtual inheritance.
 * This specialization is used when the user class DOES already inherit from CTBasicSharedPtrCreator
 */
template< typename T, class LockType >
class CTBasicSharedPtrCreatorAccessor< T, LockType, plain_true > : public T
{
    public:

    typedef T                                                           TBasicSharedPtrContainedType;
    typedef LockType                                                    TBasicSharedPtrLockType;
    typedef CTBasicSharedPtr< T, LockType >                             TBasicSharedPtrType;
    typedef CTBasicSharedPtrCreator< T, LockType >                      TBasicSharedPtrCreatorType;
    typedef CTBasicSharedPtrCreatorAccessor< T, LockType, plain_true >  TBasicSharedPtrCreatorAccessorType;
    typedef T                                                           TBasicSharedPtrCreatorAccessorBaseType;
    typedef TBasicSharedPtrCreatorAccessorType                          TypeWhenAllocated;

    /**
     *  Default constructor creates an uninitialized shared ptr creator, meaning you must use InitializeSharedPtrCreatorData() soon thereafter
     */
    CTBasicSharedPtrCreatorAccessor( void );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

    /**
     *  Non-default move constructor that also takes the original address and destructor
     *  Allows you to move the pieces that can be moved while still keeping the correct memory references
     *  for the non-independent lifecycle management via the shared data and dynamic destructor which are incorporated into the object memory footprint
     */
    CTBasicSharedPtrCreatorAccessor( CTBasicSharedPtrCreatorAccessor&& src       ,
                                     TypeWhenAllocated* originalAddressAsCreated ,
                                     CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT;
    #endif

    /**
     *  Constructor of accessor template class which includes passing the original address of this class's instance or of a derived class
     *  Its important the correct pointer is passed here so that the destructor can do its job correctly
     *  Typically this would be 'this' from the most derived class. If you do not derived from this class but rather use it directly
     *  the more typical usage would be to pass the address used for a placement new operation's address for an object of this class to be the value passed here
     */
    CTBasicSharedPtrCreatorAccessor( TypeWhenAllocated* originalAddressAsCreated          ,
                                     CIDynamicVoidDestructor* voidDestructor = GUCEF_NULL );

    virtual ~CTBasicSharedPtrCreatorAccessor() GUCEF_VIRTUAL_OVERRIDE;

    virtual CTBasicSharedPtr< T, LockType > CreateBasicSharedPtr( T* dummyForCppNameMangling = GUCEF_NULL ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTBasicSharedPtrCreatorAccessor( const CTBasicSharedPtrCreatorAccessor& src ) GUCEF_DELETED_MEMBER;              /**< dont use, nonsensical copy */
    CTBasicSharedPtrCreatorAccessor& operator=( const CTBasicSharedPtrCreatorAccessor& src ) GUCEF_DELETED_MEMBER;   /**< dont use, nonsensical copy */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >::CTBasicSharedPtr( void )
    : m_shared( GUCEF_NULL )
    , m_ptr( GUCEF_NULL )
{GUCEF_TRACE;

    // Note that if this constructor is used an assignment is required at
    // a later time to initialize the shared pointer
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >::CTBasicSharedPtr( const TBasicSharedPtrSharedData< LockType >* shared ,
                                                   const T* ptr                                        )
    : m_shared( GUCEF_NULL )
    , m_ptr( GUCEF_NULL )
{GUCEF_TRACE;

    if ( GUCEF_NULL != shared )
    {
        MT::CObjectScopeLock lock( shared );
        m_shared = const_cast< TBasicSharedPtrSharedData< LockType >* >( shared );
        ++m_shared->m_refCounter;
        m_ptr = const_cast< T* >( ptr );
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >::CTBasicSharedPtr( T* ptr                                          ,
                                                   CIDynamicVoidDestructor* voidedObjectDestructor ,
                                                   void* originalAddressAsCreated                  )
    : m_shared( GUCEF_NULL )
    , m_ptr( GUCEF_NULL )
{GUCEF_TRACE;

    Initialize( ptr, voidedObjectDestructor, originalAddressAsCreated );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >::CTBasicSharedPtr( const CTBasicSharedPtr< T, LockType >& src )
    : m_shared( GUCEF_NULL )
    , m_ptr( GUCEF_NULL )
{GUCEF_TRACE;

    // disable Clang warning: reference cannot be bound to dereferenced null pointer in well-defined C++ code; comparison may be assumed to always evaluate to true [-Wtautological-undefined-compare]
    // while technically the null reference should never occur this is intended to catch bad memory access a bit earlier in the call stack
    #ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wtautological-undefined-compare"
    #endif
    if ( GUCEF_NULL != &src && GUCEF_NULL != src.m_shared )
    {
        MT::CObjectScopeLock lock( &src );
        if ( GUCEF_NULL != src.m_shared )
        {
            m_shared = src.m_shared;
            ++(m_shared->m_refCounter);
        }
        m_ptr = src.m_ptr;
    }
    #ifdef __clang__
    #pragma clang diagnostic pop
    #endif
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >::CTBasicSharedPtr( CTBasicSharedPtr< T, LockType >&& src ) GUCEF_NOEXCEPT
    : m_shared( GUCEF_MOVE( src.m_shared ) )
    , m_ptr( GUCEF_MOVE( src.m_ptr ) )
{GUCEF_TRACE;

    src.m_shared = GUCEF_NULL;
    src.m_ptr = GUCEF_NULL;
}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >::~CTBasicSharedPtr()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
void
CTBasicSharedPtr< T, LockType >::Initialize( T* ptr                                          ,
                                             CIDynamicVoidDestructor* voidedObjectDestructor ,
                                             void* originalAddressAsCreated                  )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    {
        // If you get an assert here:
        //    You have an error in your decending class: you cannot initialize twice
        assert( m_shared == GUCEF_NULL );
        assert( m_ptr == GUCEF_NULL );
    }
    #endif

    #ifdef GUCEF_DEBUG_MODE
    {
        // If you get an assert here:
        //    Inconsistent params provided, check your code
        assert( ( ptr == GUCEF_NULL && originalAddressAsCreated == GUCEF_NULL ) ||
                ( ptr != GUCEF_NULL && originalAddressAsCreated != GUCEF_NULL && voidedObjectDestructor != GUCEF_NULL ) );
    }
    #endif

    // Just in case we did not hit the asserts above because the code was compiled in release
    // mode without asserts we will still allow the scenario by unlinking first
    Unlink();

    if ( GUCEF_NULL != ptr )
    {
        m_shared = GUCEF_NEW TBasicSharedPtrSharedData< LockType >( true );
        m_shared->m_refCounter = 1UL;
        m_shared->m_voidDestructor = voidedObjectDestructor;
        m_shared->m_originalAddressAsCreated = originalAddressAsCreated;
        m_ptr = ptr;
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTBasicSharedPtr< T, LockType >::RelinquishOwnership( T*& rawPtr )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( GUCEF_NULL == m_shared || m_ptr == GUCEF_NULL )
    {
        // we are releasing ownership of a NULL pointer
        rawPtr = GUCEF_NULL;
        return true;
    }

    if ( 1 == m_shared->m_refCounter )
    {
        --(m_shared->m_refCounter);
        rawPtr = m_ptr;
        m_ptr = GUCEF_NULL;
        Unlink();
        return true;
    }
    else
    {
        /*
         *  If you get here you should check your code flow
         *  If you want to use this member function you should ensure that the local scope has the last remaining reference
         */
        rawPtr = GUCEF_NULL;
        GUCEF_ASSERT_ALWAYS;
        return false;
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
void
CTBasicSharedPtr< T, LockType >::OverrideDestructor( CIDynamicVoidDestructor* voidedObjectDestructor )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( GUCEF_NULL == m_shared )
    {
        m_shared->m_voidDestructor = voidedObjectDestructor;
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CIDynamicVoidDestructor*
CTBasicSharedPtr< T, LockType >::GetDestructor( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( GUCEF_NULL != m_shared )
    {
        return m_shared->m_voidDestructor;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
bool
CTBasicSharedPtr< T, LockType >::IsNULL( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
UInt32
CTBasicSharedPtr< T, LockType >::GetReferenceCount( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    if ( GUCEF_NULL != m_shared )
        return m_shared->m_refCounter;
    return 0;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >&
CTBasicSharedPtr< T, LockType >::operator=( int nullValue )
{GUCEF_TRACE;

    assert( nullValue == (int) GUCEF_NULL );
    Unlink();
    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >&
CTBasicSharedPtr< T, LockType >::operator=( const CTBasicSharedPtr< T, LockType >& src )
{GUCEF_TRACE;

    // guard against self assignment
    if ( this != &src && m_shared != src.m_shared )
    {
        InitializeUsingRelatedType( src );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator<( const CTBasicSharedPtr< T, LockType >& other ) const
{GUCEF_TRACE;

    return m_ptr < other.m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator<( const void* other ) const
{GUCEF_TRACE;

    return m_ptr < other;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator==( const CTBasicSharedPtr< T, LockType >& other ) const
{GUCEF_TRACE;

    return m_ptr == other.m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator==( const void* other ) const
{GUCEF_TRACE;

    return other == m_ptr;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_32BIT

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator==( Int32 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL == m_ptr;
    if ( sizeof( int ) != sizeof( m_ptr ) )
        return false;
    return reinterpret_cast< void* >( other ) == m_ptr;
}

#else

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator==( Int32 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL == m_ptr;
    return false;
}

#endif

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_64BIT

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator==( Int64 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL == m_ptr;
    if ( sizeof( int ) != sizeof( m_ptr ) )
        return false;
    return reinterpret_cast< void* >( other ) == m_ptr;
}

#else

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator==( Int64 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL == m_ptr;
    return false;
}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator!=( const CTBasicSharedPtr< T, LockType >& other ) const
{GUCEF_TRACE;

    return m_ptr != other.m_ptr;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_32BIT

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator!=( Int32 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL != m_ptr;
    if ( sizeof( int ) != sizeof( m_ptr ) )
        return true;
    return reinterpret_cast< void* >( other ) != m_ptr;
}

#else

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator!=( Int32 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL != m_ptr;
    return true;
}

#endif

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_64BIT

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator!=( Int64 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL != m_ptr;
    if ( sizeof( int ) != sizeof( m_ptr ) )
        return true;
    return reinterpret_cast< void* >( other ) != m_ptr;
}

#else

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator!=( Int64 other ) const
{GUCEF_TRACE;

    if ( 0 == other )
        return GUCEF_NULL != m_ptr;
    return true;
}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
CTBasicSharedPtr< T, LockType >::operator!=( const void* other ) const
{GUCEF_TRACE;

    return other != m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator==( const T* ptr, const CTBasicSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return other == ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator==( const Int64 ptr, const CTBasicSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return other == ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator==( const Int32 ptr, const CTBasicSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return other == ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator!=( const T* ptr, const CTBasicSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return other != ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator!=( Int32 intPtr, const CTBasicSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return other != intPtr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline bool
operator!=( Int64 intPtr, const CTBasicSharedPtr< T, LockType >& other )
{GUCEF_TRACE;

    return other != intPtr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline T&
CTBasicSharedPtr< T, LockType >::operator*( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_ptr )
    {
        return *(m_ptr);
    }

    // Someone forgot to initialize the shared pointer with an assignment
    GUCEF_EMSGTHROW( ENotInitialized, "CTBasicSharedPtr< T, LockType >::operator*( void ): uninitialized pointer usage" );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline const T&
CTBasicSharedPtr< T, LockType >::operator*( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_ptr )
    {
        return *m_ptr;
    }

    // Someone forgot to initialize the shared pointer with an assignment
    GUCEF_EMSGTHROW( ENotInitialized, "CTBasicSharedPtr< T, LockType >::operator*( void ) const: uninitialized pointer usage" );

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline T*
CTBasicSharedPtr< T, LockType >::operator->( void )
{GUCEF_TRACE;

    return m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline const T*
CTBasicSharedPtr< T, LockType >::operator->( void ) const
{GUCEF_TRACE;

    return m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline
CTBasicSharedPtr< T, LockType >::operator bool() const
{GUCEF_TRACE;

    return GUCEF_NULL != m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline T*
CTBasicSharedPtr< T, LockType >::GetPointer( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_ptr )
    {
        return m_ptr;
    }

    // Someone forgot to initialize the shared pointer with an assignment
    GUCEF_EMSGTHROW( ENotInitialized, "CTBasicSharedPtr< T, LockType >::operator->( void ): uninitialized pointer usage" );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline const T*
CTBasicSharedPtr< T, LockType >::GetPointer( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_ptr )
    {
        return m_ptr;
    }

    // Someone forgot to initialize the shared pointer with an assignment
    GUCEF_EMSGTHROW( ENotInitialized, "CTBasicSharedPtr< T, LockType >::operator->( void ) const: uninitialized pointer usage" );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline T*
CTBasicSharedPtr< T, LockType >::GetPointerAlways( void )
{GUCEF_TRACE;

    return m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
inline const T*
CTBasicSharedPtr< T, LockType >::GetPointerAlways( void ) const
{GUCEF_TRACE;

    return m_ptr;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
void
CTBasicSharedPtr< T, LockType >::Unlink( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_shared )
    {
        m_shared->m_lock.Lock(); // We cannot use CObjectScopeLock here because we may have to delete the lock itself
        if ( GUCEF_NULL != m_ptr )
        {
            --m_shared->m_refCounter;
            assert( m_shared->m_refCounter >= 0 );
            if ( 0 >= m_shared->m_refCounter )
            {                
                TBasicSharedPtrSharedData< LockType >* localSharedRef = m_shared;
                bool sharedDataIsIndependent = localSharedRef->m_hasIndependentLifeCycle;

                // We should check if the destructor pointer is not-NULL
                // A descending class may NULL it for its own purposes.
                if ( GUCEF_NULL != localSharedRef->m_voidDestructor )
                {
                    // Given there is no retry mechanism and we assume destroy succeeds we will null the pointers right away
                    // thus any invalid reuse of the same memory space is more likely to run into a null pointer rather than a dangling pointer
                    CIDynamicVoidDestructor* voidDestructor = localSharedRef->m_voidDestructor;
                    void* originalAddressAsCreated = localSharedRef->m_originalAddressAsCreated;
                    localSharedRef->m_voidDestructor = GUCEF_NULL;
                    localSharedRef->m_originalAddressAsCreated = GUCEF_NULL;

                    GUCEF_CHECKALLOCPTR( originalAddressAsCreated );
                    voidDestructor->DestroyKnownVoidedObject( originalAddressAsCreated );
                }
                m_ptr = GUCEF_NULL;

                if ( sharedDataIsIndependent )
                {                    
                    m_shared = GUCEF_NULL;
                    localSharedRef->m_lock.Unlock();
                    GUCEF_DELETE localSharedRef;
                }
                else
                {
                    m_shared = GUCEF_NULL;
                }
            }
            else
            {
                // Not the last reference but we are unlinking our usage
                TBasicSharedPtrSharedData< LockType >* localSharedRef = m_shared;

                // this object may not have been the last reference but we still have to NULL
                // the attributes to allow this object to be re-used
                m_shared = GUCEF_NULL;
                m_ptr = GUCEF_NULL;

                localSharedRef->m_lock.Unlock();
            }
        }
        else
        {
            // All we ever had was a lock
            TBasicSharedPtrSharedData< LockType >* localSharedRef = m_shared;
            m_shared = GUCEF_NULL;
            if ( GUCEF_NULL != localSharedRef )
            {
                bool sharedDataIsIndependent = localSharedRef->m_hasIndependentLifeCycle;

                // We should check if the destructor pointer is not-NULL
                // Under most conditions there would be no object destructor if there is no object pointed to
                if ( GUCEF_NULL != localSharedRef->m_voidDestructor )
                {
                    // Given there is no retry mechanism and we assume destroy succeeds we will null the pointers right away
                    // thus any invalid reuse of the same memory space is more likely to run into a null pointer rather than a dangling pointer
                    CIDynamicVoidDestructor* voidDestructor = localSharedRef->m_voidDestructor;
                    void* originalAddressAsCreated = localSharedRef->m_originalAddressAsCreated;
                    localSharedRef->m_voidDestructor = GUCEF_NULL;
                    localSharedRef->m_originalAddressAsCreated = GUCEF_NULL;

                    GUCEF_CHECKALLOCPTR( originalAddressAsCreated );
                    voidDestructor->DestroyKnownVoidedObject( originalAddressAsCreated );
                }

                if ( sharedDataIsIndependent )
                {
                    localSharedRef->m_lock.Unlock();
                    GUCEF_DELETE localSharedRef;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
const MT::CILockable*
CTBasicSharedPtr< T, LockType >::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
MT::TLockStatus
CTBasicSharedPtr< T, LockType >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_shared )
    {
        return m_shared->m_lock.Lock( lockWaitTimeoutInMs );
    }
    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
MT::TLockStatus
CTBasicSharedPtr< T, LockType >::Unlock( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_shared )
    {
        return m_shared->m_lock.Unlock();
    }
    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CICloneable*
CTBasicSharedPtr< T, LockType >::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CTBasicSharedPtr< T, LockType >( *this );
}

/*-------------------------------------------------------------------------*/

template< class LockType >
TBasicSharedPtrSharedData< LockType >::TBasicSharedPtrSharedData( bool hasIndependentLifeCycle )
    : MT::CILockable()
    , m_refCounter( 0 )
    , m_lock()
    , m_hasIndependentLifeCycle( hasIndependentLifeCycle )
    , m_voidDestructor( GUCEF_NULL )
    , m_originalAddressAsCreated( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< class LockType >
TBasicSharedPtrSharedData< LockType >::TBasicSharedPtrSharedData( TBasicSharedPtrSharedData&& src           ,
                                                                  void* originalAddressAsCreated            ,
                                                                  CIDynamicVoidDestructor* voidedDestructor ) GUCEF_NOEXCEPT
    : MT::CILockable()
    , m_refCounter( src.m_refCounter )
    , m_lock( GUCEF_MOVE( src.m_lock ) )
    , m_hasIndependentLifeCycle( src.m_hasIndependentLifeCycle )
    , m_voidDestructor( voidedDestructor )
    , m_originalAddressAsCreated( originalAddressAsCreated )
{GUCEF_TRACE;

    src.m_refCounter = 0;
    src.m_voidDestructor = GUCEF_NULL;
    src.m_originalAddressAsCreated = GUCEF_NULL;
}

#endif

/*-------------------------------------------------------------------------*/

template< class LockType >
TBasicSharedPtrSharedData< LockType >::~TBasicSharedPtrSharedData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class LockType >
bool
TBasicSharedPtrSharedData< LockType >::InitSharedData( void* originalAddressAsCreated            ,
                                                       CIDynamicVoidDestructor* voidedDestructor )
{GUCEF_TRACE;

    // This is intended as a one-time init during construction
    // Any null value indicates improper usage
    // make sure you call the correct specialization of this Init function. Its split up the way it is to avoid any accidental misuse
    // since doing so could lead to hard to debug memory corruption issues
    GUCEF_ASSERT( GUCEF_NULL != originalAddressAsCreated );
    GUCEF_ASSERT( GUCEF_NULL != voidedDestructor );
    GUCEF_ASSERT( GUCEF_NULL == m_originalAddressAsCreated );
    GUCEF_ASSERT( GUCEF_NULL == m_voidDestructor );

    MT::CObjectScopeLock lock( this );

    if GUCEF_PREDICT_TRUE( ( GUCEF_NULL != originalAddressAsCreated && GUCEF_NULL == m_originalAddressAsCreated ) &&
                            ( GUCEF_NULL != voidedDestructor && GUCEF_NULL == m_voidDestructor )  )
    {
        m_originalAddressAsCreated = originalAddressAsCreated;
        m_voidDestructor = voidedDestructor;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< class LockType >
bool
TBasicSharedPtrSharedData< LockType >::InitSharedDataDestructor( CIDynamicVoidDestructor* voidedDestructor )
{GUCEF_TRACE;

    // This is intended as a one-time init during construction
    // Any null value indicates improper usage
    // make sure you call the correct specialization of this Init function. Its split up the way it is to avoid any accidental misuse
    // since doing so could lead to hard to debug memory corruption issues
    GUCEF_ASSERT( GUCEF_NULL != voidedDestructor );        
    GUCEF_ASSERT( GUCEF_NULL == m_voidDestructor );
    GUCEF_ASSERT( GUCEF_NULL != m_originalAddressAsCreated );

    MT::CObjectScopeLock lock( this );

    if GUCEF_PREDICT_TRUE( GUCEF_NULL != m_originalAddressAsCreated &&
                            GUCEF_NULL != voidedDestructor && GUCEF_NULL == m_voidDestructor )
    {
        m_voidDestructor = voidedDestructor;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< class LockType >
bool
TBasicSharedPtrSharedData< LockType >::InitSharedDataOrgAddr( void* originalAddressAsCreated )
{GUCEF_TRACE;

    // This is intended as a one-time init during construction
    // Any null value indicates improper usage
    // make sure you call the correct specialization of this Init function. Its split up the way it is to avoid any accidental misuse
    // since doing so could lead to hard to debug memory corruption issues
    GUCEF_ASSERT( GUCEF_NULL != originalAddressAsCreated );        
    GUCEF_ASSERT( GUCEF_NULL == m_originalAddressAsCreated );

    MT::CObjectScopeLock lock( this );

    if GUCEF_PREDICT_TRUE( GUCEF_NULL == m_originalAddressAsCreated &&
                           GUCEF_NULL != originalAddressAsCreated    )
    {
        m_originalAddressAsCreated = originalAddressAsCreated;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< class LockType >
void
TBasicSharedPtrSharedData< LockType >::OverrideSharedData( void* originalAddressAsCreated            ,
                                                           CIDynamicVoidDestructor* voidedDestructor )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    m_originalAddressAsCreated = originalAddressAsCreated;
    m_voidDestructor = voidedDestructor;
}

/*-------------------------------------------------------------------------*/

template< class LockType >
MT::TLockStatus
TBasicSharedPtrSharedData< LockType >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

template< class LockType >
MT::TLockStatus
TBasicSharedPtrSharedData< LockType >::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

template< class LockType >
const MT::CILockable*
TBasicSharedPtrSharedData< LockType >::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::CTBasicSharedPtrCreator( void  )
    : m_shared( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::CTBasicSharedPtrCreator( TypeWhenAllocated* originalAddressAsCreated )
    : m_shared( false )
{GUCEF_TRACE;

    InitializeSharedPtrCreatorDataOrgAddr( originalAddressAsCreated );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::CTBasicSharedPtrCreator( TypeWhenAllocated* originalAddressAsCreated ,
                                                                                    CIDynamicVoidDestructor* voidDestructor     )
    : m_shared( false )
{GUCEF_TRACE;

    InitializeSharedPtrCreatorData( originalAddressAsCreated, voidDestructor );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::CTBasicSharedPtrCreator( CTBasicSharedPtrCreator&& src               ,
                                                                                    TypeWhenAllocated* originalAddressAsCreated ,
                                                                                    CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT
    : m_shared( GUCEF_MOVE( src.m_shared ), originalAddressAsCreated, voidDestructor )
{GUCEF_TRACE;

}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::~CTBasicSharedPtrCreator( void )
{GUCEF_TRACE;

    GUCEF_ASSERT( 0 == m_shared.m_refCounter );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtr< T, LockType >
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::CreateBasicSharedPtr( T* dummyForCppNameMangling ) const
{GUCEF_TRACE;

    const TypeWhenAllocated* topLevelObjPtr = reinterpret_cast< const TypeWhenAllocated* >( m_shared.m_originalAddressAsCreated );
    CTBasicSharedPtr< T, LockType > retVal( &m_shared, static_cast< const T* >( topLevelObjPtr ) );
    return retVal;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
bool
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::InitializeSharedPtrCreatorData( TypeWhenAllocated* originalAddressAsCreated, CIDynamicVoidDestructor* voidDestructor )
{GUCEF_TRACE;

    return m_shared.InitSharedData( originalAddressAsCreated, voidDestructor );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
bool
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::InitializeSharedPtrCreatorDataDestructor( CIDynamicVoidDestructor* voidedDestructor )
{GUCEF_TRACE;

    return m_shared.InitSharedDataDestructor( voidedDestructor );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
bool
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::InitializeSharedPtrCreatorDataOrgAddr( TypeWhenAllocated* originalAddressAsCreated )
{GUCEF_TRACE;

    return m_shared.InitSharedDataOrgAddr( originalAddressAsCreated );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
void
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::OverrideSharedPtrCreatorData( TypeWhenAllocated* originalAddressAsCreated, CIDynamicVoidDestructor* voidDestructor )
{GUCEF_TRACE;

    return m_shared.OverrideSharedData( originalAddressAsCreated, voidDestructor );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
const TBasicSharedPtrSharedData< LockType >&
CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::GetBasicSharedPtrData( void ) const
{GUCEF_TRACE;

    return m_shared;
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreatorExpansion< T, LockType, TypeWhenAllocated >::CTBasicSharedPtrCreatorExpansion( void )
    : T()
    , CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreatorExpansion< T, LockType, TypeWhenAllocated >::CTBasicSharedPtrCreatorExpansion( CTBasicSharedPtrCreatorExpansion&& src      ,
                                                                                                      TypeWhenAllocated* originalAddressAsCreated ,
                                                                                                      CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT
    : T( GUCEF_MOVE( src ) )
    , CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >( GUCEF_MOVE( src ), originalAddressAsCreated, voidDestructor )
{GUCEF_TRACE;

}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreatorExpansion< T, LockType, TypeWhenAllocated >::CTBasicSharedPtrCreatorExpansion( TypeWhenAllocated* originalAddressAsCreated ,
                                                                                                      CIDynamicVoidDestructor* voidDestructor     )
    : T()
    , CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >( originalAddressAsCreated, voidDestructor )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtrCreatorExpansion< T, LockType, TypeWhenAllocated >::~CTBasicSharedPtrCreatorExpansion()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType, typename TypeWhenAllocated >
CTBasicSharedPtr< T, LockType >
CTBasicSharedPtrCreatorExpansion< T, LockType, TypeWhenAllocated >::CreateBasicSharedPtr( T* dummyForCppNameMangling ) const
{GUCEF_TRACE;

    // Call the base class implementation explicitly to get the shared pointer
    return CTBasicSharedPtrCreator< T, LockType, TypeWhenAllocated >::CreateBasicSharedPtr( dummyForCppNameMangling );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false >::CTBasicSharedPtrCreatorAccessor( void )
    : CTBasicSharedPtrCreatorExpansion< T, LockType, TBasicSharedPtrCreatorAccessorType >()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false >::CTBasicSharedPtrCreatorAccessor( TypeWhenAllocated* originalAddressAsCreated ,
                                                                                              CIDynamicVoidDestructor* voidDestructor     )
    : CTBasicSharedPtrCreatorExpansion< T, LockType, TBasicSharedPtrCreatorAccessorType >( originalAddressAsCreated, voidDestructor )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false >::CTBasicSharedPtrCreatorAccessor( CTBasicSharedPtrCreatorAccessor&& src       ,
                                                                                              TypeWhenAllocated* originalAddressAsCreated ,
                                                                                              CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT
    : CTBasicSharedPtrCreatorExpansion< T, LockType, TBasicSharedPtrCreatorAccessorType >( GUCEF_MOVE( src ), originalAddressAsCreated, voidDestructor )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    // If you get an assert here you should check your code flow as this constructor is not allowed to 'move' when there are active references
    // its only intended to be used during initial construction 
    const TBasicSharedPtrSharedData< LockType >& srcBasicSharedPtrData = src.GetBasicSharedPtrData();
    GUCEF_ASSERT( 0 == srcBasicSharedPtrData.m_refCounter );
    #endif
}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false >::~CTBasicSharedPtrCreatorAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_false  >::CreateBasicSharedPtr( T* dummyForCppNameMangling ) const
{GUCEF_TRACE;

    // Call the base class implementation explicitly to get the shared pointer

    return CTBasicSharedPtrCreatorExpansion< T, LockType, TBasicSharedPtrCreatorAccessorType >::CreateBasicSharedPtr( dummyForCppNameMangling );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_true >::CTBasicSharedPtrCreatorAccessor( void )
    : T()
{GUCEF_TRACE;

    // When using this constructor we specifically mandate a subsequent call to InitializeSharedPtrCreatorData()
    // We want to keep the behavior consistent across the case where T is or is not derived from CTBasicSharedPtrCreator
    // Hence one can always call InitializeSharedPtrCreatorData() after default construction with an CTBasicSharedPtrCreatorAccessor
    T::OverrideSharedPtrCreatorData( GUCEF_NULL, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_true >::CTBasicSharedPtrCreatorAccessor( TypeWhenAllocated* originalAddressAsCreated ,
                                                                                             CIDynamicVoidDestructor* voidDestructor     )
    : T()
{GUCEF_TRACE;

    T::OverrideSharedPtrCreatorData( originalAddressAsCreated, voidDestructor );
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_true >::CTBasicSharedPtrCreatorAccessor( CTBasicSharedPtrCreatorAccessor&& src       ,
                                                                                             TypeWhenAllocated* originalAddressAsCreated ,
                                                                                             CIDynamicVoidDestructor* voidDestructor     ) GUCEF_NOEXCEPT
    : T( GUCEF_MOVE( src ) )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    // If you get an assert here you should check your code flow as this constructor is not allowed to 'move' when there are active references
    // its only intended to be used during initial construction 
    const TBasicSharedPtrSharedData< LockType >& srcBasicSharedPtrData = src.GetBasicSharedPtrData();
    GUCEF_ASSERT( 0 == srcBasicSharedPtrData.m_refCounter );
    #endif

    T::OverrideSharedPtrCreatorData( originalAddressAsCreated, voidDestructor );
}

#endif

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_true >::~CTBasicSharedPtrCreatorAccessor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename T, class LockType >
CTBasicSharedPtr< T, LockType >
CTBasicSharedPtrCreatorAccessor< T, LockType, plain_true  >::CreateBasicSharedPtr( T* dummyForCppNameMangling ) const
{GUCEF_TRACE;

    // Call the base class implementation explicitly to get the shared pointer
    return T::CreateBasicSharedPtr( dummyForCppNameMangling );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTBASICSHAREDPTR_H ? */
