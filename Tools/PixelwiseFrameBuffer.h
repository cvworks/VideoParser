/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <array>
#include "SimpleMatrix.h"

namespace vpl {

/*!
*/
class PixelBuffer
{
protected:
	std::vector<double> values;
	size_t numElements;
	size_t lastPos;

	int confidence;
	time_t start_time;
	time_t end_time;

	double valueSum;
	bool validStats;
	double meanVal;
	double varianceVal;

	bool isBackground;

	void compute_stats()
	{
		validStats = true;
		meanVal = 0;
		varianceVal = 0;

		if (!empty())
		{
			for (unsigned i = 0; i < numElements; i++)
				meanVal += values[i];

			meanVal /= numElements;

			for (unsigned i = 0; i < numElements; i++) 
				varianceVal += (values[i] - meanVal) * (values[i] - meanVal);

			varianceVal /= values.size();
		}
	}

public:
	PixelBuffer() 
	{ 
		clear();
	}

	void clear()
	{
		numElements = 0;
		lastPos = 0;
		valueSum = 0;
		start_time = 0;
		end_time = 0;
		confidence = 0;
		validStats = false;
		isBackground = false;
	}

	void operator=(const PixelBuffer& rhs)
	{
		values       = rhs.values;
		numElements  = rhs.numElements;
		lastPos      = rhs.lastPos;
		valueSum     = rhs.valueSum;
		start_time   = rhs.start_time;
		end_time     = rhs.end_time;
		confidence   = rhs.confidence;
		validStats   = rhs.validStats;
		meanVal      = rhs.meanVal;
		varianceVal  = rhs.varianceVal;
		isBackground = rhs.isBackground;
	}

	bool is_background() const
	{
		return isBackground;
	}

	void set_background_state(bool val)
	{
		isBackground = val;
	}

	unsigned size() const
	{
		return numElements;
	}

	bool empty() const
	{
		return numElements == 0;
	}

	void add_confidence(int conf_delta)
	{
		confidence += conf_delta;
	}
	
	time_t elapsed_time(time_t currTime) const
	{
		return currTime - end_time;
	}

	time_t elapsed_time() const
	{
		return end_time - start_time;
	}

	int get_confidence() const
	{
		return confidence;
	}

	void reset_start_time(time_t t)
	{
		start_time = t;
		end_time = t;
	}

	void set_size(size_t sz)
	{
		ASSERT(sz >= numElements);

		values.resize(sz);
	}

	void push_back(const double& val, time_t timestamp)
	{
		// If first element, store timestamp
		if (numElements == 0)
			start_time = timestamp;

		end_time = timestamp;

		valueSum += val;

		validStats = false;

		// Treat as circular array of there is no enough space
		if (++lastPos >= values.size())
			lastPos = 0;

		if (numElements < values.size())
			numElements++;

		values[lastPos] = val;
	}

	double mean() const
	{
		//return empty() ? 0 : valueSum / numElements;

		if (!validStats)
			const_cast<PixelBuffer*>(this)->compute_stats();

		return meanVal;
	}

	double variance() const
	{
		if (!validStats)
			const_cast<PixelBuffer*>(this)->compute_stats();

		return varianceVal;
	}

	double std_dev() const
	{
		return sqrt(variance());
	}
};

class PixelwiseFrameBuffer : public SimpleMatrix<PixelBuffer>
{
	typedef SimpleMatrix<PixelBuffer> ParentClass;

	// Delete the resize function in teh base class
	void resize(unsigned ni, unsigned nj);
	void resize(unsigned ni, unsigned nj, PixelBuffer val);

public:
	PixelwiseFrameBuffer()
	{
	}

	PixelwiseFrameBuffer(unsigned ni, unsigned nj, unsigned max_size) 
		: ParentClass(ni, nj)
	{
		set_size(ni, nj, max_size);
	}

	void set_size(unsigned ni, unsigned nj, unsigned max_size)
	{
		ParentClass::resize(ni, nj);

		for (unsigned i = 0; i < ni; i++)
			for (unsigned j = 0; j < nj; j++)
				get(i, j).set_size(max_size);

		// Make sure that all buffers are cleared
		clear_pixel_buffers();
	}

	//! Clears all pixel buffers
	void clear_pixel_buffers()
	{
		for (unsigned i = 0; i < ni(); i++)
			for (unsigned j = 0; j < nj(); j++)
				get(i, j).clear();
	}
};

} // vpl namespcace

