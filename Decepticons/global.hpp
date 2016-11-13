#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/*
 *	Arch-portable typing, endianness, enumerations, etc.
 */

#include <iostream>
#include <netinet/in.h>

//typedef unsigned int		uint32_t;
//typedef unsigned long		uint64_t;
//typedef unsigned short	uint16_t;
//typedef unsigned char		uint8_t;
//typedef signed int		int32_t;

// ints / bytes
typedef uint64_t		uint64;
typedef uint32_t		uint32;
typedef uint16_t		uint16;
typedef uint8_t			uint8;
typedef	unsigned char	byte;

// endianness, arch-specific behavior
uint64 hton64(uint64 input);

template <typename T>
inline T fix_endian(T input)
{
#ifdef LITTLE_ENDIAN	// #TODO:  test on a big endian system!
	if (sizeof(T) == 2)
		return htons(input);
	if (sizeof(T) == 4)
		return htonl(input);
	if (sizeof(T) == 8)
		return hton64(input);
#endif // LITTLE_ENDIAN
    return input;
}

template <typename T>
inline void fix_endian(T *input)
{
#ifdef LITTLE_ENDIAN	// #TODO:  test on a big endian system!
	if (sizeof(T) == 2)
		*input = htons(*input);
	if (sizeof(T) == 4)
		*input = htonl(*input);
	if (sizeof(T) == 8)
		*input = hton64(*input);
#endif // LITTLE_ENDIAN
}

/*******
 *******	Helper functions
 *******/

/***
 ***	Bitmasks -- defined as [0th, 1st, 2nd, ..., 31st]
 ***	BITMASK<T,start,end>
 ***	Gives you a T with the following bits set:
 ***		[start,end)
 ***			start inclusive, end exclusive
 ***			zero indexed
 ***	BITMASK<char,0,4> : 1111 0000
 ***	BITMASK<int16,8,16> : 0000 0000 1111 1111
 ***
 ***	similar to:	for (i=start;i<end;i++) setbit(i);
 ***
 ***	this is all resolved at compile time, so feel free
 ***	to macro this up; it won't bloat the executable.
 ***/

#define BITMASK(T,x,y) (bitmask<T,(x),(y)-(x)>::value)
// Generates bitmasks recursively.
template <typename T, uint32 low, uint32 span>
struct bitmask
{
	enum {
		typesize = (sizeof(T) * 8),
		bit_to_set = low + (span-1),
		mask = static_cast<T>(static_cast<uint64>(1) << ((typesize-1)-bit_to_set)) | static_cast<T>(static_cast<uint64>(bitmask<T, low, span - 1>::mask))
	};
	const static T value = static_cast<T>(mask);
};

// Base case; the above is defined recursively.
template <typename T, uint32 low>
struct bitmask<T, low, 0>
{
	enum {
		mask = 0
	};
	const static T value = static_cast<T>(mask);
};

// These two functions could use a little refactoring / cleanup #TODO

template <typename T, int start_inclusive, int end_exclusive>
T data_from_bitfield(byte* data)
{
	const int byte_increment = start_inclusive / 8;
	const byte* byte_index = data + byte_increment;
	const T* index_of_bits = reinterpret_cast<const T*>(byte_index);
	T bits = *index_of_bits;
    fix_endian(&bits);
	
	// Mask the bits and rightshift them to obtain correct value.
	const int width = sizeof(T) * 8;
	const int start_inclusive_with_offset	= (start_inclusive	- byte_increment*8);
	const int end_exclusive_with_offset		= (end_exclusive	- byte_increment*8);
	const int rightshift = width - end_exclusive_with_offset;
	const T value = bits & BITMASK(T,start_inclusive_with_offset,end_exclusive_with_offset);
	return value >> rightshift;
}

template <typename T, int start_position, int width>
void data_to_bitfield(byte* data, T input)
{    
    fix_endian(&input);
    
    const int bitpos = start_position % 8;
    const int byteshift = start_position / 8;
    
    T* target_bits = reinterpret_cast<T*>(data + byteshift);
    // Zero out the bits, then OR in our data:
    T mask = static_cast<T>(    ~BITMASK(T,bitpos,bitpos+width) );
    *target_bits &= fix_endian(mask);
    const int shiftleft = (sizeof(T)*8 - width);    // this left-aligns the bitfield
    const int shiftright = bitpos;  // this puts the bitfield at its start position
    const T input_bits = (input << shiftleft) >> shiftright;
    *target_bits |= input_bits;
    
    return;
}


#endif // global.h

