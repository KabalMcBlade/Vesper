// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Core\core_defines.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include <iostream>

//////////////////////////////////////////////////////////////////////////
// PREPROCESSORS
//////////////////////////////////////////////////////////////////////////


#if defined(VESPERENGINE_DLL_EXPORT)             // When exporting from the DLL
#define VESPERENGINE_API __declspec(dllexport)
#elif defined(VESPERENGINE_DLL_IMPORT)           // When importing to the DLL
#define VESPERENGINE_API __declspec(dllimport)
#else                                           // When using the static library
#define VESPERENGINE_API
#endif


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
#define VESPERENGINE_ALIGN16 alignas(16)

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

// Special for UBO,
// In std140, a scalar like an int does not require 16-byte alignment unless it's in a struct or packing arrays of them
typedef struct AlignedInt
{
    VESPERENGINE_ALIGN16 int32 value;

    // Implicit conversion
	operator int32() const { return value; }

    // Assignment from int32
    AlignedInt& operator=(int32 newValue)
    {
        value = newValue;
        return *this;
    }

    // Assignment from AlignedInt
    AlignedInt& operator=(const AlignedInt& other)
    {
        value = other.value;
        return *this;
    }

    // Equality operator
    bool operator==(const AlignedInt& other) const
    {
        return value == other.value;
    }

    // Prefix increment
    AlignedInt& operator++()
    {
        ++value;
        return *this;
    }

    // Postfix increment
    AlignedInt operator++(int)
    {
        AlignedInt temp = *this;
        ++value;
        return temp;
    }

    // Prefix decrement
    AlignedInt& operator--()
    {
        --value;
        return *this;
    }

    // Postfix decrement
    AlignedInt operator--(int)
    {
        AlignedInt temp = *this;
        --value;
        return temp;
    }

    // Addition
    AlignedInt operator+(const AlignedInt& other) const
    {
        return AlignedInt{ value + other.value };
    }

    // Subtraction
    AlignedInt operator-(const AlignedInt& other) const
    {
        return AlignedInt{ value - other.value };
    }

} alignInt32;

// Special for UBO,
// In std140, a scalar like an int does not require 16-byte alignment unless it's in a struct or packing arrays of them
typedef struct AlignedFloat
{
    VESPERENGINE_ALIGN16 float value;

    // Implicit conversion
	operator float() const { return value; }

    // Assignment from float
    AlignedFloat& operator=(float newValue)
    {
        value = newValue;
        return *this;
    }

    // Assignment from AlignedFloat
    AlignedFloat& operator=(const AlignedFloat& other)
    {
        value = other.value;
        return *this;
    }

    // Equality operator
    bool operator==(const AlignedFloat& other) const
    {
        return value == other.value;
    }

    // Prefix increment
    AlignedFloat& operator++()
    {
        ++value;
        return *this;
    }

    // Postfix increment
    AlignedFloat operator++(int)
    {
        AlignedFloat temp = *this;
        ++value;
        return temp;
    }

    // Prefix decrement
    AlignedFloat& operator--()
    {
        --value;
        return *this;
    }

    // Postfix decrement
    AlignedFloat operator--(int)
    {
        AlignedFloat temp = *this;
        --value;
        return temp;
    }

    // Addition
    AlignedFloat operator+(const AlignedFloat& other) const
    {
        return AlignedFloat{ value + other.value };
    }

    // Subtraction
    AlignedFloat operator-(const AlignedFloat& other) const
    {
        return AlignedFloat{ value - other.value };
    }

} alignFloat;


//////////////////////////////////////////////////////////////////////////
// SYSTEM/RENDER SPECIFIC
//////////////////////////////////////////////////////////////////////////

#define VESPERENGINE_PUSHCONSTANT_DEFAULTRANGE 128


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