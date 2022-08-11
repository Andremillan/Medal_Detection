#ifndef MSER_H
#define MSER_H

#include <vector>
#include <stdint.h>

/// The MSER class extracts maximally stable extremal regions from a grayscale (8 bits) image.
/// @note The MSER class is not reentrant, so if you want to extract regions in parallel, each
/// thread needs to have its own MSER class instance.
class MSER
{
public:
	/// A Maximally Stable Extremal Region.
	struct Region
	{
		int level_; ///< Level at which the region is processed.
		int pixel_; ///< Index of the initial pixel (y * width + x).
		int area_; ///< Area of the region (moment zero).
		double moments_[5]; ///< First and second moments of the region (x, y, x^2, xy, y^2).
		double variation_; ///< MSER variation.

		double cx, cy; ///< centroid (mean)
		double e0, e1; ///< eigenvalues
		double e0sq, e1sq; ///< desired norm of the eigenvectors
		double v0x, v0y, v1x, v1y; ///< eigenvectors
		double n0, n1; ///< norms of eigenvectors

		/// Constructor.
		/// @param[in] level Level at which the region is processed.
		/// @param[in] pixel Index of the initial pixel (y * width + x).
		Region(int level = 256, int pixel = 0);

		/// Calculate the centroid and eigenvalues and eigenvectors.
		void estimate();

		// Implementation details (could be moved outside this header file)
	private:
		bool stable_; // Flag indicating if the region is stable
		Region * parent_; // Pointer to the parent region
		Region * child_; // Pointer to the first child
		Region * next_; // Pointer to the next (sister) region
		
		void accumulate(int x, int y);
		void merge(Region * child);
		void detect(int delta, int minArea, int maxArea, double maxVariation, double minDiversity,
					std::vector<Region> & regions);
		void process(int delta, int minArea, int maxArea, double maxVariation);
		bool check(double variation, int area) const;
		void save(double minDiversity, std::vector<Region> & regions);
		
		friend class MSER;
	};
	
	/// Constructor.
	/// @param[in] delta DELTA parameter of the MSER algorithm. Roughly speaking, the stability of a
	/// region is the relative variation of the region area when the intensity is changed by delta.
	/// @param[in] minArea Minimum area of any stable region relative to the image domain area.
	/// @param[in] maxArea Maximum area of any stable region relative to the image domain area.
	/// @param[in] maxVariation Maximum variation (absolute stability score) of the regions.
	/// @param[in] minDiversity Minimum diversity of the regions. When the relative area of two
	/// nested regions is below this threshold, then only the most stable one is selected.
	/// @param[in] eight Use 8-connected pixels instead of 4-connected.
	MSER(int delta = 2, double minArea = 0.001, double maxArea = 0.5, double maxVariation = 0.5,
		 double minDiversity = 0.33, bool eight = false);
	
	/// Extracts maximally stable extremal regions from a grayscale (8 bits) image.
	/// @param[in] bits Pointer to the first scanline of the image.
	/// @param[in] width Width of the image.
	/// @param[in] height Height of the image.
	/// @param[out] regions Detected MSER.
	void operator()(const uint8_t * bits, int width, int height, std::vector<Region> & regions);
	
	// Implementation details (could be moved outside this header file)
private:
	// Helper method
	void processStack(int newPixelGreyLevel, int pixel, std::vector<Region *> & regionStack);
	
	// Double the size of the memory pool
	std::ptrdiff_t doublePool(std::vector<Region *> & regionStack);
	
	// Parameters
	int delta_;
	double minArea_;
	double maxArea_;
	double maxVariation_;
	double minDiversity_;
	bool eight_;
	
	// Memory pool of regions for faster allocation
	std::vector<Region> pool_;
	std::size_t poolIndex_;
};

#endif
