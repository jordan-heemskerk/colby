#include <colby/algorithm.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <functional>
#include <memory>
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
		cell_.insert(p);
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
	auto ptr = const_cast<vertex *>(&v);
	for (auto && n : v.adj_list_) {
		n->adj_list_.erase(ptr);
		n->adj_list_.insert(this);
	}
	adj_list_.erase(ptr);
	owner_.vertices_.erase(v);
}

sp3000_color_by_numbers::graph::vertex::neighbors_type sp3000_color_by_numbers::graph::vertex::neighbors () const noexcept {
	return neighbors_type(adj_list_.begin(),adj_list_.end());
}

std::size_t sp3000_color_by_numbers::graph::hasher::operator () (const vertex & v) const noexcept {
	std::hash<const vertex *> impl;
	return impl(&v);
}

bool sp3000_color_by_numbers::graph::equals::operator () (const vertex &, const vertex &) const noexcept {
	return false;
}

sp3000_color_by_numbers::graph::vertex & sp3000_color_by_numbers::graph::add () {
	auto pair = vertices_.emplace(*this);
	return const_cast<vertex &>(*pair.first);
}

sp3000_color_by_numbers::graph::vertex * sp3000_color_by_numbers::graph::find (cv::Point p) {
	auto iter = lookup_.find(p);
	if (iter == lookup_.end()) return nullptr;
	return iter->second;
}

cv::Mat sp3000_color_by_numbers::convert_to_lab (const cv::Mat & img) const {
	cv::Mat retr;
	cv::cvtColor(img,retr,CV_BGR2Lab);
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

std::unique_ptr<sp3000_color_by_numbers::graph> sp3000_color_by_numbers::divide (const cv::Mat & img) const {
	auto unvisited = image_as_cell(img);
	auto retr = std::make_unique<graph>();
	std::vector<cv::Point> stack;
	cell set;
	float tolerance = flood_fill_tolerance_ * flood_fill_tolerance_;
	while (!unvisited.empty()) {
		auto && point = *unvisited.begin();
		auto && color = img.at<cv::Vec3f>(point);
		auto && vertex = retr->add();
		set = flood_fill(
			img,
			point,
			[&] (auto && curr) noexcept {
				auto n = retr->find(curr);
				if (n) {
					vertex.add(*n);
					return false;
				}
				auto && curr_color = img.at<cv::Vec3f>(curr);
				auto diff = curr_color - color;
				auto squared_norm = (diff[0] * diff[0]) + (diff[1] * diff[1]) + (diff[2] * diff[2]);
				if (squared_norm < tolerance) {
					vertex.add(curr);
					return true;
				}
				return false;
			},
			std::move(set),
			stack
		);
		subtract(unvisited,set);
	}
	return retr;
}

sp3000_color_by_numbers::sp3000_color_by_numbers (float flood_fill_tolerance) : flood_fill_tolerance_(flood_fill_tolerance) {	}

sp3000_color_by_numbers::result sp3000_color_by_numbers::convert (const cv::Mat & src) {
	if (src.type() != CV_32FC3) throw std::logic_error(
		"Expected cv::Mat::type to return CV_32FC3"
	);
	//	1. Convert the pixels to the CIELAB colour space
	auto lab = convert_to_lab(src);
	//	2. Divide the image into like-colored cells using flood fill
	auto graph = divide(lab);
	

	throw 1;
}

}
