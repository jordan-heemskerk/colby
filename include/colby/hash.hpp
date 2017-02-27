/**
 *	\file
 */

#pragma once

#include <boost/functional/hash.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <cstddef>
#include <functional>

namespace std {

template <>
class hash<cv::Point> {
public:
	using argument_type = cv::Point;
	using result_type = std::size_t;
	result_type operator () (const argument_type & point) const noexcept {
		std::size_t retr = 0;
		boost::hash_combine(retr, point.x);
		boost::hash_combine(retr, point.y);
		return retr;
	}
};

template <typename T, int N>
class hash<cv::Vec<T,N>> {
public:
	using argument_type = cv::Vec<T,N>;
	using result_type = std::size_t;
	result_type operator () (const argument_type & vec) const noexcept {
		std::size_t retr = 0;
		for (int i = 0; i < N; ++i) boost::hash_combine(retr, vec[i]);
		return retr;
	}
};


}
