/**
 *	\file
 */

#pragma once

#include "color_by_numbers.hpp"
#include "hash.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <unordered_set>

namespace colby {

/**
 *	Implements an extended version of the algorithm
 *	laid out by Stack Overflow user Sp3000 <a href="http://codegolf.stackexchange.com/a/42235">in
 *	this answer</a>.
 *
 *	The extension is that rather than converting the
 *	image to use a smaller number of colors it instead
 *	converts it to a form suitable to color by numbers
 *	(i.e. the cells are not filled with the corresponding
 *	color).
 */
class sp3000_color_by_numbers : public color_by_numbers {
private:
	float flood_fill_tolerance_;
	using cell = std::unordered_set<cv::Point>;
	using divided = std::vector<cell>;
	cv::Mat convert_to_lab (const cv::Mat &) const;
	static cell image_as_cell (const cv::Mat &);
	static void subtract (cell &, const cell &);
	divided divide (const cv::Mat &) const;
public:
	virtual result convert (const cv::Mat & src) override;
};

}
