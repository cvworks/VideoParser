/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _GRAPHCUT_BACKGROUND_SUBTRACTOR_H_
#define _GRAPHCUT_BACKGROUND_SUBTRACTOR_H_

#include "../BackgroundSubtractor.h"
#include "Graph.h"

namespace vpl {

/*!
	Supposedly, it should be able to deal with object shadows.
*/
class GraphcutBackgroundSubtractor : public BackgroundSubtractor
{
	//CvBGStatModel* m_pBackgroundModel;

	////////////////////////////////////////
	// Algorithm parameters
	double defaultBackgroundVariance; //!< deviation^2
	double expan;                     //!< expan for the deviation for background modes
	double PFprior;                   //!< the prior for a pixel to be foreground pixel
	double PF;                        //!< the possibility given a color to be foreground (ideally should be 1/(255)^3)

	double shadow_calculation_threshold_soft; //!<  0 - 1
	double shadow_calculation_threshold_hard; //!<  0 - 1
	double shadow_sigma_hard;                 //!< shadow length [0 - sqrt(3)]
	double shadow_sigma_soft;                 //!< shadow length [0 - sqrt(3)]

	double CUT_alpha;
	double CUT_fWeight;

	////////////////////////////////////////
	// Auxiliar variables
	double* Mr; 
	double* Mg; 
	double* Mb; 
	double* Crr;
	double* Cgg;
	double* Cbb;

	double* finalWeight;
	double* backgroundWeight;
	double* shadowWeight;

	graphcut::Graph::node_id* nodes;

	////////////////////////////////////////
	// Output variables
	IplImage* frame1;
	IplImage* mean_image;
	IplImage* test1_image;
	IplImage* test2_image;
	IplImage* test3_image;
	IplImage* test4_image;

	ImgSize m_frameSize;

protected:
	void AllocateMemory(int width, int height);
	void FindPreliminaryForeground();
	void FindBlobs();

	virtual void LearnBackgroundModelOffline();
	

	virtual void InitializeBackgroundModel(unsigned width, unsigned height)
	{
		// nothing to do
	}

	virtual void ProcessBackgroundFrame(RGBImg rgbImg, FloatImg greyImg, fnum_t frameIndex);
	virtual void FindForeground(RGBImg rgbImg, FloatImg greyImg);

public:
	GraphcutBackgroundSubtractor()
	{
		m_frameSize.width = 0;
		m_frameSize.height = 0;
	}

	virtual void ReadParamsFromUserArguments();

	virtual void Clear();

	virtual std::string ClassName() const
	{
		return "GraphcutBackgroundSubtractor";
	}
};

} // namespace vpl

#endif //_GRAPHCUT_BACKGROUND_SUBTRACTOR_H_