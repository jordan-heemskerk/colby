#include <colby/algorithm.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace colby {

sp3000_color_by_numbers::graph::vertex::vertex (graph & owner) : color_(0,0,0), owner_(owner) {	}

void sp3000_color_by_numbers::graph::vertex::add (cv::Point p, cv::Vec3f c) {
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
		throw;
	}
	color_ *= float(cell_.size() - 1U);
	color_ += c;
	color_ /= float(cell_.size());
}

void sp3000_color_by_numbers::graph::vertex::add (vertex & v) {
	if (&v == this) throw std::logic_error("Loop not allowed");
	auto pair = adj_list_.insert(&v);
	if (!pair.second) return;
	try {
		v.adj_list_.insert(this);
	} catch (...) {
		adj_list_.erase(pair.first);
		throw;
	}
}

void sp3000_color_by_numbers::graph::vertex::merge (const vertex & v, bool avg) {
	//	Note: If anything in this method throws, die
	//	as the state is corrupted and it's not worth
	//	the effort to give a strong exception guarantee
	//
	//	TODO: Perhaps in the future clean this up
	if (avg) {
		color_ *= float(cell_.size());
		color_ += v.color_ * float(v.cell_.size());
		color_ /= float(cell_.size() + v.cell_.size());
	}
	for (auto && p : v.cell_) {
		cell_.insert(p);
		owner_.lookup_[p] = this;
	}
	auto ptr = const_cast<vertex *>(&v);
	for (auto && n : v.adj_list_) {
		n->adj_list_.erase(ptr);
		n->adj_list_.insert(this);
	}
	owner_.vertices_.erase(v);
}

sp3000_color_by_numbers::graph::vertex::neighbors_type sp3000_color_by_numbers::graph::vertex::neighbors () const noexcept {
	return neighbors_type(adj_list_.begin(),adj_list_.end());
}

std::size_t sp3000_color_by_numbers::graph::vertex::size () const noexcept {
	return cell_.size();
}

cv::Vec3f sp3000_color_by_numbers::graph::vertex::color () const noexcept {
	return color_;
}

const sp3000_color_by_numbers::cell & sp3000_color_by_numbers::graph::vertex::points () const noexcept {
	return cell_;
}

void sp3000_color_by_numbers::graph::vertex::print () const {
	std::cout << "\nPoints: " << cell_.size() << ", Neighbors: " << adj_list_.size() << "\n";
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

sp3000_color_by_numbers::graph::vertices_type sp3000_color_by_numbers::graph::vertices () noexcept {
	return vertices_type(vertices_.begin(),vertices_.end());
}

void sp3000_color_by_numbers::graph::print () const {
	std::cout << "Vertices: " << vertices_.size();
	for (auto && v : vertices_) v.print();
}

cv::Mat sp3000_color_by_numbers::convert_bgr_to_lab (const cv::Mat & img) const {
	cv::Mat retr;
	cv::cvtColor(img,retr,CV_BGR2RGB);
	if (!retr.data) throw std::runtime_error("cv::cvtColor failed to convert image to Lab");
	return retr;
}

cv::Mat sp3000_color_by_numbers::convert_lab_to_bgr (const cv::Mat & img) const {
	cv::Mat retr;
	cv::cvtColor(img,retr,CV_RGB2BGR);
	if (!retr.data) throw std::runtime_error("cv::cvtColor failed to convert image to BGR");
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
			[&] (auto && curr) {
				auto n = retr->find(curr);
				if (n) {
					vertex.add(*n);
					return false;
				}
				auto && curr_color = img.at<cv::Vec3f>(curr);
				auto diff = curr_color - color;
				auto squared_norm = (diff[0] * diff[0]) + (diff[1] * diff[1]) + (diff[2] * diff[2]);
				if (squared_norm < tolerance) {
					vertex.add(curr,curr_color);
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

void sp3000_color_by_numbers::merge_small_cells (graph & g) const {
	//	The criterion for choosing which neighbor to merge
	//	a small cell into is implemented by Sp3000 as:
	//
	//	closest_cell = max(neighbour_cells, key=neighbour_cells.count)
	//
	//	We will implement this check in the same way, however we
	//	observe that it might be better to choose the cell with
	//	which the small cell has the longest border
	//
	//	However it is possible that these cells are small enough
	//	that "longest border" isn't particularly meaningful...
	auto vertices = g.vertices();
	auto begin = vertices.begin();
	auto end = vertices.end();
	while (begin != end) {
		auto && curr = *(begin++);
		if (curr.size() > small_cell_threshold_) continue;
		auto ns = curr.neighbors();
		auto iter = std::max_element(ns.begin(),ns.end(),[] (auto && a, auto && b) noexcept {
			return a.size() < b.size();
		});
		if (iter == ns.end()) throw std::logic_error("Small cell with no neighbors");
		std::cout << iter->size() << std::endl;
		//	We don't allow this cell to contribute to average of
		//	larger cell:
		//
		//	"You don't want to pollute your large region with that
		//	tiny little bit."
		//
		//	—Jordan Heemskerk, 2017
		iter->merge(curr,false);
	}
}

sp3000_color_by_numbers::result sp3000_color_by_numbers::convert_impl (const cv::Mat & src) {
	//	1. Convert the pixels to the CIELAB colour space
	auto lab = convert_bgr_to_lab(src);
	//	2. Divide the image into like-colored cells using flood fill
	auto g = divide(lab);
	//g->print();
	//	3. Merge together small cells with their neighbours
	merge_small_cells(*g);


	cv::Mat mat(src.rows,src.cols,CV_32FC3);
	for (auto && v : g->vertices()) {
		for (auto && p : v.points()) {
			mat.at<cv::Vec3f>(p) = v.color();
		}
	}
	return result(convert_lab_to_bgr(mat));
}

sp3000_color_by_numbers::sp3000_color_by_numbers (
	float flood_fill_tolerance,
	std::size_t small_cell_threshold
)	:	flood_fill_tolerance_(flood_fill_tolerance),
		small_cell_threshold_(small_cell_threshold)
{	}

sp3000_color_by_numbers::result sp3000_color_by_numbers::convert (const cv::Mat & src) {
	//	TODO: More comprehensive conversion/handling
	if (src.type() != CV_8UC3) throw std::logic_error("Expected 3 channel 8 bit image");
	cv::Mat mat;
	src.convertTo(mat,CV_32FC3,1.0f / 255.0f);
	if (!mat.data) throw std::runtime_error("cv::Mat::convertTo failed to convert to 3 channel 32 bit float image");
	auto retr = convert_impl(mat);
	retr.image().convertTo(mat,CV_8UC3,255);
	if (!mat.data) throw std::runtime_error("cv::Mat::convertTo failed to convert to 3 channel 8 bit image");
	retr.image() = std::move(mat);
	return retr;
}

}
