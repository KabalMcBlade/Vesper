#pragma once

#include <iostream>

//////////////////////////////////////////////////////////////////////////
// PREPROCESSORS
//////////////////////////////////////////////////////////////////////////

// If VESPERENGINE_DYNAMIC_LIB is defined, then the export/import symbols are generated
//#define VESPERENGINE_DYNAMIC_LIB

#ifdef VESPERENGINE_DYNAMIC_LIB

#ifdef VESPERENGINE_EXPORTS
#define VESPERENGINE_DLL __declspec(dllexport)
#else
#define VESPERENGINE_DLL __declspec(dllimport)
#endif 

#else

#define VESPERENGINE_DLL 

#endif // VESPERENGINE_DYNAMIC_LIB




// defines for easy namespace
#define VESPERENGINE_NAMESPACE_BEGIN namespace vesper {
#define VESPERENGINE_NAMESPACE_END };  

#define VESPERENGINE_USING_NAMESPACE using namespace vesper; 

#define VESPERENGINE_OPTIMIZATION_OFF __pragma(optimize("",off))
#define VESPERENGINE_OPTIMIZATION_ON __pragma(optimize("",on))

/// forces a function to be in lined
#define VESPERENGINE_INLINE    __forceinline

// tells the compiler to never inline a particular function
#define VESPERENGINE_NO_INLINE    __declspec(noinline)

// Memory alignment
#define VESPERENGINE_MEMORY_ALIGNMENT_SIZE 16
#define VESPERENGINE_MEMORY_ALIGNMENT(x)    __declspec(align(x))
#define VESPERENGINE_IS_ALIGNED(ptr, alignment)    ((std::uintptr_t)ptr & (alignment - 1)) == 0
#define VESPERENGINE_MEMORY_ALIGNED		VESPERENGINE_MEMORY_ALIGNMENT(VESPERENGINE_MEMORY_ALIGNMENT_SIZE)

// BIT MANIPULATVESPERENGINE
#define VESPERENGINE_BIT_SET(value, bitpos)          ((value) |= (1<<(bitpos)))
#define VESPERENGINE_BIT_SET_IFF(value, iff, bitpos) ((value) ^= (-iff ^ (value)) & (1 << (bitpos)))
#define VESPERENGINE_BIT_SET_VALUE(value, mask, set) ((value) = (((value) & (mask)) | (set)))

#define VESPERENGINE_BIT_CHECK(value, bitpos)        ((value) & (1<<(bitpos))) 
#define VESPERENGINE_BIT_CLEAR(value, bitpos)        ((value) &= ~((1) << (bitpos)))
#define VESPERENGINE_BIT_TOGGLE(value, bitpos)       ((value) ^= (1<<(bitpos)))
#define VESPERENGINE_BIT_GET(value, mask)            ((value) & (mask)) 

#define VESPERENGINE_UNUSED(x)   (void)(x)


//////////////////////////////////////////////////////////////////////////
// TYPEDEFS
//////////////////////////////////////////////////////////////////////////

typedef char                int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;


//////////////////////////////////////////////////////////////////////////
// ASSERT
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define assertMsg( condition, message ) \
    if( !(condition) ) { \
        std::cerr << "Assert: " << (#condition) << std::endl; \
        std::cerr << "Message: " << message << std::endl; \
        std::cerr << "File: " << __FILE__ << std::endl; \
        std::cerr << "Line: " << __LINE__ << std::endl << std::endl; \
    }
#else
#define assertMsg( condition, message )
#endif // DEBUG

#define assertMsgReturnVoid( condition, message ) \
    assertMsg( condition, message )\
    if( !(condition) ) { \
        return;\
    }

#define assertMsgReturnValue( condition, message, return_value ) \
    assertMsg( condition, message )\
    if( !(condition) ) { \
        return return_value;\
    }


 //////////////////////////////////////////////////////////////////////////

#pragma warning( disable : 4251 )