#pragma once

#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <chrono>

namespace Chess {
	typedef int8_t i8;
	typedef uint8_t u8;
	typedef int16_t i16;
	typedef uint16_t u16;
	typedef int32_t i32;
	typedef uint32_t u32;
	typedef int64_t i64;
	typedef uint64_t u64;

	class PRNG {
	public:
		static void seed_32(u32 seed);
		static void seed_64(u64 seed);

		static u32 get_32();
		static u64 get_64();

	private:
		static u32 state_u32_;
		static u64 state_u64_;
	};

	class Timer {
	public:
		using Microseconds = u64;
		Timer();
		Microseconds get_elapsed_microseconds() const;
		void reset();

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> start_;
	};
}


#endif // UTIL_H