#include <colby/algorithm.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace colby {

sp3000_color_by_numbers::graph::vertex::vertex (graph & owner) : owner_(owner) {	}

const sp3000_color_by_numbers::cell & sp3000_color_by_numbers::graph::vertex::cell () const noexcept {
	return cell_;
}

void sp3000_color_by_numbers::graph::vertex::add (cv::Point p) {
	auto pair = owner_.lookup_.emplace(p,this);
	if (!pair.second) {
		if (pair.first->second == this) return;
		std::ostringstream ss;
		ss << '(' << p.x << ", " << p.y << ") already owned by another vertex";
		throw std::logic_error(ss.str());
	}
	try {
		cell.insert(p);
	} catch (...) {
		owner_.lookup_.erase(pair.first);
	}
}

void sp3000_color_by_numbers::graph::vertex::add (vertex & v) {
	if (&v == this) throw std::logic_error("Loop not allowed");
	auto pair = adj_list_.insert(&v);
	if (!pair.second) return;
	try {
		v.adj_list_.insert(this);
	} catch (...) {
		adj_list_.erase(pair.first);
	}
}

void sp3000_color_by_numbers::graph::vertex::merge (const vertex & v) {
	//	Note: If anything in this method throws, die
	//	as the state is corrupted and it's not worth
	//	the effort to give a strong exception guarantee
	//
	//	TODO: Perhaps in the future clean this up
	for (auto && p : v.cell_) cell_.insert(p);
	for (auto && n : v.adj_list_) {
		n.adj_list_.erase(&v);
		n.adj_list_.insert(this);
	}
	adj_list_.erase(&v);
	
}

sp3000_color_by_numbers::vertex & sp3000_color_by_numbers::graph::add () {
	vertices_.emplace_back();
	return vertices_.back();
}

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

sp3000_color_by_numbers::graph sp3000_color_by_numbers::divide (const cv::Mat & img) const {
	auto unvisited = image_as_cell(img);
	graph retr;
	std::vector<cv::Point> stack;
	float tolerance = flood_fill_tolerance_ * flood_fill_tolerance_;
	while (!unvisited.empty()) {
		auto && point = *unvisited.begin();
		auto && color = img.at<cv::Vec3f>(point);
		auto && vertex = retr.add();
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
		vertex.cell() = std::move(set);
	}
	return retr;
}

sp3000_color_by_numbers::result sp3000_color_by_numbers::convert (const cv::Mat & src) {
	if (src.type() != CV_32FC3) throw std::logic_error(
		"Expected cv::Mat::type to return CV_32FC3"
	);
	//	1. Convert the pixels to the CIELAB colour space
	auto lab = convert_to_lab(src);
	//	2. Divide the image into like-colored cells using flood fill
	auto div = divide(lab);
	throw 1;
}

}
