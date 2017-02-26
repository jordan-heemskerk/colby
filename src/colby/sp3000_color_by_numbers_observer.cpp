#include <colby/sp3000_color_by_numbers_observer.hpp>

namespace colby {

sp3000_color_by_numbers_observer::~sp3000_color_by_numbers_observer () noexcept {	}

sp3000_color_by_numbers_observer::base_event::base_event (image_factory & factory) noexcept
	:	factory_(factory)
{	}

cv::Mat sp3000_color_by_numbers_observer::base_event::image () const {
	return factory_.image();
}

}
