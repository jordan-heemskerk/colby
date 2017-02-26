#include <colby/color_by_numbers.hpp>
#include <utility>

namespace colby {

color_by_numbers::~color_by_numbers () noexcept {	}

color_by_numbers::result::result (cv::Mat img)
	:	img_(std::move(img))
{	}

const cv::Mat & color_by_numbers::result::image () const & noexcept {
	return img_;
}

cv::Mat & color_by_numbers::result::image () & noexcept {
	return img_;
}

cv::Mat && color_by_numbers::result::image () && noexcept {
	return std::move(img_);
}

}
