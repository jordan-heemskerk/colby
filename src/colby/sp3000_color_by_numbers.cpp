#include <boost/iterator/filter_iterator.hpp>
#include <colby/algorithm.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
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
	if (!pair.second) {
		assert(v.adj_list_.count(this));
		return;
	}
	try {
		auto pair2 = v.adj_list_.insert(this);
		assert(pair2.second);
	} catch (...) {
		adj_list_.erase(pair.first);
		throw;
	}
}

void sp3000_color_by_numbers::graph::vertex::merge (vertex & v, bool avg) {
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
	auto erased = v.adj_list_.erase(this);
	assert(erased);
	erased = adj_list_.erase(&v);
	assert(erased);
	while (!v.adj_list_.empty()) {
		auto iter = v.adj_list_.begin();
		auto n = *iter;
		v.adj_list_.erase(iter);
		erased = n->adj_list_.erase(&v);
		assert(erased);
		auto back_reference_created = n->adj_list_.insert(this).second;
		auto forward_reference_created = adj_list_.insert(n).second;
		assert(back_reference_created == forward_reference_created);
	}
	erased = owner_.vertices_.erase(&v);
	assert(erased);
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

sp3000_color_by_numbers::graph::vertex & sp3000_color_by_numbers::graph::add () {
	vertices_storage_.emplace_back(*this);
	auto & retr = vertices_storage_.back();
	try {
		vertices_.insert(&retr);
	} catch (...) {
		vertices_storage_.pop_back();
		throw;
	}
	return retr;
}

sp3000_color_by_numbers::graph::vertex * sp3000_color_by_numbers::graph::find (cv::Point p) {
	auto iter = lookup_.find(p);
	if (iter == lookup_.end()) return nullptr;
	return iter->second;
}

sp3000_color_by_numbers::graph::vertices_type sp3000_color_by_numbers::graph::vertices () noexcept {
	return vertices_type(vertices_.begin(),vertices_.end());
}

std::size_t sp3000_color_by_numbers::graph::size () const noexcept {
	return vertices_.size();
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

static float squared_distance (const cv::Vec3f & a, const cv::Vec3f & b) noexcept {
	auto diff = a - b;
	return (diff[0] * diff[0]) + (diff[1] * diff[1]) + (diff[2] * diff[2]);
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
				if (squared_distance(curr_color,color) < tolerance) {
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

void sp3000_color_by_numbers::merge_small_cells_impl (graph & g, std::size_t size) const {
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
	auto filter = [&] (auto && vertex) noexcept {	return vertex.size() == size;	};
	auto vertices = g.vertices();
	auto begin = boost::make_filter_iterator(filter,vertices.begin(),vertices.end());
	auto end = boost::make_filter_iterator(filter,vertices.end(),vertices.end());
	while (begin != end) {
		auto && curr = *(begin++);
		auto ns = curr.neighbors();
		auto iter = std::max_element(ns.begin(),ns.end(),[] (auto && a, auto && b) noexcept {
			return a.size() < b.size();
		});
		if (iter == ns.end()) throw std::logic_error("Small cell with no neighbors");
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

void sp3000_color_by_numbers::merge_small_cells (graph & g) const {
	std::size_t i = 0;
	while ((i++) < small_cell_threshold_) merge_small_cells_impl(g,i);
}

void sp3000_color_by_numbers::merge_similar_cells (graph & g) const {
	auto compare = [] (auto a, auto b) noexcept {
		if (a->size() != b->size()) return a->size() < b->size();
		std::less<> cmp;
		return cmp(a,b);
	};
	std::vector<graph::vertex *> sorted;
	std::vector<graph::vertex *> to_merge;
	sorted.reserve(g.size());
	bool changed;
	do {
		std::cout << g.size() << std::endl;
		auto copy = [] (auto && ref) noexcept {	return &ref;	};
		auto vertices = g.vertices();
		std::transform(vertices.begin(),vertices.end(),std::back_inserter(sorted),copy);
		std::sort(sorted.begin(),sorted.end(),compare);
		changed = false;
		while (!sorted.empty()) {
			std::cout << sorted.size() << std::endl;
			auto && curr = *sorted.back();
			sorted.pop_back();
			auto pred = [&] (auto && v) noexcept {
				return squared_distance(v.color(),curr.color()) < similar_cell_tolerance_;
			};
			auto neighbors = curr.neighbors();
			auto begin = boost::make_filter_iterator(pred,neighbors.begin(),neighbors.end());
			auto end = boost::make_filter_iterator(pred,neighbors.end(),neighbors.end());
			to_merge.clear();
			std::transform(begin,end,std::back_inserter(to_merge),copy);
			for (auto && ptr : to_merge) {
				auto iter = std::lower_bound(sorted.begin(),sorted.end(),ptr,compare);
				if (iter != sorted.end()) sorted.erase(iter);
				curr.merge(*ptr);
				changed = true;
			}
		}
	} while (changed);
}

sp3000_color_by_numbers::result sp3000_color_by_numbers::convert_impl (const cv::Mat & src) {
	//	1. Convert the pixels to the CIELAB colour space
	auto lab = convert_bgr_to_lab(src);
	//	2. Divide the image into like-colored cells using flood fill
	auto g = divide(lab);
	//	3. Merge together small cells with their neighbours
	merge_small_cells(*g);
	//	4. Merge together similarly-coloured regions
	merge_similar_cells(*g);


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
	std::size_t small_cell_threshold,
	float similar_cell_tolerance
)	:	flood_fill_tolerance_(flood_fill_tolerance),
		small_cell_threshold_(small_cell_threshold),
		similar_cell_tolerance_(similar_cell_tolerance)
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
