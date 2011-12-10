/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "PeopleDetector.h"
#include <Tools/CvMatView.h>
#include <Tools/cv.h>

namespace vpl {

/*!
	@brief Wrapper for a generic people detector algorithm
*/
class CvPeopleDetector : public PeopleDetector
{
	struct Params {
		enum {SMALL_WIN, MEDIUM_WIN, LARGE_WIN};
		int type;
		double hitThreshold;
		int groupThreshold;
		double scaleFactor;
	};

	CvMatView m_img; // RGB
	std::vector<cv::Rect> m_found_filtered;

	Params m_params;

public:	

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	virtual std::string ClassName() const
	{
		return "CvPeopleDetector";
	}
	
	virtual void Run();

	virtual void Draw(const DisplayInfoIn& dii) const;
	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const;
	
	virtual int NumOutputImages() const 
	{ 
		return 1; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Detected people";
		}

		return "error";
	}
};

} // namespace vpl

