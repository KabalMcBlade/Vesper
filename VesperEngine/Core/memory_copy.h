// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\VesperEngine\Core\memory_copy.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "Core/core_defines.h"

#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_2__) || defined(__SSE4_1__) || defined(__SSSE3__) || defined(__SSE3__) || defined(__SSE2__) || defined(_M_X64)
#define VESPERENGINE_MEM_COPY_SIMD
#include <immintrin.h>
#endif

#include <cstddef>


VESPERENGINE_NAMESPACE_BEGIN


#ifdef VESPERENGINE_MEM_COPY_SIMD

VESPERENGINE_INLINE void MemCpySIMD64(void* _dst, const void* _src, std::size_t _len)
{
	uint8* dst = (uint8*)_dst;
	const uint8* src = (const uint8*)_src;

	std::size_t i = 0;
	for (; i + 256 <= _len; i += 256)
	{
		__m256i d0 = _mm256_load_si256((__m256i*) & src[i + 0 * 32]);
		__m256i d1 = _mm256_load_si256((__m256i*) & src[i + 1 * 32]);
		__m256i d2 = _mm256_load_si256((__m256i*) & src[i + 2 * 32]);
		__m256i d3 = _mm256_load_si256((__m256i*) & src[i + 3 * 32]);
		__m256i d4 = _mm256_load_si256((__m256i*) & src[i + 4 * 32]);
		__m256i d5 = _mm256_load_si256((__m256i*) & src[i + 5 * 32]);
		__m256i d6 = _mm256_load_si256((__m256i*) & src[i + 6 * 32]);
		__m256i d7 = _mm256_load_si256((__m256i*) & src[i + 7 * 32]);

		_mm256_stream_si256((__m256i*) & dst[i + 0 * 32], d0);
		_mm256_stream_si256((__m256i*) & dst[i + 1 * 32], d1);
		_mm256_stream_si256((__m256i*) & dst[i + 2 * 32], d2);
		_mm256_stream_si256((__m256i*) & dst[i + 3 * 32], d3);
		_mm256_stream_si256((__m256i*) & dst[i + 4 * 32], d4);
		_mm256_stream_si256((__m256i*) & dst[i + 5 * 32], d5);
		_mm256_stream_si256((__m256i*) & dst[i + 6 * 32], d6);
		_mm256_stream_si256((__m256i*) & dst[i + 7 * 32], d7);
	}

	for (; i + 32 <= _len; i += 32)
	{
		__m256i d = _mm256_load_si256((__m256i*) & src[i]);
		_mm256_stream_si256((__m256i*) & dst[i], d);
	}

	for (; i + 16 <= _len; i += 16)
	{
		__m128i d = _mm_load_si128((__m128i*) & src[i]);
		_mm_stream_si128((__m128i*) & dst[i], d);
	}

	for (; i + 4 <= _len; i += 4)
	{
		*(uint32*)&dst[i] = *(const uint32*)&src[i];
	}

	for (; i < _len; ++i)
	{
		dst[i] = src[i];
	}

	_mm_sfence();
}

VESPERENGINE_INLINE void MemCpySIMD(void* _dst, const void* _src, std::size_t _len)
{
	uint8* dst = (uint8*)_dst;
	const uint8* src = (const uint8*)_src;

	std::size_t i = 0;
	for (; i + 128 <= _len; i += 128)
	{
		__m128i d0 = _mm_load_si128((__m128i*) & src[i + 0 * 16]);
		__m128i d1 = _mm_load_si128((__m128i*) & src[i + 1 * 16]);
		__m128i d2 = _mm_load_si128((__m128i*) & src[i + 2 * 16]);
		__m128i d3 = _mm_load_si128((__m128i*) & src[i + 3 * 16]);
		__m128i d4 = _mm_load_si128((__m128i*) & src[i + 4 * 16]);
		__m128i d5 = _mm_load_si128((__m128i*) & src[i + 5 * 16]);
		__m128i d6 = _mm_load_si128((__m128i*) & src[i + 6 * 16]);
		__m128i d7 = _mm_load_si128((__m128i*) & src[i + 7 * 16]);
		_mm_stream_si128((__m128i*) & dst[i + 0 * 16], d0);
		_mm_stream_si128((__m128i*) & dst[i + 1 * 16], d1);
		_mm_stream_si128((__m128i*) & dst[i + 2 * 16], d2);
		_mm_stream_si128((__m128i*) & dst[i + 3 * 16], d3);
		_mm_stream_si128((__m128i*) & dst[i + 4 * 16], d4);
		_mm_stream_si128((__m128i*) & dst[i + 5 * 16], d5);
		_mm_stream_si128((__m128i*) & dst[i + 6 * 16], d6);
		_mm_stream_si128((__m128i*) & dst[i + 7 * 16], d7);
	}
	for (; i + 16 <= _len; i += 16)
	{
		__m128i d = _mm_load_si128((__m128i*) & src[i]);
		_mm_stream_si128((__m128i*) & dst[i], d);
	}
	for (; i + 4 <= _len; i += 4)
	{
		*(uint32*)&dst[i] = *(const uint32*)&src[i];
	}
	for (; i < _len; ++i)
	{
		dst[i] = src[i];
	}
	_mm_sfence();
}
#endif


VESPERENGINE_INLINE void MemCpy(void* _dst, const void* _src, std::size_t _len)
{
#ifdef VESPERENGINE_MEM_COPY_SIMD
	if (VESPERENGINE_IS_ALIGNED(_dst, 64) && VESPERENGINE_IS_ALIGNED(_src, 64))
	{
		MemCpySIMD64(_dst, _src, _len);
	}
	else if (VESPERENGINE_IS_ALIGNED(_dst, 16) && VESPERENGINE_IS_ALIGNED(_src, 16))
	{
		MemCpySIMD(_dst, _src, _len);
	}
	else
	{
		memcpy(_dst, _src, _len);
	}
#else
	memcpy(_dst, _src, _len);
#endif
}

VESPERENGINE_NAMESPACE_END