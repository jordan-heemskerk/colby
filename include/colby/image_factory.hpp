/**
 *	\file
 */

#pragma once

#include <opencv2/core/mat.hpp>

namespace colby {

/**
 *	Allows a cv::Mat to be produced lazily.
 */
class image_factory {
public:
	image_factory () = default;
	image_factory (const image_factory &) = delete;
	image_factory (image_factory &&) = delete;
	image_factory & operator = (const image_factory &) = delete;
	image_factory & operator = (image_factory &&) = delete;
	/**
	 *	Allows derived classes to be cleaned up
	 *	through pointer or reference to base.
	 */
	virtual ~image_factory () noexcept;
	/**
	 *	Lazily produces a cv::Mat representing
	 *	an image.
	 *
	 *	\return
	 *		The generated cv::Mat.
	 */
	virtual cv::Mat image () = 0;
};

}
