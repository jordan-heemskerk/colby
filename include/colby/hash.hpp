/**
 *	\file
 */

#pragma once

#include <boost/functional/hash.hpp>
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

}
