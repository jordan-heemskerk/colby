/**
 *	\file
 */

#pragma once

#include "color_by_numbers.hpp"
#include "hash.hpp"
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <cstddef>
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
			sp3000_color_by_numbers::cell cell_;
			std::unordered_set<vertex *> adj_list_;
			graph & owner_;
		public:
			explicit vertex (graph &);
			vertex (const vertex &) = delete;
			vertex (vertex &&) = delete;
			vertex & operator = (const vertex &) = delete;
			vertex & operator = (vertex &&) = delete;
			const sp3000_color_by_numbers::cell & cell () const noexcept;
			void add (cv::Point);
			void add (vertex &);
			void merge (const vertex &);
		};
	private:
		class hasher {
		public:
			std::size_t operator () (const vertex &) const noexcept;
		};
		class equals {
		public:
			bool operator () (const vertex &, const vertex &) const noexcept;
		};
		std::unordered_set<vertex,hasher,equals> vertices_;
		std::unordered_map<cv::Point,vertex *> lookup_;
	public:
		graph () = default;
		graph (const graph &) = delete;
		graph (graph &&) = delete;
		graph & operator = (const graph &) = delete;
		graph & operator = (graph &&) = delete;
		vertex & add ();
		vertex * find (cv::Point);
	};
	float flood_fill_tolerance_;
	cv::Mat convert_to_lab (const cv::Mat &) const;
	static cell image_as_cell (const cv::Mat &);
	static void subtract (cell &, const cell &);
	std::unique_ptr<graph> divide (const cv::Mat &) const;
public:
	virtual result convert (const cv::Mat & src) override;
};

}
