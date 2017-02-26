/**
 *	\file
 */

#pragma once

#include <opencv2/core/mat.hpp>

namespace colby {

/**
 *	An abstract base class which may be derived to
 *	provide a strategy for converting an input image
 *	into color by numbers.
 */
class color_by_numbers {
public:
	color_by_numbers () = default;
	color_by_numbers (const color_by_numbers &) = delete;
	color_by_numbers (color_by_numbers &&) = delete;
	color_by_numbers & operator = (const color_by_numbers &) = delete;
	color_by_numbers & operator = (color_by_numbers &&) = delete;
	/**
	 *	Allows derived classes to be cleaned up
	 *	through pointer or reference to base.
	 */
	virtual ~color_by_numbers () noexcept;
	/**
	 *	Encapsulates the result of converting an
	 *	image to a color by numbers representation.
	 */
	class result {
	private:
		cv::Mat img_;
	public:
		result () = delete;
		result (const result &) = default;
		result (result &&) = default;
		result & operator = (const result &) = default;
		result & operator = (result &&) = default;
		/**
		 *	Creates a new result object.
		 *
		 *	\param [in] img
		 *		The cv::Mat containing the resulting
		 *		image.
		 */
		explicit result (cv::Mat img);
		/**
		 *	Returns the resulting image.
		 *
		 *	\return
		 *		The resulting image.
		 */
		const cv::Mat & image () const & noexcept;
		cv::Mat & image () & noexcept;
		cv::Mat && image () && noexcept;
	};
	/**
	 *	Converts a source image to a color by numbers
	 *	representation.
	 *
	 *	\param [in] src
	 *		The source image.
	 *
	 *	\return
	 *		A \ref result object.
	 */
	virtual result convert (const cv::Mat & src) = 0;
};

}
