#include "util.h"

namespace Chess {

	// PRNG
	u32 PRNG::state_u32_ = 1;
	u64 PRNG::state_u64_ = 1;

	void PRNG::seed_32(u32 seed) { state_u32_ = seed; }
	void PRNG::seed_64(u64 seed) { state_u64_ = seed; }

	u32 PRNG::get_32() {
		u32 x = state_u32_;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		state_u32_ = x;
		return x;
	}

	u64 PRNG::get_64() {
		u64 x = state_u64_;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		state_u64_ = x;
		return x;
	}

	// Timer
	Timer::Timer() {
		start_ = std::chrono::high_resolution_clock::now();
	}

	Timer::Microseconds Timer::get_elapsed_microseconds() const {
		auto end = std::chrono::high_resolution_clock::now();
		return std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
	}

	void Timer::reset() {
		start_ = std::chrono::high_resolution_clock::now();
	}

}