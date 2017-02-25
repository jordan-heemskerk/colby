/**
 *	\file
 */

#pragma once

#include "color_by_numbers.hpp"
#include "hash.hpp"
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <cstddef>
#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace colby {

/**
 *	Implements an extended version of the algorithm
 *	laid out by Stack Overflow user Sp3000 <a href="http://codegolf.stackexchange.com/a/42235">in
 *	this answer</a>.
 *
 *	The extension is that rather than converting the
 *	image to use a smaller number of colors it instead
 *	converts it to a form suitable to color by numbers
 *	(i.e. the cells are not filled with the corresponding
 *	color).
 */
class sp3000_color_by_numbers : public color_by_numbers {
private:
	using cell = std::unordered_set<cv::Point>;
	class graph {
	public:
		class vertex {
		private:
			cell cell_;
			using adjacency_list = std::unordered_set<vertex *>;
			adjacency_list adj_list_;
			cv::Vec3f color_;
			graph & owner_;
		public:
			explicit vertex (graph &);
			vertex (const vertex &) = delete;
			vertex (vertex &&) = delete;
			vertex & operator = (const vertex &) = delete;
			vertex & operator = (vertex &&) = delete;
			void add (cv::Point, cv::Vec3f);
			void add (vertex &);
			void merge (vertex &, bool avg = true);
			using neighbors_type = boost::iterator_range<boost::indirect_iterator<adjacency_list::const_iterator>>;
			neighbors_type neighbors () const noexcept;
			std::size_t size () const noexcept;
			cv::Vec3f color () const noexcept;
			const cell & points () const noexcept;
		};
	private:
		std::deque<vertex> vertices_storage_;
		using vertices_internal = std::unordered_set<vertex *>;
		vertices_internal vertices_;
		std::unordered_map<cv::Point,vertex *> lookup_;
	public:
		graph () = default;
		graph (const graph &) = delete;
		graph (graph &&) = delete;
		graph & operator = (const graph &) = delete;
		graph & operator = (graph &&) = delete;
		vertex & add ();
		vertex * find (cv::Point);
		using vertices_type = boost::iterator_range<boost::indirect_iterator<vertices_internal::iterator>>;
		vertices_type vertices () noexcept;
		std::size_t size () const noexcept;
	};
	float flood_fill_tolerance_;
	std::size_t small_cell_threshold_;
	cv::Mat convert_bgr_to_lab (const cv::Mat &) const;
	cv::Mat convert_lab_to_bgr (const cv::Mat &) const;
	static cell image_as_cell (const cv::Mat &);
	static void subtract (cell &, const cell &);
	std::unique_ptr<graph> divide (const cv::Mat &) const;
	void merge_small_cells (graph &) const;
	result convert_impl (const cv::Mat & src);
public:
	sp3000_color_by_numbers () = delete;
	/**
	 *
	 */
	explicit sp3000_color_by_numbers (float flood_fill_tolerance, std::size_t small_cell_threshold);
	virtual result convert (const cv::Mat & src) override;
};

}
