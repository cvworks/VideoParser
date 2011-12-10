/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "Blob.h"
#include <Tools/CvMatView.h>

using namespace vpl;

int Blob::s_blobCount = 0;

/*!
	Contructs a blob from its external contour.

	@param img optional parameter. If it's not empty, the color histrogram 
	of the blob is computed.
*/
Blob::Blob(const std::vector<cv::Point>& contour, time_t t, FloatImg img) 
	: m_contour(1, contour)
{
	s_blobCount++;

	cv::Mat pts(m_contour.front()); // shallow copy to see pts in matrix form

	m_timestamp = t;

	cv::Rect bbox = cv::boundingRect(pts);

	m_width = bbox.width;
	m_height = bbox.height;
	m_area = cv::contourArea(pts);

	int mean_x = 0, mean_y = 0;

	m_mask = cv::Mat::zeros(m_height, m_width, 
		cv::DataType<uchar>::type);

	for (auto it = m_contour.front().begin(); it != m_contour.back().end(); ++it)
	{
		mean_x += it->x;
		mean_y += it->y;
	}

	double numPts = m_contour.size();

	m_centroid.x = bbox.x + bbox.width / 2;
	m_centroid.y = bbox.y + bbox.height / 2;

	cv::drawContours(m_mask, m_contour, -1, cv::Scalar(255), CV_FILLED, 8, 
		std::vector<cv::Vec4i>(), INT_MAX, cv::Point(-bbox.x, -bbox.y));

	if (img.size() > 0)
		computeHistogram(img);
}

void Blob::computeHistogram(FloatImg img)
{
	// Init the histogram to zero
	m_color_histogram.assign(0);

	float bin;
	XYCoord orig = ul();

	for (int y = 0; y < m_mask.rows; y++)
	{
		for (int x = 0; x < m_mask.cols; x++)
		{
			if (m_mask.at<uchar>(y, x))
			{
				bin = img(x + orig.x, y + orig.y) / float(HISTO_SIZE);

				m_color_histogram[(int)floor(bin)]++;
			}
		}
	}
}

void Blob::print(std::ostream& os) const
{
	PRINT(os, m_centroid.x);
	PRINT(os, m_centroid.x);
	PRINTN(os, m_timestamp);
}

void Blob::merge(const Blob& rhs)
{
	// Update the centroid first
	size_t new_size = m_contour.size() + rhs.m_contour.size();

	/*const double sum_x = m_centroid.x * m_contour.size() 
		+ rhs.m_centroid.x * rhs.m_contour.size();

	const double sum_y = m_centroid.y * m_contour.size() 
		+ rhs.m_centroid.y * rhs.m_contour.size();

	m_centroid.x = int(sum_x / new_size);
	m_centroid.y = int(sum_y / new_size);*/

	// Merge the contours
	copy_back(rhs.m_contour, m_contour);

	// Compute the new bounding box
	cv::Rect new_bbox;

	new_bbox.x = MIN(ul().x, rhs.ul().x);
	new_bbox.y = MIN(ul().y, rhs.ul().y);

	new_bbox.width = MAX(br().x, rhs.br().x) - new_bbox.x + 1;
	new_bbox.height = MAX(br().y, rhs.br().y) - new_bbox.y + 1;

	// Update the centroid
	m_centroid.x = new_bbox.x + new_bbox.width / 2;
	m_centroid.y = new_bbox.y + new_bbox.height / 2;

	// Update width and height
	m_width = new_bbox.width;
	m_height = new_bbox.height;

	// Update the mask
	m_mask = cv::Mat::zeros(m_height, m_width, cv::DataType<uchar>::type);

	cv::drawContours(m_mask, m_contour, -1, cv::Scalar(255), CV_FILLED, 8, 
		std::vector<cv::Vec4i>(), INT_MAX, cv::Point(-new_bbox.x, -new_bbox.y));

	// Merge the histograms
	for (int i = 0; i < Blob::HISTO_SIZE; i++)
		m_color_histogram[i] += rhs.m_color_histogram[i];

	// Update the area
	m_area += rhs.m_area;
		
	// Choose the earlies timestamp
	if (rhs.m_timestamp < m_timestamp)
		m_timestamp = rhs.m_timestamp; 
}

void Blob::drawMask(ByteImg img) const
{
	XYCoord orig = ul();

	for (int y = 0; y < m_mask.rows; y++)
		for (int x = 0; x < m_mask.cols; x++)
			if (m_mask.at<uchar>(y, x))
				img(x + orig.x, y + orig.y) = 255; 
}

void Blob::drawMask(RGBImg img) const
{
	XYCoord orig = ul();
	RGBColor white(255, 255, 255);

	for (int y = 0; y < m_mask.rows; y++)
		for (int x = 0; x < m_mask.cols; x++)
			if (m_mask.at<uchar>(y, x))
				img(x + orig.x, y + orig.y) = white; 
}

void Blob::draw(RGBImg img) const
{
	drawMask(img);

	CvMatView mat(img);

	cv::drawContours(mat, m_contour, -1, cv::Scalar(255, 0, 0));

	cv::circle(mat, cv::Point(centroid().x, centroid().y), 2, 
		cv::Scalar(0, 255, 0), CV_FILLED);

	cv::rectangle(mat, bbox(), cv::Scalar(255, 255, 0));
}
