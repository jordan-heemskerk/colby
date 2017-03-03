#include <colby/color_by_numbers.hpp>
#include <utility>

namespace colby {

color_by_numbers::~color_by_numbers () noexcept {	}

color_by_numbers::result::result (cv::Mat img, palette_type p)
	:	img_(std::move(img)),
		p_(std::move(p))
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

const color_by_numbers::result::palette_type & color_by_numbers::result::palette () const & noexcept {
	return p_;
}

color_by_numbers::result::palette_type & color_by_numbers::result::palette () & noexcept {
	return p_;
}

color_by_numbers::result::palette_type && color_by_numbers::result::palette () && noexcept {
	return std::move(p_);
}

}
