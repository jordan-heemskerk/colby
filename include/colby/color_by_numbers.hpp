/**
 *	\file
 */

#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <vector>

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
	public:
		/**
		 *	Maps numbers to colors.
		 *
		 *	The zero relative Nth color maps to all
		 *	cells with the one relative Mth number.
		 */
		using palette_type = std::vector<cv::Vec3f>;
	private:
		cv::Mat img_;
		palette_type p_;
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
		 *	\param [in] p
		 *		The \ref palette_type containing the
		 *		resulting color palette.
		 */
		result (cv::Mat img, palette_type p);
		/**
		 *	Returns the resulting image.
		 *
		 *	\return
		 *		The resulting image.
		 */
		const cv::Mat & image () const & noexcept;
		cv::Mat & image () & noexcept;
		cv::Mat && image () && noexcept;
		/**
		 *	Returns the resulting palette.
		 *
		 *	\return
		 *		The resulting palette.
		 */
		const palette_type & palette () const & noexcept;
		palette_type & palette () & noexcept;
		palette_type && palette () && noexcept;
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
