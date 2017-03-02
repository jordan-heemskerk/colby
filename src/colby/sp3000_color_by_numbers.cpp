#include <boost/iterator/filter_iterator.hpp>
#include <colby/algorithm.hpp>
#include <colby/image_factory.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <colby/sp3000_color_by_numbers_observer.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
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
	//	Swap v.cell_ to allow underlying datatypes to perform cleanup
	decltype(v.cell_) tmp;
	using std::swap;
	swap(tmp,v.cell_);
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

void sp3000_color_by_numbers::graph::vertex::color (cv::Vec3f c) noexcept {
	color_ = c;
}

const sp3000_color_by_numbers::cell & sp3000_color_by_numbers::graph::vertex::points () const noexcept {
	return cell_;
}

sp3000_color_by_numbers::graph::graph (const cv::Mat & img)
	:	rows_(img.rows),
		cols_(img.cols)
{	}

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

sp3000_color_by_numbers::graph::const_vertices_type sp3000_color_by_numbers::graph::vertices () const noexcept {
	return const_vertices_type(vertices_.begin(),vertices_.end());
}

std::size_t sp3000_color_by_numbers::graph::size () const noexcept {
	return vertices_.size();
}

template <typename Callback>
optional<sp3000_color_by_numbers::graph::neighbors_type> sp3000_color_by_numbers::graph::optimal_neighbors (Callback callback) {
	optional<neighbors_type> retr;
	for (auto && v : vertices()) {
		for (auto && n : v.neighbors()) {
			if (!retr || callback(neighbors_type(v,n),*retr)) retr.emplace(v,n);
		}
	}
	return retr;
}

cv::Mat sp3000_color_by_numbers::graph::mat () const {
	cv::Mat retr(rows_,cols_,CV_32FC3);
	for (auto && v : vertices()) {
		auto c = v.color();
		for (auto && p : v.points()) {
			retr.at<cv::Vec3f>(p) = c;
		}
	}
	return retr;
}

cv::Mat sp3000_color_by_numbers::convert_bgr_to_lab (const cv::Mat & img) {
	cv::Mat retr;
	cv::cvtColor(img,retr,CV_BGR2RGB);
	if (!retr.data) throw std::runtime_error("cv::cvtColor failed to convert image to Lab");
	return retr;
}

