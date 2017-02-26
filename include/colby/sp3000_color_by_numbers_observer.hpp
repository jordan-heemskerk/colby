/**
 *	\file
 */

#pragma once

#include "image_factory.hpp"
#include <opencv2/core/mat.hpp>

namespace colby {

/**
 *	Allows derived classes to receive events
 *	from a \ref sp3000_color_by_numbers object.
 */
class sp3000_color_by_numbers_observer {
public:
	sp3000_color_by_numbers_observer () = default;
	sp3000_color_by_numbers_observer (const sp3000_color_by_numbers_observer &) = delete;
	sp3000_color_by_numbers_observer (sp3000_color_by_numbers_observer &&) = delete;
	sp3000_color_by_numbers_observer & operator = (const sp3000_color_by_numbers_observer &) = delete;
	sp3000_color_by_numbers_observer & operator = (sp3000_color_by_numbers_observer &&) = delete;
	/**
	 *	Allows derived classes to be cleaned
	 *	up through pointer or reference to base.
	 */
	virtual ~sp3000_color_by_numbers_observer () noexcept;
	/**
	 *	A base class for events from
	 *	\ref sp3000_color_by_numbers objects.
	 */
	class base_event {
	private:
		image_factory & factory_;
	public:
		base_event () = delete;
		base_event (const base_event &) = default;
		base_event (base_event &&) = default;
		base_event & operator = (const base_event &) = default;
		base_event & operator = (base_event &&) = default;
		explicit base_event (image_factory &) noexcept;
		/**
		 *	Retrieves the image associated with the
		 *	event.
		 *
		 *	\return
		 *		A cv::Mat.
		 */
		cv::Mat image () const;
	};
	/**
	 *	Encapsulates all information about the
	 *	flood fill event.
	 */
	using flood_fill_event = base_event;
	/**
	 *	Invoked when the flood fill event occurs.
	 *	This occurs when the algorithm uses flood
	 *	fill and a Euclidean distance tolerance to
	 *	divide the image into cells.
	 *
	 *	\param [in] e
	 *		An object encapsulating all information
	 *		about the event.
	 */
	virtual void flood_fill (flood_fill_event e) = 0;
	/**
	 *	Encapsulates all information about the
	 *	merge small cells event.
	 */
	using merge_small_cells_event = base_event;
	/**
	 *	Invoked when the merge small cells event
	 *	occurs.  This occurs when the algorithm
	 *	merges small cells (i.e. those below or
	 *	at a certain threshold) into their largest
	 *	neighbor.
	 *
	 *	\param [in] e
	 *		An object encapsulating all information
	 *		about the event.
	 */
	virtual void merge_small_cells (merge_small_cells_event e) = 0;
	/**
	 *	Encapsulates all information about the
	 *	merge similar cells event.
	 */
	using merge_similar_cells_event = base_event;
	/**
	 *	Invoked when the merge similar cells event
	 *	occurs.  This occurs when the algorithm
	 *	merges similar cells (i.e. those whose
	 *	colors are within a certain Euclidean
	 *	distance).
	 *
	 *	\param [in] e
	 *		An object encapsulating all information
	 *		about the event.
	 */
	virtual void merge_similar_cells (merge_similar_cells_event e) = 0;
	/**
	 *	Encapsulates all information about the
	 *	N-merge event.
	 */
	using n_merge_event = base_event;
	/**
	 *	Invoked when the N-merge event occurs.
	 *	This occurs when the algorithm merges cells
	 *	until there are no more than N cells
	 *	remaining.
	 *
	 *	\param [in] e
	 *		An object encapsulating all information
	 *		about the event.
	 */
	virtual void n_merge (n_merge_event e) = 0;
	/**
	 *	Encapsulates all information about the
	 *	P-merge event.
	 */
	using p_merge_event = base_event;
	/**
	 *	Invoked when the P-merge event occurs.
	 *	This occurs when the algorithm uses
	 *	k-means to recolor cells until only
	 *	P unique colors are used.
	 *
	 *	\param [in] e
	 *		An object encapsulating all information
	 *		about the event.
	 */
	virtual void p_merge (p_merge_event e) = 0;
	/**
	 *	Encapsulates all information about the
	 *	gaussian smooth event.
	 */
	using gaussian_smooth_event = base_event;
	/**
	 *	Invoked when the Gaussian smooth event
	 *	occurs.  This occurs when the algorithm
	 *	uses Gaussian smoothing to smooth the
	 *	image before the final merging passes.
	 *
	 *	\param [in] e
	 *		An object encapsulating all information
	 *		about the event.
	 */
	virtual void gaussian_smooth (gaussian_smooth_event e) = 0;
	/**
	 *	Encapsulates all information about the
	 *	Laplacian event.
	 */
	using laplacian_event = base_event;
	/**
	 *	Invoked when the Laplacian event occurs.
	 *	This occurs when the algorithm applies the
	 *	Laplacian to the image.
	 *
	 *	\param [in] e
	 *		An object encapsulating all information
	 *		about the event.
	 */
	virtual void laplacian (laplacian_event e) = 0;
};

}
