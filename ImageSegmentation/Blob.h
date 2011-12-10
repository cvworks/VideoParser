/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <map>
#include <array>
#include <Tools/ImageUtils.h>
#include <Tools/BasicTempTypes.h>
#include <Tools/cv.h>

namespace vpl {

/*!
	hiararchy: Contain information about the image topology. It has 
	as many elements as the number of contours. For each contour 
	contours[i] , the elements hierarchy[i][0] , hiearchy[i][1] , 
	hiearchy[i][2] , hiearchy[i][3] will be set to 0-based indices 
	in contours of the next and previous contours at the same 
	hierarchical level, the first child contour and the parent 
	contour, respectively. If for some contour i there is no next, 
	previous, parent or nested contours, the corresponding elements 
	of hierarchy[i] will be negative.
*/
class Blob
{
public:
	enum {HISTO_SIZE = 16};
	typedef std::array<int, HISTO_SIZE> ColorHistogram;

	std::vector<std::vector<cv::Point>> m_contour;
	double m_area;
	XYCoord m_centroid;
	int m_width;
	int m_height;
	ColorHistogram m_color_histogram;    //!< grayscale histogram of the foreground pixels
	cv::Mat m_mask;

	time_t m_timestamp;   //!< the date/time when the feature was detected

	static int s_blobCount;

	Blob(time_t t)
	{
		s_blobCount++;

		m_area = 0;
		m_timestamp = t;
	}

	Blob(time_t t, int centroid_x, int centroid_y, int width, int height)
	{
		s_blobCount++;

		m_timestamp  = t;
		m_centroid.x = centroid_x;
		m_centroid.y = centroid_y;
		m_width      = width;
		m_height     = height;

		m_area = 0;
	}

	Blob(const Blob& rhs)
	{
		s_blobCount++;

		operator=(rhs);
	}

	Blob(const std::vector<cv::Point>& contour, time_t t, FloatImg img);

	~Blob()
	{
		s_blobCount--;
	}
	
	void operator=(const Blob& rhs)
	{
		m_contour = rhs.m_contour;
		m_color_histogram = rhs.m_color_histogram;

		m_area      = rhs.m_area;
		m_width     = rhs.m_width;
		m_height    = rhs.m_height;
		m_centroid  = rhs.m_centroid;
		m_mask      = rhs.m_mask;
		m_timestamp = rhs.m_timestamp;  
	}

	void computeHistogram(FloatImg img);

	const ColorHistogram& color_histogram() const
	{
		return m_color_histogram;
	}

	time_t timestamp() const
	{
		return m_timestamp;
	}

	int height() const
	{
		return m_height;
	}

	int width() const
	{
		return m_width;
	}

	XYCoord ul() const
	{
		return XYCoord(centroid().x - width() / 2, 
                       centroid().y - height() / 2);
	}

	XYCoord br() const
	{
		return XYCoord(centroid().x + (int) ceil(width() / 2.0), 
                       centroid().y + (int) ceil(height() / 2.0));
	}

	cv::Rect bbox() const
	{
		XYCoord orig = ul();

		return cv::Rect(orig.x, orig.y, m_width, m_height);
	}

	void print(std::ostream& os = std::cout) const;

	void merge(const Blob& rhs);

	const XYCoord& centroid() const
	{
		return m_centroid;
	}

	Point centerPoint() const
	{
		return Point(m_centroid.x, m_centroid.y);
	}

	const double& area() const
	{
		return m_area;
	}

	int size() const
	{
		return ROUND_NUM(m_area);
	}

	void drawMask(ByteImg img) const;

	void drawMask(RGBImg img) const;

	void draw(RGBImg img) const;
};

typedef std::shared_ptr<Blob> BlobPtr;
typedef std::list<BlobPtr> BlobTrace;
typedef std::vector<BlobTrace> BlobTraces;

inline void smoothBlobTrace(BlobTrace& nodes, unsigned numSmoothIter)
{
	if (nodes.size() < 3)
		return;

	std::vector<Point> pts(nodes.size());
	unsigned i = 0;

	for (auto it = nodes.begin(); it != nodes.end(); ++it, ++i)
	{
		pts[i].x = (*it)->centroid().x;
		pts[i].y = (*it)->centroid().y;
	}

	std::vector<Point> aux_pts(pts.size());

	aux_pts.front() = pts.front();
	aux_pts.back() = pts.back();

	for (i = 0; i < numSmoothIter; i++)
	{
		auto it0 = pts.begin();
		auto it1 = ++pts.begin();
		auto it2 = ++++pts.begin();
		auto it3 = ++aux_pts.begin();

		for (; it2 != pts.end(); ++it0, ++it1, ++it2, ++it3)
		{
			it3->x = (it0->x + it1->x + it2->x) / 3.0;
			it3->y = (it0->y + it1->y + it2->y) / 3.0;
		}

		pts = aux_pts;
	}

	i = 0;

	for (auto it = nodes.begin(); it != nodes.end(); ++it, ++i)
	{
		(*it)->m_centroid.x = (int)pts[i].x;
		(*it)->m_centroid.y = (int)pts[i].y;
	}
}

} // namespace vpl

