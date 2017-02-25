#include <colby/algorithm.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <utility>
#include <vector>

namespace colby {

cv::Mat sp3000_color_by_numbers::convert_to_lab (const cv::Mat & img) const {
	cv::Mat retr;
	cv::cvtColor(img, retr, CV_BGR2Lab);
	return retr;
}

sp3000_color_by_numbers::cell sp3000_color_by_numbers::image_as_cell (const cv::Mat & img) {
	cell retr;
	for (int x = 0; x < img.cols; ++x) for (int y = 0; y < img.rows; ++y) {
		retr.emplace(x,y);
	}
	return retr;
}

void sp3000_color_by_numbers::subtract (cell & lhs, const cell & rhs) {
	for (auto && obj : rhs) lhs.erase(obj);
}

sp3000_color_by_numbers::divided sp3000_color_by_numbers::divide (const cv::Mat & img) const {
	auto unvisited = image_as_cell(img);
	divided retr;
	std::vector<cv::Point> stack;
	float tolerance = flood_fill_tolerance_ * flood_fill_tolerance_;
	while (!unvisited.empty()) {
		auto && point = *unvisited.begin();
		auto && color = img.at<cv::Vec3f>(point);
		auto set = flood_fill(
			img,
			point,
			[&] (auto && curr) noexcept {
				auto && curr_color = img.at<cv::Vec3f>(curr);
				auto diff = curr_color - color;
				auto squared_norm = (diff[0] * diff[0]) + (diff[1] * diff[1]) + (diff[2] * diff[2]);
				return squared_norm < tolerance;
			},
			cell{},
			stack
		);
		subtract(unvisited,set);
		retr.push_back(std::move(set));
	}
	return retr;
}

sp3000_color_by_numbers::result sp3000_color_by_numbers::convert (const cv::Mat & src) {
	if (src.type() != CV_32FC3) throw std::logic_error(
		"Expected cv::Mat::type to return CV_32FC3"
	);
	//	1. Convert the pixels to the CIELAB colour space
	auto lab = convert_to_lab(src);
	//	2. Divide the image into like-coloured cells using flood fill
	auto div = divide(lab);
	throw 1;
}

}
