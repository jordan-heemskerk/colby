/**
 *	\file
 */

#pragma once

#include "color_by_numbers.hpp"
#include "optional.hpp"
#include "hash.hpp"
#include "sp3000_color_by_numbers_observer.hpp"
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
#include <utility>

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
			void color (cv::Vec3f) noexcept;
			const cell & points () const noexcept;
		};
	private:
		std::deque<vertex> vertices_storage_;
		using vertices_internal = std::unordered_set<vertex *>;
		vertices_internal vertices_;
		std::unordered_map<cv::Point,vertex *> lookup_;
		int rows_;
		int cols_;
		template <typename Iterator>
		using make_vertices_type = boost::iterator_range<boost::indirect_iterator<Iterator>>;
	public:
		graph () = delete;
		graph (const graph &) = delete;
		graph (graph &&) = delete;
		graph & operator = (const graph &) = delete;
		graph & operator = (graph &&) = delete;
		explicit graph (const cv::Mat &);
		vertex & add ();
		vertex * find (cv::Point);
		using vertices_type = make_vertices_type<vertices_internal::iterator>;
		vertices_type vertices () noexcept;
		using const_vertices_type = make_vertices_type<vertices_internal::const_iterator>;
		const_vertices_type vertices () const noexcept;
		std::size_t size () const noexcept;
		using neighbors_type = std::pair<vertex &,vertex &>;
		template <typename Callback>
		optional<neighbors_type> optimal_neighbors (Callback);
		cv::Mat mat () const;
	};
	float flood_fill_tolerance_;
	std::size_t small_cell_threshold_;
	float similar_cell_tolerance_;
	std::size_t max_final_cells_;
	std::size_t max_final_colors_;
	sp3000_color_by_numbers_observer * o_;
	static cv::Mat convert_byte_to_float (const cv::Mat &);
	static cv::Mat convert_float_to_byte (const cv::Mat &);
	static cv::Mat convert_short_to_byte (const cv::Mat &);
	static cv::Mat convert_to_grayscale (const cv::Mat &);
	static cv::Mat laplacian (const cv::Mat &);
	static cell image_as_cell (const cv::Mat &);
	static void subtract (cell &, const cell &);
	std::unique_ptr<graph> divide (const cv::Mat &, bool exact = false) const;
	void merge_small_cells_impl (graph &, std::size_t, bool) const;
	void merge_small_cells (graph &, bool avg = true) const;
	void merge_similar_cells (graph &) const;
	void n_merge (graph &, std::size_t) const;
	void p_merge (graph &, std::size_t) const;
	cv::Mat gaussian_smooth (const graph &, std::size_t) const;
	result palette (const cv::Mat &, const graph &) const;
	result convert_impl (const cv::Mat & src);
public:
	sp3000_color_by_numbers () = delete;
	/**
	 *	Creates a new sp3000_color_by_numbers.
	 *
	 *	\param [in] max_final_cells
	 *		The maximum number of regions in the resulting
	 *		images.
	 *	\param [in] max_final_colors
	 *		The maximum number of unique colors to use in the
	 *		resulting images.
	 *	\param [in] flood_fill_tolerance
	 *		The tolerance for the flood fill portion
	 *		of the algorithm as the distance in
	 *		Euclidean space between CIELAB color
	 *		coordinates.  All distances less than
	 *		this value shall be admitted into the same
	 *		region by the flood fill.  Defaults to a
	 *		sensible value.
	 *	\param [in] small_cell_threshold
	 *		The threshold (in number of pixels) at or
	 *		below which a cell shall be considered
	 *		"small" for the small cell merging portion
	 *		of the algorithm.  Cells deemed to be small
	 *		shall be merged into their largest neighbor.
	 *		Defaults to a sensible value.
	 *	\param [in] similar_cell_tolerance
	 *		The tolerance for the similar cell merging
	 *		portion of the algorithm as the distance in
	 *		Euclidean space between CIELAB color
	 *		coordinates.  All distances less than this
	 *		value shall result in the neighboring cells
	 *		being merged.  Defaults to a sensible value.
	 */
	sp3000_color_by_numbers (
		std::size_t max_final_cells,
		std::size_t max_final_colors,
		float flood_fill_tolerance = 10.f,
		std::size_t small_cell_threshold = 10,
		float similar_cell_tolerance = 5.f
	);
	/**
	 *	Creates a new sp3000_color_by_numbers.
	 *
	 *	\param [in] o
	 *		A \ref sp3000_color_by_numbers_observer object
	 *		which shall receive events emitted by this object.
	 *		This reference must remain valid for the lifetime
	 *		of the constructed object or the behavior is
	 *		undefined.
	 *	\param [in] max_final_cells
	 *		The maximum number of regions in the resulting
	 *		images.
	 *	\param [in] max_final_colors
	 *		The maximum number of unique colors to use in the
	 *		resulting images.
	 *	\param [in] flood_fill_tolerance
	 *		The tolerance for the flood fill portion
	 *		of the algorithm as the distance in
	 *		Euclidean space between CIELAB color
	 *		coordinates.  All distances less than
	 *		this value shall be admitted into the same
	 *		region by the flood fill.  Defaults to a
	 *		sensible value.
	 *	\param [in] small_cell_threshold
	 *		The threshold (in number of pixels) at or
	 *		below which a cell shall be considered
	 *		"small" for the small cell merging portion
	 *		of the algorithm.  Cells deemed to be small
	 *		shall be merged into their largest neighbor.
	 *		Defaults to a sensible value.
	 *	\param [in] similar_cell_tolerance
	 *		The tolerance for the similar cell merging
	 *		portion of the algorithm as the distance in
	 *		Euclidean space between CIELAB color
	 *		coordinates.  All distances less than this
	 *		value shall result in the neighboring cells
	 *		being merged.  Defaults to a sensible value.
	 */
	sp3000_color_by_numbers (
		sp3000_color_by_numbers_observer & o,
		std::size_t max_final_cells,
		std::size_t max_final_colors,
		float flood_fill_tolerance = 10.f,
		std::size_t small_cell_threshold = 10,
		float similar_cell_tolerance = 5.f
	);
	virtual result convert (const cv::Mat & src) override;
};

}