cv::Mat sp3000_color_by_numbers::convert_lab_to_bgr (const cv::Mat & img) {
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
	auto retr = std::make_unique<graph>(img);
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
		iter->merge(curr);
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
		auto copy = [] (auto && ref) noexcept {	return &ref;	};
		auto vertices = g.vertices();
		std::transform(vertices.begin(),vertices.end(),std::back_inserter(sorted),copy);
		std::sort(sorted.begin(),sorted.end(),compare);
		changed = false;
		while (!sorted.empty()) {
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

template <typename T>
static float n_merge_weight (const T & a) {
	auto && bigger = std::max(a.first,a.second,[] (auto && a, auto && b) noexcept {	return a.size() < b.size();	});
	return squared_distance(a.first.color(),a.second.color()) * bigger.size();
}

void sp3000_color_by_numbers::n_merge (graph & g, std::size_t n) const {
	while (g.size() > n) {
		auto opt = g.optimal_neighbors([] (auto && a, auto && b) noexcept {
			return n_merge_weight(a) < n_merge_weight(b);
		});
		//	This should never happen
		if (!opt) break;
		opt->first.merge(opt->second);
	}
}

void sp3000_color_by_numbers::p_merge (graph & g, std::size_t p) const {
	std::vector<cv::Vec3f> colors;
	auto vertices = g.vertices();
	for (auto && v : vertices) {
		colors.resize(colors.size() + v.size(),v.color());
	}
	cv::Mat best_labels;
	cv::TermCriteria term_crit;
	term_crit.type = cv::TermCriteria::EPS|cv::TermCriteria::COUNT;
	term_crit.maxCount = 1000;
	term_crit.epsilon = 0.01f;
	cv::Mat centers;
	cv::kmeans(colors,p,best_labels,term_crit,50,cv::KMEANS_RANDOM_CENTERS,centers);
	assert(centers.cols == 3);
	assert(centers.type() == CV_32FC1);
	assert(best_labels.type() == CV_32SC1);
	int i = 0;
	for (auto && v : vertices) {
		v.color(centers.at<cv::Vec3f>(cv::Point(0,best_labels.at<int>(i))));
		//	This has the effect of "skipping"
		//	to the beginning of the next vertices
		//	region of colors
		//
		//	Note that this is an int/std::size_t
		//	signed/unsigned mismatch, but OpenCV
		//	insists on using ints for indices
		//	everywhere so que sera sera
		i += v.size();
	}
}

cv::Mat sp3000_color_by_numbers::gaussian_smooth (const graph & g, std::size_t kernel_size) const {
	auto img = g.mat();
	cv::Mat retr;
	cv::GaussianBlur(img,retr,cv::Size(kernel_size,kernel_size),0);
	for (int i = 0; i < img.rows; ++i) {
		for (int j = 0; j < img.cols; ++j) {
			auto p = cv::Point(j,i);
			auto c = img.at<cv::Vec3f>(p);
			auto && c_blur = retr.at<cv::Vec3f>(p);
			auto dist = squared_distance(c,c_blur);
			neighbors(img, p, [&] (auto && n) noexcept {
				auto n_c = img.at<cv::Vec3f>(n);
				auto d = squared_distance(n_c,c_blur);
				if (d < dist) {
					c = n_c;
					dist = d;
				}
			});
			c_blur = c;
		}
	}
	return retr;
}

sp3000_color_by_numbers::result sp3000_color_by_numbers::convert_impl (const cv::Mat & src) {
	class lazy_image_factory : public image_factory {
	private:
		const std::unique_ptr<graph> & g_;
	public:
		lazy_image_factory () = delete;
		lazy_image_factory (const std::unique_ptr<graph> & g) noexcept : g_(g) {	}
		virtual cv::Mat image () override {
			auto mat = g_->mat();
			return convert_lab_to_bgr(mat);
		}
	};
	class immediate_image_factory : public image_factory {
	private:
		const cv::Mat & mat_;
	public:
		immediate_image_factory () = delete;
		immediate_image_factory (const cv::Mat & mat) noexcept : mat_(mat) {	}
		virtual cv::Mat image () override {
			return convert_lab_to_bgr(mat_);
		}
	};
	//	1. Convert the pixels to the CIELAB colour space
	auto lab = convert_bgr_to_lab(src);
	//	2. Divide the image into like-colored cells using flood fill
	auto g = divide(lab);
	lazy_image_factory factory(g);
	sp3000_color_by_numbers_observer::base_event e(factory);
	if (o_) o_->flood_fill(e);
	//	3. Merge together small cells with their neighbours
	merge_small_cells(*g);
	if (o_) o_->merge_small_cells(e);
	//	4. Merge together similarly-colored regions
	merge_similar_cells(*g);
	if (o_) o_->merge_similar_cells(e);
	//	5. Merge until we have less than 1.5N cells (N-merging)
	std::size_t max_final_cells_15 = max_final_cells_;
	max_final_cells_15 += max_final_cells_ / 2U;
	n_merge(*g,max_final_cells_15);
	if (o_) o_->n_merge(e);
	//	6. Merge until we have less than P colours, using k-means (P-merging)
	p_merge(*g,max_final_colors_);
	if (o_) o_->p_merge(e);
	//	7. Gaussian Smoothing
	auto smoothed = gaussian_smooth(*g,7);	//	TODO: Make this configurable
	immediate_image_factory smoothed_factory(smoothed);
	if (o_) o_->gaussian_smooth(
		sp3000_color_by_numbers_observer::gaussian_smooth_event(
			smoothed_factory
		)
	);
	//	8. Do another flood fill pass to work the new regions
	g = divide(smoothed);
	if (o_) o_->flood_fill(e);
	//	9. Do another small cell merge
	merge_small_cells(*g);
	if (o_) o_->merge_small_cells(e);
	//	10. Merge until we have less than N cells (N-merging)
	n_merge(*g,max_final_cells_);
	if (o_) o_->n_merge(e);
	return result(factory.image());
}

sp3000_color_by_numbers::sp3000_color_by_numbers (
	std::size_t max_final_cells,
	std::size_t max_final_colors,
	float flood_fill_tolerance,
	std::size_t small_cell_threshold,
	float similar_cell_tolerance
)	:	flood_fill_tolerance_(flood_fill_tolerance),
		small_cell_threshold_(small_cell_threshold),
		similar_cell_tolerance_(similar_cell_tolerance),
		max_final_cells_(max_final_cells),
		max_final_colors_(max_final_colors),
		o_(nullptr)
{	}

sp3000_color_by_numbers::sp3000_color_by_numbers (
	sp3000_color_by_numbers_observer & o,
	std::size_t max_final_cells,
	std::size_t max_final_colors,
	float flood_fill_tolerance,
	std::size_t small_cell_threshold,
	float similar_cell_tolerance
)	:	sp3000_color_by_numbers(
			max_final_cells,
			max_final_colors,
			flood_fill_tolerance,
			small_cell_threshold,
			similar_cell_tolerance
		)
{
	o_ = &o;
}

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
