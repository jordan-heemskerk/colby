/**
 *	\file
 */

#pragma once

#include <chrono>

namespace colby {

/**
 *	Implements a timer over a C++ clock.
 *
 *	\tparam Clock
 *		The clock to use.
 */
template <typename Clock>
class basic_timer {
public:
	using clock = Clock;
	using rep = typename clock::rep;
	using period = typename clock::period;
	using duration = typename clock::duration;
	using time_point = typename clock::time_point;
private:
	time_point begin_;
public:
	/**
	 *	Creates and starts the timer.
	 */
	basic_timer () : begin_(clock::now()) {	}
	basic_timer (const basic_timer &) = default;
	basic_timer (basic_timer &&) = default;
	basic_timer & operator = (const basic_timer &) = default;
	basic_timer & operator = (basic_timer &&) = default;
	/**
	 *	Restarts the timer.
	 */
	void restart () {
		begin_ = clock::now();
	}
	/**
	 *	Determines how long the timer has been
	 *	running.
	 *
	 *	\return
	 *		A duration.
	 */
	duration elapsed () const {
		return clock::now() - begin_;
	}
	/**
	 *	Determines the point in time at which the
	 *	timer was last started or restarted.
	 *
	 *	\return
	 *		A point in time.
	 */
	time_point started () const {
		return begin_;
	}
};

/**
 *	An instantiation of \ref basic_time which
 *	uses std::chrono::high_resolution_clock.
 */
using timer = basic_timer<std::chrono::high_resolution_clock>;

}
