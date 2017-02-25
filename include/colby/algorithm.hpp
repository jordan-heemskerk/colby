/**
 *	\file
 */

#pragma once

#include "hash.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <unordered_set>
#include <utility>
#include <vector>

namespace colby {

/**
 *	Determines whether a cv::Point represents
 *	a valid position in a cv::Mat.
 *
 *	\param [in] mat
 *		A cv::Mat.
 *	\param [in] loc
 *		A cv::Point.
 *
 *	\return
 *		\em true if \em loc represents a valid
 *		position in \em mat, \em false otherwise.
 */
inline bool valid (const cv::Mat & mat, cv::Point loc) noexcept {
	if (loc.x < 0) return false;
	if (loc.y < 0) return false;
	if (loc.x >= mat.cols) return false;
	if (loc.y >= mat.rows) return false;
	return true;
}

/**
 *	Generates a callback for each valid point in
 *	the four-neighborhood of a given point within
 *	a given image.
 *
 *	\tparam Callback
 *		The type of functor which shall be invoked.
 *
 *	\param [in] mat
 *		A cv::Mat.
 *	\param [in] start
 *		A cv::Point from which the search for
 *		neighbors shall begin.
 *	\param [in] callback
 *		An instance of type \em Callback which shall
 *		be invoked and passed each valid neighbor as
 *		a cv::Point as its sole argument.
 */
template <typename Callback>
void neighbors (const cv::Mat & mat, cv::Point start, Callback callback) {
	cv::Point left(start.x - 1,start.y);
	if (valid(mat,left)) callback(left);
	cv::Point right(start.x + 1, start.y);
	if (valid(mat,right)) callback(right);
	cv::Point up(start.x,start.y + 1);
	if (valid(mat,up)) callback(up);
	cv::Point down(start.x,start.y - 1);
	if (valid(mat,down)) callback(down);
}

/**
 *	Performs a flood fill using the four-neighborhood
 *	of visited points.
 *
 *	\tparam Callback
 *		The type of callback which shall be invoked to
 *		determine whether points are included or excluded.
 *
 *	\param [in] mat
 *		The image in which to search.
 *	\param [in] start
 *		The point in \em mat at which the search shall
 *		begin.
 *	\param [in] callback
 *		The callback which shall be invoked to test prospective
 *		points.  The sole parameter shall be a cv::Point
 *		representing the point under consideration.  A boolean
 *		value shall be returned: \em true indicates that the
 *		considered point shall be included, \em false indicates
 *		that it shall be excluded.
 *	\param [in] retr
 *		An std::unordered_set which will be used to build
 *		the return value.  It will be cleared before the
 *		return value is built.  This allows for preallocated
 *		memory to be provided.
 *	\param [in,out] stack
 *		A std::vector which will be used to hold cv::Points
 *		pending recursion.  It will be cleared before it is
 *		used.  This allows for preallocated memory to be
 *		provided.
 *
 *	\return
 *		The set of points which matched.
 */
template <typename Callback>
std::unordered_set<cv::Point> flood_fill (const cv::Mat & mat, cv::Point start, Callback callback, std::unordered_set<cv::Point> retr, std::vector<cv::Point> & stack = std::vector<cv::Point>{}) {
	retr.clear();
	if (!callback(start)) return retr;
	stack.clear();
	stack.push_back(start);
	retr.insert(std::move(start));
	do {
		auto p = std::move(stack.back());
		stack.pop_back();
		neighbors(mat,p,[&] (auto && p) {
			if (!callback(p)) return;
			if (retr.count(p) != 0) return;
			stack.push_back(p);
			retr.insert(p);
		});
	} while (!stack.empty());
	return retr;
}
template <typename Callback>
std::unordered_set<cv::Point> flood_fill (const cv::Mat & mat, cv::Point start, Callback callback, std::unordered_set<cv::Point> retr = std::unordered_set<cv::Point>{}) {
	std::vector<cv::Point> stack;
	return flood_fill(mat,std::move(start),std::move(callback),std::move(retr),stack);
}

}
